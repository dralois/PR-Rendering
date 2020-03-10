#include <dirent.h>

#include "SceneManager.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"

string FILE_TEMP_PATH = "";
string FILE_FINAL_PATH = "";
string FILE_OBJ_PATH = "";

// Create new scene manager
SceneManager::SceneManager(PxPhysics* gPhysics, PxScene* gScene, PxCooking* gCooking,
													PxFoundation* gFoundation, PxMaterial* gMaterial, AtNode* camera,
													AtNode* options, AtNode* driver, AtArray* outputs_array,
													vector<PxMeshConvex*> physx_objs, vector<AiMesh*> sim_objs,
													int start_count, int obj_per_sim, rapidjson::Document* CONFIG_FILE,
													AtNode* shader_obj_depth, AtNode* shader_bck_depth, AtNode* shader_blendBG) :
	gPhysics(gPhysics), gScene(gScene), gCooking(gCooking), camera(camera), gFoundation(gFoundation),
	gMaterial(gMaterial), options(options), driver(driver), outputs_array(outputs_array), physx_objs(physx_objs),
	sim_objs(sim_objs), start_count(start_count), obj_per_sim(obj_per_sim), CONFIG_FILE(CONFIG_FILE),
	shader_obj_depth(shader_obj_depth), shader_bck_depth(shader_bck_depth), shader_blendBG(shader_blendBG),
	intrinsic_scene(), myScene(NULL), sceneAiMesh(NULL), scenePxMesh(NULL),scene_count(0)
{
	// Fetch path to output location and meshes
	FILE_OBJ_PATH = (*CONFIG_FILE)["models"].GetString();
	FILE_TEMP_PATH = (*CONFIG_FILE)["temp_files_path"].GetString();
	FILE_FINAL_PATH = (*CONFIG_FILE)["final_imgs_path"].GetString();

	// Load intrinsics
	intrinsics_render_out.fx = (*CONFIG_FILE)["fx"].GetFloat();
	intrinsics_render_out.fy = (*CONFIG_FILE)["fy"].GetFloat();
	intrinsics_render_out.ox = (*CONFIG_FILE)["ox"].GetFloat();
	intrinsics_render_out.oy = (*CONFIG_FILE)["oy"].GetFloat();

	// Create renderer
	render = new Render((*CONFIG_FILE)["shaders"].GetString());
}

SceneManager::~SceneManager() {
	delete render;
	if (scenePxMesh != nullptr) {
		delete scenePxMesh;
	}
}

// Set path
void SceneManager::set_scene_path(string scene_path)
{
	this->scene_path = scene_path;
}

// Create physx representation of scan scene
void SceneManager::draw_scene()
{
	string mesh_path = scene_path + "/mesh.refined.obj";
	// Create physx mesh, shape and rigidbody of scan scene
	scenePxMesh = new PxMeshTriangle(mesh_path, 0, 100, gPhysics, gScene, gCooking, gMaterial);
	scenePxMesh->calculateBounds = true;
	scenePxMesh->doubleNorms = true;
	scenePxMesh->LoadFile();
	scenePxMesh->CreateMesh();
	vector<float> pos{ 0, 0, 0 };
	vector<float> rot{ 0, 0, 0, 1 };
	scenePxMesh->CreateRigidbody(pos, rot);
}

// Create random physx objects to drop into scene
void SceneManager::generate_objects()
{
	srand(time(NULL) % 1000);
	// For each object
	for (PxU32 i = 0; i < obj_per_sim; i++)
	{
		// Fetch random object
		int objManagerPos = rand() % physx_objs.size();
		PxMeshConvex* objManager = physx_objs.at(objManagerPos);
		// Random position, same rotation
		float y = (rand() % 10) + 1;
		float x = (rand() % ((int)(scenePxMesh->xmax * 100 - scenePxMesh->xmin * 100))) + scenePxMesh->xmin * 100;
		float z = (rand() % ((int)(scenePxMesh->ymax * 100 - scenePxMesh->ymin * 100))) + scenePxMesh->ymin * 100;
		vector<float> pos{ x, y, z };
		vector<float> rot{ -0.7071068, 0, 0, 0.7071068 };

		// Save rigidbody and mesh in vector
		PxRigidDynamic* body = objManager->CreateRigidbody(pos, rot);
		sim_objects.push_back(make_pair(objManager, body));
	}
}

// Cleanup physx meshes
void SceneManager::destroy_meshes()
{
	// Cleanup scene mesh
	if (scenePxMesh != nullptr)
		delete scenePxMesh;
	// Cleanup random objects
	for (auto obj : sim_objects)
	{
		PxMesh::DestroyRigidbody(obj.second);
	}
	sim_objects.clear();
}

// Run physx simulation
void SceneManager::run_physx_sim()
{
	// 10000 steps
	for (PxU32 i = 0; i < 10000; i++)
	{
		// PxTransform tempTrans = sim_objects[0].second->getGlobalPose();
		// PxVec3 pxPos = tempTrans.p;
		gScene->simulate(2.f / 60.0f);
		gScene->fetchResults(true);

	}
	// Save results
	fetch_results();
}

// Fetch and save simulation results
void SceneManager::fetch_results()
{
	PxTransform tempTrans;
	int i = 0;
	// For each physx object
	for (auto obj : sim_objects)
	{
		i++;
		// Get position
		tempTrans = obj.second->getGlobalPose();
		// Create pose struct
		SceneManager::ObjectInfo* currInfo = new SceneManager::ObjectInfo(obj.first->GetID(), i);
		// Fetch and convert physx position & rotation
		PxQuat quat = tempTrans.q.getNormalized();
		PxVec3 pxPos = tempTrans.p;
		Vector3f* pos = new Vector3f(pxPos.x, pxPos.y, pxPos.z);
		Quaterniond* rot = new Quaterniond(quat.w, quat.x, quat.y, quat.z);
		// Save poses in vector
		currInfo->set_pose(pos, rot);
		curr_objects.push_back(currInfo);
	}
}

// Render scene depth
vector<tuple<cv::Mat, cv::Mat> > SceneManager::render_scenes_gl() {

	// Render depth with OpenGL
	vector<tuple<cv::Mat, cv::Mat> > renderings = render->render_scenes(scene_path, cameraPoses,
																																			intrinsic_scene.fx, intrinsic_scene.fy,
																																			intrinsic_scene.ox, intrinsic_scene.oy);

	// For each image
	for (int render_count = 0; render_count < renderings.size(); render_count++)
	{
		ostringstream out;
		out << std::internal << std::setfill('0') << std::setw(6) << render_count;

		string tmp_depth_path = FILE_TEMP_PATH + "/scene_depth/img_" + out.str() + ".png";
		string tmp_rgb_path = FILE_TEMP_PATH + "/rgb/img_" + out.str() + ".png";

		// For Debugging
		if (false) {
			cv::Mat dep_2 = cv::imread(tmp_depth_path, cv::IMREAD_ANYDEPTH);
			cv::Mat dep_1 = get<1>(renderings[render_count]);

			double min;
			double max;
			cv::minMaxIdx(dep_1, &min, &max);
			cv::convertScaleAbs(dep_1, dep_1, 255 / max);
			cv::convertScaleAbs(dep_2, dep_2, 255 / max);


			cv::resize(dep_1, dep_1, cv::Size(960, 480));
			cv::resize(dep_2, dep_2, cv::Size(960, 480));

			cv::Mat diff = cv::abs(dep_1 - dep_2);

			cv::imshow("Arnold Dep", dep_2);
			cv::imshow("OpenGL Dep", dep_1);
			cv::imshow("diff Dep", diff);
			cv::waitKey();
		}

		// For Debugging
		if (false) {
			cv::Mat col_2 = cv::imread(tmp_rgb_path);
			cv::Mat col_1 = get<0>(renderings[render_count]);

			cv::resize(col_1, col_1, cv::Size(960, 480));
			cv::resize(col_2, col_2, cv::Size(960, 480));

			cv::Mat diff = cv::abs(col_1 - col_2);

			cv::imshow("Arnold RGB", col_2);
			cv::imshow("OpenGL RGB", col_1);
			cv::imshow("diff RGB", diff);
			cv::waitKey();
		}

		// Write rgb and depth out
		cv::imwrite(tmp_rgb_path, get<0>(renderings[render_count]));
		cv::imwrite(tmp_depth_path, get<1>(renderings[render_count]));

		cv::waitKey(10);
	}

	scene_count = 0;

	return renderings;
}

void SceneManager::render_scene_depth_imgs()
{
	// Load scan
	string mesh_path = scene_path + "/mesh.refined.obj";
	sceneAiMesh = new AiMesh(mesh_path, 0, 100);
	vector<float> pos{ 0, 0, 0 };
	vector<float> rot{ 0, 0, 0, 1 };
	// Load scene
	sceneAiMesh->isScene = true;
	sceneAiMesh->LoadFile();
	// Create mesh
	AtNode* sceneMesh = sceneAiMesh->CreateMesh(0, pos, rot);
	AtNode* sceneNode = AiNodeLookUpByName("body0_0");

	// Set shader values
	AiNodeSetBool(shader_obj_depth, "is_body", true);
	AiNodeSetPtr(sceneNode, "shader", shader_obj_depth);
	AiNodeSetFlt(shader_bck_depth, "force_val", 20000);
	AiNodeSetPtr(options, "background", shader_bck_depth);

	// Destroy old filter
	AtNode* myfilter = AiNodeLookUpByName("myfilter");
	AiNodeDestroy(myfilter);
	// Create new null filter
	myfilter = AiNode("null_filter");
	AiNodeSetFlt(myfilter, "width", 1);
	AiNodeSetStr(myfilter, "name", "myfilter");

	// Set render settings
	AiNodeSetInt(driver, "format", AI_TYPE_INT);
	AiNodeSetInt(options, "xres", 1920);
	AiNodeSetInt(options, "yres", 1080);
	AiNodeSetInt(options, "GI_diffuse_depth", 1);
	AiNodeSetInt(options, "AA_samples", 1);
	AiNodeSetInt(options, "GI_glossy_samples", 1);

	// For each scene
	for (scene_count = 0; scene_count < cameraPoses.size(); scene_count++)
	{
		// Load camera matrix
		load_cam_mat(intrinsic_scene.fx, intrinsic_scene.fy, intrinsic_scene.ox, intrinsic_scene.oy);

		ostringstream out;
		out << std::internal << std::setfill('0') << std::setw(6) << scene_count;

		string buf = FILE_TEMP_PATH + "/scene_depth/img_" + out.str() + ".png";

		// Render scene into depth images
		AiNodeSetStr(driver, "filename", buf.c_str());
		AiArraySetStr(outputs_array, 0, "RGBA RGBA myfilter mydriver");
		AiNodeSetArray(options, "outputs", outputs_array);
		AiRender(AI_RENDER_MODE_CAMERA);
	}
	scene_count = 0;

	// Destroy arnold scene mesh
	AiMesh::DestroyMesh("body0_0");
}

// Splits string into vector of strings
std::vector<std::string> split(std::string str, char delimiter)
{
	// helper function
	using namespace std;

	vector<string> internal;
	stringstream ss(str); // Turn the string into a stream.
	string tok;

	while (getline(ss, tok, delimiter))
	{
		internal.push_back(tok);
	}

	return internal;
}

// Calculate and saves 2D bounding box
void setBBox(BodyAnnotation& ann, cv::Mat mask)
{
	// Calculate bounding box using the mask
	cv::Rect Min_Rect = cv::boundingRect(mask);
	Min_Rect.x += Min_Rect.width / 2.f;
	Min_Rect.y += Min_Rect.height / 2.f;
	// Save in vector
	ann.bbox.push_back(Min_Rect.x);
	ann.bbox.push_back(Min_Rect.y);
	ann.bbox.push_back(Min_Rect.width);
	ann.bbox.push_back(Min_Rect.height);
}

// Saves world position and rotation of annotation
void SceneManager::setAnnPose(BodyAnnotation& ann, Vector3f* pos, Quaterniond* q)
{
	// Get rotation matrix
	Matrix<float, 4, 4> bRotMat = Eigen::Matrix4f::Identity();
	bRotMat.block(0, 0, 3, 3) = q->normalized().toRotationMatrix().cast<float>();
	// Camera space -> World space
	bRotMat = camMat.transpose().inverse() * bRotMat;

	// Normalize rotation
	Matrix3f temp = bRotMat.block(0, 0, 3, 3);
	Quaternionf qr = Quaternionf(temp).normalized();

	// Save position in vector
	ann.trans.push_back(bRotMat(0, 3) * 10);
	ann.trans.push_back(bRotMat(1, 3) * 10);
	ann.trans.push_back(bRotMat(2, 3) * 10);
	// Save rotation in vector
	ann.quat.push_back(qr.w());
	ann.quat.push_back(qr.x());
	ann.quat.push_back(qr.y());
	ann.quat.push_back(qr.z());
}

// Write out annotation file for rendered objects
void SceneManager::set_annotations(cv::Mat seg, cv::Mat segMasked)
{
	// For each mesh
	for (auto currBody : curr_objects)
	{
		// Determine label sum unmasked
		float segBodySum = cv::sum(seg == (currBody->object_sim_id + 1) * 10)[0];
		// Stop if object completely covered
		if (segBodySum == 0)
			continue;
		// Determine label sum masked
		float segBodySumMasked = cv::sum(segMasked == (currBody->object_sim_id + 1) * 10)[0];
		float percent = segBodySumMasked / segBodySum;
		// Stop if less then 30% coverage
		if (percent <= 0.3 || segBodySumMasked / 255 < 2000)
			continue;

		BodyAnnotation currAnn;
		currAnn.trans = std::vector<float>();
		currAnn.quat = std::vector<float>();

		currAnn.bbox = std::vector<float>();
		currAnn.id = currBody->shape_id;
		currAnn.sim_id = (currBody->object_sim_id + 1) * 10;
		// Set bounding box & annotiation
		setBBox(currAnn, seg == (currBody->object_sim_id + 1) * 10);
		setAnnPose(currAnn, currBody->get_pos(), currBody->get_rot());

		ostringstream out;
		out << std::internal << std::setfill('0') << std::setw(2) << currAnn.id;
		string buf = "obj_" + out.str();

		// Add to annotation file
		ANNOTATIONS_FILE << start_count << ", " << currAnn.bbox[0] << ", " << currAnn.bbox[1] << ", "
			<< currAnn.bbox[2] << ", " << currAnn.bbox[3] << ", " << buf << ", " << currAnn.quat[0] << ", "
			<< currAnn.quat[1] << ", " << currAnn.quat[2] << ", " << currAnn.quat[3] << ", "
			<< "0"
			<< ", "
			<< "0"
			<< ", " << currAnn.trans[0] << ", " << currAnn.trans[1] << ", " << currAnn.trans[2] << ", "
			<< currAnn.sim_id
			<< " [" << intrinsics_render_out.fx << ", " << intrinsics_render_out.fy << ", " << intrinsic_scene.ox << ", " << intrinsic_scene.oy << "]" << "\n";
	}
}

// Create arnold meshes
void SceneManager::draw_bodies()
{
	sim_objs = {};
	for (int i = 0; i < physx_objs.size() + 1; i++)
		sim_objs.push_back(NULL);
	// For each created object
	for (auto body : curr_objects)
	{
		// Create arnold mesh
		AiMesh* mesh = sim_objs.at(body->shape_id);
		if (sim_objs.at(body->shape_id) == NULL)
		{
			ostringstream out;
			out << std::internal << std::setfill('0') << std::setw(2) << (body->shape_id);
			string buf = FILE_OBJ_PATH + "/obj_" + out.str() + ".obj";
			// Load arnold mesh accordingly
			mesh = new AiMesh(buf, body->shape_id, 0.1);
			mesh->LoadFile();
			sim_objs.at(body->shape_id) = mesh;
		}

		// Set position and rotation accordingly
		Vector3f* p = body->get_pos();
		Quaterniond* q = body->get_rot();
		vector<float> pos{ p->x(), p->y(), p->z() };
		vector<float> rot{ (float)q->x(), (float)q->y(), (float)q->z(), (float)q->w() };
		mesh->CreateMesh(body->object_sim_id, pos, rot);
	}
}

// Check if pose is close to camera center
bool SceneManager::checkIfCenterOnImage(ObjectInfo* body)
{
	// Pose rotation matrix
	Quaterniond* q = body->get_rot();
	Matrix<float, 4, 4> bRotMat = Eigen::Matrix4f::Identity();
	bRotMat.block(0, 0, 3, 3) = q->normalized().toRotationMatrix().cast<float>();

	// Set position
	Vector3f* pos = body->get_pos();
	bRotMat(0, 3) = pos->x();
	bRotMat(1, 3) = pos->y();
	bRotMat(2, 3) = pos->z();

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

// Render objects depth
bool SceneManager::render_bodies_depth()
{
	bool valid = false;
	// For each arnold mesh
	for (auto body : curr_objects)
	{
		AtNode* curr = AiNodeLookUpByName(body->get_name().c_str());
		AiNodeSetPtr(curr, "shader", shader_obj_depth);
		valid = true;
	}

	// Set shader parameters
	AiNodeSetBool(shader_obj_depth, "is_body", true);
	AiNodeSetFlt(shader_bck_depth, "force_val", 30000);
	AiNodeSetPtr(options, "background", shader_bck_depth);

	// Destroy old filter
	AtNode* myfilter = AiNodeLookUpByName("myfilter");
	AiNodeDestroy(myfilter);
	// Create new filter
	myfilter = AiNode("null_filter");
	AiNodeSetFlt(myfilter, "width", 1);
	AiNodeSetStr(myfilter, "name", "myfilter");

	// Temp file name
	ostringstream out;
	out << std::internal << std::setfill('0') << std::setw(6) << scene_count;
	string buf = FILE_TEMP_PATH + "/" + "body_depth/img_" + out.str() + ".png";

	// Render settings
	AiNodeSetStr(driver, "filename", buf.c_str());
	AiNodeSetInt(driver, "format", AI_TYPE_INT);
	AiNodeSetInt(options, "xres", 1920);
	AiNodeSetInt(options, "yres", 1080);
	AiNodeSetInt(options, "GI_diffuse_depth", 1);
	AiNodeSetInt(options, "AA_samples", 1);
	AiNodeSetInt(options, "GI_glossy_samples", 1);

	// Setup rendering
	AiArraySetStr(outputs_array, 0, "RGBA RGBA myfilter mydriver");
	AiNodeSetArray(options, "outputs", outputs_array);

	// Render
	if (valid)
		AiRender(AI_RENDER_MODE_CAMERA);

	return valid;
}

// Determine depth mask
bool SceneManager::calculate_mask()
{
	ostringstream out;
	out << std::internal << std::setfill('0') << std::setw(6) << scene_count;
	string sbuf = FILE_TEMP_PATH + "/scene_depth/img_" + out.str() + ".png";
	string bbuf = FILE_TEMP_PATH + "/body_depth/img_" + out.str() + ".png";

	// Read depth images
	cvSceneD = cv::imread(sbuf, cv::IMREAD_ANYDEPTH);
	cvBodiesD = cv::imread(bbuf, cv::IMREAD_ANYDEPTH);

	// For debugging
	if (false)
	{
		double min;
		double max;
		cv::minMaxIdx(cvSceneD, &min, &max);
		cv::convertScaleAbs(cvSceneD, cvSceneD, 255 / max);
		cv::convertScaleAbs(cvBodiesD, cvBodiesD, 255 / max);

		cv::imshow("scene depth", cvSceneD);
		cv::imshow("bodies depth", cvBodiesD);
		cv::imshow("mask", cvBodiesD <= (cvSceneD + 0.025));
		cv::waitKey();
	}

	// 2.5cm buffer
	cvMask = cvBodiesD <= cvSceneD;

	// Return if object visible
	cv::Scalar maskMean = cv::mean(cvMask);
	return maskMean[0] >= 1.f;
}

// Render object label image (IDs as color?)
void SceneManager::render_bodies_seg()
{
	// For each arnold mesh
	for (auto body : curr_objects)
	{
		AtNode* shader_obj_label;
		string sbuffer = "label_" + std::to_string(body->object_sim_id);
		// Try to find mesh node
		shader_obj_label = AiNodeLookUpByName(sbuffer.c_str());
		// If not found create
		if (shader_obj_label == NULL)
		{
			shader_obj_label = AiNode("labelshader");
			AiNodeSetStr(shader_obj_label, "name", sbuffer.c_str());
			AiNodeSetInt(shader_obj_label, "id", (body->object_sim_id + 1) * 10);
		}
		// Save node
		AtNode* curr = AiNodeLookUpByName(body->get_name().c_str());
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
	AiNodeSetPtr(options, "background", shader_bck_label);

	// Destroy old null filter
	AtNode* myfilter = AiNodeLookUpByName("myfilter");
	AiNodeDestroy(myfilter);
	// Create new null filter
	myfilter = AiNode("null_filter");
	AiNodeSetFlt(myfilter, "width", 1);
	AiNodeSetStr(myfilter, "name", "myfilter");

	// Temp file
	ostringstream out;
	out << std::internal << std::setfill('0') << std::setw(6) << scene_count;
	string buf = FILE_TEMP_PATH + "/body_label/img_" + out.str() + ".png";

	// Setup render settings
	AiNodeSetStr(driver, "filename", buf.c_str());
	AiNodeSetInt(driver, "format", AI_TYPE_INT);
	AiNodeSetInt(options, "xres", 1920);
	AiNodeSetInt(options, "yres", 1080);
	AiNodeSetInt(options, "GI_diffuse_depth", 1);
	AiNodeSetInt(options, "AA_samples", 1);
	AiNodeSetInt(options, "GI_glossy_samples", 1);

	// Render image
	AiArraySetStr(outputs_array, 0, "RGBA RGBA myfilter mydriver");
	AiNodeSetArray(options, "outputs", outputs_array);
	AiRender(AI_RENDER_MODE_CAMERA);
}

// Combine scene and object depth images
void SceneManager::blend_depth()
{
	cv::Mat cvBodiesD, cvOut;

	ostringstream out;
	out << std::internal << std::setfill('0') << std::setw(6) << scene_count;
	string sbuf = FILE_TEMP_PATH + "/scene_depth/img_" + out.str() + ".png";
	string bbuf = FILE_TEMP_PATH + "/body_depth/img_" + out.str() + ".png";

	// Calculate minimal depth from scene and objects
	cvSceneD = cv::imread(sbuf, cv::IMREAD_ANYDEPTH);
	cvBodiesD = cv::imread(bbuf, cv::IMREAD_ANYDEPTH);
	cvOut = cv::min(cvSceneD, cvBodiesD);

	// Temp file
	ostringstream out_start;
	out_start << std::internal << std::setfill('0') << std::setw(6) << start_count;

	// Write out blended image
	string obuf = FILE_FINAL_PATH + "/depth/img_" + out_start.str() + ".png";
	cv::imwrite(obuf, cvOut);
}

// Combine mask and label images
void SceneManager::blend_seg()
{
	cv::Mat cvOut;

	// Temp file
	ostringstream out;
	out << std::internal << std::setfill('0') << std::setw(6) << scene_count;

	ostringstream out_start;
	out_start << std::internal << std::setfill('0') << std::setw(6) << start_count;
	string bbuf = FILE_TEMP_PATH + "/body_label/img_" + out.str() + ".png";
	string obuf = FILE_FINAL_PATH + "/segs/img_" + out_start.str() + ".png";

	// Read label image
	cvBodiesS = cv::imread(bbuf, cv::IMREAD_ANYDEPTH);

	// Write out masked image
	cvBodiesS.copyTo(cvOut, cvMask);
	cv::imwrite(obuf, cvOut);

	// Write out annotation file
	set_annotations(cvBodiesS, cvOut);
}

// Final image blend
void SceneManager::blend_rgb()
{
	// For each object
	for (auto body : curr_objects)
	{
		string buffer = "blend_" + std::to_string(body->shape_id);
		float ks = physx_objs[body->shape_id - 1]->GetMetallic();
		// Try to find mesh node
		AtNode* curr = AiNodeLookUpByName(body->get_name().c_str());
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
		out << std::internal << std::setfill('0') << std::setw(2) << (body->shape_id);
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
	AiNodeSetPtr(shader_blendBG, "mask", (void*)&cvMask);
	AiNodeSetPtr(shader_blendBG, "blend_image", (void*)&cvScene);
	AiNodeSetPtr(shader_blendBG, "rend_image", (void*)&cvRend);
	AiNodeSetPtr(options, "background", shader_blendBG);

	// Destroy old null filter
	AtNode* myfilter = AiNodeLookUpByName("myfilter");
	AiNodeDestroy(myfilter);
	// Create new gauss (blur?) filter?
	myfilter = AiNode("gaussian_filter");
	AiNodeSetFlt(myfilter, "width", 1);
	AiNodeSetStr(myfilter, "name", "myfilter");

	// Final image path
	ostringstream out;
	out << std::internal << std::setfill('0') << std::setw(6) << start_count;
	string buf = FILE_FINAL_PATH + "/rgb/img_" + out.str() + ".png";

	// Setup render settings
	AiNodeSetStr(driver, "filename", buf.c_str());
	AiNodeSetInt(driver, "format", AI_TYPE_RGBA);
	AiNodeSetFlt(driver, "gamma", 1.0f);
	AiNodeSetInt(options, "xres", 960);
	AiNodeSetInt(options, "yres", 540);
	AiNodeSetInt(options, "AA_samples", 6);
	AiNodeSetInt(options, "GI_diffuse_depth", 6);
	AiNodeSetInt(options, "GI_glossy_samples", 6);

	// Load camera matrix
	load_cam_mat(intrinsics_render_out.fx, intrinsics_render_out.fy, intrinsics_render_out.ox, intrinsics_render_out.oy);

	// Render final image/object blend
	AiArraySetStr(outputs_array, 0, "RGBA RGBA myfilter mydriver");
	AiNodeSetArray(options, "outputs", outputs_array);
	AiRender(AI_RENDER_MODE_CAMERA);
}

// Loads camera matrix with fov
void SceneManager::load_cam_mat(float fx, float fy, float ox, float oy)
{
	int i = 0;
	std::string line;
	std::ifstream inFile;

	// Setup camera
	AiNodeSetStr(camera, "name", "mycamera");

	// Open camera pose file
	inFile.open(cameraPoses.at(scene_count));

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

	// Convert to arnold view matrix (?)
	camMat = switchAxisMat2 * camMat.transpose();
	AtMatrix camMatArn = { {camMat(0, 0), camMat(0, 1), camMat(0, 2), camMat(0, 3)},
												{camMat(1, 0), camMat(1, 1), camMat(1, 2), camMat(1, 3)},
												{camMat(2, 0), camMat(2, 1), camMat(2, 2), camMat(2, 3)},
												{camMat(3, 0), camMat(3, 1), camMat(3, 2), 1} };
	AiNodeSetMatrix(camera, "matrix", camMatArn);

	// Convert fov and save (?)
	double fovx = 2.0f * atan(960.0f / (2.0f * fx));
	double fovy = 2.0f * atan(540.0f / (2.0f * fy));
	AtArray* fovArr = AiArray(2, 1, AI_TYPE_FLOAT, fovx * 180.f / 3.14f, fovy * 180.f / 3.14f);
	AiNodeSetArray(camera, "fov", fovArr);

	// Calculate image dimensions (?)
	float min_x = (ox - (960 / 2)) / (960 / 2) - 1;
	float max_x = (ox - (960 / 2)) / (960 / 2) + 1;
	float min_y = (oy - (540 / 2)) / (540 / 2) - 1;
	float max_y = (oy - (540 / 2)) / (540 / 2) + 1;

	// Save
	AiNodeSetPnt2(camera, "screen_window_min", min_x, min_y);
	AiNodeSetPnt2(camera, "screen_window_max", max_x, max_y);
}

// Loads camera intrinsics
void SceneManager::load_intrinsics()
{
	std::string line;
	std::ifstream inFile;

	// Input file
	string buf = scene_path + "/rgbd/_info.txt";

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
			intrinsic_scene.fx = std::stof(entries[2]);  // fx
			intrinsic_scene.fy = std::stof(entries[7]);  // fy
			intrinsic_scene.ox = std::stof(entries[4]);  // ox
			intrinsic_scene.oy = std::stof(entries[8]);  // oy
			break;
		}
	}
}

// Cleanup arnold meshes
void SceneManager::remove_bodies_ai()
{
	for (ObjectInfo* curr : curr_objects)
	{
		AiMesh::DestroyMesh(curr->get_name());
	}
}

// Filter out blurry images (based on variance)
float SceneManager::computeVarianceOfLaplacian(const cv::Mat& image) {
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

// Determine images to process
void SceneManager::getFilesInAdirectory(string path, float variance_threshold)
{
	DIR* dir;
	struct dirent* ent;

	// Init vectors
	camImages = std::vector<std::string>();
	cameraPoses = std::vector<std::string>();

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
				float variance = computeVarianceOfLaplacian(image);
				// Skip blurry images
				if (variance > variance_threshold)
				{
					camImages.push_back(buf);
					string cam_file = buf;
					cam_file.replace(cam_file.find("color.jpg"), sizeof("color.jpg") - 1, "pose.txt");
					// Save pose file in vector
					cameraPoses.push_back(cam_file);
				}
			}
		}
		// Finally sort vectors and close dir
		std::sort(cameraPoses.begin(), cameraPoses.end());
		std::sort(camImages.begin(), camImages.end());
		closedir(dir);
	}
}

// Run simulation
bool SceneManager::run(int iter, int max_count)
{
	// Load camera intrinsics (fov)
	load_intrinsics();

	// Get non blurry images
	getFilesInAdirectory(scene_path + "/rgbd", 400.f);
	if (camImages.size() <= 0)
		return true;

	// Render scene depth with OpenGL
	vector<tuple<cv::Mat, cv::Mat> >  renderings = render_scenes_gl();

	// For each iteration
	for (int count = 0; count < iter; count++)
	{
		// Create physx representation of scan scene
		draw_scene();
		// Create physx objects
		generate_objects();
		// Run the simulation
		run_physx_sim();
		// Create arnold meshes
		draw_bodies();

		// Create annotaions file
		string buf = FILE_FINAL_PATH + "/labels.csv";
		ANNOTATIONS_FILE.open(buf, std::ios_base::app);

		// For each scene
		for (scene_count = 0; scene_count < cameraPoses.size(); scene_count++)
		{
			// Load camera matrix
			load_cam_mat(intrinsics_render_out.fx, intrinsics_render_out.fy, intrinsics_render_out.ox, intrinsics_render_out.oy);

			// If rendering successful and visible
			if (render_bodies_depth() && calculate_mask())
			{
				// Render object labels (IDs as color)
				render_bodies_seg();
				// Blend depth images
				blend_depth();
				// Blend label and mask images
				blend_seg();

				// Read scene color image
				cvScene = cv::imread(camImages.at(scene_count), cv::IMREAD_COLOR);
				cvRend = get<0>(renderings[scene_count]);
				cv::resize(cvRend, cvRend, cv::Size(cvScene.cols, cvScene.rows));

				// For debugging: Undistort images
				if (false)
				{
					float k1, k2, k3;
					cv::Mat intrinsic = (cv::Mat1d(3, 3) << intrinsic_scene.fx, 0, intrinsic_scene.ox, 0, intrinsic_scene.fy, intrinsic_scene.oy, 0, 0, 1);
					cv::Mat distCoeffs = (cv::Mat1d(1, 5) << k1, k2, 0, 0, k3);
					cv::undistort(cvScene, cvScene, intrinsic, distCoeffs);
				}

				// For debugging: Show images
				if (false)
				{
					cv::Mat dep_1 = get<1>(renderings[scene_count]);
					double min;
					double max;
					cv::minMaxIdx(dep_1, &min, &max);
					cv::convertScaleAbs(dep_1, dep_1, 255 / max);
					cv::resize(dep_1, dep_1, cv::Size(960, 480));
					cv::imshow("rend Dep", dep_1);
					cv::imshow("rend image", cvRend);
					cv::imshow("blend image", cvScene);
					cv::waitKey();
				}

				// Final image blend
				blend_rgb();
				start_count++;
			}

			// If already done (?)
			if (start_count >= max_count)
			{
				// Cleanup
				destroy_meshes();
				remove_bodies_ai();
				curr_objects.clear();
				ANNOTATIONS_FILE.close();
				// Unsuccessful (?)
				return false;
			}
		}
		// Cleanup
		destroy_meshes();
		remove_bodies_ai();
		curr_objects.clear();
		ANNOTATIONS_FILE.close();
	}
	// Success
	return true;
}

// New object transform
SceneManager::ObjectInfo::ObjectInfo(int shape_id, int object_sim_id) :
	shape_id(shape_id),
	object_sim_id(object_sim_id),
	pos(NULL),
	rot(NULL)
{
};

// Set position and rotation
void SceneManager::ObjectInfo::set_pose(Vector3f* pos, Quaterniond* rot)
{
	this->pos = pos;
	this->rot = rot;
}

// Get position
Vector3f* SceneManager::ObjectInfo::get_pos()
{
	return pos;
}

// Get rotation
Quaterniond* SceneManager::ObjectInfo::get_rot()
{
	return rot;
}

// Get name
string SceneManager::ObjectInfo::get_name()
{
	string ret = "body" + to_string(shape_id) + "_" + to_string(object_sim_id);
	return ret;
}

// Cleanup object transform
SceneManager::ObjectInfo::~ObjectInfo()
{
	free(pos);
	free(rot);
}
