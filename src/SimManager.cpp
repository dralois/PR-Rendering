#include <SimManager.h>

using namespace physx;

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
void SimManager::X_LoadConfig(const boost::filesystem::path& configPath)
{
	std::cout << "Reading config file:\t" << configPath << std::endl;
	// Open file in binary mode
	FILE* pFile = fopen(configPath.string().c_str(), "rb");
	// Determine size
	size_t fileSize = boost::filesystem::file_size(configPath);
	// Open und parse config file
	char* buffer = new char[fileSize];
	rapidjson::FileReadStream inFile(pFile, buffer, fileSize);
	jsonConfig.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(inFile);
	// Create settings
	pRenderSettings = new Settings(jsonConfig);
}

//---------------------------------------
// Load all required meshes (physx, rendering)
//---------------------------------------
void SimManager::X_LoadMeshes()
{
	auto objects = jsonConfig["render_objs"].GetArray();

	// Initialize vectors
	vecpPxMesh.reserve(objects.Size());
	vecpRenderMesh.reserve(objects.Size());

	// If path exists & directory
	if (boost::filesystem::exists(pRenderSettings->GetMeshesPath()))
	{
		if (boost::filesystem::is_directory(pRenderSettings->GetMeshesPath()))
		{
			// For each object
			for (int i = 0; i < objects.Size(); i++)
			{
				// Build paths
				boost::filesystem::path meshPath(pRenderSettings->GetMeshesPath());
				meshPath.append(objects[i].GetString());
				meshPath.concat(".obj");
				boost::filesystem::path texturePath(pRenderSettings->GetMeshesPath());
				texturePath.append(objects[i].GetString());
				texturePath.concat("_color.png");

				// File must exist
				boost::filesystem::ifstream meshFile(meshPath);
				if (!meshFile.good())
				{
					std::cout << "Did not find obj " << meshPath << "... Skipping." << std::endl;
					continue;
				}
				else
				{
					std::cout << "Loading obj " << meshPath.filename() << std::endl;
				}

				// Create and save physx mesh
				PxMeshConvex* pxCurr = new PxMeshConvex(meshPath, i, pPxCooking, pPxMaterial);
				pxCurr->SetScale(PxVec3(jsonConfig["obj_scale"].GetFloat()));
				pxCurr->CreateMesh();
				vecpPxMesh.push_back(pxCurr);

				// Create and save arnold mesh
				RenderMesh* renderCurr = new RenderMesh(meshPath, texturePath, i);
				vecpRenderMesh.push_back(renderCurr);
			}
		}
	}
}

//---------------------------------------
// Determine scenes to process
//---------------------------------------
void SimManager::X_SaveSceneFolders(const boost::filesystem::path& path)
{
	vecSceneFolders = std::vector<boost::filesystem::path>();

	// Search provided directory
	if (boost::filesystem::exists(path))
	{
		if (boost::filesystem::is_directory(path))
		{
			for (auto entry : boost::filesystem::directory_iterator(path))
			{
				// Save folder in vector
				if (boost::filesystem::is_directory(entry.path()))
				{
					vecSceneFolders.push_back(entry);
				}
			}
			// Finally sort vector and close dir
			std::sort(vecSceneFolders.begin(), vecSceneFolders.end());
		}
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
	std::vector<Light> lights;
	for(int i = 0; i < 8; i++)
	{
		LightParamsBase* params = (LightParamsBase*)(new PointLightParams());
		Light addLight(params, Eigen::Vector3f(1.0f, 1.0f, 1.0f), 2.0f, 1.0f);
		// Add 8 lights in a box pattern
		addLight.SetPosition(Eigen::Vector3f(
			i % 2 == 0 ? 1000.0f : -1000.0f,
			(i >> 2) % 2 == 0 ? 100.0f : 1000.0f,
			(i >> 1) % 2 == 0 ? 1000.0f : -1000.0f)
		);
		lights.push_back(addLight);
	}

	// Render all scenes
	int currImageCount = 0;
	for (auto folder : vecSceneFolders)
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
SimManager::SimManager(const boost::filesystem::path& configPath)
{
	boost::filesystem::path scenesPath(jsonConfig["scenes_path"].GetString());
	// Init
	X_LoadConfig(configPath);
	X_InitPhysx();
	X_LoadMeshes();
	X_SaveSceneFolders(scenesPath);
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
