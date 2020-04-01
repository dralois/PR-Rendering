#include "SceneManager.h"

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
// Create new scene manager
//---------------------------------------
SceneManager::SceneManager(PxScene* pPxScene, PxCooking* pPxCooking, PxMaterial* pPxMaterial,
	AtNode* aiCamera, AtNode* aiOptions, AtNode* aiDriver, AtArray* aiOutputArray,
	vector<PxMeshConvex*> vecPhysxObjs, vector<AiMesh*> vecArnoldObjs,
	int startCount, int objPerSim, rapidjson::Document* CONFIG_FILE,
	AtNode* aiShaderObjDepth, AtNode* aiShaderSceneDepth, AtNode* aiShaderBlend) :
	pPxScene(pPxScene), pPxCooking(pPxCooking), pPxMaterial(pPxMaterial),
	aiCamera(aiCamera), aiOptions(aiOptions), aiDriver(aiDriver), aiArrOutputs(aiOutputArray),
	vecpPxMeshObjs(vecPhysxObjs), vecpAiMeshObjs(vecArnoldObjs),
	imageCount(startCount), objsPerSim(objPerSim), CONFIG_FILE(CONFIG_FILE),
	aiShaderDepthObj(aiShaderObjDepth), aiShaderDepthScene(aiShaderSceneDepth), aiShaderBlendImage(aiShaderBlend),
	intrOriginal(), pPxActorScene(NULL), pAiMeshScene(NULL), pPxMeshScene(NULL), poseCount(0)
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

	// Create renderer
	pRenderer = new Renderer::Render((*CONFIG_FILE)["shaders_gl"].GetString(), 1920, 1080);
}

//---------------------------------------
// Cleanup scene
//---------------------------------------
SceneManager::~SceneManager()
{
	delete pRenderer;
	// Scene physx cleanup
	if (pPxMeshScene != NULL)
	{
		delete pPxMeshScene;
	}
}

//---------------------------------------
// Create physx representation of scan scene
//---------------------------------------
void SceneManager::X_PxCreateScene()
{
	string mesh_path = scenePath + "/mesh.refined.obj";
	// Create physx mesh, shape and rigidbody of scan scene
	pPxMeshScene = new PxMeshTriangle(mesh_path, 0, 0, pPxScene, pPxCooking, pPxMaterial);
	pPxMeshScene->CreateMesh(true, (*CONFIG_FILE)["scene_scale"].GetFloat());
	// Mesh needs to be rotated 90* around X for the simulation
	PxVec3 pos(0, 0, 0);
	PxQuat rot(-AI_PIOVER2, PxVec3(1, 0, 0));
	pPxActorScene = (PxRigidStatic*) pPxMeshScene->AddRigidActor(pos, rot);
}

//---------------------------------------
// Create random physx objects to drop into scene
//---------------------------------------
void SceneManager::X_PxCreateObjs()
{
	// Seed & setup
	srand(time(NULL) % 1000);
	vecpPxActorCurrObjs.reserve(objsPerSim);

	// For each object
	for (PxU32 i = 0; i < objsPerSim; i++)
	{
		// Fetch random object
		int currPos = rand() % vecpPxMeshObjs.size();
		PxMeshConvex* currObj = new PxMeshConvex(*vecpPxMeshObjs.at(currPos));
		currObj->SetObjId(i);

		// Random position, same rotation
		float y = (rand() % 10) + 1;
		float x = (rand() % ((int)(pPxMeshScene->GetXMax() - pPxMeshScene->GetXMin()))) + pPxMeshScene->GetXMin();
		float z = (rand() % ((int)(pPxMeshScene->GetYMax() - pPxMeshScene->GetYMin()))) + pPxMeshScene->GetYMin();
		PxVec3 pos(x, y, z);
		PxQuat rot(-0.7071068, 0, 0, 0.7071068);

		// Save rigidbody and mesh in vector
		PxRigidActor* body = currObj->AddRigidActor(pos, rot);
		vecpPxActorCurrObjs.push_back(make_pair(currObj, (PxRigidDynamic*)body));
	}
}

//---------------------------------------
// Cleanup physx meshes
//---------------------------------------
void SceneManager::X_PxDestroy()
{
	// Cleanup scene
	if (pPxMeshScene != NULL)
	{
		PxMesh::DestroyRigidbody(pPxActorScene);
		delete pPxMeshScene;
	}
	// Cleanup random objects
	for (auto obj : vecpPxActorCurrObjs)
	{
		PxMesh::DestroyRigidbody(obj.second);
		delete obj.first;
	}
	// As well as vector
	vecpPxActorCurrObjs.clear();
}

//---------------------------------------
// Run physx simulation
//---------------------------------------
void SceneManager::X_PxRunSim()
{
	// Simulate in 16ms steps
	for (PxU32 i = 0; i < 300; i++)
	{
		pPxScene->simulate(1.0f / 30.0f);
		pPxScene->fetchResults(true);

	}
	// Save results
	X_PxSaveSimResults();
}

//---------------------------------------
// Fetch and save simulation results
//---------------------------------------
void SceneManager::X_PxSaveSimResults()
{
	int i = 0;
	PxTransform tempTrans;
	// For each physx object
	for (auto obj : vecpPxActorCurrObjs)
	{
		// Get position
		tempTrans = obj.second->getGlobalPose();
		// Create pose struct
		ObjectInfo currInfo;
		// Undo 90* around X rotation
		PxQuat undo(AI_PIOVER2, PxVec3(1, 0, 0));
		tempTrans.q *= undo;
		tempTrans.p = undo.rotate(tempTrans.p);
		// Fetch and convert physx position & rotation
		PxQuat pxRot = tempTrans.q.getNormalized();
		PxVec3 pxPos = tempTrans.p;
		Vector3f pos(pxPos.x, pxPos.z, pxPos.y);
		Quaternionf rot(pxRot.w, pxRot.x, pxRot.y, pxRot.z);
		// Save poses in vector
		currInfo.meshId = obj.first->GetMeshId();
		currInfo.objId = obj.first->GetObjId();
		currInfo.objName = obj.first->GetName();
		currInfo.pos = pos;
		currInfo.rot = rot;
		vecCurrObjs.push_back(currInfo);
#if DEBUG || _DEBUG
		// Object transform update for pvd
		obj.second->setGlobalPose(tempTrans, false);
	}
	// Scene transform update for pvd
	pPxActorScene->setGlobalPose(PxTransform(PxIDENTITY::PxIdentity));
	// Simulate once for pvd
	pPxScene->simulate(0.1f);
	pPxScene->fetchResults(true);
#else
}
#endif
}

//---------------------------------------
// Create arnold meshes
//---------------------------------------
void SceneManager::X_AiCreateObjs()
{
	// For each created object
	for (auto body : vecCurrObjs)
	{
		// Fetch pose
		Vector3f p = body.pos;
		Quaternionf q = body.rot;
		vector<float> pos{ p.x(), p.y(), p.z() };
		vector<float> rot{ q.w(), q.x(), q.y(), q.z() };

		// Create the mesh & save in vector
		AiMesh* mesh = new AiMesh(*vecpAiMeshObjs[body.meshId]);
		mesh->SetObjId(body.objId);
		mesh->CreateMesh(pos, rot, (*CONFIG_FILE)["obj_scale"].GetFloat());
		vecpAiMeshCurrObj.push_back(mesh);
	}
}

//---------------------------------------
// Cleanup arnold meshes
//---------------------------------------
void SceneManager::X_AiDestroy()
{
	// Destroy nodes
	for (auto curr : vecCurrObjs)
	{
		AiMesh::DestroyMesh(curr.objName);
	}
	// Destroy meshes
	for (auto curr : vecpAiMeshCurrObj)
	{
		delete curr;
	}
	// Cleanup vector
	vecpAiMeshCurrObj.clear();
}

//---------------------------------------
// Render scene depth
//---------------------------------------
vector<tuple<cv::Mat, cv::Mat> > SceneManager::X_RenderSceneDepth() const
{
	// Render depth with OpenGL
	vector<tuple<cv::Mat, cv::Mat> > renderings =
		pRenderer->RenderScenes(scenePath, vecCameraPoses,
			intrOriginal.fx, intrOriginal.fy,
			intrOriginal.ox, intrOriginal.oy);

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

		cv::waitKey(10);
	}

	return renderings;
}

//---------------------------------------
// Saves world position and rotation of annotation
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
	int width = 960;
	int height = 540;
	float x = width / 2.f + bRotMat(0, 3) * 756 / bRotMat(2, 3);
	float y = height / 2.f + bRotMat(1, 3) * 756 / bRotMat(2, 3);
	x += 20;
	y += 20;

	// Determine if object center at least 20 pixels from the egde
	return (x > 40 && x < width && y > 40 && y < height);
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
	AiNodeSetInt(aiOptions, "xres", 1920);
	AiNodeSetInt(aiOptions, "yres", 1080);
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
	AiNodeSetInt(aiOptions, "xres", 1920);
	AiNodeSetInt(aiOptions, "yres", 1080);
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

		// Image node and object material (?)
		AtNode* shaderMaterial = AiNode("standard");
		AtNode* image = AiNode("image");

		// Texture path
		ostringstream out;
		out << std::internal << std::setfill('0') << std::setw(2) << (body.meshId);
		string imgbuffer = "obj_" + out.str() + "_color.png";
		string imgPath = FILE_OBJ_PATH + "/" + imgbuffer;

		// Setup image node
		AiNodeSetStr(image, "filename", imgPath.c_str());
		AiNodeLink(image, "Kd_color", shaderMaterial);

		// Setup blending shader
		AtNode* shader_blend = AiNode("blendshader");
		AiNodeSetStr(shader_blend, "name", buffer.c_str());
		AiNodeSetPtr(shader_blend, "mask", (void*)&cvMask);
		AiNodeSetPtr(shader_blend, "blend_image", (void*)&cvScene);
		AiNodeSetPtr(shader_blend, "rend_image", (void*)&cvRend);
		AiNodeSetBool(shader_blend, "force_scene", false);
		// Setup object material
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
	AiNodeSetInt(aiOptions, "xres", 960);
	AiNodeSetInt(aiOptions, "yres", 540);
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
void SceneManager::X_LoadCameraMatrix(float fx, float fy, float ox, float oy)
{
	std::string line;
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

	// Extract rotation & position from extrinsics
	matCamera.transposeInPlace();
	Matrix3f rotExt = matCamera.block<3, 3>(0, 0).transpose();
	Vector3f posExt = -100.0f * rotExt * matCamera.block<3, 1>(0, 3);

	// Convert
	AtMatrix aiMatRot = AiM4Identity();
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			aiMatRot[i][j] = rotExt(j, i);
		}
	}

	// Save in camera
	AtVector pos(posExt.x(), posExt.y(), posExt.z());
	AtMatrix aiMatPos = AiM4Translation(pos);
	AtMatrix aiMatTrans = AiM4Mult(aiMatRot, aiMatPos);
	AiNodeSetMatrix(aiCamera, "matrix", aiMatTrans);

	// Calculate fov and save
	float fovx = 2.0f * atan(960.0f / (2.0f * fx)) * (180.0f / AI_PI);
	float fovy = 2.0f * atan(540.0f / (2.0f * fy)) * (180.0f / AI_PI);
	AtArray* fovArr = AiArray(2, 1, AI_TYPE_FLOAT, fovx, fovy);
	AiNodeSetArray(aiCamera, "fov", fovArr);

	// Calculate NDC coordinates
	float min_x = (ox - (960 / 2)) / (960 / 2) - 1;
	float max_x = (ox - (960 / 2)) / (960 / 2) + 1;
	float min_y = (oy - (540 / 2)) / (540 / 2) - 1;
	float max_y = (oy - (540 / 2)) / (540 / 2) + 1;

	// Save
	AiNodeSetVec2(aiCamera, "screen_window_min", min_x, min_y);
	AiNodeSetVec2(aiCamera, "screen_window_max", max_x, max_y);
}

//---------------------------------------
// Loads camera intrinsics
//---------------------------------------
void SceneManager::X_LoadSceneIntrinsics()
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
	X_LoadSceneIntrinsics();

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
		X_PxRunSim();
		// Create arnold meshes
		X_AiCreateObjs();

		// Create/open annotation file
		string buf = FILE_FINAL_PATH + "/labels.csv";
		ANNOTATIONS_FILE.open(buf, std::ios_base::app);

		// For each camera pose
		for (poseCount = 0; poseCount < vecCameraPoses.size(); poseCount++)
		{
			// Load camera matrix with custom intrinsics
			X_LoadCameraMatrix(intrCustom.fx, intrCustom.fy, intrCustom.ox, intrCustom.oy);

			// If rendering successful and objects visible
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
	}

	return true;
}
