#include <SceneManager.h>

#if _DEBUG || DEBUG
#define STORE_DEBUG_TEX 1
#endif //_DEBUG || DEBUG

#define USE_AO 1
#define USE_ESTIMATOR 1

#define PTR_RELEASE(x) if(x != NULL) { delete x; x = NULL; }

using namespace physx;

//---------------------------------------
// Create scan scene render mesh
//---------------------------------------
RenderMesh SceneManager::X_CreateSceneMesh() const
{
	float toMeters = SafeGet<float>(renderSettings.GetJSONConfig(), "scene_unit");
	std::string sceneMesh(SafeGet<const char*>(renderSettings.GetJSONConfig(), "scene_mesh"));
	// Create mesh file path
	ModifiablePath meshPath(renderSettings.GetScenePath());
	meshPath.append(sceneMesh);
	// Create & return mesh
	RenderMesh meshScene(meshPath, "scene", "pbr", 0, true);
	meshScene.SetScale(Eigen::Vector3f().setConstant(toMeters));
	return meshScene;
}

//---------------------------------------
// Create physx scan scene mesh
//---------------------------------------
PxMeshTriangle SceneManager::X_PxCreateSceneMesh() const
{
	float toMeters = SafeGet<float>(renderSettings.GetJSONConfig(), "scene_unit");
	std::string sceneMesh(SafeGet<const char*>(renderSettings.GetJSONConfig(), "scene_mesh"));
	// Create mesh file path
	ModifiablePath meshPath(renderSettings.GetScenePath());
	meshPath.append(sceneMesh);
	// Create physx mesh of scan scene
	PxMeshTriangle pxMeshScene(meshPath, "scene", 0);
	pxMeshScene.CreateMesh();
	pxMeshScene.SetObjId(0);
	pxMeshScene.SetScale(PxVec3(toMeters));
	std::cout << std::endl;
	// Return new mesh
	return pxMeshScene;
}

//---------------------------------------
// Create simulation scene
//---------------------------------------
physx::PxScene* SceneManager::X_PxCreateSimulation(
	PxMeshTriangle& sceneMesh,
	float& maxDist
) const
{
	// For human readable depth: Maximal possible distance
	maxDist = sceneMesh.GetGlobalBounds().getDimensions().magnitude();

	// Standart gravity & continuous collision detection & GPU rigidbodies
	PxSceneDesc sceneDesc(PxGetPhysics().getTolerancesScale());
	sceneDesc.broadPhaseType = PxManager::GetInstance().GetCudaManager() ? PxBroadPhaseType::eGPU : PxBroadPhaseType::eABP;
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(std::thread::hardware_concurrency());
	sceneDesc.cudaContextManager = PxManager::GetInstance().GetCudaManager();
	sceneDesc.filterShader = PxManager::CCDFilterShader;
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.gpuMaxNumPartitions = 8;

	// Enable GPU rigidbodies if available
	if (PxManager::GetInstance().GetCudaManager())
	{
		sceneDesc.flags = PxSceneFlag::eENABLE_GPU_DYNAMICS | PxSceneFlag::eENABLE_STABILIZATION | PxSceneFlag::eENABLE_CCD | PxSceneFlag::eENABLE_PCM;
	}
	else
	{
		sceneDesc.flags = PxSceneFlag::eENABLE_STABILIZATION | PxSceneFlag::eENABLE_CCD | PxSceneFlag::eENABLE_PCM;
	}

	// Objects should never spawn outside the scene bounds
	sceneDesc.sanityBounds = sceneMesh.GetGlobalBounds();

	// Create scene & add mesh
	PxScene* pCurrScene = PxGetPhysics().createScene(sceneDesc);
	sceneMesh.AddRigidActor(pCurrScene, sceneMesh.GetTransform());

	// Return simulation
	return pCurrScene;
}

//---------------------------------------
// Create physx object meshes
//---------------------------------------
std::vector<PxMeshConvex> SceneManager::X_PxCreateObjs(
	std::default_random_engine& generator,
	PxMeshTriangle& sceneMesh,
	physx::PxScene* simulation
) const
{
	// Helper lambdas
	auto eigenToPx = [](Eigen::Vector3f in) -> PxVec3
	{
		return PxVec3(in.x(), in.y(), in.z());
	};
	auto uniformRandInt = [&](int lower, int upper) -> int
	{
		std::uniform_int_distribution<int> distr(lower, upper);
		return distr(generator);
	};
	auto uniformRandVec = [&](PxVec3 lower, PxVec3 upper) -> PxVec3
	{
		std::uniform_real_distribution<float> distX(lower.x, upper.x);
		std::uniform_real_distribution<float> distY(lower.y, upper.y);
		std::uniform_real_distribution<float> distZ(lower.z, upper.z);
		return PxVec3(distX(generator), distY(generator), distZ(generator));
	};
	auto applyForce = [&](float likelyhood) -> bool
	{
		std::bernoulli_distribution distr(likelyhood);
		return distr(generator);
	};

	// Simulation objects
	std::vector<PxMeshConvex> newBodies;
	newBodies.reserve(renderSettings.GetSimulationSettings().SimulationObjects);

	// Setup & fetch params
	const PxVec3 sceneCenter = sceneMesh.GetGlobalBounds().getCenter();
	const PxVec3 sceneExtends = sceneMesh.GetGlobalBounds().getExtents();
	const PxVec3 spawnMin = eigenToPx(renderSettings.GetSpawnSettings().SpawnMin);
	const PxVec3 spawnMax = eigenToPx(renderSettings.GetSpawnSettings().SpawnMax);
	const PxVec3 velMax = eigenToPx(renderSettings.GetSpawnSettings().VelocityMax);
	const PxVec3 trqMax = eigenToPx(renderSettings.GetSpawnSettings().TorqueMax);

	// For each object
	for (int i = 0; i < renderSettings.GetSimulationSettings().SimulationObjects; ++i)
	{
		// Fetch random object & create instance with new id
		int randObj = uniformRandInt(0, vecpPxMeshObjs.size() - 1);
		PxMeshConvex currObj(*vecpPxMeshObjs[randObj]);
		currObj.SetObjId(i + 1);
		currObj.CreateMesh();

		// Random position in scene
		PxVec3 randPos = sceneCenter +
			uniformRandVec(spawnMin, spawnMax).multiply(sceneExtends);

		// Set pose & actor
		PxTransform pose(randPos, PxQuat(PxIdentity));
		currObj.AddRigidActor(simulation, pose);

		// Possibly add random velocity & torque impulses
		if (applyForce(renderSettings.GetSpawnSettings().ApplyProbability))
		{
			currObj.AddVelocity(uniformRandVec(-velMax, velMax));
			currObj.AddTorque(uniformRandVec(-trqMax, trqMax));
		}

		// Place in vector
		newBodies.push_back(std::move(currObj));
	}

	// Return simulation objects
	return newBodies;
}

//---------------------------------------
// Cleanup scene & iteration
//---------------------------------------
void SceneManager::X_CleanupScene(
	physx::PxScene* simulation,
	AnnotationsManager* annotations,
	Blender::BlenderRenderer* renderer,
	int threadID
) const
{
	// Cleanup annotations
	PTR_RELEASE(annotations);

	// Cleanup physx scene
	if (simulation)
	{
		simulation->flushSimulation();
		auto dispatcher = (PxDefaultCpuDispatcher*)simulation->getCpuDispatcher();
		PX_RELEASE(dispatcher);
		PX_RELEASE(simulation);
	}

	// Reload render process thread
	renderer->UnloadProcess(threadID);
}

//---------------------------------------
// Run physx simulation
//---------------------------------------
void SceneManager::X_PxRunSim(
	physx::PxScene* simulation,
	float timestep,
	int stepCount
) const
{
	// Simulate in steps
	for (int i = 0; i < stepCount; ++i)
	{
		simulation->simulate(timestep);
		simulation->fetchResults(true);
	}

	// Some status logging
	std::cout << "\33[2K\r" << "Done simulating (" << timestep << "s steps, "
		<< timestep * stepCount << "s total)" << std::endl;
}

//---------------------------------------
// Fetch and save simulation results
//---------------------------------------
std::vector<RenderMesh> SceneManager::X_PxSaveSimResults(
	std::vector<PxMeshConvex>& simulationObjs
) const
{
	std::vector<RenderMesh> newMeshes;
	newMeshes.reserve(simulationObjs.size());

	// For each physx object
	for (auto& currPx : simulationObjs)
	{
		// Get position & transform to Blender system (90* around X)
		PxTransform pose = currPx.GetTransform();
		PxTransform adjustPos = PxTransform(PxQuat(PxHalfPi, PxVec3(1.0f, 0.0f, 0.0f))).transform(pose);
		// Transform rotation to Blender system (-90* around local X)
		PxVec3 localX = adjustPos.q.rotate(PxVec3(1.0f, 0.0f, 0.0f));
		PxTransform adjustRot = PxTransform(PxQuat(-PxHalfPi, localX)).transform(adjustPos);

		// Save & create mesh for rendering
		RenderMesh currMesh(*vecpRenderMeshObjs[currPx.GetMeshId()]);
		currMesh.SetObjId(currPx.GetObjId());

		// Build transform from simulated pose
		Eigen::Affine3f currTrans;
		currTrans.fromPositionOrientationScale(
			Eigen::Vector3f(adjustPos.p.x, adjustPos.p.y, adjustPos.p.z),
			Eigen::Quaternionf(adjustRot.q.w, adjustRot.q.x, adjustRot.q.y, adjustRot.q.z),
			currMesh.GetScale()
		);

		// Store it
		currMesh.SetTransform(currTrans.matrix());
		newMeshes.push_back(std::move(currMesh));
	}

	// Return converted meshes
	return newMeshes;
}

//---------------------------------------
// Adds provided scene to renderfile
//---------------------------------------
void SceneManager::X_ConvertToRenderfile(
	JSONWriterRef writer,
	std::vector<RenderMesh>& meshes,
	std::vector<Camera>& cams,
	std::vector<Light>& lights
) const
{
	writer.StartObject();

	// Add settings
	writer.Key("settings");
	renderSettings.AddToJSON(writer);

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
	for (auto& currMesh : meshes)
	{
		currMesh.AddToJSON(writer);
	}
	writer.EndArray();

	// Add lights
	writer.Key("lights");
	writer.StartArray();
	for (auto& currLight : lights)
	{
		currLight.AddToJSON(writer);
	}
	writer.EndArray();

	writer.EndObject();
}

//---------------------------------------
// Build scene depth renderfile
//---------------------------------------
void SceneManager::X_BuildSceneDepth(
	JSONWriterRef writer,
	RenderMesh& sceneMesh,
	std::vector<RenderMesh>& meshes,
	std::vector<Camera>& cams,
	std::vector<Light>& lights,
	std::vector<Texture>& results,
	float maxDist
) const
{
	std::vector<Camera> toRender;
	toRender.reserve(cams.size());

	// Determine render resolution
	Eigen::Vector2i renderRes = camBlueprint.GetIntrinsics().GetResolution();
	renderRes *= renderSettings.GetEngineSettings().RenderScale;

	// For every pose
	for (int curr = 0; curr < cams.size(); ++curr)
	{
		// Determine depth output file
		std::string depthPath(cams[curr].GetSourceFile().string());
		boost::algorithm::replace_last(depthPath, "pose.txt", "depth.tiff");
		// Create depth output texture
		Texture currDepth(true, true);
		// Mark for rendering or load already
		if (boost::filesystem::exists(depthPath))
		{
			currDepth.SetPath(depthPath, false);
			currDepth.LoadTexture();
		}
		else
		{
			currDepth.SetPath(depthPath, true, "exr");
			// Create & setup camera
			Camera currCam = Camera(cams[curr]);
			currCam.SetupRendering(
				currDepth.GetPath(),
				renderRes,
				true,
				1,
				0,
				"depth",
				false
			);
			// Mark for rendering
			toRender.push_back(std::move(currCam));
		}
		// Place in output vector
		results.emplace_back(std::move(currDepth));
	}

	// Set shader & temporarily mark direct mesh
	DepthShader* shader = new DepthShader(FLT_EPSILON, maxDist);
	sceneMesh.SetShader(shader);
	sceneMesh.SetIndirect(false);

	// If any cameras are marked for rendering
	if (toRender.size() > 0)
	{
		// Add configured scene to renderfile
		std::vector<RenderMesh> scene = { sceneMesh };
		X_ConvertToRenderfile(writer, scene, toRender, lights);
	}

	// Mark indirect again
	sceneMesh.SetIndirect(true);
}

//---------------------------------------
// Build objects depth renderfile
//---------------------------------------
void SceneManager::X_BuildObjectsDepth(
	JSONWriterRef writer,
	RenderMesh& sceneMesh,
	std::vector<RenderMesh>& meshes,
	std::vector<Camera>& cams,
	std::vector<Light>& lights,
	std::vector<Texture>& results,
	float maxDist
) const
{
	// Determine render resolution
	Eigen::Vector2i renderRes = camBlueprint.GetIntrinsics().GetResolution();
	renderRes *= renderSettings.GetEngineSettings().RenderScale;

	// For every pose
	for (int curr = 0; curr < cams.size(); ++curr)
	{
		// Create depth output texture
		Texture currDepth(true, true);
		currDepth.SetPath(renderSettings.GetImagePath("body_depth", cams[curr].GetImageNum()), true, "exr");
		// Setup rendering params
		cams[curr].SetupRendering(
			currDepth.GetPath(),
			renderRes,
			true,
			1,
			0,
			"depth",
			false
		);
		// Place in output vector
		results.emplace_back(std::move(currDepth));
	}

	// Set shaders
	for (auto& currMesh : meshes)
	{
		DepthShader* currShader = new DepthShader(FLT_EPSILON, maxDist);
		currMesh.SetShader(currShader);
	}

	// Add configured scene to renderfile
	X_ConvertToRenderfile(writer, meshes, cams, lights);
}

//---------------------------------------
// Build object labels renderfile
//---------------------------------------
void SceneManager::X_BuildObjectsLabel(
	JSONWriterRef writer,
	RenderMesh& sceneMesh,
	std::vector<RenderMesh>& meshes,
	std::vector<Camera>& cams,
	std::vector<Light>& lights,
	std::vector<Texture>& results
) const
{
	// Determine render resolution
	Eigen::Vector2i renderRes = camBlueprint.GetIntrinsics().GetResolution();
	renderRes *= renderSettings.GetEngineSettings().RenderScale;

	// For every pose
	for (int curr = 0; curr < cams.size(); ++curr)
	{
		// Create label output texture
		Texture currLabel(true, false);
		currLabel.SetPath(renderSettings.GetImagePath("body_label", cams[curr].GetImageNum()), true, "exr");
		// Setup rendering params
		cams[curr].SetupRendering(
			currLabel.GetPath(),
			renderRes,
			true,
			1,
			0,
			"",
			false
		);
		// Place in output vector
		results.emplace_back(std::move(currLabel));
	}

	// Set shaders
	for (auto& currMesh : meshes)
	{
		// Store encoded ID as BGR
		LabelShader* currShader = new LabelShader(EncodeInt(currMesh.GetObjId()));
		currMesh.SetShader(currShader);
	}

	// Add configured scene to renderfile
	X_ConvertToRenderfile(writer, meshes, cams, lights);
}

//---------------------------------------
// Build objects PBR renderfile
//---------------------------------------
void SceneManager::X_BuildObjectsPBR(
	JSONWriterRef writer,
	RenderMesh& sceneMesh,
	std::vector<RenderMesh>& meshes,
	std::vector<Camera>& cams,
	std::vector<Light>& lights,
	std::vector<Texture>& results
) const
{
	// Determine render resolution
	Eigen::Vector2i renderRes = camBlueprint.GetIntrinsics().GetResolution();
	renderRes *= renderSettings.GetEngineSettings().RenderScale;

	// For every pose
	for (int curr = 0; curr < cams.size(); ++curr)
	{
		// Create PBR output texture
		Texture currPBR(false, false);
		currPBR.SetPath(renderSettings.GetImagePath("body_rgb", cams[curr].GetImageNum()), false);
		// Setup rendering params
		cams[curr].SetupRendering(
			currPBR.GetPath(),
			renderRes,
			false,
			4,
			-1,
			"",
#if USE_ESTIMATOR
			true
#else
			false
#endif
		);
		// Place in output vector
		results.emplace_back(std::move(currPBR));
	}

	// Set shaders
	for (auto& currMesh : meshes)
	{
		// Test metal shader?
		if(currMesh.GetShaderType() == "metal")
		{
			MetalShader* currShader = new MetalShader();
			currMesh.SetShader(currShader);
		}
		// Test glass shader?
		else if (currMesh.GetShaderType() == "glass")
		{
			GlassShader* currShader = new GlassShader();
			currMesh.SetShader(currShader);
		}
		// Otherwise PBR
		else
		{
			// Create diffuse texture
			Texture currDiffuse;
			currDiffuse.SetPath(currMesh.GetTexturePath(), false);
			// Set PBR shader
			PBRShader* currShader = new PBRShader(currDiffuse);
			currMesh.SetShader(currShader);
		}
	}

	// Setup scene for indirect light & shadows
	Texture diffuseScene;
	diffuseScene.SetPath(renderSettings.GetScenePath() / "mesh.refined_0.png", false);
	PBRShader* scenePBR = new PBRShader(diffuseScene);
	sceneMesh.SetShader(scenePBR);

	// Add configured scene to renderfile
	meshes.push_back(sceneMesh);
	X_ConvertToRenderfile(writer, meshes, cams, lights);
	meshes.pop_back();
}

//---------------------------------------
// Build objects ambient occlusion renderfile
//---------------------------------------
void SceneManager::X_BuildObjectsAO(
	JSONWriterRef writer,
	RenderMesh& sceneMesh,
	std::vector<RenderMesh>& meshes,
	std::vector<Camera>& cams,
	std::vector<Light>& lights,
	std::vector<Texture>& results
) const
{
	// Determine render resolution
	Eigen::Vector2i renderRes = camBlueprint.GetIntrinsics().GetResolution();
	renderRes *= renderSettings.GetEngineSettings().RenderScale;

	// For every pose
	for (int curr = 0; curr < cams.size(); ++curr)
	{
		// Create ambient occlusion output texture
		Texture currAO(false, false);
		currAO.SetPath(renderSettings.GetImagePath("body_ao", cams[curr].GetImageNum()), false);
		// Setup rendering params
		cams[curr].SetupRendering(
			currAO.GetPath(),
			renderRes,
			false,
			2,
			-1,
			"ambient_occlusion",
			false
		);
		// Place in output vector
		results.emplace_back(std::move(currAO));
	}

	// Add configured scene to renderfile
	meshes.push_back(sceneMesh);
	X_ConvertToRenderfile(writer, meshes, cams, lights);
	meshes.pop_back();
}

//---------------------------------------
// Render coverage masks & depths
//---------------------------------------
std::vector<Mask> SceneManager::X_RenderDepthMasks(
	Blender::BlenderRenderer* renderer,
	int threadID,
	RenderMesh& sceneMesh,
	std::vector<RenderMesh>& meshes,
	std::vector<Camera>& cams,
	std::vector<Light>& lights,
	boost::mutex* syncPoint,
	float maxDist
) const
{
	// Initialize output vectors
	std::vector<Mask> maskedResults(cams.size(), Mask());
	std::vector<Texture> objectDepths, sceneDepths;
	objectDepths.reserve(cams.size());
	sceneDepths.reserve(cams.size());

	// Only one thread at a time renders scene depth
	syncPoint->lock();

	RENDERFILE_DEPTH(renderer, threadID, X_BuildSceneDepth, sceneMesh, meshes, cams, lights, sceneDepths, maxDist);

	// For every pose
	for (int curr = 0; curr < cams.size(); ++curr)
	{
		// Load scene depth if not yet loaded
		if (sceneDepths[curr].GetTexture().empty())
		{
			// Load & unpack & replace scene depth
			sceneDepths[curr].LoadTexture(UnpackDepth);
			sceneDepths[curr].ReplacePacked();
			sceneDepths[curr].StoreTexture();
#if STORE_DEBUG_TEX
			// Store human readable
			sceneDepths[curr].StoreDepth01(FLT_EPSILON, maxDist);
#endif //STORE_DEBUG_TEX
		}
	}

	// Now other threads may load
	syncPoint->unlock();

	// Create & process renderfile
	RENDERFILE_DEPTH(renderer, threadID, X_BuildObjectsDepth, sceneMesh, meshes, cams, lights, objectDepths, maxDist);

	// For every pose
	for (int curr = 0; curr < cams.size(); ++curr)
	{
		// Load & unpack object depth
		objectDepths[curr].LoadTexture(UnpackDepth);
		objectDepths[curr].ReplacePacked();
		// Load scene depth if not yet loaded
		sceneDepths[curr].LoadTexture();
#if STORE_DEBUG_TEX
		// Store human readable
		objectDepths[curr].StoreDepth01(FLT_EPSILON, maxDist);
#endif //STORE_DEBUG_TEX

		// Sanity check
		if (!sceneDepths[curr].TextureExists() || !objectDepths[curr].TextureExists())
			continue;

		// Create blended depth texture & coverage mask
		maskedResults[curr].LoadBlendedDepth(ComputeDepthBlend(objectDepths[curr].GetTexture(), sceneDepths[curr].GetTexture()));
		maskedResults[curr].SetPath(renderSettings.GetImagePath("body_mask", cams[curr].GetImageNum()), false);
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
	Blender::BlenderRenderer* renderer,
	int threadID,
	AnnotationsManager* annotations,
	RenderMesh& sceneMesh,
	std::vector<RenderMesh>& meshes,
	std::vector<Camera>& cams,
	std::vector<Light>& lights,
	std::vector<Mask>& masks
) const
{
	// Initialize output vector
	std::vector<Texture> objectLabels;
	objectLabels.reserve(cams.size());

	// Create & process renderfile
	RENDERFILE_SINGLE(renderer, threadID, X_BuildObjectsLabel, sceneMesh, meshes, cams, lights, objectLabels);

	// For every pose
	for (int curr = 0; curr < cams.size(); ++curr)
	{
		// Load & unpack label texture
		objectLabels[curr].LoadTexture(UnpackLabel);
		objectLabels[curr].ReplacePacked();
#if STORE_DEBUG_TEX
		objectLabels[curr].StoreTexture();
#endif //STORE_DEBUG_TEX

		// Sanity check
		if (!objectLabels[curr].TextureExists() || !masks[curr].TextureExists())
			continue;

		// Create & store masked segmentation texture
		Texture segResult(false, true);
		segResult.SetPath(renderSettings.GetImagePath("segs", cams[curr].GetImageNum(), true), false);
		segResult.SetTexture(ComputeSegmentMask(objectLabels[curr].GetTexture(), masks[curr].GetTexture()));
		segResult.StoreTexture();

		// Create annotation file
		annotations->Begin(cams[curr].GetImageNum());
		// Add all visible objects
		for (auto& currMesh : meshes)
		{
			annotations->Write(
				currMesh,
				objectLabels[curr].GetTexture(),
				segResult.GetTexture(),
				cams[curr]
			);
		}
		// Store & close
		annotations->End();
	}
}

//---------------------------------------
// Render synthetic objects & create blend
//---------------------------------------
void SceneManager::X_RenderPBRBlend(
	Blender::BlenderRenderer* renderer,
	int threadID,
	RenderMesh& sceneMesh,
	std::vector<RenderMesh>& meshes,
	std::vector<Camera>& cams,
	std::vector<Light>& lights,
	std::vector<Mask>& masks,
	std::vector<SceneImage>& sceneRGBs
) const
{
	// Initialize output vector
	std::vector<Texture> objectPBRs, objectsAO;
	objectPBRs.reserve(cams.size());
	objectsAO.reserve(cams.size());

	// Create & process ambient occlusion renderfile
#if USE_AO
	RENDERFILE_SINGLE(renderer, threadID, X_BuildObjectsAO, sceneMesh, meshes, cams, lights, objectsAO);
#else
	objectsAO.assign(cams.size(), Texture(false, false));
#endif

	// Create & process PBR renderfile
	RENDERFILE_SINGLE(renderer, threadID, X_BuildObjectsPBR, sceneMesh, meshes, cams, lights, objectPBRs);

	// For every pose
	for (int curr = 0; curr < cams.size(); ++curr)
	{
		// Load PBR & AO object texture
		objectPBRs[curr].LoadTexture();
#if USE_AO
		objectsAO[curr].LoadTexture(UnpackAO);
#else
		objectsAO[curr].SetTexture(cv::Mat::ones(
			objectPBRs[curr].GetTexture().rows,
			objectPBRs[curr].GetTexture().cols,
			CV_32FC1
		));
#endif

		// Sanity check
		if (!objectPBRs[curr].TextureExists() || !objectsAO[curr].TextureExists())
			continue;

		// Potentially resize original scene image
		sceneRGBs[curr].ResizeSceneTexture(objectPBRs[curr].GetTexture());

		// Blend & store result
		Texture blendResult(false, false);
		blendResult.SetPath(renderSettings.GetImagePath("rgb", cams[curr].GetImageNum(), true), false);
		blendResult.SetTexture(ComputeRGBBlend(
			objectPBRs[curr].GetTexture(),
			objectsAO[curr].GetTexture(),
			sceneRGBs[curr].GetSceneTexture(),
			masks[curr].GetTexture())
		);
		blendResult.StoreTexture();
	}
}

//---------------------------------------
// Places lights according to scene dims
//---------------------------------------
std::vector<Light> SceneManager::X_PlaceLights(
	Eigen::Vector3f min,
	Eigen::Vector3f max
) const
{
	std::vector<Light> newLights;

#if USE_ESTIMATOR
	// Read estimated lights
	rapidjson::Document lights;
	CanReadJSONFile(renderSettings.GetScenePath() / "lights.json", lights);

	// For each light json object
	for (auto& currLight : lights.GetArray())
	{
		LightParamsBase* params = NULL;
		const rapidjson::Value* typeVal;
		// If has type
		if (SafeHasMember(currLight, "type", typeVal))
		{
			// Parse as string
			std::string typeStr(SafeGetValue<const char*>(*typeVal));
			// If it isn't a sun light the default is point light
			if (typeStr == "SUN")
			{
				params = new SunLightParams();
			}
			else
			{
				params = new PointLightParams();
			}
		}
		else
		{
			// No type given: Create a point light
			params = new PointLightParams();
		}
		// Create and add to vector
		Light addLight(params, currLight);
		newLights.push_back(std::move(addLight));
	}
#else
	// Adjust light intensity to scene dims
	float intensity = 5.0f * (max - min).norm();

	// Create lights in rectangle pattern
	for (int i = 0; i < 4; i++)
	{
		PointLightParams* params = new PointLightParams();
		Light addLight(params, Eigen::Vector3f().setOnes(), intensity, 1.0f);
		// Add lights above scene in a rect pattern
		addLight.SetPosition(Eigen::Vector3f(
			(i >> 0) % 2 == 0 ? max.x() : min.x(),
			(i >> 1) % 2 == 0 ? max.z() : min.z(),
			(i >> 2) % 2 == 0 ? max.y() : max.y())
		);
		newLights.push_back(std::move(addLight));
	}
#endif

	// Return predicted / default lights
	return newLights;
}

//---------------------------------------
// Computes prefiltered non-blurry image list
//---------------------------------------
void SceneManager::X_ComputeImagesToProcess(
	ReferencePath dir
) const
{
	using namespace boost::filesystem;

	// Try to read filtered image file
	fstream filterFile(dir / "filteredList.txt", std::ios_base::in);

	// If it exists
	if (filterFile.good())
	{
		// Load parameters
		float fileEdgeThreshold, fileEdgeWeak, fileEdgeStrong, fileEdgeFactor, fileFrequencyFactor;
		filterFile >> fileEdgeThreshold >> fileEdgeWeak >> fileEdgeStrong >> fileEdgeFactor >> fileFrequencyFactor;

		// If they match, the list still accurate
		if (renderSettings.GetFilterSettings().EdgeThreshold == fileEdgeThreshold &&
			renderSettings.GetFilterSettings().EdgeWeak == fileEdgeWeak &&
			renderSettings.GetFilterSettings().EdgeStrong == fileEdgeStrong &&
			renderSettings.GetFilterSettings().EdgeFactor == fileEdgeFactor &&
			renderSettings.GetFilterSettings().FrequencyFactor == fileFrequencyFactor)
		{
			return;
		}
		else
		{
			// Otherwise close the file
			filterFile.close();
		}
	}

	// Clear file and store parameters
	filterFile.open(dir / "filteredList.txt", std::ios_base::out | std::ios_base::trunc);
	filterFile << renderSettings.GetFilterSettings().EdgeThreshold << " "
		<< renderSettings.GetFilterSettings().EdgeWeak << " "
		<< renderSettings.GetFilterSettings().EdgeStrong << " "
		<< renderSettings.GetFilterSettings().EdgeFactor << " "
		<< renderSettings.GetFilterSettings().FrequencyFactor << "\n";

	// Used to track candidates
	struct ClearCandidate
	{
		ModifiablePath Path;
		float Frequency;
		float EdgeWeight;
	};

	// Tracking variables
	float avgFreq = 0.0f;
	float avgEdge = 0.0f;
	int totalImgs = 0;
	int acceptedImgs = 0;

	// Potential non-blurry images
	std::vector<ClearCandidate> candidates;

	// If path exists & directory
	if (exists(dir))
	{
		if (is_directory(dir))
		{
			// For each file
			for (auto entry : directory_iterator(dir))
			{
				// If file is rgb image
				if (boost::algorithm::contains(entry.path().filename().string(), "color"))
				{
					ClearCandidate curr{ entry.path(), 0.0f, 0.0f };

					// Compute blurriness
					if (!ComputeIsBlurry(
						cv::imread(curr.Path.string()),
						renderSettings.GetFilterSettings().EdgeWeak,
						renderSettings.GetFilterSettings().EdgeStrong,
						renderSettings.GetFilterSettings().EdgeThreshold,
						curr.EdgeWeight,
						curr.Frequency
					))
					{
						// Mark as candidate if not obviously blurry
						avgFreq += curr.Frequency;
						candidates.push_back(curr);
					}

					// For average edge value
					avgEdge += curr.EdgeWeight;
					totalImgs++;
				}
			}
		}
	}

	// Calculate threshold values (average edge of all images, average frequency of candidates)
	float normedEdge = ((1.0f - (static_cast<float>(candidates.size()) / totalImgs)) * avgEdge) / totalImgs;
	float edgeThreshold = renderSettings.GetFilterSettings().EdgeFactor * normedEdge;
	float freqThreshold = renderSettings.GetFilterSettings().FrequencyFactor * (avgFreq / candidates.size());

	// Store non-blurry candidates in the filtered list file
	for (const auto& curr : candidates)
	{
		if (curr.EdgeWeight >= edgeThreshold && curr.Frequency >= freqThreshold)
		{
			filterFile << curr.Path.lexically_normal().string() << "\n";
			acceptedImgs++;
		}
	}

	// Some logging & cleanup
	std::cout << acceptedImgs << "/" << candidates.size()
		<< " accepted (" << totalImgs << " total)" << std::endl;
	filterFile.close();
}

//---------------------------------------
// Generates lighting information for scene
//---------------------------------------
bool SceneManager::X_EstimateLighting(
	ReferencePath dir
) const
{
#if USE_ESTIMATOR
	// If estimation enabled and not yet performed
	if(!boost::filesystem::exists(dir / "lights.json"))
	{
		// Call estimator
		HDR::LightEstimator estimator;
		estimator.Estimate(dir.string());
	}
	// Return if lights were detected
	rapidjson::Document lights;
	if (CanReadJSONFile(renderSettings.GetScenePath() / "lights.json", lights))
	{
		if (lights.IsArray())
		{
			return lights.GetArray().Size() > 0;
		}
	}
	// If not, scene is probably not good
	return false;
#else
	return true;
#endif
}

//---------------------------------------
// Loads real images from prefiltered list
//---------------------------------------
std::vector<SceneImage> SceneManager::X_GetImagesToProcess(
	ReferencePath dir
) const
{
	std::vector<SceneImage> clearImages;

	// Open the filter file
	boost::filesystem::ifstream filterFile(dir / "filteredList.txt");

	// Skip the first line (parameters)
	std::string line;
	std::getline(filterFile, line);

	// For each non-blurry image
	while (std::getline(filterFile, line))
	{
		// Store path to real RGB image
		SceneImage currImage;
		currImage.SetScenePath(line);
		// Compute & store frame name
		ModifiablePath path(line);
		currImage.SetFrame(path.stem().stem().string());
		// Compute & store path to camera pose
		std::string pose(line);
		boost::algorithm::replace_last(pose, "color.jpg", "pose.txt");
		currImage.SetPosePath(ModifiablePath(pose));
		// Place in output vector
		clearImages.emplace_back(std::move(currImage));
	}

	// Return shuffled images & pose file
	filterFile.close();
	std::random_device rd;
	std::default_random_engine gen(rd());
	std::shuffle(clearImages.begin(), clearImages.end(), gen);
	return clearImages;
}

//---------------------------------------
// Processing is done multithreaded
//---------------------------------------
void SceneManager::X_ProcessThread(
	Blender::BlenderRenderer* renderer,
	boost::mutex* syncPoint,
	int threadID
)
{
	// Only the first thread computes non-blurry images and generates lighting information
	syncPoint->lock();
	X_ComputeImagesToProcess(renderSettings.GetSceneRGBPath());
	bool hasLights = X_EstimateLighting(renderSettings.GetScenePath());
	syncPoint->unlock();

	// Get non blurry images
	std::vector<SceneImage> sceneImages = X_GetImagesToProcess(renderSettings.GetSceneRGBPath());

	// Make sure there are any images
	if (sceneImages.empty() || !hasLights)
		return;

	// Create camera blueprint for scene
	camBlueprint.LoadIntrinsics(renderSettings);

	// Control params
	int maxIters = renderSettings.GetSimulationSettings().SceneIterations;
	size_t poseCount = sceneImages.size();
	size_t batchSize = renderSettings.GetSimulationSettings().BatchSize;
	size_t batchMax = ceil(static_cast<float>(poseCount) / static_cast<float>(batchSize));
	ModifiablePath scenePath = boost::filesystem::relative(renderSettings.GetSceneRGBPath());

	// For each scene iteration
	for (int iter = 0; iter < maxIters && imgCountScene < renderSettings.GetSimulationSettings().SceneLimit; ++iter)
	{
		renderer->LogPerformance("Iteration " + std::to_string(iter + 1), threadID);

		// Create annotations manager
		ModifiablePath annotationPath = renderSettings.GetFinalPath() / "annotations";
		Eigen::Vector2i renderRes = camBlueprint.GetIntrinsics().GetResolution();
		renderRes *= renderSettings.GetEngineSettings().RenderScale;
		auto annotations = new AnnotationsManager(annotationPath, renderRes);

		syncPoint->lock();
		// Create render mesh of scan scene
		auto meshScene = X_CreateSceneMesh();
		// Create physx mesh of scan scene
		auto pxMeshScene = X_PxCreateSceneMesh();
		syncPoint->unlock();

		// Create simulation
		float maxDist = 0.0f;
		auto simulation = X_PxCreateSimulation(pxMeshScene, maxDist);

		// Load scene exposures
		rapidjson::Document exposures;
		bool hasExposure = CanReadJSONFile(renderSettings.GetScenePath() / "exposures.json", exposures);

		// Create lights according to scene size (or from estimations)
		auto vecLights = X_PlaceLights(
			Eigen::Vector3f(
				pxMeshScene.GetGlobalBounds().minimum.x,
				pxMeshScene.GetGlobalBounds().minimum.y,
				pxMeshScene.GetGlobalBounds().minimum.z
			),
			Eigen::Vector3f(
				pxMeshScene.GetGlobalBounds().maximum.x,
				pxMeshScene.GetGlobalBounds().maximum.y,
				pxMeshScene.GetGlobalBounds().maximum.z
			)
		);

		// Init random generator
		std::random_device randDev;
		auto randGen = std::default_random_engine(randDev());

		// Create physx objects
		auto vecPxObjs = X_PxCreateObjs(randGen, pxMeshScene, simulation);

		// Run the simulation
		X_PxRunSim(simulation, 1.0f / 50.0f, renderSettings.GetSimulationSettings().SimulationSteps);

		// Save results
		auto vecObjs = X_PxSaveSimResults(vecPxObjs);

		// For every batch
		for (size_t batch = 0; batch < batchMax && imgCountScene < renderSettings.GetSimulationSettings().SceneLimit; ++batch)
		{
			renderer->LogPerformance("Batch " + std::to_string(batch + 1), threadID);
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
				// Load exposure if it exists
				if (hasExposure)
				{
					currCams[batchPose].SetExposure(SafeGet<float>(exposures, currImages[batchPose].GetFrame()));
				}
				// Store & update image number atomically
				syncPoint->lock();
				currCams[batchPose].SetImageNum(++imgCountDepth);
				syncPoint->unlock();
			}

			// Render depths & masks
			renderer->LogPerformance("Depth & Masks", threadID);
			std::vector<Mask> masks = X_RenderDepthMasks(
				renderer,
				threadID,
				meshScene,
				vecObjs,
				currCams,
				vecLights,
				syncPoint,
				maxDist
			);
			renderer->LogPerformance("Depth & Masks", threadID);

			// Determine which images should be processed further
			std::vector<Mask> unoccludedMasks;
			std::vector<Camera> unoccludedCams;
			std::vector<SceneImage> unoccludedImages;
			for (size_t check = 0; check < masks.size(); ++check)
			{
				// Only process unoccluded images
				if (!masks[check].Occluded())
				{
					// Store & update image number atomically
					syncPoint->lock();
					currCams[check].SetImageNum(++imgCountUnoccluded);
					syncPoint->unlock();
					// Store the blended depth
					ModifiablePath depthPath = renderSettings.GetImagePath("depth", imgCountUnoccluded, true);
#if STORE_DEBUG_TEX
					masks[check].StoreBlendedDepth01(depthPath, FLT_EPSILON, maxDist);
#else
					masks[check].StoreBlendedDepth(depthPath);
#endif
					// Move corresponding poses, masks & real images
					unoccludedCams.push_back(std::move(currCams[check]));
					unoccludedMasks.push_back(std::move(masks[check]));
					unoccludedImages.push_back(std::move(currImages[check]));
				}
			}

			// If batch contains useful images
			if (!unoccludedImages.empty())
			{
				// Render labels & create annotations
				renderer->LogPerformance("Labels & Annotating", threadID);
				X_RenderSegments(
					renderer,
					threadID,
					annotations,
					meshScene,
					vecObjs,
					unoccludedCams,
					vecLights,
					unoccludedMasks
				);
				renderer->LogPerformance("Labels & Annotating", threadID);

				// Render synthetic image & blend with real one
				renderer->LogPerformance("PBR Render & Blend", threadID);
				X_RenderPBRBlend(
					renderer,
					threadID,
					meshScene,
					vecObjs,
					unoccludedCams,
					vecLights,
					unoccludedMasks,
					unoccludedImages
				);
				renderer->LogPerformance("PBR Render & Blend", threadID);
			}

			// Update scene limit & output duration
			syncPoint->lock();
			imgCountScene += unoccludedImages.size();
			syncPoint->unlock();
			renderer->LogPerformance("Batch " + std::to_string(batch + 1), threadID);
		}

		// Done with iteration
		renderer->LogPerformance("Iteration " + std::to_string(iter + 1), threadID);
		X_CleanupScene(simulation, annotations, renderer, threadID);
	}
}

//---------------------------------------
// Run simulation & render synthetic images
//---------------------------------------
int SceneManager::ProcessNext(
	int imageCount
)
{
	// Update total images & reset scene limit
	imgCountUnoccluded = imageCount;
	imgCountScene = 0;

	// Create threaded renderer (Each process needs ~4GB!)
	auto syncPoint = new boost::mutex();
	auto cpuCount = std::thread::hardware_concurrency() / 2U;
	auto memCount = (SafeGet<int>(renderSettings.GetJSONConfig(), "mem_available") - 1U) / 4U;
	auto processCount = std::min(cpuCount, memCount);
	auto render = new Blender::BlenderRenderer(processCount);

	// Create one thread / core
	std::vector<boost::thread*> threads;
	for (int i = 0; i < processCount; ++i)
	{
		threads.push_back(new boost::thread(&SceneManager::X_ProcessThread, this, render, syncPoint, i));
	}

	// Wait until done
	for (int i = 0; i < processCount; ++i)
	{
		threads[i]->join();
		delete threads[i];
	}

	// Cleanup
	PTR_RELEASE(syncPoint);
	PTR_RELEASE(render);

	// Return how many images were rendered
	return (imgCountUnoccluded - imageCount);
}

//---------------------------------------
// Create new scene manager
//---------------------------------------
SceneManager::SceneManager(
	const Settings& settings,
	const std::vector<PxMeshConvex*>& vecPxMeshObjs,
	const std::vector<RenderMesh*>& vecRenderMeshObjs
) :
	camBlueprint(),
	vecpPxMeshObjs(vecPxMeshObjs),
	vecpRenderMeshObjs(vecRenderMeshObjs),
	renderSettings(settings),
	imgCountDepth(0),
	imgCountUnoccluded(0),
	imgCountScene(0)
{
}
