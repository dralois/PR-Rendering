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
	}
}

//---------------------------------------
// Fetch and save simulation results
//---------------------------------------
void SceneManager::X_PxSaveSimResults()
{
	float undoScale = 1.0f / SafeGet<float>(pRenderSettings->GetJSONConfig(), "scene_scale");
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

		// Convert physx position & rotation
		PxMat44 pxmat(pose);
		Eigen::Matrix4f mat(pxmat.front());

		// Save & create mesh for rendering
		RenderMesh* currMesh = new RenderMesh(*this->vecpRenderMeshObjs[obj->GetMeshId()]);
		((MeshBase*)currMesh)->SetObjId(obj->GetObjId());
		currMesh->SetTransform(mat);
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
	RenderResult renderings = pRenderer->RenderScenes(
		pRenderSettings->GetScenePath(),
		vecCameraPoses,
		renderCam.GetIntrinsics().GetFocalLenght().x(),
		renderCam.GetIntrinsics().GetFocalLenght().y(),
		renderCam.GetIntrinsics().GetPrincipalPoint().x(),
		renderCam.GetIntrinsics().GetPrincipalPoint().y()
	);

	// For each image
	for (int render_count = 0; render_count < renderings.size(); render_count++)
	{
		// Get output paths
		ModifiablePath depthPath = pRenderSettings->GetImagePath("scene_depth", render_count, true);
		ModifiablePath rgbPath = pRenderSettings->GetImagePath("rgb", render_count, true);

		// Write rgb and depth out
		cv::imwrite(rgbPath.string(), std::get<0>(renderings[render_count]));
		cv::imwrite(depthPath.string(), std::get<1>(renderings[render_count]));
	}

	// Finally return rendered images
	return renderings;
}

//---------------------------------------
// Render objects depth
// TODO: Switch to blender render
//---------------------------------------
void SceneManager::X_RenderObjsDepth()
{
	bool valid = false;
	// For each arnold mesh
	for (auto body : vecCurrObjs)
	{
		AtNode* curr = AiNodeLookUpByName(body.objName.c_str());
		AiNodeSetPtr(curr, "shader", aiShaderDepthObj);
		AiNodeSetDisabled(curr, true);
		valid = true;
	}

	// Set shader parameters
	AiNodeSetBool(aiShaderDepthObj, "is_body", true);
	AiNodeSetFlt(aiShaderDepthScene, "force_val", 30000);
	AiNodeSetPtr(aiOptions, "background", aiShaderDepthScene);

	// Destroy old filter
	AtNode* nullFilter = AiNodeLookUpByName("null_filter");
	AiNodeDestroy(nullFilter);
	// Create new filter
	nullFilter = AiNode("null_filter");
	AiNodeSetFlt(nullFilter, "width", 1);
	AiNodeSetStr(nullFilter, "name", "null_filter");

	// Temp file name
	ostringstream out;
	out << std::internal << std::setfill('0') << std::setw(6) << poseCount;
	string buf = FILE_TEMP_PATH + "/body_depth/img_" + out.str() + ".png";

	// Render settings
	AiNodeSetStr(aiDriver, "filename", buf.c_str());
	AiNodeSetInt(aiDriver, "format", 1);
	AiNodeSetInt(aiOptions, "xres", useCustomIntr ? intrCustom.w : intrOriginal.w);
	AiNodeSetInt(aiOptions, "yres", useCustomIntr ? intrCustom.h : intrOriginal.h);
	AiNodeSetInt(aiOptions, "AA_samples", 1);
	AiNodeSetInt(aiOptions, "GI_diffuse_depth", 1);
	AiNodeSetInt(aiOptions, "GI_specular_samples", 1);

	// Setup rendering
	AiArraySetStr(aiArrOutputs, 0, "RGBA RGBA null_filter outputDriver");
	AiNodeSetArray(aiOptions, "outputs", aiArrOutputs);

	// Render
	if (valid)
		AiRender(AI_RENDER_MODE_CAMERA);
}

//---------------------------------------
// Render object label image (IDs as color)
// TODO: Switch to blender render
//---------------------------------------
void SceneManager::X_RenderObjsLabel()
{
	// For each arnold mesh
	for (auto body : vecCurrObjs)
	{
		AtNode* shader_obj_label;
		string sbuffer = "label_" + std::to_string(body.objId);
		// Try to find mesh node
		shader_obj_label = AiNodeLookUpByName(sbuffer.c_str());
		// If not found create
		if (shader_obj_label == NULL)
		{
			shader_obj_label = AiNode("labelshader");
			AiNodeSetStr(shader_obj_label, "name", sbuffer.c_str());
			AiNodeSetInt(shader_obj_label, "id", (body.objId + 1) * 10);
		}
		// Save node
		AtNode* curr = AiNodeLookUpByName(body.objName.c_str());
		AiNodeSetPtr(curr, "shader", shader_obj_label);
	}

	// Try to find background node
	AtNode* shader_bck_label = AiNodeLookUpByName("label_background");
	// If not found create
	if (shader_bck_label == NULL)
	{
		shader_bck_label = AiNode("labelshader");
		AiNodeSetStr(shader_bck_label, "name", "label_background");
		AiNodeSetInt(shader_bck_label, "id", 0);
	}
	// Save node
	AiNodeSetPtr(aiOptions, "background", shader_bck_label);

	// Destroy old null filter
	AtNode* nullFilter = AiNodeLookUpByName("null_filter");
	AiNodeDestroy(nullFilter);
	// Create new null filter
	nullFilter = AiNode("null_filter");
	AiNodeSetFlt(nullFilter, "width", 1);
	AiNodeSetStr(nullFilter, "name", "null_filter");

	// Temp file
	ostringstream out;
	out << std::internal << std::setfill('0') << std::setw(6) << poseCount;
	string buf = FILE_TEMP_PATH + "/body_label/img_" + out.str() + ".png";

	// Setup render settings
	AiNodeSetStr(aiDriver, "filename", buf.c_str());
	AiNodeSetInt(aiDriver, "format", 1);
	AiNodeSetInt(aiOptions, "xres", useCustomIntr ? intrCustom.w : intrOriginal.w);
	AiNodeSetInt(aiOptions, "yres", useCustomIntr ? intrCustom.h : intrOriginal.h);
	AiNodeSetInt(aiOptions, "AA_samples", 1);
	AiNodeSetInt(aiOptions, "GI_diffuse_depth", 1);
	AiNodeSetInt(aiOptions, "GI_specular_samples", 1);

	// Render image
	AiArraySetStr(aiArrOutputs, 0, "RGBA RGBA null_filter outputDriver");
	AiNodeSetArray(aiOptions, "outputs", aiArrOutputs);
	AiRender(AI_RENDER_MODE_CAMERA);
}

//---------------------------------------
// Final image blend
// TODO: Switch to blender render
//---------------------------------------
void SceneManager::X_RenderImageBlend()
{
	// For each object
	for (auto body : vecCurrObjs)
	{
		string buffer = "blend_" + std::to_string(body.meshId);
		float ks = vecpPxMeshObjs[body.meshId]->GetMetallic();
		// Try to find mesh node
		AtNode* curr = AiNodeLookUpByName(body.objName.c_str());
		AtNode* blendTemp = AiNodeLookUpByName(buffer.c_str());
		// If not found create
		if (blendTemp != nullptr)
		{
			AiNodeSetPtr(curr, "shader", blendTemp);
			AiNodeSetPtr(blendTemp, "mask", (void*)&cvMask);
			AiNodeSetPtr(blendTemp, "blend_image", (void*)&cvScene);
			AiNodeSetPtr(blendTemp, "rend_image", (void*)&cvRend);
			continue;
		}

		// Texture and object shader nodes
		AtNode* shaderMaterial = AiNode("standard");
		AtNode* image = AiNode("image");

		// Texture path
		ostringstream out;
		out << std::internal << std::setfill('0') << std::setw(2) << (body.meshId);
		string imgbuffer = "obj_" + out.str() + "_color.png";
		string imgPath = FILE_OBJ_PATH + "/" + imgbuffer;

		// Setup texture node and link it
		AiNodeSetStr(image, "filename", imgPath.c_str());
		AiNodeLink(image, "Kd_color", shaderMaterial);

		// Setup blending shader
		AtNode* shader_blend = AiNode("blendshader");
		AiNodeSetStr(shader_blend, "name", buffer.c_str());
		AiNodeSetPtr(shader_blend, "mask", (void*)&cvMask);
		AiNodeSetPtr(shader_blend, "blend_image", (void*)&cvScene);
		AiNodeSetPtr(shader_blend, "rend_image", (void*)&cvRend);
		AiNodeSetBool(shader_blend, "force_scene", false);
		// Setup object shader node
		AiNodeSetFlt(shaderMaterial, "diffuse_roughness", 0.5);
		AiNodeSetFlt(shaderMaterial, "Ks", ks);
		AiNodeLink(shaderMaterial, "Kd_bcolor", shader_blend);
		AiNodeSetPtr(curr, "shader", shader_blend);
	}

	// Setup background image and mask
	AiNodeSetPtr(aiShaderBlendImage, "mask", (void*)&cvMask);
	AiNodeSetPtr(aiShaderBlendImage, "blend_image", (void*)&cvScene);
	AiNodeSetPtr(aiShaderBlendImage, "rend_image", (void*)&cvRend);
	AiNodeSetPtr(aiOptions, "background", aiShaderBlendImage);

	// Destroy old null filter
	AtNode* gaussFilter = AiNodeLookUpByName("null_filter");
	AiNodeDestroy(gaussFilter);
	// Create new gauss filter
	gaussFilter = AiNode("gaussian_filter");
	AiNodeSetFlt(gaussFilter, "width", 1);
	AiNodeSetStr(gaussFilter, "name", "null_filter");

	// Final image path
	ostringstream out;
	out << std::internal << std::setfill('0') << std::setw(6) << imageCount;
	string buf = FILE_FINAL_PATH + "/rgb/img_" + out.str() + ".png";

	// Setup render settings
	AiNodeSetStr(aiDriver, "filename", buf.c_str());
	AiNodeSetInt(aiDriver, "format", 1);
	AiNodeSetInt(aiOptions, "xres", (useCustomIntr ? intrCustom.w : intrOriginal.w) / 2);
	AiNodeSetInt(aiOptions, "yres", (useCustomIntr ? intrCustom.h : intrOriginal.h) / 2);
	AiNodeSetInt(aiOptions, "AA_samples", 6);
	AiNodeSetInt(aiOptions, "GI_diffuse_depth", 6);
	AiNodeSetInt(aiOptions, "GI_specular_samples", 6);

	// Render final image/object blend
	AiArraySetStr(aiArrOutputs, 0, "RGBA RGBA null_filter outputDriver");
	AiNodeSetArray(aiOptions, "outputs", aiArrOutputs);
	AiRender(AI_RENDER_MODE_CAMERA);
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
						ModifiablePath pose(entry.path());
						std::cout << "Using image " << img.filename() << std::endl;
						// Save image in vector
						vecCameraImages.push_back(img);
						// Save pose file in vector
						boost::algorithm::replace_last(pose, "color.jpg", "pose.txt");
						vecCameraPoses.push_back(pose);
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

	// Create/open annotation file
	pAnnotations->Begin(pRenderSettings);

	// Get non blurry images
	X_GetImagesToProcess(pRenderSettings->GetSceneRGBPath(), 400.f);
	if (vecCameraImages.empty())
		return 0;

	// Render scene depth with OpenGL
	RenderResult renderings = X_RenderSceneDepth();

	// For each scene iteration
	for (int iter = 0; iter < pRenderSettings->GetIterationCount(); iter++)
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
		for (int currPose = 0; currPose < vecCameraPoses.size(); currPose++)
		{
			// Load camera matrix for pose
			renderCam.LoadExtrinsics(vecCameraPoses.at(currPose));

			// Render object depths
			X_RenderObjsDepth();
			// Load rendered images
			Texture sceneDepth(pRenderSettings->GetImagePath("scene_depth", currPose, true), true);
			Texture bodiesDepth(pRenderSettings->GetImagePath("body_depth", currPose, true), true);

			// Create occlusion mask
			bool objectsOccluded;
			Texture bodiesMasked(false);
			bodiesMasked.SetTexture(ComputeOcclusionMask(bodiesDepth.GetTexture(), sceneDepth.GetTexture(), objectsOccluded));

			// If any objects visible
			if (!objectsOccluded)
			{
				int currImg = imageCount + newImages;
				// Render object labels (IDs as color)
				X_RenderObjsLabel();
				// Load rendered image
				Texture bodiesLabeled(pRenderSettings->GetImagePath("body_label", currImg, true), false);

				// Blend depth images
				Texture blendedDepth(true);
				blendedDepth.SetPath(pRenderSettings->GetImagePath("depth", currImg, false));
				blendedDepth.SetTexture(BlendDepth(bodiesDepth.GetTexture(), sceneDepth.GetTexture(), blendedDepth.GetPath()));

				// Blend label and mask images
				Texture bodiesSegmented(false);
				bodiesSegmented.SetPath(pRenderSettings->GetImagePath("segs", currPose, false));
				bodiesSegmented.SetTexture(BlendLabel(bodiesLabeled.GetTexture(), bodiesMasked.GetTexture(), bodiesSegmented.GetPath()));

				// Save annotations
				for (auto currMesh : vecpRenderMeshCurrObjs)
					pAnnotations->Write(currMesh, bodiesLabeled.GetTexture(),
						bodiesSegmented.GetTexture(), renderCam, currImg);

				// Render final image blend
				X_RenderImageBlend();
				newImages++;
			}

			// Exit if target reached
			if (imageCount + newImages >= renderSettings.GetMaxImageCount())
			{
				X_CleanupScene();
				return newImages;
			}
		}

		// Done rendering
		X_CleanupScene();
	}

	return newImages;
}

//---------------------------------------
// Create new scene manager
//---------------------------------------
SceneManager::SceneManager(
	const Settings* settings,
	const std::vector<Light>& vecLights,
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

	// Load intrinsics (custom or provided ones)
	if (SafeGet<bool>(pRenderSettings->GetJSONConfig(), "custom_intrinsics"))
	{
		renderCam.SetIntrinsics(pRenderSettings->GetIntrinsics());
	}
	else
	{
		Intrinsics fromFile;
		// Load from file
		fromFile.LoadIntrinsics(
			pRenderSettings->GetSceneRGBPath().append("_info.txt"),
			pRenderSettings->GetRenderResolution()
		);
		// Store in camera
		renderCam.SetIntrinsics(fromFile);
	}

	// Create OpenGL renderer
	pRenderer = new Renderer::Render(
		ModifiablePath(SafeGet<const char*>(pRenderSettings->GetJSONConfig(), "shaders_gl")),
		renderCam.GetIntrinsics().GetWidth(), renderCam.GetIntrinsics().GetHeight(),
		renderCam.GetClipping().x(), renderCam.GetClipping().y()
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
}
