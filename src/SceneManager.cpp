#include <SceneManager.h>

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

		// Random position in scene
		float y = (rand() % 10) + 1;
		float x = (rand() % ((int)(pPxMeshScene->GetMaximum().x - pPxMeshScene->GetMinimum().x))) + pPxMeshScene->GetMinimum().x;
		float z = (rand() % ((int)(pPxMeshScene->GetMaximum().z - pPxMeshScene->GetMinimum().z))) + pPxMeshScene->GetMinimum().z;

		// Set pose and save mesh in vector
		PxTransform pose(PxVec3(x, y, z), PxQuat(-0.7071068, 0, 0, 0.7071068));
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
		currTrans.fromPositionOrientationScale(Eigen::Vector3f(pose.p.x, pose.p.y, pose.p.z),
			Eigen::Quaternionf(pose.q.w, pose.q.x, pose.q.y, pose.q.z), currMesh->GetScale() * objScale);
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
RenderResult SceneManager::X_RenderSceneDepth() const
{
	// Render depth with OpenGL
	auto renders = pRenderer->RenderScenes(
		pRenderSettings->GetScenePath(),
		vecCameraPoses,
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
		// Create & store rgb texture
		Texture rgbScene(false);
		rgbScene.SetTexture(std::get<0>(renders[render_count]));
		rgbScene.SetPath(pRenderSettings->GetImagePath("rgb", render_count));
		rgbScene.StoreTexture();
		// Create & store depth texture
		Texture depthScene(true);
		depthScene.SetTexture(std::get<1>(renders[render_count]));
		depthScene.SetPath(pRenderSettings->GetImagePath("scene_depth", render_count));
		depthScene.StoreTexture();
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
	for(auto currLight : vecpLights)
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
	Texture packed(false);
	ModifiablePath packedPath(result.GetPath().parent_path());
	packedPath.append(result.GetPath().stem().string());
	packedPath.concat("_packed.png");
	packed.SetPath(packedPath);

	// Set shaders
	for (auto currMesh : vecpRenderMeshCurrObjs)
	{
		DepthShader* currShader = new DepthShader(renderCam.GetClipping().x(), renderCam.GetClipping().y());
		currMesh->SetShader((OSLShader*)currShader);
	}

	// Set raytracing settings
	renderCam.SetDepthOnly(true);
	renderCam.SetAASamples(16);
	renderCam.SetRayBounces(0);

	// Send render command
	X_ProcessRenderfile(packed);

	// Convert to linear depth
	result.SetTexture(UnpackRGBDepth(packed.GetTexture()));
}

//---------------------------------------
// Render label texture (object Ids as color)
//---------------------------------------
void SceneManager::X_RenderObjsLabel(Texture& result)
{
	// Set shaders
	for (auto currMesh : vecpRenderMeshCurrObjs)
	{
		LabelShader* currShader = new LabelShader(currMesh->GetLabelId());
		currMesh->SetShader((OSLShader*)currShader);
	}

	// Set raytracing settings
	renderCam.SetDepthOnly(false);
	renderCam.SetAASamples(1);
	renderCam.SetRayBounces(0);

	// Send render command
	X_ProcessRenderfile(result);
}

//---------------------------------------
// Render objects physically based
// TODO: Object color rendering
//---------------------------------------
void SceneManager::X_RenderObjsRGB(Texture& result)
{
	// Set shaders
	for (auto currMesh : vecpRenderMeshCurrObjs)
	{
		// TODO
	}

	// Set raytracing settings
	renderCam.SetDepthOnly(false);
	renderCam.SetAASamples(16);
	renderCam.SetRayBounces(-1);

	// Send render command
	X_ProcessRenderfile(result);
}

//---------------------------------------
// Final image blend
// TODO: Proper render setup
//---------------------------------------
void SceneManager::X_RenderImageBlend(
	Texture& result,
	const Texture& occlusion,
	const Texture& original,
	const Texture& rendered
)
{
	// Setup camera shader
	BlendShader* finalBlend = new BlendShader(occlusion, original, rendered);
	renderCam.SetEffect((OSLShader*)finalBlend);

	// Set raytracing settings
	renderCam.SetDepthOnly(false);
	renderCam.SetAASamples(1);
	renderCam.SetRayBounces(0);

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
// TODO: Object color rendering
//---------------------------------------
int SceneManager::Run(int imageCount)
{
	int newImages = 0;

	// Create/open annotation file
	pAnnotations->Begin(pRenderSettings);

	// Get non blurry images
	X_GetImagesToProcess(pRenderSettings->GetSceneRGBPath(), 400.f);
	if (vecCameraImages.empty())
		return 0;

	// Fetch camera intrinsics
	renderCam.LoadIntrinsics(pRenderSettings);

	// Render scene depth with OpenGL
	RenderResult sceneRenders = X_RenderSceneDepth();

	// For each scene iteration
	int maxIters = pRenderSettings->GetIterationCount();
	ModifiablePath scenePath = boost::filesystem::relative(pRenderSettings->GetSceneRGBPath());
	for (int iter = 0; iter < maxIters; iter++)
	{
		// Create physx representation of scan scene
		X_PxCreateScene();
		// Create physx objects
		X_PxCreateObjs();
		// Run the simulation
		X_PxRunSim(1.0f / 50.0f, 400);
		// Save results
		X_PxSaveSimResults();

		// For each camera pose
		int lastPose = vecCameraPoses.size();
		for (int currPose = 0; currPose < lastPose; currPose++)
		{
			std::cout << "Scene "<< scenePath << ": Iteration " << iter << "/" << maxIters
				<< ": Pose " << currPose << "/" << lastPose << std::endl;

			// Load camera pose
			renderCam.LoadExtrinsics(vecCameraPoses.at(currPose));

			// Render object depths
			Texture bodiesDepth(true);
			bodiesDepth.SetPath(pRenderSettings->GetImagePath("body_depth", currPose));
			X_RenderObjsDepth(bodiesDepth);
			bodiesDepth.StoreTexture();

			// Load scene depth
			Texture sceneDepth = std::get<1>(sceneRenders[currPose]);

			// Create occlusion mask
			bool objectsOccluded;
			Texture bodiesMasked(false);
			bodiesMasked.SetTexture(ComputeOcclusionMask(bodiesDepth.GetTexture(), sceneDepth.GetTexture(), objectsOccluded));

			// If any objects visible
			if (!objectsOccluded)
			{
				int currImg = imageCount + newImages;

				// Render object labels (IDs as color)
				Texture bodiesLabeled(false);
				bodiesLabeled.SetPath(pRenderSettings->GetImagePath("body_label", currImg));
				X_RenderObjsLabel(bodiesLabeled);

				// Blend depth images
				Texture blendedDepth(true);
				blendedDepth.SetPath(pRenderSettings->GetImagePath("depth", currImg, true));
				blendedDepth.SetTexture(ComputeDepthBlend(bodiesDepth.GetTexture(), sceneDepth.GetTexture()));
				blendedDepth.StoreTexture();

				// Blend label and mask images
				Texture bodiesSegmented(false);
				bodiesSegmented.SetPath(pRenderSettings->GetImagePath("segs", currPose, true));
				bodiesSegmented.SetTexture(ComputeSegmentMask(bodiesLabeled.GetTexture(), bodiesMasked.GetTexture()));
				bodiesSegmented.StoreTexture();

				// Save annotations
				for (auto currMesh : vecpRenderMeshCurrObjs)
					pAnnotations->Write(currMesh, bodiesLabeled.GetTexture(),
						bodiesSegmented.GetTexture(), renderCam, currImg);

				// Render final image blend
				Texture finalBlend(false);
				finalBlend.SetPath(pRenderSettings->GetImagePath("rgb", currPose, true));
				// TODO: Replace with correct images
				X_RenderImageBlend(finalBlend, bodiesSegmented, Texture(), Texture());

				newImages++;
			}

			// Exit if target reached
			if (imageCount + newImages >= renderSettings.GetMaxImageCount())
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
	renderCam.SetClipping(
		SafeGet<float>(pRenderSettings->GetJSONConfig(), "near_z"),
		SafeGet<float>(pRenderSettings->GetJSONConfig(), "far_z")
	);

	// Create lights
	for (int i = 0; i < 8; i++)
	{
		LightParamsBase* params = (LightParamsBase*)(new PointLightParams());
		Light* addLight = new Light(params, Eigen::Vector3f(1.0f, 1.0f, 1.0f), 2.0f, 1.0f);
		// Add 8 lights in a box pattern
		addLight->SetPosition(Eigen::Vector3f(
			i % 2 == 0 ? 1000.0f : -1000.0f,
			(i >> 2) % 2 == 0 ? 100.0f : 1000.0f,
			(i >> 1) % 2 == 0 ? 1000.0f : -1000.0f)
		);
		vecpLights.push_back(addLight);
	}

	// Create OpenGL renderer
	pRenderer = new Renderer::Render(
		ModifiablePath(SafeGet<const char*>(pRenderSettings->GetJSONConfig(), "shaders_gl")),
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
	for(auto currLight : vecpLights)
	{
		delete currLight;
	}
	vecpLights.clear();
}
