#include <SimManager.h>

//---------------------------------------
// Load and parse json config file
//---------------------------------------
void SimManager::X_LoadConfig(ReferencePath configPath)
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
				ModifiablePath meshPath(pRenderSettings->GetMeshesPath());
				meshPath.append(SafeGet<const char*>(objects[i]));
				meshPath.concat(".obj");
				ModifiablePath texturePath(pRenderSettings->GetMeshesPath());
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
void SimManager::X_SaveSceneFolders(ReferencePath path)
{
	vecSceneFolders = std::vector<ModifiablePath>();

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
	SceneManager curr(pRenderSettings, vecpPxMesh, vecpRenderMesh);

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
SimManager::SimManager(ReferencePath configPath)
{
	// Init
	PxManager::GetInstance().InitPhysx();
	X_LoadConfig(configPath);
	X_LoadMeshes();
	// Setup scenes
	ModifiablePath scenesPath(SafeGet<const char*>(jsonConfig, "scenes_path"));
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
