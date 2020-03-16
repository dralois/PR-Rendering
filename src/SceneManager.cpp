#include "SceneManager.h"

string FILE_TEMP_PATH = "";
string FILE_FINAL_PATH = "";
string FILE_OBJ_PATH = "";

//---------------------------------------
// Splits string into vector of strings
//---------------------------------------
std::vector<std::string> split(std::string str, char delimiter)
{
	using namespace std;

	vector<string> internal;
	stringstream ss(str);
	string tok;

	while (getline(ss, tok, delimiter))
	{
		internal.push_back(tok);
	}

	return internal;
}

//---------------------------------------
// Calculate and saves 2D bounding box
//---------------------------------------
void setBBox(BodyAnnotation& ann, const cv::Mat& mask)
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
	pScene(pPxScene), pCooking(pPxCooking), pMaterial(pPxMaterial),
	aiCamera(aiCamera), aiOptions(aiOptions), aiDriver(aiDriver), aiArrOutputs(aiOutputArray),
	vecpPhysxObjs(vecPhysxObjs), vecpArnoldObjs(vecArnoldObjs),
	startCount(startCount), objsPerSim(objPerSim), CONFIG_FILE(CONFIG_FILE),
	aiShaderDepthObj(aiShaderObjDepth), aiShaderDepthScene(aiShaderSceneDepth), aiShaderBlendImage(aiShaderBlend),
	camIntrinsicScene(), pSceneRigidbody(NULL), pArnoldScene(NULL), pPhysxScene(NULL), sceneCount(0)
{
	// Fetch path to output location and meshes
	FILE_OBJ_PATH = (*CONFIG_FILE)["models"].GetString();
	FILE_TEMP_PATH = (*CONFIG_FILE)["temp_files_path"].GetString();
	FILE_FINAL_PATH = (*CONFIG_FILE)["final_imgs_path"].GetString();

	// Load intrinsics
	camIntrinsicsRender.fx = (*CONFIG_FILE)["fx"].GetFloat();
	camIntrinsicsRender.fy = (*CONFIG_FILE)["fy"].GetFloat();
	camIntrinsicsRender.ox = (*CONFIG_FILE)["ox"].GetFloat();
	camIntrinsicsRender.oy = (*CONFIG_FILE)["oy"].GetFloat();

	// Create renderer
	pRenderer = new Render((*CONFIG_FILE)["shaders_gl"].GetString());
}

//---------------------------------------
// Cleanup scene
//---------------------------------------
SceneManager::~SceneManager()
{
	delete pRenderer;
	// Scene physx cleanup
	if (pPhysxScene != NULL)
	{
		delete pPhysxScene;
	}
}

//---------------------------------------
// Create physx representation of scan scene
//---------------------------------------
void SceneManager::X_PxCreateScene()
{
	string mesh_path = scenePath + "/mesh.refined.obj";
	// Create physx mesh, shape and rigidbody of scan scene
	pPhysxScene = new PxMeshTriangle(mesh_path, 0, 100, pScene, pCooking, pMaterial);
	pPhysxScene->CreateMesh(true, false);
	vector<float> pos{ 0, 0, 0 };
	vector<float> rot{ 0, 0, 0, 1 };
	pPhysxScene->CreateRigidbody(pos, rot);
}

//---------------------------------------
// Create random physx objects to drop into scene
//---------------------------------------
void SceneManager::X_PxCreateObjs()
{
	srand(time(NULL) % 1000);
	// For each object
	for (PxU32 i = 0; i < objsPerSim; i++)
	{
		// Fetch random object
		int objManagerPos = rand() % vecpPhysxObjs.size();
		PxMeshConvex* objManager = vecpPhysxObjs.at(objManagerPos);
		// Random position, same rotation
		float y = (rand() % 10) + 1;
		float x = (rand() % ((int)(pPhysxScene->GetXMax() - pPhysxScene->GetXMin()))) + pPhysxScene->GetXMin();
		float z = (rand() % ((int)(pPhysxScene->GetYMax() - pPhysxScene->GetYMin()))) + pPhysxScene->GetYMin();
		vector<float> pos{ x, y, z };
		vector<float> rot{ -0.7071068, 0, 0, 0.7071068 };

		// Save rigidbody and mesh in vector
		PxRigidActor* body = objManager->CreateRigidbody(pos, rot);
		dicObjsRigidbodies.push_back(make_pair(objManager, (PxRigidDynamic*)body));
	}
}

//---------------------------------------
// Cleanup physx meshes
//---------------------------------------
void SceneManager::X_PxDestroy()
{
	// Cleanup scene mesh
	if (pPhysxScene != nullptr)
		delete pPhysxScene;
	// Cleanup random objects
	for (auto obj : dicObjsRigidbodies)
	{
		PxMesh::DestroyRigidbody(obj.second);
	}
	// As well as vector
	dicObjsRigidbodies.clear();
}

//---------------------------------------
// Run physx simulation
//---------------------------------------
void SceneManager::X_PxRunSim()
{
	// 10000 steps
	for (PxU32 i = 0; i < 2000; i++)
	{
		pScene->simulate(2.f / 60.0f);
		pScene->fetchResults(true);

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
	for (auto obj : dicObjsRigidbodies)
	{
		// Get position
		tempTrans = obj.second->getGlobalPose();
		// Create pose struct
		ObjectInfo currInfo;
		// Fetch and convert physx position & rotation
		PxQuat pxRot = tempTrans.q.getNormalized();
		PxVec3 pxPos = tempTrans.p;
		Vector3f pos(pxPos.x, pxPos.y, pxPos.z);
		Quaterniond rot(pxRot.w, pxRot.x, pxRot.y, pxRot.z);
		// Save poses in vector
		currInfo.shapeId = obj.first->GetMeshId();
		currInfo.objSimId = i++;
		currInfo.pos = pos;
		currInfo.rot = rot;
		currInfo.name = "body" + to_string(currInfo.shapeId) + "_" + to_string(currInfo.objSimId);
		vecCurrObjs.push_back(currInfo);
	}
}

//---------------------------------------
// Render scene depth
//---------------------------------------
vector<tuple<cv::Mat, cv::Mat> > SceneManager::X_RenderSceneDepth() const
{
	// Render depth with OpenGL
	vector<tuple<cv::Mat, cv::Mat> > renderings =
		pRenderer->render_scenes(scenePath, vecCameraPoses,
			camIntrinsicScene.fx, camIntrinsicScene.fy,
			camIntrinsicScene.ox, camIntrinsicScene.oy);

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
void SceneManager::X_SaveAnnotationPose(BodyAnnotation& ann, const Vector3f& pos, const Quaterniond& rot) const
{
	// Create pose matrix
	Matrix<float, 4, 4> bRotMat = Eigen::Matrix4f::Identity();
	bRotMat.block(0, 0, 3, 3) = rot.normalized().toRotationMatrix().cast<float>();
	bRotMat.block(3, 0, 1, 3) = pos;

	// Camera space -> World space
	bRotMat = camMat.transpose().inverse() * bRotMat;

	// Normalize rotation
	Matrix3f temp = bRotMat.block(0, 0, 3, 3);
	Quaternionf qr = Quaternionf(temp).normalized();

	// Save position in vector
	ann.vecPos.push_back(bRotMat(0, 3) * 10);
	ann.vecPos.push_back(bRotMat(1, 3) * 10);
	ann.vecPos.push_back(bRotMat(2, 3) * 10);
	// Save rotation in vector
	ann.vecRot.push_back(qr.w());
	ann.vecRot.push_back(qr.x());
	ann.vecRot.push_back(qr.y());
	ann.vecRot.push_back(qr.z());
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
		float segBodySum = cv::sum(seg == (currBody.objSimId + 1) * 10)[0];
		// Stop if object completely covered
		if (segBodySum == 0)
			continue;
		// Determine label sum masked
		float segBodySumMasked = cv::sum(segMasked == (currBody.objSimId + 1) * 10)[0];
		float percent = segBodySumMasked / segBodySum;
		// Stop if less then 30% coverage
		if (percent <= 0.3 || segBodySumMasked / 255 < 2000)
			continue;

		BodyAnnotation currAnn;
		currAnn.vecPos = std::vector<float>();
		currAnn.vecRot = std::vector<float>();
		currAnn.vecBBox = std::vector<float>();
		currAnn.shapeId = currBody.shapeId;
		currAnn.objId = (currBody.objSimId + 1) * 10;

		// Set bounding box & annotiation
		setBBox(currAnn, seg == (currBody.objSimId + 1) * 10);
		X_SaveAnnotationPose(currAnn, currBody.pos, currBody.rot);

		ostringstream out;
		out << std::internal << std::setfill('0') << std::setw(2) << currAnn.shapeId;
		string buf = "obj_" + out.str();

		// Add to annotation file
		ANNOTATIONS_FILE << startCount << ", " << currAnn.vecBBox[0] << ", " << currAnn.vecBBox[1] << ", "
			<< currAnn.vecBBox[2] << ", " << currAnn.vecBBox[3] << ", " << buf << ", " << currAnn.vecRot[0] << ", "
			<< currAnn.vecRot[1] << ", " << currAnn.vecRot[2] << ", " << currAnn.vecRot[3] << ", "
			<< "0"
			<< ", "
			<< "0"
			<< ", " << currAnn.vecPos[0] << ", " << currAnn.vecPos[1] << ", " << currAnn.vecPos[2] << ", "
			<< currAnn.objId
			<< " [" << camIntrinsicsRender.fx << ", " << camIntrinsicsRender.fy << ", " << camIntrinsicScene.ox << ", " << camIntrinsicScene.oy << "]" << "\n";
	}
}

//---------------------------------------
// Create arnold meshes
//---------------------------------------
void SceneManager::X_AiCreateObjs()
{
	// FixMe: Possible memory leaks here
	vecpArnoldObjs.clear();
	vecpArnoldObjs = {};
	for (int i = 0; i < vecpPhysxObjs.size() + 1; i++)
		vecpArnoldObjs.push_back(NULL);

	// Threads
	vector<void*> vecThreads;
	vector<AiMeshInput> vecThreadData;

	// For each created object
	for (auto body : vecCurrObjs)
	{
		// Create arnold mesh
		AiMesh* mesh = vecpArnoldObjs.at(body.shapeId);
		if (vecpArnoldObjs.at(body.shapeId) == NULL)
		{
			ostringstream out;
			out << std::internal << std::setfill('0') << std::setw(2) << (body.shapeId);
			string buf = FILE_OBJ_PATH + "/obj_" + out.str() + ".obj";
			// Load arnold mesh accordingly
			mesh = new AiMesh(buf, body.shapeId, 0.1);
			vecpArnoldObjs.at(body.shapeId) = mesh;
		}

		// Set position and rotation accordingly
		Vector3f p = body.pos;
		Quaterniond q = body.rot;
		vector<float> pos{ p.x(), p.y(), p.z() };
		vector<float> rot{ (float)q.x(), (float)q.y(), (float)q.z(), (float)q.w() };

		// Create thread input
		AiMeshInput input;
		input.pAiMesh = mesh;
		input.objSimId = body.objSimId;
		input.pos = pos;
		input.rot = rot;

		// Save in vector
		vecThreadData.push_back(input);

		// Create thread and save in vector
		vecThreads.push_back(AiThreadCreate(mesh->CreateMeshThread, &vecThreadData.back(), AI_PRIORITY_NORMAL));
	}

	// Wait until for all threads to finish and close
	for (auto thread : vecThreads)
	{
		AiThreadWait(thread);
		AiThreadClose(thread);
	}

	// Cleanup
	vecThreadData.clear();
	vecThreads.clear();
}

//---------------------------------------
// Check if pose is close to camera center
//---------------------------------------
bool SceneManager::X_CheckIfImageCenter(const ObjectInfo& body) const
{
	// Pose rotation matrix
	Quaterniond q = body.rot;
	Matrix<float, 4, 4> bRotMat = Eigen::Matrix4f::Identity();
	bRotMat.block(0, 0, 3, 3) = q.normalized().toRotationMatrix().cast<float>();

	// Set position
	Vector3f pos = body.pos;
	bRotMat(0, 3) = pos.x();
	bRotMat(1, 3) = pos.y();
	bRotMat(2, 3) = pos.z();

	// Transform pose into camera space
	bRotMat = camMat.transpose().inverse() * bRotMat;

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
bool SceneManager::X_RenderObjsDepth() const
{
	bool valid = false;
	// For each arnold mesh
	for (auto body : vecCurrObjs)
	{
		AtNode* curr = AiNodeLookUpByName(body.name.c_str());
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
	out << std::internal << std::setfill('0') << std::setw(6) << sceneCount;
	string buf = FILE_TEMP_PATH + "/body_depth/img_" + out.str() + ".png";

	ofstream createFile;
	createFile.open(buf.c_str(), ios_base::binary | ios_base::out);
	createFile.write("0", 1);
	createFile.close();

	// Render settings
	AiNodeSetStr(aiDriver, "filename", buf.c_str());
	AiNodeSetInt(aiDriver, "format", AI_TYPE_INT);
	AiNodeSetInt(aiOptions, "xres", 1920);
	AiNodeSetInt(aiOptions, "yres", 1080);
	AiNodeSetInt(aiOptions, "GI_diffuse_depth", 1);
	AiNodeSetInt(aiOptions, "AA_samples", 1);
	AiNodeSetInt(aiOptions, "GI_glossy_samples", 1);

	// Setup rendering
	AiArraySetStr(aiArrOutputs, 0, "RGBA RGBA nullfilter outputDriver");
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
	out << std::internal << std::setfill('0') << std::setw(6) << sceneCount;
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
void SceneManager::X_RenderObjsLabel() const
{
	// For each arnold mesh
	for (auto body : vecCurrObjs)
	{
		AtNode* shader_obj_label;
		string sbuffer = "label_" + std::to_string(body.objSimId);
		// Try to find mesh node
		shader_obj_label = AiNodeLookUpByName(sbuffer.c_str());
		// If not found create
		if (shader_obj_label == NULL)
		{
			shader_obj_label = AiNode("labelshader");
			AiNodeSetStr(shader_obj_label, "name", sbuffer.c_str());
			AiNodeSetInt(shader_obj_label, "id", (body.objSimId + 1) * 10);
		}
		// Save node
		AtNode* curr = AiNodeLookUpByName(body.name.c_str());
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
	out << std::internal << std::setfill('0') << std::setw(6) << sceneCount;
	string buf = FILE_TEMP_PATH + "/body_label/img_" + out.str() + ".png";

	// Setup render settings
	AiNodeSetStr(aiDriver, "filename", buf.c_str());
	AiNodeSetInt(aiDriver, "format", AI_TYPE_INT);
	AiNodeSetInt(aiOptions, "xres", 1920);
	AiNodeSetInt(aiOptions, "yres", 1080);
	AiNodeSetInt(aiOptions, "GI_diffuse_depth", 1);
	AiNodeSetInt(aiOptions, "AA_samples", 1);
	AiNodeSetInt(aiOptions, "GI_glossy_samples", 1);

	// Render image
	AiArraySetStr(aiArrOutputs, 0, "RGBA RGBA nullfilter outputDriver");
	AiNodeSetArray(aiOptions, "outputs", aiArrOutputs);
	AiRender(AI_RENDER_MODE_CAMERA);
}

//---------------------------------------
// Combine scene and object depth images
//---------------------------------------
void SceneManager::X_CvBlendDepth()
{
	cv::Mat cvBodiesD, cvOut;

	ostringstream out;
	out << std::internal << std::setfill('0') << std::setw(6) << sceneCount;
	string sbuf = FILE_TEMP_PATH + "/scene_depth/img_" + out.str() + ".png";
	string bbuf = FILE_TEMP_PATH + "/body_depth/img_" + out.str() + ".png";

	// Calculate minimal depth from scene and objects
	cvSceneD = cv::imread(sbuf, cv::IMREAD_ANYDEPTH);
	cvBodiesD = cv::imread(bbuf, cv::IMREAD_ANYDEPTH);
	cvOut = cv::min(cvSceneD, cvBodiesD);

	// Temp file
	ostringstream out_start;
	out_start << std::internal << std::setfill('0') << std::setw(6) << startCount;

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
	out << std::internal << std::setfill('0') << std::setw(6) << sceneCount;

	ostringstream out_start;
	out_start << std::internal << std::setfill('0') << std::setw(6) << startCount;
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
		string buffer = "blend_" + std::to_string(body.shapeId);
		float ks = vecpPhysxObjs[body.shapeId - 1]->GetMetallic();
		// Try to find mesh node
		AtNode* curr = AiNodeLookUpByName(body.name.c_str());
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
		out << std::internal << std::setfill('0') << std::setw(2) << (body.shapeId);
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

	// For debugging
	if (false)
	{
		cv::imshow("mask", cvMask);
		cv::imshow("scene", cvScene);
		cv::waitKey();
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
	out << std::internal << std::setfill('0') << std::setw(6) << startCount;
	string buf = FILE_FINAL_PATH + "/rgb/img_" + out.str() + ".png";

	// Setup render settings
	AiNodeSetStr(aiDriver, "filename", buf.c_str());
	AiNodeSetInt(aiDriver, "format", AI_TYPE_RGBA);
	AiNodeSetFlt(aiDriver, "gamma", 1.0f);
	AiNodeSetInt(aiOptions, "xres", 960);
	AiNodeSetInt(aiOptions, "yres", 540);
	AiNodeSetInt(aiOptions, "AA_samples", 6);
	AiNodeSetInt(aiOptions, "GI_diffuse_depth", 6);
	AiNodeSetInt(aiOptions, "GI_glossy_samples", 6);

	// Load camera matrix
	X_LoadCamMat(camIntrinsicsRender.fx, camIntrinsicsRender.fy, camIntrinsicsRender.ox, camIntrinsicsRender.oy);

	// Render final image/object blend
	AiArraySetStr(aiArrOutputs, 0, "RGBA RGBA nullfilter outputDriver");
	AiNodeSetArray(aiOptions, "outputs", aiArrOutputs);
	AiRender(AI_RENDER_MODE_CAMERA);
}

//---------------------------------------
// Loads camera matrix with fov
//---------------------------------------
void SceneManager::X_LoadCamMat(float fx, float fy, float ox, float oy)
{
	int i = 0;
	std::string line;
	std::ifstream inFile;

	// Setup camera
	AiNodeSetStr(aiCamera, "name", "renderCam");

	// Open camera pose file
	inFile.open(vecCameraPoses.at(sceneCount));

	// For each camera pose
	while (std::getline(inFile, line))
	{
		// Read matrix
		std::vector<std::string> entries = split(line, ' ');
		for (int j = 0; j < entries.size(); j++)
		{
			camMat(i, j) = std::stof(entries.at(j));
		}
		i++;
	}

	// Meters -> Millimeters;
	float pos0 = camMat(0, 3) * 100;
	float pos1 = camMat(1, 3) * 100;
	float pos2 = camMat(2, 3) * 100;

	// Reset position
	camMat(0, 3) = 0;
	camMat(1, 3) = 0;
	camMat(2, 3) = 0;

	Matrix4f switchAxisMat;
	Matrix4f switchAxisMat2;
	// 90° around X, 180° around Z axis
	switchAxisMat << -1, 0, 0, 0,
		0, 0, -1, 0,
		0, -1, 0, 0,
		0, 0, 0, 1;
	// Mirrors X position
	switchAxisMat2 << -1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1;

	// Mirror on X (?)
	camMat = (switchAxisMat * camMat);

	// Restore position
	camMat(0, 3) = pos0;
	camMat(2, 3) = pos1;
	camMat(1, 3) = pos2;

	// Convert to arnold view matrix
	camMat = switchAxisMat2 * camMat.transpose();
	AtMatrix camMatArn = { {camMat(0, 0), camMat(0, 1), camMat(0, 2), camMat(0, 3)},
												{camMat(1, 0), camMat(1, 1), camMat(1, 2), camMat(1, 3)},
												{camMat(2, 0), camMat(2, 1), camMat(2, 2), camMat(2, 3)},
												{camMat(3, 0), camMat(3, 1), camMat(3, 2), 1} };
	AiNodeSetMatrix(aiCamera, "matrix", camMatArn);

	// Calculate fov and save
	double fovx = 2.0f * atan(960.0f / (2.0f * fx));
	double fovy = 2.0f * atan(540.0f / (2.0f * fy));
	AtArray* fovArr = AiArray(2, 1, AI_TYPE_FLOAT, fovx * 180.f / 3.14f, fovy * 180.f / 3.14f);
	AiNodeSetArray(aiCamera, "fov", fovArr);

	// Calculate NDC coordinates
	float min_x = (ox - (960 / 2)) / (960 / 2) - 1;
	float max_x = (ox - (960 / 2)) / (960 / 2) + 1;
	float min_y = (oy - (540 / 2)) / (540 / 2) - 1;
	float max_y = (oy - (540 / 2)) / (540 / 2) + 1;

	// Save
	AiNodeSetPnt2(aiCamera, "screen_window_min", min_x, min_y);
	AiNodeSetPnt2(aiCamera, "screen_window_max", max_x, max_y);
}

//---------------------------------------
// Loads camera intrinsics
//---------------------------------------
void SceneManager::X_LoadCamIntrinsics()
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
		if (line.find("m_calibrationColorIntrinsic") != std::string::npos) {
			std::vector<std::string> entries = split(line, ' ');
			// Save them
			camIntrinsicScene.fx = std::stof(entries[2]);
			camIntrinsicScene.fy = std::stof(entries[7]);
			camIntrinsicScene.ox = std::stof(entries[4]);
			camIntrinsicScene.oy = std::stof(entries[8]);
			break;
		}
	}
}

//---------------------------------------
// Cleanup arnold meshes
//---------------------------------------
void SceneManager::X_AiDestroy()
{
	for (auto curr : vecCurrObjs)
	{
		AiMesh::DestroyMesh(curr.name);
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
void SceneManager::X_GetFilesInDir(string path, float varThreshold)
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
bool SceneManager::Run(int iterations, int maxCount)
{
	// Load camera intrinsics (fov)
	X_LoadCamIntrinsics();

	// Get non blurry images
	X_GetFilesInDir(scenePath + "/rgbd", 400.f);
	if (vecCameraImages.size() <= 0)
		return true;

	// Render scene depth with OpenGL
	vector<tuple<cv::Mat, cv::Mat> >  renderings = X_RenderSceneDepth();

	// For each iteration
	for (int count = 0; count < iterations; count++)
	{
		// Create physx representation of scan scene
		X_PxCreateScene();
		// Create physx objects
		X_PxCreateObjs();
		// Run the simulation
		X_PxRunSim();
		// Create arnold meshes
		X_AiCreateObjs();

		// Create annotaions file
		string buf = FILE_FINAL_PATH + "/labels.csv";
		ANNOTATIONS_FILE.open(buf, std::ios_base::app);

		// For each scene
		for (sceneCount = 0; sceneCount < vecCameraPoses.size(); sceneCount++)
		{
			// Load camera matrix
			X_LoadCamMat(camIntrinsicsRender.fx, camIntrinsicsRender.fy, camIntrinsicsRender.ox, camIntrinsicsRender.oy);

			// If rendering successful and visible
			if (X_RenderObjsDepth() && X_CvComputeObjsMask())
			{
				// Render object labels (IDs as color)
				X_RenderObjsLabel();
				// Blend depth images
				X_CvBlendDepth();
				// Blend label and mask images
				X_CvBlendLabel();

				// Read scene color image
				cvScene = cv::imread(vecCameraImages.at(sceneCount), cv::IMREAD_COLOR);
				cvRend = get<0>(renderings[sceneCount]);
				cv::resize(cvRend, cvRend, cv::Size(cvScene.cols, cvScene.rows));

				// For debugging: Undistort images
				if (false)
				{
					float k1, k2, k3;
					cv::Mat intrinsic = (cv::Mat1d(3, 3) << camIntrinsicScene.fx, 0, camIntrinsicScene.ox, 0, camIntrinsicScene.fy, camIntrinsicScene.oy, 0, 0, 1);
					cv::Mat distCoeffs = (cv::Mat1d(1, 5) << k1, k2, 0, 0, k3);
					cv::undistort(cvScene, cvScene, intrinsic, distCoeffs);
				}

				// Final image blend
				X_RenderImageBlend();
				startCount++;
			}

			// If already done (?)
			if (startCount >= maxCount)
			{
				// Cleanup
				X_PxDestroy();
				X_AiDestroy();
				vecCurrObjs.clear();
				ANNOTATIONS_FILE.close();
				// Unsuccessful (?)
				return false;
			}
		}
		// Cleanup
		X_PxDestroy();
		X_AiDestroy();
		vecCurrObjs.clear();
		ANNOTATIONS_FILE.close();
	}
	// Success
	return true;
}
