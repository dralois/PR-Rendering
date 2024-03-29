#include <SimManager.h>

#define PTR_RELEASE(x) if(x != NULL) { delete x; x = NULL; }
#define VEC_RELEASE(x) for(auto curr : x) { delete curr; } x.clear();

//---------------------------------------
// Creates output folder structure
//---------------------------------------
void SimManager::X_CreateOutputFolders()
{
	using namespace boost::filesystem;

	// Save the paths
	ModifiablePath finalDir(pRenderSettings->GetFinalPath());
	ModifiablePath tempDir(pRenderSettings->GetTemporaryPath());

	// Create final output directories
	if (!exists(finalDir))
	{
		create_directories(finalDir);
	}
	if (is_empty(finalDir))
	{
		create_directories(finalDir / "rgb");
		create_directories(finalDir / "depth");
		create_directories(finalDir / "segs");
		create_directories(finalDir / "models");
		create_directories(finalDir / "annotations");
	}

	// Create temporary output directories
	if (!exists(tempDir))
	{
		create_directories(tempDir);
	}
	if (is_empty(tempDir))
	{
		create_directories(tempDir / "body_depth");
		create_directories(tempDir / "body_label");
		create_directories(tempDir / "body_mask");
		create_directories(tempDir / "body_rgb");
		create_directories(tempDir / "body_ao");
	}

}

//---------------------------------------
// Load all required meshes (physx, rendering)
//---------------------------------------
void SimManager::X_LoadMeshes()
{
	using namespace boost::filesystem;

	// Cleanup old meshes
	VEC_RELEASE(vecpRenderMesh);
	VEC_RELEASE(vecpPxMesh);

	// Load values from json & initialize

	rapidjson::Value objects = SafeGetArray(pRenderSettings->GetJSONConfig(), "render_objs");
	vecpPxMesh.reserve(objects.Size());
	vecpRenderMesh.reserve(objects.Size());

	// If path exists & directory
	if (exists(pRenderSettings->GetMeshesPath()))
	{
		if (is_directory(pRenderSettings->GetMeshesPath()))
		{
			// For each object
			for (int i = 0; i < static_cast<int>(objects.Size()); ++i)
			{
				// Get mesh descriptor
				const rapidjson::Value& currVal = objects[i];

				// Build paths
				ModifiablePath basePath(SafeGet<const char*>(currVal, "mesh_path"));
				ModifiablePath meshPath = basePath.has_parent_path() ?
					basePath : pRenderSettings->GetMeshesPath() / basePath;
				// Texture path has a default option if no path is given
				auto texPath = SafeGet<const char*>(currVal, "mesh_texture");
				ModifiablePath texturePath(texPath ? 
					(basePath.has_parent_path() ? basePath : pRenderSettings->GetMeshesPath()) / texPath :
					((basePath.has_parent_path() ? basePath.parent_path() : pRenderSettings->GetMeshesPath()) / basePath.stem()) += "_color.png");

				// Mesh file must exist
				if (!exists(meshPath))
				{
					std::cout << "\33[2K\r" << "Mesh missing:\t" << meshPath.relative_path() << ", skipping" << std::endl;
					continue;
				}

				// Extract other infos
				float objScl = SafeGet<float>(currVal, "mesh_unit");
				std::string meshClass(SafeGet<const char*>(currVal, "mesh_class"));
				std::string meshShader = "";

				// Shader property does not need to exist
				const rapidjson::Value* shaderVal;
				if(SafeHasMember(currVal, "mesh_shader", shaderVal))
				{
					meshShader = std::string(SafeGetValue<const char*>(*shaderVal));
				}

				// Create and save physx mesh
				PxMeshConvex* pxCurr = new PxMeshConvex(meshPath, meshClass, i);
				pxCurr->SetObjId(0);
				pxCurr->CreateMesh();
				pxCurr->SetScale(physx::PxVec3(objScl));
				vecpPxMesh.push_back(pxCurr);

				// Create and save render mesh
				RenderMesh* renderCurr = new RenderMesh(meshPath, texturePath, meshClass, meshShader, i);
				renderCurr->SetObjId(0);
				renderCurr->CreateMesh();
				renderCurr->SetScale(Eigen::Vector3f().setConstant(objScl));
				vecpRenderMesh.push_back(renderCurr);

				// Copy the mesh to final folder
				boost::system::error_code res;
				copy_file(
					meshPath,
					pRenderSettings->GetFinalPath() / "models" / meshPath.filename(),
					copy_option::fail_if_exists,
					res
				);

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
			// Shuffle scenes for more varied outputs
			std::random_device rd;
			std::default_random_engine gen(rd());
			std::shuffle(vecSceneFolders.begin(), vecSceneFolders.end(), gen);
		}
	}
}

//---------------------------------------
// Run simulation and rendering
//---------------------------------------
void SimManager::RunSimulation()
{
	// Create mananger
	SceneManager sceneMgr(*pRenderSettings, vecpPxMesh, vecpRenderMesh);

	// Render all scenes
	int currImageCount = 0;
	for (const auto& folder : vecSceneFolders)
	{
		// Set path
		pRenderSettings->SetScenePath(folder);
		// Stop at max rendered images
		currImageCount += sceneMgr.ProcessNext(currImageCount);
		if (currImageCount >= pRenderSettings->GetSimulationSettings().TotalLimit)
			break;
		else
			std::cout << "Switching scene, progress: " << currImageCount << "/"
			<< pRenderSettings->GetSimulationSettings().TotalLimit << " images generated" << std::endl;
	}
}

//---------------------------------------
// Creates new simulation
//---------------------------------------
SimManager::SimManager(Settings* pSettings) :
	pRenderSettings(pSettings),
	vecSceneFolders(),
	vecpRenderMesh(),
	vecpPxMesh()
{
	// Init
	PxManager::GetInstance().InitPhysx();
	X_CreateOutputFolders();
	X_LoadMeshes();

	try
	{
		// Setup scenes
		ModifiablePath scenesPath(SafeGet<const char*>(pRenderSettings->GetJSONConfig(), "scenes_path"));
		X_SaveSceneFolders(boost::filesystem::canonical(scenesPath, pSettings->GetBasePath()));
	}
	catch (const boost::filesystem::filesystem_error&)
	{
		std::cout << "Scenes folder does not exist." << std::endl;
	}
}

//---------------------------------------
// Cleanup simulation
//---------------------------------------
SimManager::~SimManager()
{
	// Cleanup render & physx meshes
	VEC_RELEASE(vecpRenderMesh);
	VEC_RELEASE(vecpPxMesh);

	// Delete temporary output
#if !_DEBUG && !DEBUG
	ModifiablePath tempDir(pRenderSettings->GetTemporaryPath());
	if (boost::filesystem::exists(tempDir))
	{
		boost::filesystem::remove_all(tempDir);
	}
#endif //!_DEBUG && !DEBUG

	// Cleanup config & settings
	PTR_RELEASE(pRenderSettings);
	PxManager::GetInstance().DeletePhysx();
}
