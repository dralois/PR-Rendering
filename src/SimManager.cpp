#include <SimManager.h>

//---------------------------------------
// Load and parse json config file
//---------------------------------------
void SimManager::X_LoadConfig(ReferencePath configPath)
{
	std::cout << "Reading config file:\t" << boost::filesystem::relative(configPath) << std::endl;
	// Open file in binary mode
	FILE* pFile = fopen(configPath.string().c_str(), "rb");
	// Determine size
	size_t fileSize = boost::filesystem::file_size(configPath);
	// Open und parse config file
	char* buffer = new char[fileSize];
	rapidjson::FileReadStream inFile(pFile, buffer, fileSize);
	jsonConfig.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(inFile);
	// Create settings
	pRenderSettings = new Settings(&jsonConfig);
	delete[] buffer;
}

//---------------------------------------
// Load all required meshes (physx, rendering)
//---------------------------------------
void SimManager::X_LoadMeshes()
{
	// Load values from json & initialize
	float toMeters = SafeGet<float>(jsonConfig, "objs_unit");
	std::string format = SafeGet<const char*>(jsonConfig, "mesh_format");
	rapidjson::Value objects = SafeGetArray<const char*>(jsonConfig, "render_objs");
	vecpPxMesh.reserve(objects.Size());
	vecpRenderMesh.reserve(objects.Size());

	// If path exists & directory
	if (boost::filesystem::exists(pRenderSettings->GetMeshesPath()))
	{
		if (boost::filesystem::is_directory(pRenderSettings->GetMeshesPath()))
		{
			// For each object
			for (int i = 0; i < static_cast<int>(objects.Size()); ++i)
			{
				// Build paths
				ModifiablePath meshPath(pRenderSettings->GetMeshesPath());
				meshPath.append(SafeGet<const char*>(objects[i]));
				meshPath.concat(".");
				meshPath.concat(format.empty() ? "obj" : format);
				ModifiablePath texturePath(pRenderSettings->GetMeshesPath());
				texturePath.append(SafeGet<const char*>(objects[i]));
				texturePath.concat("_color.png");

				// Mesh file must exist
				if (!boost::filesystem::exists(meshPath))
				{
					std::cout << "\r\33[2K" << "Mesh missing:\t" << meshPath.relative_path() << ", skipping" << std::endl;
					continue;
				}

				// Create and save physx mesh
				PxMeshConvex* pxCurr = new PxMeshConvex(meshPath, i);
				pxCurr->CreateMesh();
				pxCurr->SetScale(physx::PxVec3(toMeters));
				vecpPxMesh.push_back(pxCurr);

				// Create and save render mesh
				RenderMesh* renderCurr = new RenderMesh(meshPath, texturePath, i);
				renderCurr->SetScale(Eigen::Vector3f().setConstant(toMeters));
				vecpRenderMesh.push_back(renderCurr);
				std::cout << std::endl;
			}
		}
	}
	// Cleanup
	SafeDeleteArray(objects);
}

//---------------------------------------
// Determine scenes to process
//---------------------------------------
void SimManager::X_SaveSceneFolders(ReferencePath path)
{
	vecSceneFolders.clear();

	// Search provided directory
	if (boost::filesystem::exists(path))
	{
		if (boost::filesystem::is_directory(path))
		{
			for (auto entry : boost::filesystem::directory_iterator(path))
			{
				// Save folder in vector, if it is a scene
				if (boost::filesystem::is_directory(entry.path()))
				{
					if (boost::filesystem::exists(ModifiablePath(entry.path()).append("rgbd")))
					{
						vecSceneFolders.push_back(entry);
					}
				}
			}
			// Finally sort vector
			std::sort(vecSceneFolders.begin(), vecSceneFolders.end());
		}
	}
}

//---------------------------------------
// Run simulation and rendering
//---------------------------------------
void SimManager::RunSimulation()
{
	// Create mananger
	SceneManager sceneMgr(pRenderSettings, vecpPxMesh, vecpRenderMesh);

	// Render all scenes
	int currImageCount = 0;
	for (const auto& folder : vecSceneFolders)
	{
		// Set path
		pRenderSettings->SetScenePath(folder);
		// Stop at max rendered images
		currImageCount += sceneMgr.ProcessNext(currImageCount);
		if (currImageCount >= pRenderSettings->GetMaxImageCount())
			break;
	}
}

//---------------------------------------
// Creates new simulation
//---------------------------------------
SimManager::SimManager(
	ReferencePath configPath
) :
	vecpPxMesh(),
	vecpRenderMesh(),
	jsonConfig(NULL),
	pRenderSettings(NULL),
	vecSceneFolders()
{
	// Init
	PxManager::GetInstance().InitPhysx();
	X_LoadConfig(configPath);
	X_LoadMeshes();
	// Setup scenes
	ModifiablePath scenesPath(SafeGet<const char*>(jsonConfig, "scenes_path"));
	X_SaveSceneFolders(boost::filesystem::absolute(scenesPath));
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
	delete pRenderSettings;
	jsonConfig.GetAllocator().Clear();
	PxManager::GetInstance().DeletePhysx();
}
