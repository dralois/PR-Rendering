#include "SceneManager.h"

#pragma warning(push, 0)
#include <dirent.h>
#pragma warning(pop)

string FILE_TEMP_PATH = "";
string FILE_FINAL_PATH = "";
string FILE_OBJ_PATH = "";

//---------------------------------------
// Splits string into vector of strings
//---------------------------------------
std::vector<std::string> split(const std::string& str, char delimiter)
{
	using namespace std;

	string tok;
	stringstream ss(str);
	vector<string> internal;
	// Push each part into vector
	while (getline(ss, tok, delimiter))
	{
		internal.push_back(tok);
	}
	// Return it
	return internal;
}

//---------------------------------------
// Calculate and saves 2D bounding box
//---------------------------------------
void SetBBox(BodyAnnotation& ann, const cv::Mat& mask)
{
	// Calculate bounding box using the mask
	cv::Rect Min_Rect = cv::boundingRect(mask);
	Min_Rect.x += Min_Rect.width / 2.f;
	Min_Rect.y += Min_Rect.height / 2.f;
	// Save in vector
	ann.vecBBox.push_back(Min_Rect.x);
	ann.vecBBox.push_back(Min_Rect.y);
	ann.vecBBox.push_back(Min_Rect.width);
	ann.vecBBox.push_back(Min_Rect.height);
}

//---------------------------------------
// Create physx scan scene mesh
//---------------------------------------
void SceneManager::X_PxCreateScene()
{
	// Create physx mesh of scan scene
	string path = scenePath + "/mesh.refined.obj";
	pPxMeshScene = new PxMeshTriangle(path, 0, pPxCooking, pPxMaterial);
	pPxMeshScene->SetScale((*CONFIG_FILE)["scene_scale"].GetFloat());
	pPxMeshScene->SetObjId(0);
	pPxMeshScene->CreateMesh();

	// Standart gravity & continuous collision detection
	PxSceneDesc sceneDesc(PxGetPhysics().getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = pPxDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;
	// Objects should never be outside twice the scene bounds
	sceneDesc.sanityBounds = PxBounds3(pPxMeshScene->GetMinimum() * 2.0f, pPxMeshScene->GetMaximum() * 2.0f);

	// Create scene
	pPxScene = PxGetPhysics().createScene(sceneDesc);

	// Scene meshes need to be rotated 90* around X during the simulation
	PxTransform pose(PxVec3(0, 0, 0), PxQuat(-AI_PIOVER2, PxVec3(1, 0, 0)));
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
	vecpPxMeshCurrObjs.reserve(objsPerSim);

	// For each object
	for (PxU32 i = 0; i < objsPerSim; i++)
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
void SceneManager::X_PxDestroy()
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
		PX_RELEASE(pPxScene);
	}
}

//---------------------------------------
// Run physx simulation
//---------------------------------------
void SceneManager::X_PxRunSim(float timestep, int stepCount) const
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
	float undoScale = 1.0f / (*CONFIG_FILE)["scene_scale"].GetFloat();
	// For each physx object
	for (auto obj : vecpPxMeshCurrObjs)
	{
		// Get position
		PxTransform pose = obj->GetTransform();
		// Undo 90* around X rotation
		PxQuat undoRot(AI_PIOVER2, PxVec3(1, 0, 0));
		pose.q = (pose.q * undoRot).getNormalized();
		pose.p = undoRot.rotate(pose.p);
		// Undo scene scaling
		pose.p *= undoScale;

		// Convert physx position & rotation
		PxMat44 pxmat(pose);
		Matrix4f mat(pxmat.front());
		Vector3f pos(pose.p.x, pose.p.y, pose.p.z);
		Quaternionf rot(pose.q.w, pose.q.x, pose.q.y, pose.q.z);

		// Save poses in vector
		ObjectInfo currInfo;
		currInfo.meshId = obj->GetMeshId();
		currInfo.objId = obj->GetObjId();
		currInfo.objName = obj->GetName();
		currInfo.pos = pos;
		currInfo.rot = rot;
		currInfo.mat = mat;
		vecCurrObjs.push_back(currInfo);
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
// Create arnold object meshes
//---------------------------------------
void SceneManager::X_AiCreateObjs()
{
	// For each created object
	for (auto body : vecCurrObjs)
	{
		// Create the mesh & save in vector
		AiMesh* mesh = new AiMesh(*vecpAiMeshObjs[body.meshId]);
		mesh->SetObjId(body.objId);
		mesh->CreateMesh();
		mesh->SetTransform(body.mat);
		vecpAiMeshCurrObjs.push_back(mesh);
	}
}

//---------------------------------------
// Cleanup arnold object meshes
//---------------------------------------
void SceneManager::X_AiDestroy()
{
	// Destroy arnold nodes
	for (auto curr : vecpAiMeshCurrObjs)
	{
		delete curr;
	}
	vecpAiMeshCurrObjs.clear();
}

//---------------------------------------
// Render scene depth
//---------------------------------------
vector<tuple<cv::Mat, cv::Mat> > SceneManager::X_RenderSceneDepth() const
{
	// Render depth with OpenGL
	vector<tuple<cv::Mat, cv::Mat> > renderings =
		pRenderer->RenderScenes(scenePath, vecCameraPoses,
			useCustomIntr ? intrCustom.fx : intrOriginal.fx,
			useCustomIntr ? intrCustom.fy : intrOriginal.fy,
			useCustomIntr ? intrCustom.ox : intrOriginal.ox,
			useCustomIntr ? intrCustom.oy : intrOriginal.oy);

	// For each image
	for (int render_count = 0; render_count < renderings.size(); render_count++)
	{
		ostringstream out;
		out << std::internal << std::setfill('0') << std::setw(6) << render_count;
		string tmp_depth_path = FILE_TEMP_PATH + "/scene_depth/img_" + out.str() + ".png";
		string tmp_rgb_path = FILE_TEMP_PATH + "/rgb/img_" + out.str() + ".png";

		// Write rgb and depth out
		cv::imwrite(tmp_rgb_path, get<0>(renderings[render_count]));
		cv::imwrite(tmp_depth_path, get<1>(renderings[render_count]));
	}

	return renderings;
}

//---------------------------------------
// Saves world position and rotation of annotation
// FIXME: Might be broken now
//---------------------------------------
void SceneManager::X_SaveAnnotationPose(BodyAnnotation& ann, const Vector3f& pos, const Quaternionf& rot) const
{
	// Create pose matrix
	Eigen::Affine3f bTransMat;
	bTransMat = bTransMat.fromPositionOrientationScale(pos, rot, Vector3f::Identity());

	// Camera space -> World space
	bTransMat = matCamera.transpose().inverse().matrix() * bTransMat.matrix();

	// Save position in vector
	ann.vecPos.push_back(bTransMat.translation().x() * 10);
	ann.vecPos.push_back(bTransMat.translation().y() * 10);
	ann.vecPos.push_back(bTransMat.translation().z() * 10);
	// Save rotation in vector
	Quaternionf q = Quaternionf(bTransMat.rotation().matrix());
	ann.vecRot.push_back(q.w());
	ann.vecRot.push_back(q.x());
	ann.vecRot.push_back(q.y());
	ann.vecRot.push_back(q.z());
}

//---------------------------------------
// Write out annotation file for rendered objects
// FIXME: Might be broken now
//---------------------------------------
void SceneManager::X_SaveAnnotations(const cv::Mat& seg, const cv::Mat& segMasked)
{
	// For each mesh
	for (auto currBody : vecCurrObjs)
	{
		// Determine label sum unmasked
		float segBodySum = cv::sum(seg == (currBody.objId + 1) * 10)[0];
		// Stop if object completely covered
		if (segBodySum == 0)
			continue;
		// Determine label sum masked
		float segBodySumMasked = cv::sum(segMasked == (currBody.objId + 1) * 10)[0];
		float percent = segBodySumMasked / segBodySum;
		// Stop if less then 30% coverage
		if (percent <= 0.3 || segBodySumMasked / 255 < 2000)
			continue;

		BodyAnnotation currAnn;
		currAnn.vecPos = std::vector<float>();
		currAnn.vecRot = std::vector<float>();
		currAnn.vecBBox = std::vector<float>();
		currAnn.meshId = currBody.meshId;
		currAnn.labelId = (currBody.objId + 1) * 10;

		// Set bounding box & annotiation
		SetBBox(currAnn, seg == (currBody.objId + 1) * 10);
		X_SaveAnnotationPose(currAnn, currBody.pos, currBody.rot);

		ostringstream out;
		out << std::internal << std::setfill('0') << std::setw(2) << currAnn.meshId;
		string buf = "obj_" + out.str();

		// Add to annotation file
		ANNOTATIONS_FILE << imageCount << ", " << currAnn.vecBBox[0] << ", " << currAnn.vecBBox[1] << ", "
			<< currAnn.vecBBox[2] << ", " << currAnn.vecBBox[3] << ", " << buf << ", " << currAnn.vecRot[0] << ", "
			<< currAnn.vecRot[1] << ", " << currAnn.vecRot[2] << ", " << currAnn.vecRot[3] << ", "
			<< "0"
			<< ", "
			<< "0"
			<< ", " << currAnn.vecPos[0] << ", " << currAnn.vecPos[1] << ", " << currAnn.vecPos[2] << ", "
			<< currAnn.labelId
			<< " [" << intrCustom.fx << ", " << intrCustom.fy << ", " << intrOriginal.ox << ", " << intrOriginal.oy << "]" << "\n";
	}
}

//---------------------------------------
// Check if pose is close to camera center
// FIXME: Improve this?
//---------------------------------------
bool SceneManager::X_CheckIfImageCenter(const ObjectInfo& body) const
{
	// Pose rotation matrix
	Quaternionf q = body.rot;
	Matrix<float, 4, 4> bRotMat = Eigen::Matrix4f::Identity();
	bRotMat.block(0, 0, 3, 3) = q.normalized().toRotationMatrix().cast<float>();

	// Set position
	Vector3f pos = body.pos;
	bRotMat(0, 3) = pos.x();
	bRotMat(1, 3) = pos.y();
	bRotMat(2, 3) = pos.z();

	// Transform pose into camera space
	bRotMat = matCamera.transpose().inverse() * bRotMat;

	// Some kind of camera to screen space transform
	int width = (useCustomIntr ? intrCustom.w : intrOriginal.w) / 2;
	int height = (useCustomIntr ? intrCustom.h : intrOriginal.h) / 2;
	float x = width / 2.0f + bRotMat(0, 3) * 756 / bRotMat(2, 3);
	float y = height / 2.0f + bRotMat(1, 3) * 756 / bRotMat(2, 3);
	x += 20;
	y += 20;

	// Determine if object center at least 20 pixels from the egde
	return (x > 40 && x < width&& y > 40 && y < height);
}

//---------------------------------------
// Render objects depth
//---------------------------------------
bool SceneManager::X_RenderObjsDepth()
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

	return valid;
}

//---------------------------------------
// Determine depth mask
//---------------------------------------
bool SceneManager::X_CvComputeObjsMask()
{
	ostringstream out;
	out << std::internal << std::setfill('0') << std::setw(6) << poseCount;
	string sbuf = FILE_TEMP_PATH + "/scene_depth/img_" + out.str() + ".png";
	string bbuf = FILE_TEMP_PATH + "/body_depth/img_" + out.str() + ".png";

	// Read depth images
	cvSceneD = cv::imread(sbuf, cv::IMREAD_ANYDEPTH);
	cvBodiesD = cv::imread(bbuf, cv::IMREAD_ANYDEPTH);

	// Create mask
	cvMask = cvBodiesD <= cvSceneD;

	// Return if object visible
	cv::Scalar maskMean = cv::mean(cvMask);
	return maskMean[0] >= 1.f;
}

//---------------------------------------
// Render object label image (IDs as color)
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
// Combine scene and object depth images
//---------------------------------------
void SceneManager::X_CvBlendDepth()
{
	cv::Mat cvOut;

	ostringstream out;
	out << std::internal << std::setfill('0') << std::setw(6) << poseCount;
	string sbuf = FILE_TEMP_PATH + "/scene_depth/img_" + out.str() + ".png";
	string bbuf = FILE_TEMP_PATH + "/body_depth/img_" + out.str() + ".png";

	// Calculate minimal depth from scene and objects
	cvSceneD = cv::imread(sbuf, cv::IMREAD_ANYDEPTH);
	cvBodiesD = cv::imread(bbuf, cv::IMREAD_ANYDEPTH);
	cvOut = cv::min(cvSceneD, cvBodiesD);

	// Temp file
	ostringstream out_start;
	out_start << std::internal << std::setfill('0') << std::setw(6) << imageCount;

	// Write out blended image
	string obuf = FILE_FINAL_PATH + "/depth/img_" + out_start.str() + ".png";
	cv::imwrite(obuf, cvOut);
}

//---------------------------------------
// Combine mask and label images
//---------------------------------------
void SceneManager::X_CvBlendLabel()
{
	cv::Mat cvOut;

	// Temp file
	ostringstream out;
	out << std::internal << std::setfill('0') << std::setw(6) << poseCount;

	ostringstream out_start;
	out_start << std::internal << std::setfill('0') << std::setw(6) << imageCount;
	string bbuf = FILE_TEMP_PATH + "/body_label/img_" + out.str() + ".png";
	string obuf = FILE_FINAL_PATH + "/segs/img_" + out_start.str() + ".png";

	// Read label image
	cvBodiesS = cv::imread(bbuf, cv::IMREAD_ANYDEPTH);

	// Write out masked image
	cvBodiesS.copyTo(cvOut, cvMask);
	cv::imwrite(obuf, cvOut);

	// Write out annotation file
	X_SaveAnnotations(cvBodiesS, cvOut);
}

//---------------------------------------
// Final image blend
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
// Loads camera matrix with fov
//---------------------------------------
void SceneManager::X_LoadCameraExtrinsics(const Intrinsics& intr)
{
	std::ifstream inFile;

	// Setup camera
	AiNodeSetStr(aiCamera, "name", "renderCam");

	// Open camera pose file & save as matrix
	inFile.open(vecCameraPoses.at(poseCount));
	if (inFile.is_open())
	{
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				inFile >> matCamera(i, j);
		inFile.close();
	}

	// Calculate & save view vectors
	Vector3f forward = matCamera.block<3, 3>(0, 0) * Eigen::Vector3f(0, 0, 1);
	Vector3f right = matCamera.block<3, 3>(0, 0) * Eigen::Vector3f(1, 0, 0);
	Vector3f up = right.cross(forward);
	Vector3f position = matCamera.block<3, 1>(0, 3);
	Vector3f lookat = position + 1 * forward;
	AiNodeSetVec(aiCamera, "position", position.x(), position.y(), position.z());
	AiNodeSetVec(aiCamera, "look_at", lookat.x(), lookat.y(), lookat.z());
	AiNodeSetVec(aiCamera, "up", up.x(), up.y(), up.z());

	// Save clipping planes
	AiNodeSetFlt(aiCamera, "near_clip", 0.1f);
	AiNodeSetFlt(aiCamera, "far_clip", 10.0f);

	// Calculate & save fov
	float fovx = 2.0f * atan(intr.w / (4.0f * intr.fx)) * (180.0f / AI_PI);
	float fovy = 2.0f * atan(intr.h / (4.0f * intr.fy)) * (180.0f / AI_PI);
	AtArray* fovArr = AiArray(2, 1, AI_TYPE_FLOAT, fovx, fovy);
	AiNodeSetArray(aiCamera, "fov", fovArr);

	// Calculate & save lens shift
	float shiftx = ((4.0f * intr.ox) - intr.w) / intr.w;
	float shifty = ((4.0f * intr.oy) - intr.h) / intr.h;
	AtArray* shiftArr = AiArray(2, 1, AI_TYPE_FLOAT, shiftx, shifty);
	AiNodeSetArray(aiCamera, "lens_shift", shiftArr);
}

//---------------------------------------
// Loads camera intrinsics
//---------------------------------------
void SceneManager::X_LoadCameraIntrinsics()
{
	std::string line;
	std::ifstream inFile;

	// Input file
	string buf = scenePath + "/rgbd/_info.txt";

	// Try to open
	inFile.open(buf);
	if (!inFile.is_open())
		return;

	// For each line
	while (std::getline(inFile, line))
	{
		// If it contains intrinsics
		if (line.find("m_calibrationColorIntrinsic") != std::string::npos)
		{
			std::vector<std::string> entries = split(line, ' ');
			// Save them
			intrOriginal.fx = std::stof(entries[2]);
			intrOriginal.fy = std::stof(entries[7]);
			intrOriginal.ox = std::stof(entries[4]);
			intrOriginal.oy = std::stof(entries[8]);
			intrOriginal.h = (*CONFIG_FILE)["render_height"].GetInt();
			intrOriginal.w = (*CONFIG_FILE)["render_width"].GetInt();
			break;
		}
	}
}

//---------------------------------------
// Filter out blurry images (based on variance)
//---------------------------------------
float SceneManager::X_CvComputeImageVariance(const cv::Mat& image) const
{
	cv::Mat gray;
	cv::Mat laplacianImage;
	cv::Scalar mean, stddev;
	// Convert to grayscale 
	cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
	// Create laplacian and calculate deviation
	cv::Laplacian(gray, laplacianImage, CV_64F);
	cv::meanStdDev(laplacianImage, mean, stddev, cv::Mat());
	// Return variance
	return stddev.val[0] * stddev.val[0];
}

//---------------------------------------
// Determine images to process
//---------------------------------------
void SceneManager::X_GetImagesToProcess(const string& path, float varThreshold)
{
	DIR* dir;
	struct dirent* ent;

	// Init vectors
	vecCameraImages = std::vector<std::string>();
	vecCameraPoses = std::vector<std::string>();

	// Load files
	if ((dir = opendir(path.c_str())) != NULL)
	{
		// For each file
		while ((ent = readdir(dir)) != NULL)
		{
			string buf = path + "/" + ent->d_name;
			// If has rgb image
			if (buf.find("color") != std::string::npos)
			{
				// Compute variance
				cv::Mat image = cv::imread(buf);
				float variance = X_CvComputeImageVariance(image);
				// Skip blurry images
				if (variance > varThreshold)
				{
					cout << "Using image " << ent->d_name << endl;
					// Save image in vector
					vecCameraImages.push_back(buf);
					string cam_file = buf;
					cam_file.replace(cam_file.find("color.jpg"), sizeof("color.jpg") - 1, "pose.txt");
					// Save pose file in vector
					vecCameraPoses.push_back(cam_file);
				}
			}
		}
		// Finally sort vectors and close dir
		std::sort(vecCameraPoses.begin(), vecCameraPoses.end());
		std::sort(vecCameraImages.begin(), vecCameraImages.end());
		closedir(dir);
	}
}

//---------------------------------------
// Run simulation
//---------------------------------------
bool SceneManager::Run(int sceneIters, int maxImages)
{
	// Load camera intrinsics (fov)
	X_LoadCameraIntrinsics();

	// Get non blurry images
	X_GetImagesToProcess(scenePath + "/rgbd", 400.f);
	if (vecCameraImages.size() <= 0)
		return true;

	// Render scene depth with OpenGL
	vector<tuple<cv::Mat, cv::Mat> >  renderings = X_RenderSceneDepth();

	// For each scene iteration
	for (int iter = 0; iter < sceneIters; iter++)
	{
		// Create physx representation of scan scene
		X_PxCreateScene();
		// Create physx objects
		X_PxCreateObjs();
		// Run the simulation
		X_PxRunSim(1.0f / 50.0f, 400);
		// Save results
		X_PxSaveSimResults();
		// Create arnold meshes
		X_AiCreateObjs();

		// Create/open annotation file
		string buf = FILE_FINAL_PATH + "/labels.csv";
		ANNOTATIONS_FILE.open(buf, std::ios_base::app);

		// For each camera pose
		for (poseCount = 0; poseCount < vecCameraPoses.size(); poseCount++)
		{
			// Load camera matrix for pose
			if(useCustomIntr)
				X_LoadCameraExtrinsics(intrCustom);
			else
				X_LoadCameraExtrinsics(intrOriginal);

			// If object depths rendered and visible
			if (X_RenderObjsDepth() && X_CvComputeObjsMask())
			{
				// Render object labels (IDs as color)
				X_RenderObjsLabel();
				// Blend depth images
				X_CvBlendDepth();
				// Blend label and mask images
				X_CvBlendLabel();

				// Read scene color image
				cvScene = cv::imread(vecCameraImages.at(poseCount), cv::IMREAD_COLOR);
				cvRend = get<0>(renderings[poseCount]);
				cv::resize(cvRend, cvRend, cv::Size(cvScene.cols, cvScene.rows));

				// Final image blend
				X_RenderImageBlend();
				imageCount++;
			}

			// Exit if target reached
			if (imageCount >= maxImages)
			{
				// Cleanup
				X_PxDestroy();
				X_AiDestroy();
				vecCurrObjs.clear();
				ANNOTATIONS_FILE.close();
				return false;
			}
		}

		// Cleanup
		X_PxDestroy();
		X_AiDestroy();
		vecCurrObjs.clear();
		ANNOTATIONS_FILE.close();

		return false;
	}

	return true;
}

//---------------------------------------
// Create new scene manager
//---------------------------------------
SceneManager::SceneManager(PxCpuDispatcher* pPxDispatcher, const PxCooking* pPxCooking, const PxMaterial* pPxMaterial,
	AtNode* aiCamera, AtNode* aiOptions, AtNode* aiDriver, AtArray* aiOutputArray,
	const vector<PxMeshConvex*> vecPhysxObjs, const vector<AiMesh*> vecArnoldObjs,
	int startCount, int objPerSim, const rapidjson::Document* CONFIG_FILE,
	AtNode* aiShaderObjDepth, AtNode* aiShaderSceneDepth, AtNode* aiShaderBlend) :
	pPxDispatcher(pPxDispatcher), pPxCooking(pPxCooking), pPxMaterial(pPxMaterial),
	aiCamera(aiCamera), aiOptions(aiOptions), aiDriver(aiDriver), aiArrOutputs(aiOutputArray),
	vecpPxMeshObjs(vecPhysxObjs), vecpAiMeshObjs(vecArnoldObjs),
	imageCount(startCount), objsPerSim(objPerSim), CONFIG_FILE(CONFIG_FILE),
	aiShaderDepthObj(aiShaderObjDepth), aiShaderDepthScene(aiShaderSceneDepth), aiShaderBlendImage(aiShaderBlend),
	pPxScene(NULL), pAiMeshScene(NULL), pPxMeshScene(NULL), poseCount(0), intrCustom(), intrOriginal()
{
	// Fetch path to output location and meshes
	FILE_OBJ_PATH = (*CONFIG_FILE)["models"].GetString();
	FILE_TEMP_PATH = (*CONFIG_FILE)["temp_files_path"].GetString();
	FILE_FINAL_PATH = (*CONFIG_FILE)["final_imgs_path"].GetString();

	// Load intrinsics
	intrCustom.fx = (*CONFIG_FILE)["fx"].GetFloat();
	intrCustom.fy = (*CONFIG_FILE)["fy"].GetFloat();
	intrCustom.ox = (*CONFIG_FILE)["ox"].GetFloat();
	intrCustom.oy = (*CONFIG_FILE)["oy"].GetFloat();
	intrCustom.w = (*CONFIG_FILE)["render_width"].GetInt();
	intrCustom.h = (*CONFIG_FILE)["render_height"].GetInt();

	// Determine if intrinsics are used
	useCustomIntr = (*CONFIG_FILE)["use_custom_intrinsics"].GetBool();

	// Load clipping planes
	nearClip = (*CONFIG_FILE)["clip_near"].GetFloat();
	farClip = (*CONFIG_FILE)["clip_far"].GetFloat();

	// Create renderer
	pRenderer = new Renderer::Render((*CONFIG_FILE)["shaders_gl"].GetString(),
		intrCustom.w, intrCustom.h, nearClip, farClip);
}

//---------------------------------------
// Cleanup scene
//---------------------------------------
SceneManager::~SceneManager()
{
	X_AiDestroy();
	X_PxDestroy();
	delete pRenderer;
}
