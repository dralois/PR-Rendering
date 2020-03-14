#include "SimManager.h"

// FixMe: Shaders should not need to be in seperate namespaces
namespace depth
{
	#include "../plugins/src/depthShader.cpp"
	extern AtNodeMethods* DepthShader;
}
namespace label
{
	#include "../plugins/src/labelShader.cpp"
	extern AtNodeMethods* LabelShader;
}
namespace blend
{
	#include "../plugins/src/blendShader.cpp"
	extern AtNodeMethods* BlendShader;
}
namespace filter
{
	#include "../plugins/src/nullFilter.cpp"
	extern AtNodeMethods* NullFilter;
}

//---------------------------------------
// Cleanup simulation
//---------------------------------------
SimManager::~SimManager()
{
	// Free physics
	if (pPxScene != NULL)
	{
#ifdef _DEBUG
		PxCloseExtensions();
		pPxPvdServer->disconnect();
		pPxPvdServer->getTransport()->release();
#endif
		PxGetPhysics().release();
		PX_RELEASE(pPxPvdServer);
		PX_RELEASE(pPxDispatcher);
		PX_RELEASE(pPxCooking);
		PX_RELEASE(pPxScene);
		PX_RELEASE(pPxMaterial);
		PxGetFoundation().release();
	}

	// Shutdown Arnold, automatically frees everything
	AiEnd();

	// Delete scene
	if (currScene != NULL)
		delete currScene;
}

//---------------------------------------
// Initialize physx runtime
//---------------------------------------
void SimManager::InitPhysx()
{
	// Singletons
	PxCreateFoundation(PX_PHYSICS_VERSION, pxAllocator, pxErrorCallback);
	PxCreateBasePhysics(PX_PHYSICS_VERSION, PxGetFoundation(), PxTolerancesScale(), true, NULL);
#ifdef _DEBUG
	pPxPvdServer = PxCreatePvd(PxGetFoundation());
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	pPxPvdServer->connect(*transport, PxPvdInstrumentationFlag::eDEBUG);
	PxInitExtensions(PxGetPhysics(), pPxPvdServer);
#endif

	// Set up scene
	PxSceneDesc sceneDesc(PxGetPhysics().getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -6.81f, 0.0f);
	pPxDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = pPxDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	pPxScene = PxGetPhysics().createScene(sceneDesc);

	// Set up mesh cooking
	pPxCooking = PxCreateCooking(PX_PHYSICS_VERSION, PxGetFoundation(), PxCookingParams(PxTolerancesScale()));

#ifdef _DEBUG
	// Set up visual debugger
	PxPvdSceneClient* pvdClient = pPxScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
#endif

	// Default physics material
	pPxMaterial = PxGetPhysics().createMaterial(0.6f, 0.6f, 0.f);
}

//---------------------------------------
// Load and parse json config file
//---------------------------------------
void SimManager::LoadConfig(string config_path)
{
	cout << "Reading config file:\t" << config_path << endl;

	using namespace rapidjson;

	// Open und parse config file
	char buffer[16000];
	FILE* pFile = fopen(config_path.c_str(), "rb");
	FileReadStream is(pFile, buffer, sizeof(buffer));
	CONFIG_FILE.ParseStream<0, UTF8<>, FileReadStream>(is);
}

//---------------------------------------
// Initialize Arnold for rendering
//---------------------------------------
void SimManager::InitArnold()
{
	// Set up arnold
	AiBegin();
	AiMsgSetConsoleFlags(AI_LOG_ALL);

#ifdef WIN32
	const char* SHADERS_PATH = "ShadersLib.lib";
#else
	const char* SHADERS_PATH = "ShadersLib.so";
#endif

	// Set up shaders
	AiNodeEntryInstall(AI_NODE_SHADER, AI_TYPE_RGBA, "depthshader", SHADERS_PATH, depth::DepthShader, AI_VERSION);
	AiNodeEntryInstall(AI_NODE_SHADER, AI_TYPE_RGBA, "blendshader", SHADERS_PATH, blend::BlendShader, AI_VERSION);
	AiNodeEntryInstall(AI_NODE_SHADER, AI_TYPE_RGBA, "labelshader", SHADERS_PATH, label::LabelShader, AI_VERSION);
	AiNodeEntryInstall(AI_NODE_FILTER, AI_TYPE_INT, "null_filter", SHADERS_PATH, filter::NullFilter, AI_VERSION);

	// Create initialize camera
	aiRenderCamera = AiNode("persp_camera");
	aiRenderOptions = AiUniverseGetOptions();
	AiNodeSetInt(aiRenderOptions, "AA_samples", 4);
	AiNodeSetInt(aiRenderOptions, "GI_diffuse_depth", 6);
	AiNodeSetPtr(aiRenderOptions, "camera", aiRenderCamera);

	// Create render driver
	aiOutputDriver = AiNode("driver_png");
	AiNodeSetStr(aiOutputDriver, "name", "outputDriver");
	aiOuputArray = AiArrayAllocate(1, 1, AI_TYPE_STRING);

	// Initialize Shaders
	aiShaderObjectDepth = AiNode("depthshader");
	AiNodeSetBool(aiShaderObjectDepth, "is_body", true);
	aiShaderSceneDepth = AiNode("depthshader");
	aiShaderBlend = AiNode("blendshader");
}

//---------------------------------------
// Load all required meshes (physx, rendering)
//---------------------------------------
void SimManager::LoadMeshes()
{
	DIR* dir;
	// Path to 3D models
	string mesh_path = CONFIG_FILE["models"].GetString();
	// If path exists
	if ((dir = opendir(mesh_path.c_str())) != NULL)
	{
		// For each object
		for (int i = 0; i < CONFIG_FILE["objs"].Size(); i++)
		{
			// Load path
			string obj_path = mesh_path + "/" + CONFIG_FILE["objs"][i].GetString() + ".obj";
			ifstream f(obj_path.c_str());

			// Does object exist?
			if (!f.good())
			{
				cerr << "Did not find obj " << CONFIG_FILE["objs"][i].GetString() << "... Skipping." << endl;
				continue;
			}

			cout << "Loading obj " << CONFIG_FILE["objs"][i].GetString() << endl;
			// Create physx mesh
			PxMeshConvex* curr = new PxMeshConvex(obj_path, i + 1, CONFIG_FILE["scale"].GetFloat(), pPxScene, pPxCooking, pPxMaterial);
			curr->CreateMesh();
			curr->SetMetallic(CONFIG_FILE["metallic"][i].GetFloat());
			// Save in vector
			vecMeshPhysX.push_back(curr);
		}
		// Finally close
		closedir(dir);
	}
}

//---------------------------------------
// Determine scenes to process
//---------------------------------------
void SimManager::X_SaveSceneFolders(string path)
{
	DIR* dir;
	struct dirent* ent;
	vecSceneFolders = std::vector<std::string>();

	// Search provided path
	if ((dir = opendir(path.c_str())) != NULL)
	{
		// While folders available
		while ((ent = readdir(dir)) != NULL)
		{
			// Not a folder
			if (ent->d_name[0] == '.')
				continue;
			// Save folder in vector
			std::string eName = ent->d_name;
			string buf = path + "/" + eName;
			vecSceneFolders.push_back(buf);
		}
		// Finally sort vector and close dir
		std::sort(vecSceneFolders.begin(), vecSceneFolders.end());
		closedir(dir);
	}
}

//---------------------------------------
// Run simulation and rendering
//---------------------------------------
int SimManager::RunSimulation()
{
	// Fetch 3R scan scenes
	X_SaveSceneFolders(CONFIG_FILE["3RScan_path"].GetString());

	// Create mananger
	SceneManager curr(pPxScene, pPxCooking, pPxMaterial,
										aiRenderCamera, aiRenderOptions, aiOutputDriver, aiOuputArray,
										vecMeshPhysX, vecMesh3D,
										0, CONFIG_FILE["objects_per_sim"].GetInt(), &CONFIG_FILE,
										aiShaderObjectDepth, aiShaderSceneDepth, aiShaderBlend);

	// Create lights
	AtNode* light = AiNode("point_light");
	AtNode* light1 = AiNode("point_light");
	AtNode* light2 = AiNode("point_light");
	AtNode* light3 = AiNode("point_light");
	AtNode* light4 = AiNode("point_light");
	AtNode* light5 = AiNode("point_light");
	AtNode* light6 = AiNode("point_light");

	// Setup lights
	AiNodeSetStr(light, "name", "mylight");
	AiNodeSetPnt(light, "position", -1000.f, 100.f, -1000.f);
	AiNodeSetFlt(light, "intensity", 2.f);
	AiNodeSetFlt(light, "radius", 10.f);
	AiNodeSetInt(light, "decay_type", 0);
	AiNodeSetPnt(light1, "position", -100.f, 100.f, 100.f);
	AiNodeSetFlt(light1, "intensity", 2.1f);
	AiNodeSetFlt(light1, "radius", 10.f);
	AiNodeSetInt(light1, "decay_type", 0);
	AiNodeSetPnt(light2, "position", 1000.f, 100.f, -1000.f);
	AiNodeSetFlt(light2, "intensity", 1.9f);
	AiNodeSetFlt(light2, "radius", 10.f);
	AiNodeSetInt(light2, "decay_type", 0);
	AiNodeSetPnt(light3, "position", 1000.f, 100.f, 1000.f);
	AiNodeSetFlt(light3, "intensity", 2.f);
	AiNodeSetFlt(light3, "radius", 10.f);
	AiNodeSetInt(light3, "decay_type", 0);
	AiNodeSetPnt(light4, "position", 1000.f, 1000.f, 1000.f);
	AiNodeSetFlt(light4, "intensity", 2.f);
	AiNodeSetFlt(light4, "radius", 10.f);
	AiNodeSetInt(light4, "decay_type", 0);
	AiNodeSetPnt(light5, "position", 1000.f, 1000.f, -1000.f);
	AiNodeSetFlt(light5, "intensity", 2.f);
	AiNodeSetFlt(light5, "radius", 10.f);
	AiNodeSetInt(light5, "decay_type", 0);
	AiNodeSetPnt(light6, "position", -1000.f, 1000.f, 1000.f);
	AiNodeSetFlt(light6, "intensity", 2.f);
	AiNodeSetFlt(light6, "radius", 10.f);
	AiNodeSetInt(light6, "decay_type", 0);

	int max_count = CONFIG_FILE["max_images"].GetInt();
	int iter_per_scene = CONFIG_FILE["iter_per_scene"].GetInt();

	// Render all scenes
	for (string folder : vecSceneFolders)
	{
		curr.SetScenePath(folder);
		bool count_samples = curr.Run(iter_per_scene, max_count);
		if (!count_samples)
			break;
	}

	return 0;
}
