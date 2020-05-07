#include <SimManager.h>

#pragma warning(push, 0)
#include <dirent.h>
#pragma warning(pop)

//---------------------------------------
// Initialize physx runtime
//---------------------------------------
void SimManager::X_InitPhysx()
{
	// Create foundation
	pPxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, pxAllocator, pxErrorCallback);
	pPxFoundation->setErrorLevel(PxErrorCode::eMASK_ALL);

#ifdef _DEBUG || DEBUG
	// Setup Physx Visual Debugger
	pPxPvd = PxCreatePvd(*pPxFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 100);
	pPxPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
#endif // DEBUG || _DEBUG

	// Create API
#ifdef _DEBUG || DEBUG
	pPxPhysics = PxCreateBasePhysics(PX_PHYSICS_VERSION, *pPxFoundation, PxTolerancesScale(), true, pPxPvd);
#else
	pPxPhysics = PxCreateBasePhysics(PX_PHYSICS_VERSION, *pPxFoundation, PxTolerancesScale(), true, NULL);
#endif // DEBUG || _DEBUG


	// Create mesh cooking
	PxCookingParams params(pPxPhysics->getTolerancesScale());
	params.meshPreprocessParams |= PxMeshPreprocessingFlag::eWELD_VERTICES;
	params.meshPreprocessParams |= PxMeshPreprocessingFlag::eFORCE_32BIT_INDICES;
	params.meshWeldTolerance = 0.01f;
	params.midphaseDesc = PxMeshMidPhase::eBVH34;
	pPxCooking = PxCreateCooking(PX_PHYSICS_VERSION, *pPxFoundation, params);

	// Enable extensions & create dispatcher
#ifdef _DEBUG || DEBUG
	PxInitExtensions(*pPxPhysics, pPxPvd);
#else
	PxInitExtensions(*pPxPhysics, NULL);
#endif // DEBUG || _DEBUG
	pPxDispatcher = PxDefaultCpuDispatcherCreate(4);

	// Create default material
	pPxMaterial = pPxPhysics->createMaterial(0.6f, 0.6f, 0.0f);
}

//---------------------------------------
// Initialize Arnold for rendering
//---------------------------------------
void SimManager::X_InitArnold()
{
	// Start arnold & load shader library
	AiBegin(AI_SESSION_INTERACTIVE);
	AiDeviceAutoSelect();
	AiMsgSetConsoleFlags(AI_LOG_ALL);
	AiLoadPlugins(CONFIG_FILE["shaders_ai"].GetString());

	// Create initialize camera
	aiRenderCamera = AiNode("persp_camera");

	// Setup rendering
	aiRenderOptions = AiUniverseGetOptions();
	AiNodeSetInt(aiRenderOptions, "threads", 0);
	AiNodeSetBool(aiRenderOptions, "skip_license_check", true);
	AiNodeSetInt(aiRenderOptions, "AA_samples", 4);
	AiNodeSetInt(aiRenderOptions, "GI_diffuse_depth", 6);
	AiNodeSetPtr(aiRenderOptions, "camera", aiRenderCamera);

	// Create render driver
	aiOutputDriver = AiNode("driver_png");
	AiNodeSetStr(aiOutputDriver, "name", "outputDriver");
	aiOutputArray = AiArrayAllocate(1, 1, AI_TYPE_STRING);

	// Initialize Shaders
	aiShaderObjectDepth = AiNode("depthshader");
	AiNodeSetBool(aiShaderObjectDepth, "is_body", true);
	aiShaderSceneDepth = AiNode("depthshader");
	aiShaderBlend = AiNode("blendshader");
}

//---------------------------------------
// Load and parse json config file
//---------------------------------------
void SimManager::X_LoadConfig(const string& configPath)
{
	cout << "Reading config file:\t" << configPath << endl;

	using namespace rapidjson;

	// Open und parse config file
	char buffer[16000];
	FILE* pFile = fopen(configPath.c_str(), "rb");
	FileReadStream is(pFile, buffer, sizeof(buffer));
	CONFIG_FILE.ParseStream<0, UTF8<>, FileReadStream>(is);
}

//---------------------------------------
// Load all required meshes (physx, rendering)
//---------------------------------------
void SimManager::X_LoadMeshes()
{
	DIR* dir;
	// Path to 3D models
	string meshesPath = CONFIG_FILE["models"].GetString();
	string aiPath = meshesPath + "/ai_meshes.ass";

	// Initialize vectors
	vecpPxMesh.reserve(CONFIG_FILE["objs"].Size());
	vecpAiMesh.reserve(CONFIG_FILE["objs"].Size());

	// Try to load the arnold mesh nodes
	AtParamValueMap* params = AiParamValueMap();
	AiParamValueMapSetInt(params, AtString("mask"), AI_NODE_SHAPE);
	bool aiNodesLoaded = AiSceneLoad(NULL, aiPath.c_str(), params);
	AiParamValueMapDestroy(params);

	// Fasten up node creation
	if (!aiNodesLoaded)
	{
		AiNodeSetBool(aiRenderOptions, "enable_procedural_cache", true);
		AiNodeSetBool(aiRenderOptions, "parallel_node_init", true);
	}

	// If path exists
	if ((dir = opendir(meshesPath.c_str())) != NULL)
	{
		// For each object
		for (int i = 0; i < CONFIG_FILE["objs"].Size(); i++)
		{
			// Load path
			string meshPath = meshesPath + "/" + CONFIG_FILE["objs"][i].GetString() + ".obj";
			ifstream f(meshPath.c_str());

			// Does mesh exist?
			if (!f.good())
			{
				cerr << "Did not find obj " << CONFIG_FILE["objs"][i].GetString() << "... Skipping." << endl;
				continue;
			}
			else
			{
				cout << "Loading obj " << CONFIG_FILE["objs"][i].GetString() << endl;
			}

			// Create and save physx mesh
			PxMeshConvex* pxCurr = new PxMeshConvex(meshPath, i, pPxCooking, pPxMaterial);
			pxCurr->SetScale(CONFIG_FILE["obj_scale"].GetFloat());
			pxCurr->CreateMesh();
			vecpPxMesh.push_back(pxCurr);

			string texturePath(meshPath);
			texturePath.replace(meshPath.length() - 4, 4, "_color.png");
			// Create and save arnold mesh
			AiMesh* aiCurr = new AiMesh(meshPath, texturePath, i);
			aiCurr->SetMetallic(CONFIG_FILE["metallic"][i].GetFloat());
			vecpAiMesh.push_back(aiCurr);
		}
		// Finally close
		closedir(dir);
	}

	// If base nodes were not yet created
	if (!aiNodesLoaded)
	{
		// Store arnold base nodes in file
		AtParamValueMap* params = AiParamValueMap();
		AiParamValueMapSetInt(params, AtString("mask"), AI_NODE_SHAPE);
		AiParamValueMapSetBool(params, AtString("binary"), true);
		AiParamValueMapSetBool(params, AtString("open_procs"), true);
		AiSceneWrite(NULL, aiPath.c_str(), params);
		AiParamValueMapDestroy(params);
	}
}

//---------------------------------------
// Determine scenes to process
//---------------------------------------
void SimManager::X_SaveSceneFolders(const string& path)
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
	SceneManager curr(pPxDispatcher, pPxCooking, pPxMaterial,
		aiRenderCamera, aiRenderOptions, aiOutputDriver, aiOutputArray,
		vecpPxMesh, vecpAiMesh,
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
	AiNodeSetVec(light, "position", -1000.f, 100.f, -1000.f);
	AiNodeSetFlt(light, "intensity", 2.f);
	AiNodeSetFlt(light, "radius", 10.f);
	AiNodeSetVec(light1, "position", -100.f, 100.f, 100.f);
	AiNodeSetFlt(light1, "intensity", 2.1f);
	AiNodeSetFlt(light1, "radius", 10.f);
	AiNodeSetVec(light2, "position", 1000.f, 100.f, -1000.f);
	AiNodeSetFlt(light2, "intensity", 1.9f);
	AiNodeSetFlt(light2, "radius", 10.f);
	AiNodeSetVec(light3, "position", 1000.f, 100.f, 1000.f);
	AiNodeSetFlt(light3, "intensity", 2.f);
	AiNodeSetFlt(light3, "radius", 10.f);
	AiNodeSetVec(light4, "position", 1000.f, 1000.f, 1000.f);
	AiNodeSetFlt(light4, "intensity", 2.f);
	AiNodeSetFlt(light4, "radius", 10.f);
	AiNodeSetVec(light5, "position", 1000.f, 1000.f, -1000.f);
	AiNodeSetFlt(light5, "intensity", 2.f);
	AiNodeSetFlt(light5, "radius", 10.f);
	AiNodeSetVec(light6, "position", -1000.f, 1000.f, 1000.f);
	AiNodeSetFlt(light6, "intensity", 2.f);
	AiNodeSetFlt(light6, "radius", 10.f);

	// Load config
	int maxImages = CONFIG_FILE["max_images"].GetInt();
	int sceneIters = CONFIG_FILE["iter_per_scene"].GetInt();

	// Render all scenes
	for (string folder : vecSceneFolders)
	{
		curr.SetScenePath(folder);
		bool maxReached = curr.Run(sceneIters, maxImages);
		if (!maxReached)
			break;
	}

	return 0;
}

//---------------------------------------
// Creates new simulation
//---------------------------------------
SimManager::SimManager(const string& configPath)
{
	X_LoadConfig(configPath);
	X_InitPhysx();
	X_InitArnold();
	X_LoadMeshes();
}

//---------------------------------------
// Cleanup simulation
//---------------------------------------
SimManager::~SimManager()
{
	// Cleanup physx meshes
	for (auto curr : vecpPxMesh)
	{
		delete curr;
	}
	vecpPxMesh.clear();

	// Free physics
	PX_RELEASE(pPxMaterial);
	PX_RELEASE(pPxDispatcher);
	PxCloseExtensions();
	PX_RELEASE(pPxPhysics);
	PX_RELEASE(pPxCooking);
#ifdef _DEBUG || DEBUG
	if (pPxPvd)
	{
		PxPvdTransport* transp = pPxPvd->getTransport();
		PX_RELEASE(pPxPvd);
		PX_RELEASE(transp);
	}
#endif // DEBUG || _DEBUG
	PX_RELEASE(pPxFoundation);

	// Cleanup arnold meshes
	for (auto curr : vecpAiMesh)
	{
		delete curr;
	}
	vecpAiMesh.clear();

	// Shutdown arnold
	AiEnd();
}
