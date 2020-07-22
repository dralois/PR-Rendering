#include <SimManager.h>

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
	// CHECK: Cleanup?
	delete[] buffer;
}

//---------------------------------------
// Load all required meshes (physx, rendering)
//---------------------------------------
void SimManager::X_LoadMeshes()
{
	// Sanity check
	if (!jsonConfig.HasMember("render_objs"))
		return;
	else
		if (!jsonConfig["render_objs"].IsArray())
			return;

	JSONArray objects = jsonConfig["render_objs"].GetArray();

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
				meshPath.append(SafeGet<const char*>(objects[i]));
				meshPath.concat(".obj");
				boost::filesystem::path texturePath(pRenderSettings->GetMeshesPath());
				texturePath.append(SafeGet<const char*>(objects[i]));
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
				PxMeshConvex* pxCurr = new PxMeshConvex(meshPath, i);
				pxCurr->SetScale(physx::PxVec3(SafeGet<float>(jsonConfig, "obj_scale")));
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
	// Setup lights
	std::vector<Light> lights;
	for (int i = 0; i < 8; i++)
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

	// Create mananger
	SceneManager curr(pRenderSettings, lights, vecpPxMesh, vecpRenderMesh);

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
	boost::filesystem::path scenesPath(SafeGet<const char*>(jsonConfig, "scenes_path"));
	// Init
	X_LoadConfig(configPath);
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

	// Cleanup render meshes
	for (auto curr : vecpRenderMesh)
	{
		delete curr;
	}
	vecpRenderMesh.clear();

	// Other
	PxManager::GetInstance().DeletePhysx();
	delete pRenderSettings;
}
