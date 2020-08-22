#include <SceneManager.h>

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
	float toMeters = SafeGet<float>(pRenderSettings->GetJSONConfig(), "scene_unit");
	// Create physx mesh of scan scene
	ModifiablePath meshPath(pRenderSettings->GetScenePath());
	meshPath.append("mesh.refined.obj");
	pPxMeshScene = new PxMeshTriangle(meshPath, 0);
	pPxMeshScene->CreateMesh();
	pPxMeshScene->SetObjId(0);
	pPxMeshScene->SetScale(PxVec3(toMeters));
	std::cout << std::endl;

	// For human readable depth: Maximal possible distance
	maxDist = pPxMeshScene->GetGlobalBounds().getDimensions().magnitude();

	bool cudaAvailable = PxManager::GetInstance().GetCudaManager() != nullptr;
	// Standart gravity & continuous collision detection & GPU rigidbodies
	PxSceneDesc sceneDesc(PxGetPhysics().getTolerancesScale());
	sceneDesc.flags |= PxSceneFlag::eENABLE_STABILIZATION | PxSceneFlag::eENABLE_CCD |
		(cudaAvailable ? PxSceneFlag::eENABLE_GPU_DYNAMICS | PxSceneFlag::eENABLE_PCM : PxSceneFlag::eENABLE_PCM);
	sceneDesc.broadPhaseType = PxManager::GetInstance().GetCudaManager() ? PxBroadPhaseType::eGPU : PxBroadPhaseType::eABP;
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(std::thread::hardware_concurrency());
	sceneDesc.cudaContextManager = PxManager::GetInstance().GetCudaManager();
	sceneDesc.filterShader = PxManager::CCDFilterShader;
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.gpuMaxNumPartitions = 8;
	// Objects should never spawn outside the scene bounds
	sceneDesc.sanityBounds = pPxMeshScene->GetGlobalBounds();

	// Create scene & add mesh
	pPxScene = PxGetPhysics().createScene(sceneDesc);
	pPxMeshScene->AddRigidActor(pPxScene);
}

//---------------------------------------
// Create physx object meshes
//---------------------------------------
void SceneManager::X_PxCreateObjs()
{
	// Setup & fetch params
	vecpPxMeshCurrObjs.reserve(pRenderSettings->GetObjectsPerSimulation());
	const PxVec3 sceneCenter = pPxMeshScene->GetGlobalBounds().getCenter();
	const PxVec3 sceneExtends = pPxMeshScene->GetGlobalBounds().getExtents();
	const PxVec3 velMax = PxVec3(
		fabs(pRenderSettings->GetMaxVelocity().x()),
		fabs(pRenderSettings->GetMaxVelocity().y()),
		fabs(pRenderSettings->GetMaxVelocity().z())
	);
	const PxVec3 trqMax = PxVec3(
		fabs(pRenderSettings->GetMaxTorque().x()),
		fabs(pRenderSettings->GetMaxTorque().y()),
		fabs(pRenderSettings->GetMaxTorque().z())
	);

	// Helper lambdas for random values
	auto uniformRandInt = [&](int lower, int upper) -> int
	{
		std::uniform_int_distribution<int> distr(lower, upper);
		return distr(randGen);
	};
	auto uniformRandVec = [&](PxVec3 lower, PxVec3 upper) -> PxVec3
	{
		std::uniform_real_distribution<float> distX(lower.x, upper.x);
		std::uniform_real_distribution<float> distY(lower.y, upper.y);
		std::uniform_real_distribution<float> distZ(lower.z, upper.z);
		return PxVec3(distX(randGen), distY(randGen), distZ(randGen));
	};
	auto applyForce = [&](float likelyhood) -> bool
	{
		std::bernoulli_distribution distr(likelyhood);
		return distr(randGen);
	};

	// For each object
	for (int i = 0; i < pRenderSettings->GetObjectsPerSimulation(); ++i)
	{
		// Fetch random object & create instance with new id
		int randObj = uniformRandInt(0, vecpPxMeshObjs.size() - 1);
		PxMeshConvex* currObj = new PxMeshConvex(*vecpPxMeshObjs[randObj]);
		currObj->SetObjId(i);
		currObj->CreateMesh();

		// Random position in scene
		PxVec3 randPos = sceneCenter + uniformRandVec(
			PxVec3(-0.75f, 0.25f, -0.75f),
			PxVec3(0.75f, 0.75f, 0.75f)).multiply(sceneExtends);

		// Set pose & actor
		PxTransform pose(randPos, PxQuat(PxIdentity));
		currObj->SetTransform(pose);
		currObj->AddRigidActor(pPxScene);

		// Possibly add random velocity & torque impulses
		if (applyForce(pRenderSettings->GetApplyProbability()))
		{
			currObj->AddVelocity(uniformRandVec(-velMax, velMax));
			currObj->AddTorque(uniformRandVec(-trqMax, trqMax));
		}

		// Place in vector
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
	delete pRenderMeshScene;
	if (pPxMeshScene)
	{
		delete pPxMeshScene;
		pPxMeshScene = NULL;
	}
	if (pPxScene)
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
	for (int i = 0; i < stepCount; ++i)
	{
		pPxScene->simulate(timestep);
		pPxScene->fetchResults(true);
		std::cout << "\r\33[2K" << "Simulating:\t" << (i + 1) << "/" << stepCount << std::flush;
	}
	// Formatting
	std::cout << std::endl;
}

//---------------------------------------
// Fetch and save simulation results
//---------------------------------------
void SceneManager::X_PxSaveSimResults()
{
	// For each physx object
	for (auto currPx : vecpPxMeshCurrObjs)
	{
		// Get position & transform to Blender system (90* around X)
		PxTransform pose = currPx->GetTransform();
		PxTransform adjustPos = PxTransform(PxQuat(PIOVER2, PxVec3(1.0f, 0.0f, 0.0f))).transform(pose);
		// Transform rotation to Blender system (-90* around local X)
		PxVec3 localX = adjustPos.q.rotate(PxVec3(1.0f, 0.0f, 0.0f));
		PxTransform adjustRot = PxTransform(PxQuat(-PIOVER2, localX)).transform(adjustPos);

		// Save & create mesh for rendering
		RenderMesh* currMesh = new RenderMesh(*vecpRenderMeshObjs[currPx->GetMeshId()]);
		currMesh->SetObjId(currPx->GetObjId());

		// Build transform from simulated pose
		Eigen::Affine3f currTrans;
		currTrans.fromPositionOrientationScale(
			Eigen::Vector3f(adjustPos.p.x, adjustPos.p.y, adjustPos.p.z),
			Eigen::Quaternionf(adjustRot.q.w, adjustRot.q.x, adjustRot.q.y, adjustRot.q.z),
			currMesh->GetScale()
		);

		// Store it
		currMesh->SetTransform(currTrans.matrix());
		vecpRenderMeshCurrObjs.push_back(std::move(currMesh));
	}
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
// Build scene depth renderfile
//---------------------------------------
void SceneManager::X_BuildSceneDepth(
	JSONWriterRef writer,
	std::vector<Camera>& cams,
	std::vector<Texture>& results,
	int start
)
{
	std::vector<Camera> toRender;
	toRender.reserve(cams.size());
	// For every pose
	for (int curr = 0; curr < cams.size(); ++curr)
	{
		// Determine depth output file
		std::string pose(cams[curr].GetSourceFile().string());
		boost::algorithm::replace_last(pose, "pose.txt", "depth.tiff");
		// Create depth output texture
		Texture currDepth(true, true);
		// Either mark for rendering or already load texture
		if (boost::filesystem::exists(pose))
		{
			currDepth.SetPath(pose, false);
			currDepth.LoadTexture();
		}
		else
		{
			currDepth.SetPath(pose, true, "exr");
			// Create & setup camera
			Camera currCam = Camera(cams[curr]);
			currCam.SetupRendering(
				currDepth.GetPath(),
				pRenderSettings->GetRenderResolution(),
				true,
				1,
				0,
				"depth"
			);
			// Mark for rendering
			toRender.push_back(std::move(currCam));
		}
		// Place in output vector
		results.emplace_back(std::move(currDepth));
	}

	// Set shader
	DepthShader* shader = new DepthShader(FLT_EPSILON, maxDist);
	pRenderMeshScene->SetShader(shader);

	// If any cameras are marked for rendering
	if (toRender.size() > 0)
	{
		// Add configured scene to renderfile
		X_ConvertToRenderfile(writer, std::vector<RenderMesh*>{pRenderMeshScene}, toRender);
	}
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
		DepthShader* currShader = new DepthShader(FLT_EPSILON, maxDist);
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
	Eigen::Vector2i renderRes = camBlueprint.GetIntrinsics().GetResolution();
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
	X_BuildSceneDepth(RENDERFILE_WRITER, cams, sceneDepths, start);
	RENDERFILE_PROCESS;

	// For every pose
	for (int curr = 0; curr < cams.size(); ++curr)
	{
		// Load & unpack object depth
		objectDepths[curr].LoadTexture(UnpackDepth);
		objectDepths[curr].ReplacePacked();
		// Load & unpack scene depth if not yet loaded
		if (sceneDepths[curr].GetTexture().empty())
		{
			sceneDepths[curr].LoadTexture(UnpackDepth);
			sceneDepths[curr].ReplacePacked();
			sceneDepths[curr].StoreTexture();
		}
#if STORE_DEBUG_TEX
		// Store human readable
		objectDepths[curr].StoreDepth01(FLT_EPSILON, maxDist);
		sceneDepths[curr].StoreDepth01(FLT_EPSILON, maxDist);
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
						std::cout << "Using image\t" << entry.path().filename() << std::endl;
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
int SceneManager::ProcessNext(int imageCount)
{
	// Get non blurry images
	std::vector<SceneImage> sceneImages = X_GetImagesToProcess(
		pRenderSettings->GetSceneRGBPath(),
		SafeGet<float>(pRenderSettings->GetJSONConfig(), "blurry_threshold")
	);

	// Make sure there are images
	if (sceneImages.empty())
		return 0;

	// Create camera blueprint for scene
	camBlueprint.LoadIntrinsics(*pRenderSettings);

	// Control params
	int newImages = 0;
	int maxIters = pRenderSettings->GetIterationCount();
	size_t poseCount = sceneImages.size();
	size_t batchSize = pRenderSettings->GetRenderBatchSize();
	size_t batchMax = ceil(static_cast<float>(poseCount) / static_cast<float>(batchSize));
	ModifiablePath scenePath = boost::filesystem::relative(pRenderSettings->GetSceneRGBPath());

	// For each scene iteration
	for (int iter = 0; iter < maxIters && imageCount + newImages < pRenderSettings->GetMaxImageCount(); ++iter)
	{
		pBlender->LogPerformance("Iteration " + std::to_string(iter + 1));

		// Create / open annotation file
		pAnnotations->Begin(*pRenderSettings);

		// Create scene render mesh
		float toMeters = SafeGet<float>(pRenderSettings->GetJSONConfig(), "scene_unit");
		ModifiablePath meshPath(pRenderSettings->GetScenePath());
		meshPath.append("mesh.refined.obj");
		pRenderMeshScene = new RenderMesh(meshPath, 0);
		pRenderMeshScene->SetScale(Eigen::Vector3f().setConstant(toMeters));

		// Create physx representation of scan scene
		X_PxCreateScene();
		// Create physx objects
		X_PxCreateObjs();
		// Run the simulation
		X_PxRunSim(1.0f / 50.0f, pRenderSettings->GetStepsPerSimulation());
		// Save results
		X_PxSaveSimResults();

		// For every batch
		for (size_t batch = 0; batch < batchMax && imageCount + newImages < pRenderSettings->GetMaxImageCount(); ++batch)
		{
			pBlender->LogPerformance("Batch " + std::to_string(batch + 1));
			std::cout << "Scene\t" << scenePath << ":\tIteration\t" << iter + 1 << "/" << maxIters
				<< ":\tBatch\t" << batch + 1 << "/" << batchMax << "\t(" << batchSize << " each)" << std::endl;

			// Create batch
			size_t start = batch * batchSize;
			size_t end = start + batchSize >= poseCount ? poseCount : start + batchSize;

			// Copy corresponding images
			std::vector<SceneImage> currImages;
			for (size_t i = start; i < end; ++i)
			{
				currImages.push_back(sceneImages[i]);
			}

			// Load poses
			std::vector<Camera> currCams(currImages.size(), Camera(camBlueprint));
			for (size_t batchPose = 0; batchPose < currImages.size(); ++batchPose)
			{
				currCams[batchPose].LoadExtrinsics(currImages[batchPose].GetPosePath());
			}

			// Render depths & masks
			pBlender->LogPerformance("Depth & Masks");
			std::vector<Mask> masks = X_RenderDepthMasks(currCams, start);
			pBlender->LogPerformance("Depth & Masks");

			// Determine which images should be processed further
			int unoccludedCount = 0;
			std::vector<Mask> unoccludedMasks;
			std::vector<Camera> unoccludedCams;
			std::vector<SceneImage> unoccludedImages;
			for (size_t i = 0; i < masks.size(); ++i)
			{
				// Only process unoccluded images
				if (!masks[i].Occluded())
				{
					// Store the blended depth
					ModifiablePath depthPath = pRenderSettings->GetImagePath("depth", imageCount + newImages + unoccludedCount, true);
#if STORE_DEBUG_TEX
					masks[i].StoreBlendedDepth01(depthPath, FLT_EPSILON, maxDist);
#else
					masks[i].StoreBlendedDepth(depthPath);
#endif
					// Move corresponding poses, masks & real images
					unoccludedCams.push_back(std::move(currCams[i]));
					unoccludedMasks.push_back(std::move(masks[i]));
					unoccludedImages.push_back(std::move(currImages[start + i]));
					++unoccludedCount;
			}
		}

			// If batch contains useful images
			if (unoccludedCount > 0)
			{
				// Render labels & create annotations
				pBlender->LogPerformance("Labels & Annotating");
				X_RenderSegments(unoccludedCams, unoccludedMasks, newImages);
				pBlender->LogPerformance("Labels & Annotating");

				// Render synthetic image & blend with real one
				pBlender->LogPerformance("PBR Render & Blend");
				X_RenderPBRBlend(unoccludedCams, unoccludedMasks, unoccludedImages, newImages);
				pBlender->LogPerformance("PBR Render & Blend");
			}

			// Update total count & output duration
			pBlender->LogPerformance("Batch " + std::to_string(batch + 1));
			newImages += unoccludedCount;
	}

		// Done with iteration
		pBlender->LogPerformance("Iteration " + std::to_string(iter + 1));
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
	vecpPxMeshCurrObjs(),
	vecpRenderMeshCurrObjs(),
	pRenderMeshScene(NULL),
	pPxMeshScene(NULL),
	pAnnotations(NULL),
	camBlueprint(),
	vecpLights(),
	maxDist(10.0f),
	pPxScene(NULL),
	pBlender(NULL)
{
	// Init random generator
	std::random_device randDev;
	randGen = std::default_random_engine(randDev());

	// Create annotations manager
	pAnnotations = new AnnotationsManager();

	// Create lights
	for (int i = 0; i < 8; i++)
	{
		PointLightParams* params = new PointLightParams();
		Light* addLight = new Light(params, Eigen::Vector3f().setOnes(), 75.0f, 1.0f);
		// Add 8 lights in a box pattern
		addLight->SetPosition(Eigen::Vector3f(
			i % 2 == 0 ? 5.0f : -5.0f,
			(i >> 1) % 2 == 0 ? 5.0f : -5.0f,
			(i >> 2) % 2 == 0 ? 5.0f : -5.0f)
		);
		vecpLights.push_back(addLight);
	}

	// Create Blender render interface
	pBlender = new Blender::BlenderRenderer();
}

//---------------------------------------
// Cleanup scene
//---------------------------------------
SceneManager::~SceneManager()
{
	// Meshes & simulation
	X_CleanupScene();

	// Renderer etc.
	delete pBlender;
	delete pAnnotations;

	// Lights
	for (auto currLight : vecpLights)
	{
		delete currLight;
	}
	vecpLights.clear();
}
