#include "SimManager.h"

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

string SimManager::GetFinalPath()
{
	return CONFIG_FILE["final_imgs_path"].GetString();
}

string SimManager::GetTemporaryPath()
{
	return CONFIG_FILE["temp_files_path"].GetString();
}

SimManager::~SimManager()
{
	// Free physics
	if (gPhysics != nullptr)
	{
		free(gFoundation);
		delete gPhysics;
		delete gDispatcher;
		free(gCooking);
		free(gScene);
		free(gMaterial);
	}

	// Free arnold
	if (renderCamera != nullptr)
	{
		free(renderCamera);
		free(renderOptions);
		free(outputDriver);
		delete ouputArray;
		free(shaderObjectDepth);
		free(shaderSceneDepth);
		free(shaderBlend);
	}

	// Delete scene
	if (currScene != nullptr)
		delete currScene;
}

void SimManager::InitPhysx()
{
	gAllocator;
	gErrorCallback;

	// Singletons
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, NULL);

	// Set up scene
	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -6.81f, 0.0f);
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	gScene = gPhysics->createScene(sceneDesc);

	// Set up mesh cooking
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(PxTolerancesScale()));

	// Set up visual debugger
	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	// Default physics material
	gMaterial = gPhysics->createMaterial(0.6f, 0.6f, 0.f);
}

// Loads json config file
void SimManager::LoadConfig(string config_path)
{
	cout << "reading config file:\t" << config_path << endl;
	using namespace rapidjson;

	// Open und parse config file
	char buffer[65536];
	FILE* pFile = fopen(config_path.c_str(), "rb");
	FileReadStream is(pFile, buffer, sizeof(buffer));
	CONFIG_FILE.ParseStream<0, UTF8<>, FileReadStream>(is);
}

// Sets up arnold for rendering
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
	renderCamera = AiNode("persp_camera");
	renderOptions = AiUniverseGetOptions();
	AiNodeSetInt(renderOptions, "AA_samples", 4);
	AiNodeSetInt(renderOptions, "GI_diffuse_depth", 6);
	AiNodeSetPtr(renderOptions, "camera", renderCamera);
	// Create render driver
	outputDriver = AiNode("driver_png");
	AiNodeSetStr(outputDriver, "name", "mydriver");
	ouputArray = AiArrayAllocate(1, 1, AI_TYPE_STRING);
	// Initialize Shaders
	shaderObjectDepth = AiNode("depthshader");
	AiNodeSetBool(shaderObjectDepth, "is_body", true);
	shaderSceneDepth = AiNode("depthshader");
	shaderBlend = AiNode("blendshader");
}

// Loads all required meshes (physx, rendering)
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
			PxMeshConvex* curr = new PxMeshConvex(obj_path, i + 1, CONFIG_FILE["scale"].GetFloat(), gPhysics, gScene, gCooking, gMaterial);
			curr->LoadFile();
			curr->CreateMesh();
			curr->SetMetallic(CONFIG_FILE["metallic"][i].GetFloat());
			// Save in vector
			vecMeshPhysX.push_back(curr);

			// Create arnold mesh
			AiMesh* ai_mesh_manager = new AiMesh(obj_path, i + 1, CONFIG_FILE["scale"].GetFloat());
		}
		// Finally close
		closedir(dir);
	}
}

// Determines scenes to process
void SimManager::X_GetSceneFolders(string path)
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

// Runs simulation and rendering
int SimManager::RunSimulation()
{
	// Fetch 3R scan scenes
	X_GetSceneFolders(CONFIG_FILE["3RScan_path"].GetString());
	SceneManager curr(gPhysics, gScene, gCooking, gFoundation, gMaterial, renderCamera, renderOptions, outputDriver, ouputArray,
		vecMeshPhysX, vecMesh3D, 0, CONFIG_FILE["objects_per_sim"].GetInt(), &CONFIG_FILE,
		shaderObjectDepth, shaderSceneDepth, shaderBlend);

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
		curr.set_scene_path(folder);
		bool count_samples = curr.run(iter_per_scene, max_count);
		if (!count_samples)
			break;
	}

	return 0;
}
