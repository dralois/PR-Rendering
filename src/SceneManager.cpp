#include <SceneManager.h>

#define USE_OPENGL 1
#define STORE_DEBUG_TEX 1

#define PI (3.1415926535897931f)
#define PIOVER2 (1.5707963267948966f)

#define RENDERFILE_BEGIN \
	rapidjson::StringBuffer renderstring;\
	JSONWriter writer(renderstring);\
	writer.StartArray();

#define RENDERFILE_WRITER writer

#define RENDERFILE_PROCESS \
	writer.EndArray();\
	std::string renderfile(renderstring.GetString());\
	pBlender->ProcessRenderfile(renderfile);

using namespace physx;

//---------------------------------------
// Create physx scan scene mesh
//---------------------------------------
void SceneManager::X_PxCreateScene()
{
	// Create physx mesh of scan scene
	ModifiablePath meshPath(pRenderSettings->GetScenePath());
	meshPath.append("mesh.refined.obj");
	pPxMeshScene = new PxMeshTriangle(meshPath, 0);
	pPxMeshScene->SetScale(PxVec3(SafeGet<float>(pRenderSettings->GetJSONConfig(), "scene_scale")));
	pPxMeshScene->SetObjId(0);
	pPxMeshScene->CreateMesh();

	// Standart gravity & continuous collision detection
	PxSceneDesc sceneDesc(PxGetPhysics().getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(4);
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;
	// Objects should never be outside twice the scene bounds
	sceneDesc.sanityBounds = PxBounds3(pPxMeshScene->GetMinimum() * 2.0f, pPxMeshScene->GetMaximum() * 2.0f);

	// Create scene
	pPxScene = PxGetPhysics().createScene(sceneDesc);

	// Scene meshes need to be rotated 90* around X during the simulation
	PxTransform pose(PxVec3(0, 0, 0), PxQuat(-PIOVER2, PxVec3(1, 0, 0)));
	pPxMeshScene->SetTransform(pose);
	pPxMeshScene->AddRigidActor(pPxScene);
}

//---------------------------------------
// Create physx object meshes
//---------------------------------------
void SceneManager::X_PxCreateObjs()
{
	// Seed & setup
	srand(time(NULL) % 1000);
	vecpPxMeshCurrObjs.reserve(pRenderSettings->GetObjectsPerSimulation());

	// For each object
	for (PxU32 i = 0; i < pRenderSettings->GetObjectsPerSimulation(); i++)
	{
		int currPos = rand() % vecpPxMeshObjs.size();
		// Fetch random object & create instance with new id
		PxMeshConvex* currObj = new PxMeshConvex(*vecpPxMeshObjs.at(currPos));
		currObj->SetObjId(i);
		currObj->CreateMesh();

		auto sceneSize = pPxMeshScene->GetMaximum() - pPxMeshScene->GetMinimum();
		auto sceneCenter = pPxMeshScene->GetMinimum() + (sceneSize / 2.0f);

		// Random position in scene
		float y = sceneCenter.y * 0.5f + (static_cast<float>(rand()) / RAND_MAX) * 0.5f * sceneSize.y;
		float x = pPxMeshScene->GetMinimum().x + (static_cast<float>(rand()) / RAND_MAX) * sceneSize.x;
		float z = pPxMeshScene->GetMinimum().z + (static_cast<float>(rand()) / RAND_MAX) * sceneSize.z;

		// Set pose and save mesh in vector
		PxTransform pose(PxVec3(z, y, x), PxQuat(PxIdentity));
		currObj->SetTransform(pose);
		currObj->AddRigidActor(pPxScene);
		vecpPxMeshCurrObjs.push_back(std::move(currObj));
	}
}

//---------------------------------------
// Cleanup scene & iteration
//---------------------------------------
void SceneManager::X_CleanupScene()
{
	// Cleanup object meshes & physx meshes
	for (auto currPx : vecpPxMeshCurrObjs)
	{
		delete currPx;
	}
	for (auto currRender : vecpRenderMeshCurrObjs)
	{
		delete currRender;
	}
	vecpPxMeshCurrObjs.clear();
	vecpRenderMeshCurrObjs.clear();

	// Cleanup scene mesh & physx mesh & physx
	if (pPxMeshScene != NULL)
	{
		delete pPxMeshScene;
		pPxMeshScene = NULL;
	}
	if (pPxScene != NULL)
	{
		pPxScene->flushSimulation();
		auto dispatcher = (PxDefaultCpuDispatcher*)pPxScene->getCpuDispatcher();
		PX_RELEASE(dispatcher);
		PX_RELEASE(pPxScene);
	}

	// Reload Blender rendering
	pBlender->UnloadProcesses();

	// Finish annotation
	pAnnotations->End();
}

//---------------------------------------
// Run physx simulation
//---------------------------------------
void SceneManager::X_PxRunSim(
	float timestep,
	int stepCount
) const
{
	// Simulate in steps
	for (PxU32 i = 0; i < stepCount; i++)
	{
		pPxScene->simulate(timestep);
		pPxScene->fetchResults(true);
		std::cout << '\r' << "Simulating: " << (i + 1) << "/" << stepCount << std::flush;
	}
	// Formatting
	std::cout << std::endl;
}

//---------------------------------------
// Fetch and save simulation results
//---------------------------------------
void SceneManager::X_PxSaveSimResults()
{
	float undoScale = 1.0f / SafeGet<float>(pRenderSettings->GetJSONConfig(), "scene_scale");
	float objScale = undoScale * SafeGet<float>(pRenderSettings->GetJSONConfig(), "obj_scale");
	// For each physx object
	for (auto obj : vecpPxMeshCurrObjs)
	{
		// Get position
		PxTransform pose = obj->GetTransform();
		// Undo 90* around X rotation
		PxQuat undoRot(PIOVER2, PxVec3(1, 0, 0));
		pose.q = (pose.q * undoRot).getNormalized();
		pose.p = undoRot.rotate(pose.p);
		// Undo scene scaling
		pose.p *= undoScale;

		// Save & create mesh for rendering
		RenderMesh* currMesh = new RenderMesh(*this->vecpRenderMeshObjs[obj->GetMeshId()]);
		currMesh->SetObjId(obj->GetObjId());
		// Build transform from px pose
		Eigen::Affine3f currTrans;
		currTrans.fromPositionOrientationScale(
			Eigen::Vector3f(pose.p.x, pose.p.y, pose.p.z),
			Eigen::Quaternionf(pose.q.w, pose.q.x, pose.q.y, pose.q.z),
			currMesh->GetScale() * objScale);
		currMesh->SetTransform(currTrans.matrix());
		// Store it
		vecpRenderMeshCurrObjs.push_back(std::move(currMesh));

#if DEBUG || _DEBUG
		// Object transform update for pvd
		obj->SetTransform(pose);
		obj->SetScale(obj->GetScale() * undoScale);
	}
	// Scene transform update for pvd
	pPxMeshScene->SetTransform(PxTransform(PxIdentity));
	pPxMeshScene->SetScale(pPxMeshScene->GetScale() * undoScale);
	// Simulate once for pvd
	pPxScene->simulate(0.001f);
	pPxScene->fetchResults(true);
#else
}
#endif // DEBUG || _DEBUG
}

//---------------------------------------
// Render scene depth OpenGL
//---------------------------------------
std::vector<Texture> SceneManager::X_GLSceneDepth(
	const std::vector<ModifiablePath>& poses
) const
{
	// Render depth with OpenGL
	auto renders = pRenderer->RenderScenes(
		pRenderSettings->GetScenePath(),
		poses,
		camBlueprint.GetIntrinsics().GetFocalLenght().x(),
		camBlueprint.GetIntrinsics().GetFocalLenght().y(),
		camBlueprint.GetIntrinsics().GetPrincipalPoint().x(),
		camBlueprint.GetIntrinsics().GetPrincipalPoint().y(),
		camBlueprint.GetIntrinsics().GetWidth(),
		camBlueprint.GetIntrinsics().GetHeight()
	);

	// For each image
	std::vector<Texture> results;
	for (int render_count = 0; render_count < renders.size(); render_count++)
	{
		// Create depth texture
		Texture depthScene(true, true);
		depthScene.SetTexture(std::get<1>(renders[render_count]));
		depthScene.SetPath(pRenderSettings->GetImagePath("scene_depth", render_count), false);
		// Store in result vector
		results.push_back(std::move(depthScene));
	}

	// Return depth maps
	return results;
}

//---------------------------------------
// Adds provided scene to renderfile
//---------------------------------------
void SceneManager::X_ConvertToRenderfile(
	JSONWriterRef writer,
	std::vector<RenderMesh*>& meshes,
	std::vector<Camera>& cams
)
{
	writer.StartObject();

	// Add settings
	writer.Key("settings");
	pRenderSettings->AddToJSON(writer);

	// Add provided cameras
	writer.Key("cameras");
	writer.StartArray();
	for (auto& currCam : cams)
	{
		currCam.AddToJSON(writer);
	}
	writer.EndArray();

	// Add provided meshes
	writer.Key("meshes");
	writer.StartArray();
	for (auto currMesh : meshes)
	{
		currMesh->AddToJSON(writer);
	}
	writer.EndArray();

	// Add lights
	writer.Key("lights");
	writer.StartArray();
	for (auto currLight : vecpLights)
	{
		currLight->AddToJSON(writer);
	}
	writer.EndArray();

	writer.EndObject();
}

//---------------------------------------
// TODO: Build scene depth renderfile
//---------------------------------------
void SceneManager::X_BuildSceneDepth(
	JSONWriterRef writer,
	std::vector<Camera>& cams,
	std::vector<Texture>& results,
	int start
)
{
	X_ConvertToRenderfile(writer, std::vector<RenderMesh*>{}, cams);
}

//---------------------------------------
// Build objects depth renderfile
//---------------------------------------
void SceneManager::X_BuildObjectsDepth(
	JSONWriterRef writer,
	std::vector<Camera>& cams,
	std::vector<Texture>& results,
	int start
)
{
	// For every pose
	for (int curr = 0; curr < cams.size(); ++curr)
	{
		// Create depth output texture
		Texture currDepth(true, true);
		currDepth.SetPath(pRenderSettings->GetImagePath("body_depth", start + curr), true, "exr");
		// Setup rendering params
		cams[curr].SetupRendering(
			currDepth.GetPath(),
			pRenderSettings->GetRenderResolution(),
			true,
			1,
			0,
			"depth"
		);
		// Place in output vector
		results.emplace_back(std::move(currDepth));
	}

	// Set shaders
	for (auto currMesh : vecpRenderMeshCurrObjs)
	{
		DepthShader* currShader = new DepthShader(camBlueprint.GetClipping().x(), camBlueprint.GetClipping().y());
		currMesh->SetShader(currShader);
	}

	// Add configured scene to renderfile
	X_ConvertToRenderfile(writer, vecpRenderMeshCurrObjs, cams);
}

//---------------------------------------
// Build object label renderfile
//---------------------------------------
void SceneManager::X_BuildObjectsLabel(
	JSONWriterRef writer,
	std::vector<Camera>& cams,
	std::vector<Texture>& results,
	int start
)
{
	// For every pose
	for (int curr = 0; curr < cams.size(); ++curr)
	{
		// Create label output texture
		Texture currLabel(true, false);
		currLabel.SetPath(pRenderSettings->GetImagePath("body_label", start + curr), true, "exr");
		// Setup rendering params
		cams[curr].SetupRendering(
			currLabel.GetPath(),
			pRenderSettings->GetRenderResolution(),
			true,
			1,
			0,
			""
		);
		// Place in output vector
		results.emplace_back(std::move(currLabel));
	}

	// Set shaders
	for (auto currMesh : vecpRenderMeshCurrObjs)
	{
		cv::Vec3b encodedId = EncodeInt(currMesh->GetObjId());
		// Store encoded ID as BGR
		LabelShader* currShader = new LabelShader(encodedId[2], encodedId[1], encodedId[0]);
		currMesh->SetShader(currShader);
	}

	// Add configured scene to renderfile
	X_ConvertToRenderfile(writer, vecpRenderMeshCurrObjs, cams);
}

//---------------------------------------
// Build objects PBR renderfile
//---------------------------------------
void SceneManager::X_BuildObjectsPBR(
	JSONWriterRef writer,
	std::vector<Camera>& cams,
	std::vector<Texture>& results,
	int start
)
{
	// Resolution should match real scene images
	Eigen::Vector2i renderRes(camBlueprint.GetIntrinsics().GetWidth(), camBlueprint.GetIntrinsics().GetHeight());
	// For every pose
	for (int curr = 0; curr < cams.size(); ++curr)
	{
		// Create PBR output texture
		Texture currPBR(false, false);
		currPBR.SetPath(pRenderSettings->GetImagePath("body_rgb", start + curr), false);
		// Setup rendering params
		cams[curr].SetupRendering(
			currPBR.GetPath(),
			renderRes,
			false,
			16,
			-1,
			""
		);
		// Place in output vector
		results.emplace_back(std::move(currPBR));
	}

	// Set shaders
	for (auto currMesh : vecpRenderMeshCurrObjs)
	{
		// Create diffuse texture
		Texture* currDiffuse = new Texture();
		currDiffuse->SetPath(currMesh->GetTexturePath(), false);
		// Set PBR shader
		PBRShader* currShader = new PBRShader(currDiffuse);
		currMesh->SetShader(currShader);
	}

	// Add configured scene to renderfile
	X_ConvertToRenderfile(writer, vecpRenderMeshCurrObjs, cams);
}

//---------------------------------------
// Render coverage masks & depths
//---------------------------------------
std::vector<Mask> SceneManager::X_RenderDepthMasks(
	std::vector<Camera>& cams,
	int start
)
{
	// Initialize output vectors
	std::vector<Mask> maskedResults(cams.size(), Mask());
	std::vector<Texture> objectDepths, sceneDepths;
	objectDepths.reserve(cams.size());
	sceneDepths.reserve(cams.size());

	// Create & process renderfile
	RENDERFILE_BEGIN;
	X_BuildObjectsDepth(RENDERFILE_WRITER, cams, objectDepths, start);
#if !USE_OPENGL
	X_BuildSceneDepth(RENDERFILE_WRITER, cams, sceneDepths, start);
#else
	// OpenGL: Extract poses
	std::vector<ModifiablePath> poses;
	for (const Camera& cam : cams)
	{
		poses.emplace_back(ModifiablePath(cam.GetSourceFile()));
	}
	// Immediately render
	sceneDepths = X_GLSceneDepth(poses);
#endif //!USE_OPENGL
	RENDERFILE_PROCESS;

	// For every pose
	for (int curr = 0; curr < cams.size(); ++curr)
	{
		// Load & unpack depth
		objectDepths[curr].LoadTexture(UnpackDepth);
		objectDepths[curr].ReplacePacked();
#if !USE_OPENGL
		sceneDepths[curr].LoadTexture(UnpackDepth);
		sceneDepths[curr].ReplacePacked();
#endif //!USE_OPENGL
#if STORE_DEBUG_TEX
		objectDepths[curr].StoreDepth01(camBlueprint.GetClipping().x(), camBlueprint.GetClipping().y());
#if !USE_OPENGL
		sceneDepths[curr].StoreDepth01(camBlueprint.GetClipping().x(), camBlueprint.GetClipping().y());
#endif //!USE_OPENGL
#endif //STORE_DEBUG_TEX

		// Create blended depth texture & coverage mask
		maskedResults[curr].LoadBlendedDepth(ComputeDepthBlend(objectDepths[curr].GetTexture(), sceneDepths[curr].GetTexture()));
		maskedResults[curr].SetPath(pRenderSettings->GetImagePath("body_mask", start + curr), false);
		maskedResults[curr].SetTexture(
			ComputeOcclusionMask(objectDepths[curr].GetTexture(), sceneDepths[curr].GetTexture(), maskedResults[curr].Occluded())
		);
#if STORE_DEBUG_TEX
		maskedResults[curr].StoreTexture();
#endif //STORE_DEBUG_TEX
	}

	// Return masks
	return maskedResults;
}

//---------------------------------------
// Render segments & create annotations
//---------------------------------------
void SceneManager::X_RenderSegments(
	std::vector<Camera>& cams,
	std::vector<Mask>& masks,
	int start
)
{
	// Initialize output vector
	std::vector<Texture> objectLabels;
	objectLabels.reserve(cams.size());

	// Create & process renderfile
	RENDERFILE_BEGIN;
	X_BuildObjectsLabel(RENDERFILE_WRITER, cams, objectLabels, start);
	RENDERFILE_PROCESS;

	// For every pose
	for (int curr = 0; curr < cams.size(); ++curr)
	{
		// Load & unpack label texture
		objectLabels[curr].LoadTexture(UnpackLabel);
		objectLabels[curr].ReplacePacked();
#if STORE_DEBUG_TEX
		objectLabels[curr].StoreTexture();
#endif //STORE_DEBUG_TEX

		// Create & store masked segmentation texture
		Texture segResult(false, false);
		segResult.SetPath(pRenderSettings->GetImagePath("segs", start + curr, true), false);
		segResult.SetTexture(ComputeSegmentMask(objectLabels[curr].GetTexture(), masks[curr].GetTexture()));
		segResult.StoreTexture();

		// Create & store annotations
		for (auto currMesh : vecpRenderMeshCurrObjs)
			pAnnotations->Write(currMesh, objectLabels[curr].GetTexture(),
				segResult.GetTexture(), camBlueprint, start + curr);
	}
}

//---------------------------------------
// Render synthetic objects & create blend
//---------------------------------------
void SceneManager::X_RenderPBRBlend(
	std::vector<Camera>& cams,
	std::vector<Mask>& masks,
	std::vector<SceneImage>& sceneRGBs,
	int start
)
{
	// Initialize output vector
	std::vector<Texture> objectPBRs;
	objectPBRs.reserve(cams.size());

	// Create & process renderfile
	RENDERFILE_BEGIN;
	X_BuildObjectsPBR(RENDERFILE_WRITER, cams, objectPBRs, start);
	RENDERFILE_PROCESS;

	// For every pose
	for (int curr = 0; curr < cams.size(); ++curr)
	{
		// Load PBR object texture
		objectPBRs[curr].LoadTexture();

		// Blend & store result
		Texture blendResult(false, false);
		blendResult.SetPath(pRenderSettings->GetImagePath("rgb", start + curr, true), false);
		blendResult.SetTexture(ComputeRGBBlend(objectPBRs[curr].GetTexture(), sceneRGBs[curr].GetSceneTexture(), masks[curr].GetTexture()));
		blendResult.StoreTexture();
	}
}

//---------------------------------------
// Determine & return images to process
//---------------------------------------
std::vector<SceneImage> SceneManager::X_GetImagesToProcess(
	ReferencePath path,
	float varThreshold
) const
{
	std::vector<SceneImage> clearImages;
	// If path exists & directory
	if (boost::filesystem::exists(path))
	{
		if (boost::filesystem::is_directory(path))
		{
			// For each file
			for (auto entry : boost::filesystem::directory_iterator(path))
			{
				// If file is rgb image
				if (boost::algorithm::contains(entry.path().filename().string(), "color"))
				{
					// Only use non-blurry images (= great variance)
					if (ComputeVariance(entry.path()) > varThreshold)
					{
						std::cout << "Using image " << entry.path().filename() << std::endl;
						// Store path to real RGB image
						SceneImage currImage;
						currImage.SetScenePath(entry.path());
						// Compute & store path to camera pose
						std::string pose(entry.path().string());
						boost::algorithm::replace_last(pose, "color.jpg", "pose.txt");
						currImage.SetPosePath(ModifiablePath(pose));
						// Place in output vector
						clearImages.emplace_back(std::move(currImage));
					}
				}
			}
		}
	}
	// Return images
	return clearImages;
}

//---------------------------------------
// Run simulation & render synthetic images
//---------------------------------------
int SceneManager::Run(int imageCount)
{
	// Get non blurry images
	std::vector<SceneImage> sceneImages = X_GetImagesToProcess(pRenderSettings->GetSceneRGBPath(), 400.0f);
	if (sceneImages.empty())
		return 0;

	// Create camera blueprint for scene
	camBlueprint.LoadIntrinsics(*pRenderSettings);

	// Control params
	int newImages = 0;
	int poseCount = sceneImages.size();
	int maxIters = pRenderSettings->GetIterationCount();
	int batchSize = pRenderSettings->GetRenderBatchSize();
	ModifiablePath scenePath = boost::filesystem::relative(pRenderSettings->GetSceneRGBPath());

	// For each scene iteration
	for (int iter = 0; iter < maxIters; ++iter)
	{
		// Create / open annotation file
		pAnnotations->Begin(*pRenderSettings);

		// Create physx representation of scan scene
		X_PxCreateScene();
		// Create physx objects
		X_PxCreateObjs();
		// Run the simulation
		X_PxRunSim(1.0f / 50.0f, pRenderSettings->GetStepsPerSimulation());
		// Save results
		X_PxSaveSimResults();

		// For every batch
		int batchMax = ceil(static_cast<float>(poseCount) / static_cast<float>(batchSize));
		for (int batch = 0; batch < batchMax; ++batch)
		{
			std::cout << "Scene " << scenePath << ": Iteration " << iter + 1 << "/" << maxIters
				<< ", Batch " << batch + 1 << "/" << batchMax << "("<< batchSize << " images)" << std::endl;

			// Create batch
			unsigned int start = batch * batchSize;
			unsigned int end = start + batchSize >= poseCount ? poseCount : start + batchSize;
			std::vector<SceneImage> currImages(
				sceneImages.begin() + start,
				sceneImages.begin() + end
			);

			// Load poses
			std::vector<Camera> currCams(currImages.size(), Camera(camBlueprint));
			for (int batchPose = 0; batchPose < currImages.size(); ++batchPose)
			{
				currCams[batchPose].LoadExtrinsics(currImages[batchPose].GetPosePath());
			}

			// Render depths & masks
			std::vector<Mask> masks = X_RenderDepthMasks(currCams, start);

			// Determine which images should be processed further
			int unoccludedCount = 0;
			std::vector<Mask> unoccludedMasks;
			std::vector<Camera> unoccludedCams;
			std::vector<SceneImage> unoccludedImages;
			for (int i = 0; i < masks.size(); ++i)
			{
				// Only process unoccluded images
				if (!masks[i].Occluded())
				{
					// Store the blended depth
					masks[i].StoreBlendedDepth(pRenderSettings->GetImagePath("depth", imageCount + newImages + unoccludedCount, true));
					// Move corresponding poses, masks & real images
					unoccludedCams.push_back(std::move(currCams[i]));
					unoccludedMasks.push_back(std::move(masks[i]));
					unoccludedImages.push_back(std::move(sceneImages[start + i]));
					++unoccludedCount;
				}
			}

			// If batch contains useful images
			if(unoccludedCount > 0)
			{
				// Render labels & create annotations
				X_RenderSegments(unoccludedCams, unoccludedMasks, newImages);

				// Render synthetic image & blend with real one
				X_RenderPBRBlend(unoccludedCams, unoccludedMasks, unoccludedImages, newImages);
			}

			// Exit if target now reached
			newImages += unoccludedCount;
			if (imageCount + newImages >= pRenderSettings->GetMaxImageCount())
			{
				X_CleanupScene();
				return newImages;
			}
		}

		// Done with iteration
		X_CleanupScene();
	}

	// Return how many images were rendered
	return newImages;
}

//---------------------------------------
// Create new scene manager
//---------------------------------------
SceneManager::SceneManager(
	Settings* settings,
	const std::vector<PxMeshConvex*>& vecPhysxObjs,
	const std::vector<RenderMesh*>& vecArnoldObjs
) :
	vecpPxMeshObjs(vecPhysxObjs),
	vecpRenderMeshObjs(vecArnoldObjs),
	pRenderSettings(settings),
	pAnnotations(NULL),
	pPxScene(NULL),
	pRenderMeshScene(NULL),
	pPxMeshScene(NULL)
{
	// Create annotations manager
	pAnnotations = new AnnotationsManager();

	// Load clipping planes
	camBlueprint.SetClipping(
		SafeGet<float>(pRenderSettings->GetJSONConfig(), "near_z"),
		SafeGet<float>(pRenderSettings->GetJSONConfig(), "far_z")
	);

	// Create lights
	for (int i = 0; i < 8; i++)
	{
		PointLightParams* params = new PointLightParams();
		Light* addLight = new Light(params, Eigen::Vector3f(1.0f, 1.0f, 1.0f), 75.0f, 1.0f);
		// Add 8 lights in a box pattern
		addLight->SetPosition(Eigen::Vector3f(
			i % 2 == 0 ? 5.0f : -5.0f,
			(i >> 1) % 2 == 0 ? 5.0f : -5.0f,
			(i >> 2) % 2 == 0 ? 5.0f : -5.0f)
		);
		vecpLights.push_back(addLight);
	}

	// Create OpenGL renderer
	ModifiablePath shadersPath(SafeGet<const char*>(pRenderSettings->GetJSONConfig(), "shaders_gl"));
	pRenderer = new Renderer::Render(
		boost::filesystem::absolute(shadersPath),
		pRenderSettings->GetRenderResolution().x(),
		pRenderSettings->GetRenderResolution().y(),
		camBlueprint.GetClipping().x(),
		camBlueprint.GetClipping().y()
	);

	// Create Blender render interface
	pBlender = new Blender::BlenderRenderer();
}

//---------------------------------------
// Cleanup scene
//---------------------------------------
SceneManager::~SceneManager()
{
	X_CleanupScene();
	delete pBlender;
	delete pRenderer;
	delete pAnnotations;
	for (auto currLight : vecpLights)
	{
		delete currLight;
	}
	vecpLights.clear();
}
