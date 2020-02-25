#include <dirent.h>

#include "scene_manager.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"

string FILE_TEMP_PATH = "";
string FILE_FINAL_PATH = "";
string FILE_OBJ_PATH = "";
using namespace std;

// Create new scene manager
SceneManager::SceneManager(PxPhysics *gPhysics, PxScene *gScene, PxCooking *gCooking,
                           PxFoundation *gFoundation, PxMaterial *gMaterial, AtNode *camera,
                           AtNode *options, AtNode *driver, AtArray *outputs_array,
                           vector<PxConvManager *> physx_objs, vector<AiMeshManager *> sim_objs,
                           int start_count, int obj_per_sim, rapidjson::Document *CONFIG_FILE,
                           AtNode *shader_obj_depth, AtNode *shader_bck_depth, AtNode *shader_blendBG) : gPhysics(gPhysics), gScene(gScene), gCooking(gCooking), camera(camera), gFoundation(gFoundation),
                                                                                                         gMaterial(gMaterial), options(options), driver(driver), outputs_array(outputs_array), physx_objs(physx_objs),
                                                                                                         sim_objs(sim_objs), start_count(start_count), obj_per_sim(obj_per_sim), CONFIG_FILE(CONFIG_FILE),
                                                                                                         shader_obj_depth(shader_obj_depth), shader_bck_depth(shader_bck_depth), shader_blendBG(shader_blendBG)
{
    // Fetch path to output location and meshes
    FILE_TEMP_PATH = (*CONFIG_FILE)["temp_files_path"].GetString();
    FILE_OBJ_PATH = (*CONFIG_FILE)["textured_objs"].GetString();
    FILE_FINAL_PATH = (*CONFIG_FILE)["final_imgs_path"].GetString();
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
    sceneMeshManager = new PxTriManager(mesh_path, 0, 100, gPhysics, gScene, gCooking, gMaterial);
    sceneMeshManager->calculateBounds = true;
    sceneMeshManager->doubleNorms = true;
    sceneMeshManager->load_file();
    sceneMeshManager->drawMeshShape();
    vector<float> pos{0, 0, 0};
    vector<float> rot{0, 0, 0, 1};
    sceneMeshManager->generateObj(pos, rot);
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
        PxConvManager *objManager = physx_objs.at(objManagerPos);

        // Random position, always the same rotation
        float y = - (rand() % 100);
        float x = (rand() % ((int)(sceneMeshManager->xmax * 100 - sceneMeshManager->xmin * 100))) + sceneMeshManager->xmin * 100;
        float z = (rand() % ((int)(sceneMeshManager->ymax * 100 - sceneMeshManager->ymin * 100))) + sceneMeshManager->ymin * 100;
        vector<float> pos{x, y, z};
        vector<float> rot{-0.7071068, 0, 0, 0.7071068};

        // Save rigidbody and mesh in vector
        PxRigidDynamic *body = objManager->generateObj(pos, rot);
        sim_objects.push_back(make_pair(objManager, body));
    }
}

// Cleanup physx meshes
void SceneManager::destroy_meshes()
{
    // Cleanup scene mesh
    if (sceneMeshManager != nullptr)
        free(sceneMeshManager);
    // Cleanup random physx meshes
    for (auto obj : sim_objects)
    {
        PxMeshManager::destroyObject(obj.second);
    }
    sim_objects.clear();
}

// Run physx simulation
void SceneManager::run_physx_sim()
{
    // 2000 steps
    for (PxU32 i = 0; i < 2000; i++)
    {
        // Simulate
        gScene->simulate(3.0f / 60.0f);
        // Probably not needed anymore?
        gScene->fetchResults(true);
        PxTransform tempTrans;
        tempTrans = sim_objects[0].second->getGlobalPose();
        if (i % 10 == 0)
        {
            // cout << tempTrans.p.x << "c " << tempTrans.p.y << "c " << tempTrans.p.z  << "c " << endl;
        }
    }
    // Save results and cleanup physx simulation
    fetch_results();
    destroy_meshes();
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
        SceneManager::ObjectInfo *currInfo = new SceneManager::ObjectInfo(obj.first->get_id(), i);
        // Fetch and convert physx position & rotation
        PxQuat quat = tempTrans.q.getNormalized();
        PxVec3 pxPos = tempTrans.p;
        Vector3f *pos = new Vector3f(pxPos.x, pxPos.y, pxPos.z);
        Quaterniond *rot = new Quaterniond(quat.w, quat.x, quat.y, quat.z);
        // Save poses in vector
        currInfo->set_pose(pos, rot);
        curr_objects.push_back(currInfo);
    }
}

// Render depth images
void SceneManager::render_scene_depth_imgs()
{
    // Load scan
    string mesh_path = scene_path + "/mesh.refined.obj";
    sceneAiMeshManager = new AiMeshManager(mesh_path, 0, 100);
    vector<float> pos{0, 0, 0};
    vector<float> rot{0, 0, 0, 1};
    // Load scene
    sceneAiMeshManager->is_scene = true;
    sceneAiMeshManager->load_file();
    // Create mesh
    AtNode *sceneMesh = sceneAiMeshManager->drawObj(0, pos, rot);
    AtNode *sceneNode = AiNodeLookUpByName("body0_0");

    // Set shader values
    AiNodeSetBool(shader_obj_depth, "is_body", true);
    AiNodeSetPtr(sceneNode, "shader", shader_obj_depth);
    AiNodeSetFlt(shader_bck_depth, "force_val", 20000);
    AiNodeSetPtr(options, "background", shader_bck_depth);

    // Destroy old filter
    AtNode *myfilter = AiNodeLookUpByName("myfilter");
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
        // Load camera matrices
        load_cam_mat();

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
    AiMeshManager::destroyObject("body0_0");
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

// Calculate 2D bounding box
void setBBox(BodyAnnotation *ann, cv::Mat mask)
{
    Rect Min_Rect = cv::boundingRect(mask);
    Min_Rect.x += Min_Rect.width / 2.f;
    Min_Rect.y += Min_Rect.height / 2.f;
    // Save 2D bounding box in a vector
    ann->bbox.push_back(Min_Rect.x);
    ann->bbox.push_back(Min_Rect.y);
    ann->bbox.push_back(Min_Rect.width);
    ann->bbox.push_back(Min_Rect.height);
}

// Transform pose into camera space
void SceneManager::setAnnPose(BodyAnnotation *ann, Vector3f *pos, Quaterniond *q)
{
    using namespace Eigen;

    // Camera to world matrix
    Matrix<float, 4, 4> bRotMat = Eigen::Matrix4f::Identity();
    bRotMat.block(0, 0, 3, 3) = q->normalized().toRotationMatrix().cast<float>();
    bRotMat = camMat.transpose().inverse() * bRotMat;

    // Determine camera to world rotation
    Matrix3f temp = bRotMat.block(0, 0, 3, 3);
    Quaternionf qr = Quaternionf(temp).normalized();

    // Save both position and rotation in vector
    ann->trans.push_back(bRotMat(0, 3) * 10);
    ann->trans.push_back(bRotMat(1, 3) * 10);
    ann->trans.push_back(bRotMat(2, 3) * 10);
    ann->quat.push_back(qr.w());
    ann->quat.push_back(qr.x());
    ann->quat.push_back(qr.y());
    ann->quat.push_back(qr.z());
}

// Write out annotation file for rendered objects
void SceneManager::set_annotations(Mat seg, Mat segMasked)
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

        // Create annotation
        BodyAnnotation *currAnn = new BodyAnnotation();
        currAnn->trans = std::vector<float>();
        currAnn->quat = std::vector<float>();
        // Save bounding box and convert pose to world space
        currAnn->bbox = std::vector<float>();
        currAnn->id = currBody->shape_id;
        currAnn->sim_id = (currBody->object_sim_id + 1) * 10;
        setBBox(currAnn, seg == (currBody->object_sim_id + 1) * 10);
        setAnnPose(currAnn, currBody->get_pos(), currBody->get_rot());

        ostringstream out;
        out << std::internal << std::setfill('0') << std::setw(2) << currAnn->id;
        string buf = "obj_" + out.str();

        // Save in annotation file
        ANNOTATIONS_FILE << start_count << ", " << currAnn->bbox[0] << ", " << currAnn->bbox[1] << ", "
                         << currAnn->bbox[2] << ", " << currAnn->bbox[3] << ", " << buf << ", " << currAnn->quat[0] << ", "
                         << currAnn->quat[1] << ", " << currAnn->quat[2] << ", " << currAnn->quat[3] << ", "
                         << "0"
                         << ", "
                         << "0"
                         << ", " << currAnn->trans[0] << ", " << currAnn->trans[1] << ", " << currAnn->trans[2] << ", "
                         << currAnn->sim_id << "\n";
    }
}

// Create arnold meshes
void SceneManager::draw_bodies()
{
    sim_objs = {};
    for (int i = 0; i < 16; i++)
        sim_objs.push_back(NULL);
    // For each created object
    for (auto body : curr_objects)
    {
        // Create arnold mesh
        AiMeshManager *mesh = sim_objs.at(body->shape_id);
        if (sim_objs.at(body->shape_id) == NULL)
        {
            ostringstream out;
            out << std::internal << std::setfill('0') << std::setw(2) << (body->shape_id);
            string buf = FILE_OBJ_PATH + "/obj_" + out.str() + ".obj";
            // Load arnold mesh accordingly
            mesh = new AiMeshManager(buf, body->shape_id, 0.1);
            mesh->load_file();
            sim_objs.at(body->shape_id) = mesh;
        }

        // Set position and rotation accordingly
        Vector3f *p = body->get_pos();
        Quaterniond *q = body->get_rot();
        vector<float> pos{p->x(), p->y(), p->z()};
        vector<float> rot{(float)q->x(), (float)q->y(), (float)q->z(), (float)q->w()};
        mesh->drawObj(body->object_sim_id, pos, rot);
    }
}

// Check if pose is close to camera center
bool SceneManager::checkIfCenterOnImage(ObjectInfo *body)
{
    // Pose rotation matrix
    Quaterniond *q = body->get_rot();
    Matrix<float, 4, 4> bRotMat = Eigen::Matrix4f::Identity();
    bRotMat.block(0, 0, 3, 3) = q->normalized().toRotationMatrix().cast<float>();

    // Set position
    Vector3f *pos = body->get_pos();
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
        AtNode *curr = AiNodeLookUpByName(body->get_name().c_str());

        // Too close to camera?
        bool closeToCamera = sqrt(pow(body->get_pos()->x() - camMat(3, 0), 2) + pow(body->get_pos()->y() - camMat(3, 1), 2) + pow(body->get_pos()->z() - camMat(3, 2), 2)) > 150;
        // Determine if rendering possible: Must be far enough away and visible
        if (closeToCamera || !checkIfCenterOnImage(body))
        {
            AiNodeSetPtr(curr, "shader", shader_bck_depth);
        }
        else
        {
            AiNodeSetPtr(curr, "shader", shader_obj_depth);
            valid = true;
        }
    }

    // Set shader values
    AiNodeSetBool(shader_obj_depth, "is_body", true);
    AiNodeSetFlt(shader_bck_depth, "force_val", 30000);
    AiNodeSetPtr(options, "background", shader_bck_depth);

    // Destroy old filter
    AtNode *myfilter = AiNodeLookUpByName("myfilter");
    AiNodeDestroy(myfilter);
    // Create new filter
    myfilter = AiNode("null_filter");
    AiNodeSetFlt(myfilter, "width", 1);
    AiNodeSetStr(myfilter, "name", "myfilter");

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

    // Load camera matrix
    load_cam_mat();

    // Render
    AiArraySetStr(outputs_array, 0, "RGBA RGBA myfilter mydriver");
    AiNodeSetArray(options, "outputs", outputs_array);
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
    // ?
    if (false)
    {
        cv::imshow("scene depth", cvSceneD);
        cv::imshow("bodies depth", cvBodiesD);
        cv::waitKey();
    }
    // Combine scene and objects depth
    cvMask = cvBodiesD < cvSceneD;
    // Determine if anything visible
    Scalar maskMean = cv::mean(cvMask);
    return maskMean[0] >= 1.f;
}

// Render object label image (IDs as color?)
void SceneManager::render_bodies_seg()
{
    // For each arnold mesh
    for (auto body : curr_objects)
    {
        string sbuffer = "label_" + std::to_string(body->object_sim_id);

        AtNode *shader_obj_label;
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
        AtNode *curr = AiNodeLookUpByName(body->get_name().c_str());
        AiNodeSetPtr(curr, "shader", shader_obj_label);
    }

    // Try to find background node
    AtNode *shader_bck_label = AiNodeLookUpByName("label_background");
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
    AtNode *myfilter = AiNodeLookUpByName("myfilter");
    AiNodeDestroy(myfilter);
    // Create new null filter
    myfilter = AiNode("null_filter");
    AiNodeSetFlt(myfilter, "width", 1);
    AiNodeSetStr(myfilter, "name", "myfilter");

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

    // Load camera matrix
    load_cam_mat();

    // Render objects
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

    ostringstream out_start;
    out_start << std::internal << std::setfill('0') << std::setw(6) << start_count;

    // Write the combined image out
    string obuf = FILE_FINAL_PATH + "/final_imgs/img_" + out_start.str() + "_depth.png";
    cv::imwrite(obuf, cvOut);
}

// Combine mask and label images
void SceneManager::blend_seg()
{
    cv::Mat cvOut;

    ostringstream out;
    out << std::internal << std::setfill('0') << std::setw(6) << scene_count;

    ostringstream out_start;
    out_start << std::internal << std::setfill('0') << std::setw(6) << start_count;

    string bbuf = FILE_TEMP_PATH + "/body_label/img_" + out.str() + ".png";
    string obuf = FILE_FINAL_PATH + "/final_imgs/img_" + out_start.str() + "_seg.png";

    // Read label image
    cvBodiesS = cv::imread(bbuf, cv::IMREAD_ANYDEPTH);

    // Write out masked image (?)
    cvBodiesS.copyTo(cvOut, cvMask);
    cv::imwrite(obuf, cvOut);

    // Write out annotation file (?)
    set_annotations(cvBodiesS, cvOut);
}

// Final image blend
void SceneManager::blend_rgb()
{
    // ?
    std::vector<float> map_obj_ks = {0.01, 0.05, 0.05, 0.05, 0.03, 0.01, 0.03, 0.02, 0.03, 0.02, 0.02, 0.03, 0.03};
    // For each mesh
    for (auto body : curr_objects)
    {
        // Try to find mesh node
        string buffer = "blend_" + std::to_string(body->shape_id);
        AtNode *curr = AiNodeLookUpByName(body->get_name().c_str());
        AtNode *blendTemp = AiNodeLookUpByName(buffer.c_str());
        // If not found create
        if (blendTemp != nullptr)
        {
            AiNodeSetPtr(curr, "shader", blendTemp);
            AiNodeSetPtr(blendTemp, "mask", (void *)&cvMask);
            AiNodeSetPtr(blendTemp, "blend_image", (void *)&cvScene);
            continue;
        }

        // Image node and object material (?)
        AtNode *shaderMaterial = AiNode("standard");
        AtNode *image = AiNode("image");

        ostringstream out;
        out << std::internal << std::setfill('0') << std::setw(2) << (body->shape_id);
        string imgbuffer = "obj_" + out.str() + "_color.png";
        string imgPath = FILE_OBJ_PATH + "/" + imgbuffer;

        // Setup image node
        AiNodeSetStr(image, "filename", imgPath.c_str());
        AiNodeLink(image, "Kd_color", shaderMaterial);

        // Setup blending shader
        AtNode *shader_blend = AiNode("blendshader");
        AiNodeSetStr(shader_blend, "name", buffer.c_str());
        AiNodeSetPtr(shader_blend, "mask", (void *)&cvMask);
        AiNodeSetPtr(shader_blend, "blend_image", (void *)&cvScene);
        AiNodeSetBool(shader_blend, "force_scene", false);
        // Setup object material
        AiNodeSetFlt(shaderMaterial, "diffuse_roughness", 0.5);
        AiNodeSetFlt(shaderMaterial, "Ks", map_obj_ks[body->shape_id]);
        AiNodeLink(shaderMaterial, "Kd_bcolor", shader_blend);
        AiNodeSetPtr(curr, "shader", shader_blend);
    }

    // ?
    if (false)
    {
        cv::imshow("mask", cvMask);
        cv::imshow("scene", cvScene);
        cv::waitKey();
    }

    // Setup background image and mask
    AiNodeSetPtr(shader_blendBG, "mask", (void *)&cvMask);
    AiNodeSetPtr(shader_blendBG, "blend_image", (void *)&cvScene);
    AiNodeSetPtr(options, "background", shader_blendBG);

    // Destroy old null filter
    AtNode *myfilter = AiNodeLookUpByName("myfilter");
    AiNodeDestroy(myfilter);
    // Create new gauss (blur?) filter?
    myfilter = AiNode("gaussian_filter");
    AiNodeSetFlt(myfilter, "width", 1);
    AiNodeSetStr(myfilter, "name", "myfilter");

    ostringstream out;
    out << std::internal << std::setfill('0') << std::setw(6) << start_count;

    string buf = FILE_FINAL_PATH + "/final_imgs/img_" + out.str() + ".png";

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
    load_cam_mat();

    // Render final image/object blend
    AiArraySetStr(outputs_array, 0, "RGBA RGBA myfilter mydriver");
    AiNodeSetArray(options, "outputs", outputs_array);
    AiRender(AI_RENDER_MODE_CAMERA);
}

// Load camera matrices
void SceneManager::load_cam_mat()
{
    // Arnold camera
    AiNodeSetStr(camera, "name", "mycamera");

    std::ifstream inFile;

    // Open camera pose file
    inFile.open(cameraPoses.at(scene_count));

    int i = 0;
    std::string line;
    // Load camera matrices
    while (std::getline(inFile, line))
    {
        std::vector<std::string> entries = split(line, ' ');
        for (int j = 0; j < entries.size(); j++)
        {
            camMat(i, j) = std::stof(entries.at(j));
        }
        i++;
    }

    // Save scaled position
    float pos0 = camMat(0, 3) * 100;
    float pos1 = camMat(1, 3) * 100;
    float pos2 = camMat(2, 3) * 100;
    // Reset
    camMat(0, 3) = 0;
    camMat(1, 3) = 0;
    camMat(2, 3) = 0;

    Matrix4f switchAxisMat;
    Matrix4f switchAxisMat2;

    // ?
    switchAxisMat << -1, 0, 0, 0,
        0, 0, -1, 0,
        0, -1, 0, 0,
        0, 0, 0, 1;

    // ?
    switchAxisMat2 << -1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;

    // Spin around axis?
    camMat = (switchAxisMat * camMat);

    // Restore position
    camMat(0, 3) = pos0;
    camMat(2, 3) = pos1;
    camMat(1, 3) = pos2;

    // Create world to view matrix
    camMat = switchAxisMat2 * camMat.transpose();
    AtMatrix camMatArn = {{camMat(0, 0), camMat(0, 1), camMat(0, 2), camMat(0, 3)},
                          {camMat(1, 0), camMat(1, 1), camMat(1, 2), camMat(1, 3)},
                          {camMat(2, 0), camMat(2, 1), camMat(2, 2), camMat(2, 3)},
                          {camMat(3, 0), camMat(3, 1), camMat(3, 2), 1}};

    // Set FOV and camera to world matrix
    double fovx = 2 * atan(960 / (2 * intrinsic_vals[0]));
    double fovy = 2 * atan(540 / (2 * intrinsic_vals[1]));
    AiNodeSetMatrix(camera, "matrix", camMatArn);
    AtArray *fovArr = AiArray(2, 1, AI_TYPE_FLOAT, fovx * 180.f / 3.14f, fovy * 180.f / 3.14f);
    AiNodeSetArray(camera, "fov", fovArr);
    // TODO: 0.95 and 0.9 shifts shouldn't be fixed

    // ?
    float min_x = ((intrinsic_vals[2] * 0.95) - (960 / 2)) / (960 / 2) - 1;
    float max_x = ((intrinsic_vals[2] * 0.95) - (960 / 2)) / (960 / 2) + 1;
    float min_y = (intrinsic_vals[3] * 0.9 - (540 / 2)) / (540 / 2) - 1;
    float max_y = (intrinsic_vals[3] * 0.9 - (540 / 2)) / (540 / 2) + 1;

    // ?
    AiNodeSetPnt2(camera, "screen_window_min", min_x, min_y);
    AiNodeSetPnt2(camera, "screen_window_max", max_x, max_y);
}

// ?
void SceneManager::load_intrinsics()
{
    std::ifstream inFile;

    string buf = scene_path + "/rgbd/intrinsic/intrinsic_color.txt";

    cout << "sp " << scene_path << endl;
    inFile.open(buf);

    std::string line;
    int i = 0;
    std::vector<std::string> total;
    std::cout << buf << "\tbuffed" << std::endl;
    while (std::getline(inFile, line))
    {
        std::vector<std::string> entries = split(line, ' ');
        for (int i = 0; i < entries.size(); i++)
        {
            total.push_back(entries[i]);
        }
    }

    intrinsic_vals[0] = std::stof(total[0]);
    intrinsic_vals[1] = std::stof(total[5]);
    intrinsic_vals[2] = std::stof(total[2]);
    intrinsic_vals[3] = std::stof(total[6]);
    inFile.close();
    using namespace rapidxml;
    buf = scene_path + "/calibration.xml";
    file<> xmlFile(buf.c_str());
    xml_document<> doc;
    doc.parse<0>(xmlFile.data());
    xml_node<> *node = doc.first_node()->first_node()->next_sibling();
    node = node->first_node("camera_model")->first_node("params");

    vector<string> attrs = split(node->value(), ']');
    attrs = split(attrs.at(0), ';');

    intrinsic_vals[4] = std::stof(attrs.at(4));
    intrinsic_vals[5] = std::stof(attrs.at(5));
    intrinsic_vals[6] = std::stof(attrs.at(6));
}

// Cleanup arnold meshes
void SceneManager::remove_bodies_ai()
{
    for (ObjectInfo *curr : curr_objects)
    {
        AiMeshManager::destroyObject(curr->get_name());
    }
}

// Saves image and pose files
void SceneManager::getFilesInAdirectory(string path, int type)
{
    // Poses or images?
    if (type == 1)
    {
        cameraPoses = std::vector<std::string>();
    }
    else
    {
        camImages = std::vector<std::string>();
    }

    DIR *dir;
    struct dirent *ent;

    // Load files
    if ((dir = opendir(path.c_str())) != NULL)
    {
        // For each file
        while ((ent = readdir(dir)) != NULL)
        {
            std::string eName = ent->d_name;

            string buf = path + "/" + ent->d_name;

            if (ent->d_name[0] == '.')
                continue;

            // Save in vector
            if (type == 0)
            {
                camImages.push_back((std::string)buf);
            }
            else
            {
                cameraPoses.push_back((std::string)buf);
            }
        }
        // Finally sort vectors and close dir
        std::sort(cameraPoses.begin(), cameraPoses.end());
        std::sort(camImages.begin(), camImages.end());
        closedir(dir);
    }
}

// Runs the simulation
void SceneManager::run(int iter)
{
    // ?
    load_intrinsics();

    // Load poses and images
    getFilesInAdirectory(scene_path + "/rgbd/pose", 1);
    getFilesInAdirectory(scene_path + "/rgbd/color", 0);

    // Render depth
    render_scene_depth_imgs();

    // Run physx simulation
    for (int count = 0; count < 30; count++)
    {
        // Create physx representation of scan scene
        draw_scene();
        // Create physx objects
        generate_objects();
        // Run the simulation
        run_physx_sim();
        // Create arnold meshes
        draw_bodies();

        string buf = FILE_FINAL_PATH + "/labels.csv";

        ANNOTATIONS_FILE.open(buf, std::ios_base::app);
        // For each scene
        for (scene_count = 0; scene_count < cameraPoses.size(); scene_count++)
        {
            // Render objects depth and calculate depth mask
            if (render_bodies_depth() && calculate_mask())
            {
                // Render object labels (IDs as color?)
                render_bodies_seg();
                // Blend depth images
                blend_depth();
                // Blend label and mask images
                blend_seg();

                // Read scene color image
                cvScene = cv::imread(camImages.at(scene_count), cv::IMREAD_COLOR);
                // Load camera intrinsics
                cv::Mat intrinsic = (cv::Mat1d(3, 3) << intrinsic_vals[0], 0, intrinsic_vals[2], 0, intrinsic_vals[1], intrinsic_vals[3], 0, 0, 1);
                cv::Mat distCoeffs = (cv::Mat1d(1, 5) << intrinsic_vals[4], intrinsic_vals[5], 0, 0, intrinsic_vals[6]);
                cv::Mat temp = cvScene.clone();
                // Undistort the color image (?)
                cv::undistort(cvScene, temp, intrinsic, distCoeffs);
                cvScene = temp;

                // Final image blend
                blend_rgb();
                start_count++;
            }
        }
        // Cleanup
        remove_bodies_ai();
        ANNOTATIONS_FILE.close();
    }
}

// New object transform
SceneManager::ObjectInfo::ObjectInfo(int shape_id, int object_sim_id) : shape_id(shape_id), object_sim_id(object_sim_id){};

// Set position and rotation
void SceneManager::ObjectInfo::set_pose(Vector3f *pos, Quaterniond *rot)
{
    this->pos = pos;
    this->rot = rot;
}

// Get position
Vector3f *SceneManager::ObjectInfo::get_pos()
{
    return pos;
}

// Get rotation
Quaterniond *SceneManager::ObjectInfo::get_rot()
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
