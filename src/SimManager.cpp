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
// Load and parse json config file
//---------------------------------------
void SimManager::X_LoadConfig(const string& configPath)
{
	cout << "Reading config file:\t" << configPath << endl;
	// Open file in binary mode
	FILE* pFile = fopen(configPath.c_str(), "rb");
	// Determine size
	fseek(pFile, 0, SEEK_END);
	size_t fileSize = ftell(pFile);
	rewind(pFile);
	// Open und parse config file
	char* buffer = new char[fileSize];
	FileReadStream inFile(pFile, buffer, fileSize);
	jsonConfig.ParseStream<0, UTF8<>, FileReadStream>(inFile);
	// Create settings
	pRenderSettings = new Settings(jsonConfig);
}

//---------------------------------------
// Load all required meshes (physx, rendering)
//---------------------------------------
void SimManager::X_LoadMeshes()
{
	DIR* dir;
	auto objects = jsonConfig["render_objs"].GetArray();
	string meshesPath = pRenderSettings->GetMeshesPath();

	// Initialize vectors
	vecpPxMesh.reserve(objects.Size());
	vecpRenderMesh.reserve(objects.Size());

	// If path exists
	if ((dir = opendir(meshesPath.c_str())) != NULL)
	{
		// For each object
		for (int i = 0; i < objects.Size(); i++)
		{
			// Load path
			string meshPath = meshesPath + "/" + objects[i].GetString() + ".obj";
			ifstream f(meshPath.c_str());

			// Does mesh exist?
			if (!f.good())
			{
				cout << "Did not find obj " << objects[i].GetString() << "... Skipping." << endl;
				continue;
			}
			else
			{
				cout << "Loading obj " << objects[i].GetString() << endl;
			}

			// Create and save physx mesh
			PxMeshConvex* pxCurr = new PxMeshConvex(meshPath, i, pPxCooking, pPxMaterial);
			pxCurr->SetScale(PxVec3(jsonConfig["obj_scale"].GetFloat()));
			pxCurr->CreateMesh();
			vecpPxMesh.push_back(pxCurr);

			string texturePath(meshPath);
			texturePath.replace(meshPath.length() - 4, 4, "_color.png");
			// Create and save arnold mesh
			RenderMesh* renderCurr = new RenderMesh(meshPath, texturePath, i);
			vecpRenderMesh.push_back(renderCurr);
		}
		// Finally close
		closedir(dir);
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
	// Create mananger
	SceneManager curr(pPxDispatcher, pPxCooking, pPxMaterial,
		vecpPxMesh, vecpRenderMesh, pRenderSettings);

	// Setup lights
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

	// Render all scenes
	int currImageCount = 0;
	for (string folder : vecSceneFolders)
	{
		// Set path
		pRenderSettings->SetScenePath(folder);
		// Stop at max rendered images
		currImageCount += curr.Run(currImageCount);
		if (currImageCount >= pRenderSettings->GetMaxImageCount())
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
	X_LoadMeshes();
	X_SaveSceneFolders(jsonConfig["scenes_path"].GetString());
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

	// Cleanup render meshes
	for (auto curr : vecpRenderMesh)
	{
		delete curr;
	}
	vecpRenderMesh.clear();

	// Other
	delete pRenderSettings;
}
