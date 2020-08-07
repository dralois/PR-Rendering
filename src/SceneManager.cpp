#include <SceneManager.h>

#define STORE_SCENE_TEX 1
#define STORE_DEBUG_TEX 1

#define PI (3.1415926535897931f)
#define PIOVER2 (1.5707963267948966f)

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
		PxTransform pose(PxVec3(z, y, x), PxQuat(PxIDENTITY::PxIdentity));
		currObj->SetTransform(pose);
		currObj->AddRigidActor(pPxScene);
		vecpPxMeshCurrObjs.push_back(currObj);
	}
}

//---------------------------------------
// Cleanup physx
//---------------------------------------
void SceneManager::X_CleanupScene()
{
	// Cleanup scene mesh
	if (pPxMeshScene != NULL)
	{
		delete pPxMeshScene;
		pPxMeshScene = NULL;
	}

	// Cleanup objects meshes
	for (auto obj : vecpPxMeshCurrObjs)
	{
		delete obj;
	}
	vecpPxMeshCurrObjs.clear();

	// Cleanup physx
	if (pPxScene != NULL)
	{
		pPxScene->flushSimulation();
		auto dispatcher = (PxDefaultCpuDispatcher*)pPxScene->getCpuDispatcher();
		PX_RELEASE(dispatcher);
		PX_RELEASE(pPxScene);
	}

	// Cleanup render meshes
	for (auto curr : vecpRenderMeshCurrObjs)
	{
		delete curr;
	}
	vecpRenderMeshCurrObjs.clear();

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
		((MeshBase*)currMesh)->SetObjId(obj->GetObjId());
		// Build transform from px pose
		Eigen::Affine3f currTrans;
		currTrans.fromPositionOrientationScale(
			Eigen::Vector3f(pose.p.x, pose.p.y, pose.p.z),
			Eigen::Quaternionf(pose.q.w, pose.q.x, pose.q.y, pose.q.z),
			currMesh->GetScale() * objScale);
		currMesh->SetTransform(currTrans.matrix());
		// Store it
		vecpRenderMeshCurrObjs.push_back(currMesh);

#if DEBUG || _DEBUG
		// Object transform update for pvd
		obj->SetTransform(pose);
		obj->SetScale(obj->GetScale() * undoScale);
	}
	// Scene transform update for pvd
	pPxMeshScene->SetTransform(PxTransform(PxIDENTITY::PxIdentity));
	pPxMeshScene->SetScale(pPxMeshScene->GetScale() * undoScale);
	// Simulate once for pvd
	pPxScene->simulate(0.001f);
	pPxScene->fetchResults(true);
#else
}
#endif // DEBUG || _DEBUG
}

//---------------------------------------
// Render scene depth
//---------------------------------------
RenderResult SceneManager::X_RenderSceneDepth(std::vector<ModifiablePath> poses) const
{
	// Render depth with OpenGL
	auto renders = pRenderer->RenderScenes(
		pRenderSettings->GetScenePath(),
		poses,
		renderCam.GetIntrinsics().GetFocalLenght().x(),
		renderCam.GetIntrinsics().GetFocalLenght().y(),
		renderCam.GetIntrinsics().GetPrincipalPoint().x(),
		renderCam.GetIntrinsics().GetPrincipalPoint().y(),
		renderCam.GetIntrinsics().GetWidth(),
		renderCam.GetIntrinsics().GetHeight()
	);

	// For each image
	RenderResult results;
	for (int render_count = 0; render_count < renders.size(); render_count++)
	{
		// Create rgb & depth textures
		Texture rgbScene(false, false);
		Texture depthScene(true, true);
		rgbScene.SetTexture(std::get<0>(renders[render_count]));
		depthScene.SetTexture(std::get<1>(renders[render_count]));
		rgbScene.SetPath(pRenderSettings->GetImagePath("scene_rgb", render_count));
		depthScene.SetPath(pRenderSettings->GetImagePath("scene_depth", render_count));
#if STORE_SCENE_TEX
		// Store if enabled
		rgbScene.StoreTexture();
		depthScene.StoreDepth01(renderCam.GetClipping().x(), renderCam.GetClipping().y());
#endif //STORE_SCENE_TEX
		// Store in result vector
		results.push_back(std::make_tuple(rgbScene, depthScene));
	}

	// Finally return textures
	return results;
}

//---------------------------------------
// Build & send renderfile for blender
//---------------------------------------
void SceneManager::X_ProcessRenderfile(Texture& result)
{
	// Create writer
	rapidjson::StringBuffer renderstring;
	JSONWriter writer(renderstring);
	writer.StartArray();
	writer.StartObject();

	// Add settings
	writer.Key("settings");
	pRenderSettings->AddToJSON(writer);

	// Add camera
	writer.Key("camera");
	renderCam.SetResultFile(result.GetPath());
	renderCam.AddToJSON(writer);

	// Add meshes
	writer.Key("meshes");
	writer.StartArray();
	for (auto currMesh : vecpRenderMeshCurrObjs)
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

	// Send off to blender
	writer.EndObject();
	writer.EndArray();
	std::string renderfile(renderstring.GetString());
	pBlender->ProcessRenderfile(renderfile);

	// Load result to texture after rendering
	result.LoadTexture();
}

//---------------------------------------
// Render objects depth
//---------------------------------------
void SceneManager::X_RenderObjsDepth(Texture& result)
{
	Texture packed(true, true);
	ModifiablePath packedPath(result.GetPath().parent_path());
	packedPath.append(result.GetPath().stem().string());
	packedPath.concat("_packed");
	packed.SetPath(packedPath, "exr");

	// Set shaders
	for (auto currMesh : vecpRenderMeshCurrObjs)
	{
		DepthShader* currShader = new DepthShader(renderCam.GetClipping().x(), renderCam.GetClipping().y());
		currMesh->SetShader((OSLShader*)currShader);
	}

	// Render settings
	renderCam.SetDataOnly(true);
	renderCam.SetAASamples(1);
	renderCam.SetRayBounces(0);
	renderCam.SetShadingOverride("depth");
	renderCam.SetRenderResolution(pRenderSettings->GetRenderResolution());

	// Send render command
	X_ProcessRenderfile(packed);

	// Convert to linear depth & delete packed texture
	result.SetTexture(UnpackDepth(packed.GetTexture()));
	packed.DeleteTexture();
}

//---------------------------------------
// Render label texture (object Ids as color)
//---------------------------------------
void SceneManager::X_RenderObjsLabel(Texture& result)
{
	Texture packed(true, false);
	ModifiablePath packedPath(result.GetPath().parent_path());
	packedPath.append(result.GetPath().stem().string());
	packedPath.concat("_packed");
	packed.SetPath(packedPath, "exr");

	// Set shaders
	for (auto currMesh : vecpRenderMeshCurrObjs)
	{
		cv::Vec3b encodedId = EncodeInt(currMesh->GetLabelId());
		// Store encoded Id as BGR
		LabelShader* currShader = new LabelShader(encodedId[2], encodedId[1], encodedId[0]);
		currMesh->SetShader((OSLShader*)currShader);
	}

	// Set raytracing settings
	renderCam.SetDataOnly(true);
	renderCam.SetAASamples(1);
	renderCam.SetRayBounces(0);
	renderCam.SetShadingOverride("");
	renderCam.SetRenderResolution(pRenderSettings->GetRenderResolution());

	// Send render command
	X_ProcessRenderfile(packed);

	// Convert to 8b rgb & delete packed texture
	result.SetTexture(UnpackLabel(packed.GetTexture()));
	packed.DeleteTexture();
}

//---------------------------------------
// Render objects physically based
//---------------------------------------
void SceneManager::X_RenderObjsPBR(Texture& result)
{
	// Set shaders
	std::vector<Texture*> texStorage;
	for (auto currMesh : vecpRenderMeshCurrObjs)
	{
		Texture* currTex = new Texture();
		currTex->SetPath(currMesh->GetTexturePath());
		texStorage.push_back(currTex);
		PBRShader* currShader = new PBRShader(currTex);
		currMesh->SetShader((OSLShader*)currShader);
	}

	// Set raytracing settings
	renderCam.SetDataOnly(false);
	renderCam.SetAASamples(16);
	renderCam.SetRayBounces(-1);
	renderCam.SetShadingOverride("");
	renderCam.SetRenderResolution(Eigen::Vector2i(renderCam.GetIntrinsics().GetWidth(), renderCam.GetIntrinsics().GetHeight()));

	// Send render command
	X_ProcessRenderfile(result);

	// Remove textures
	for (auto tex : texStorage)
	{
		delete tex;
	}
	texStorage.clear();
}

//---------------------------------------
// Final image blend
// TODO: Proper render setup
//---------------------------------------
void SceneManager::X_RenderImageBlend(
	Texture& result,
	Texture& occlusion,
	Texture& original,
	Texture& rendered
)
{
	// Setup camera shader
	BlendShader* finalBlend = new BlendShader(&occlusion, &original, &rendered);
	renderCam.SetEffect((OSLShader*)finalBlend);

	// Set raytracing settings
	renderCam.SetDataOnly(false);
	renderCam.SetAASamples(1);
	renderCam.SetRayBounces(0);
	renderCam.SetShadingOverride("");
	renderCam.SetRenderResolution(pRenderSettings->GetRenderResolution());

	// Send render command
	X_ProcessRenderfile(result);
}

//---------------------------------------
// Determine images to process
//---------------------------------------
void SceneManager::X_GetImagesToProcess(
	ReferencePath path,
	float varThreshold
)
{
	vecCameraImages = std::vector<ModifiablePath>();
	vecCameraPoses = std::vector<ModifiablePath>();

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
						ModifiablePath img(entry.path());
						std::cout << "Using image " << img.filename() << std::endl;
						// Save image in vector
						vecCameraImages.push_back(img);
						// Save pose file in vector
						std::string pose(entry.path().string());
						boost::algorithm::replace_last(pose, "color.jpg", "pose.txt");
						vecCameraPoses.push_back(ModifiablePath(pose));
					}
				}
			}
			// Finally sort vectors
			std::sort(vecCameraPoses.begin(), vecCameraPoses.end());
			std::sort(vecCameraImages.begin(), vecCameraImages.end());
		}
	}
}

//---------------------------------------
// Run simulation
//---------------------------------------
int SceneManager::Run(int imageCount)
{
	int newImages = 0;
	RenderResult sceneRenders;

	// Create/open annotation file
	pAnnotations->Begin(pRenderSettings);

	// Get non blurry images
	X_GetImagesToProcess(pRenderSettings->GetSceneRGBPath(), 400.f);
	if (vecCameraImages.empty())
		return 0;

	// Fetch camera intrinsics
	renderCam.LoadIntrinsics(pRenderSettings);

	// For each scene iteration
	int maxIters = pRenderSettings->GetIterationCount();
	int batchSize = pRenderSettings->GetRenderBatchSize();
	ModifiablePath scenePath = boost::filesystem::relative(pRenderSettings->GetSceneRGBPath());
	for (int iter = 0; iter < maxIters; iter++)
	{
		// Create physx representation of scan scene
		X_PxCreateScene();
		// Create physx objects
		X_PxCreateObjs();
		// Run the simulation
		X_PxRunSim(1.0f / 50.0f, pRenderSettings->GetStepsPerSimulation());
		// Save results
		X_PxSaveSimResults();

		// For each camera pose
		int lastPose = vecCameraPoses.size();
		for (int currPose = 0; currPose < lastPose; currPose++)
		{
			std::cout << "Scene " << scenePath << ": Iteration " << (iter + 1) << "/" << maxIters
				<< ": Pose " << (currPose + 1) << "/" << lastPose << std::endl;

			// Possibly render scene depth
			if(currPose % batchSize == 0)
			{
				// Remove old renders
				sceneRenders.clear();
				// Determine range
				uint start = currPose;
				uint end = currPose + batchSize >= lastPose ?
					lastPose - currPose : currPose + batchSize;
				// Build vector
				std::vector<ModifiablePath> currPoses(
					vecCameraPoses.begin() + start,
					vecCameraPoses.begin() + end
				);
				// Render scene depth with OpenGL
				sceneRenders = X_RenderSceneDepth(currPoses);
			}

			// Load camera pose
			renderCam.LoadExtrinsics(vecCameraPoses.at(currPose));

			// Render object depths
			Texture bodiesDepth(true, true);
			bodiesDepth.SetPath(pRenderSettings->GetImagePath("body_depth", currPose));
			X_RenderObjsDepth(bodiesDepth);
#if STORE_DEBUG_TEX
			bodiesDepth.StoreDepth01(renderCam.GetClipping().x(), renderCam.GetClipping().y());
#endif //STORE_DEBUG_TEX

			// Load scene depth
			Texture sceneDepth = std::get<1>(sceneRenders[currPose % batchSize]);

			// Create occlusion mask
			bool objectsOccluded;
			Texture bodiesMasked(false, true);
			bodiesMasked.SetPath(pRenderSettings->GetImagePath("body_mask", currPose));
			bodiesMasked.SetTexture(ComputeOcclusionMask(bodiesDepth.GetTexture(), sceneDepth.GetTexture(), objectsOccluded));
#if STORE_DEBUG_TEX
			bodiesMasked.StoreTexture();
#endif //STORE_DEBUG_TEX

			// If any objects visible
			if (!objectsOccluded)
			{
				int currImg = imageCount + newImages;

				// Render object labels (IDs as color)
				Texture bodiesLabeled(false, false);
				bodiesLabeled.SetPath(pRenderSettings->GetImagePath("body_label", currPose));
				X_RenderObjsLabel(bodiesLabeled);
#if STORE_DEBUG_TEX
				bodiesLabeled.StoreTexture();
#endif //STORE_DEBUG_TEX

				// Blend depth images
				Texture blendedDepth(true, true);
				blendedDepth.SetPath(pRenderSettings->GetImagePath("depth", currImg, true));
				blendedDepth.SetTexture(ComputeDepthBlend(bodiesDepth.GetTexture(), sceneDepth.GetTexture()));
#if STORE_DEBUG_TEX
				blendedDepth.StoreDepth01(renderCam.GetClipping().x(), renderCam.GetClipping().y());
#else
				blendedDepth.StoreTexture();
#endif //STORE_DEBUG_TEX

				// Blend label and mask images
				Texture bodiesSegmented(false, false);
				bodiesSegmented.SetPath(pRenderSettings->GetImagePath("segs", currImg, true));
				bodiesSegmented.SetTexture(ComputeSegmentMask(bodiesLabeled.GetTexture(), bodiesMasked.GetTexture()));
				bodiesSegmented.StoreTexture();

				// Save annotations
				for (auto currMesh : vecpRenderMeshCurrObjs)
					pAnnotations->Write(currMesh, bodiesLabeled.GetTexture(),
						bodiesSegmented.GetTexture(), renderCam, currImg);

				// PBR objects color rendering
				Texture bodiesColor(false, false);
				bodiesColor.SetPath(pRenderSettings->GetImagePath("body_rgb", currPose, false));
				X_RenderObjsPBR(bodiesColor);
#if STORE_DEBUG_TEX
				bodiesColor.StoreTexture();
#endif //STORE_DEBUG_TEX

				// Load real scene RGB image
				Texture sceneColor(false, false);
				sceneColor.SetPath(vecCameraImages[currPose], "jpg");
				sceneColor.LoadTexture();

				// Simple blend with mask
				Texture simpleBlend(false, false);
				simpleBlend.SetPath(pRenderSettings->GetImagePath("rgb", currImg, true));
				simpleBlend.SetTexture(ComputeRGBBlend(bodiesColor.GetTexture(), sceneColor.GetTexture(), bodiesMasked.GetTexture()));
				simpleBlend.StoreTexture();

				//// TODO: Render final image blend
				//Texture finalBlend(false);
				//finalBlend.SetPath(pRenderSettings->GetImagePath("rgb", currImg, true));
				//X_RenderImageBlend(finalBlend, bodiesMasked, sceneColor, bodiesColor);

				newImages++;
			}

			// Exit if target reached
			if (imageCount + newImages >= pRenderSettings->GetMaxImageCount())
			{
				X_CleanupScene();
				return newImages;
			}
		}

		// Done with iteration
		X_CleanupScene();
	}

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

	// Create camera & load clipping planes
	renderCam = Camera();
	renderCam.SetRenderResolution(pRenderSettings->GetRenderResolution());
	renderCam.SetClipping(
		SafeGet<float>(pRenderSettings->GetJSONConfig(), "near_z"),
		SafeGet<float>(pRenderSettings->GetJSONConfig(), "far_z")
	);

	// Create lights
	for (int i = 0; i < 8; i++)
	{
		LightParamsBase* params = (LightParamsBase*)(new PointLightParams());
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
		renderCam.GetClipping().x(),
		renderCam.GetClipping().y()
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
