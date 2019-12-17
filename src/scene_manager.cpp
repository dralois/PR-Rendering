#include <dirent.h>

#include "scene_manager.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"

string FILE_TEMP_PATH = "";
string FILE_FINAL_PATH = "";
string FILE_OBJ_PATH = "";
using namespace std;

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

    FILE_TEMP_PATH = (*CONFIG_FILE)["temp_files_path"].GetString();
    FILE_OBJ_PATH = (*CONFIG_FILE)["textured_objs"].GetString();
    FILE_FINAL_PATH = (*CONFIG_FILE)["final_imgs_path"].GetString();
}

void SceneManager::set_scene_path(string scene_path)
{
    this->scene_path = scene_path;
}

void SceneManager::draw_scene()
{
    string mesh_path = scene_path + "/mesh.refined.obj";

    sceneMeshManager = new PxTriManager(mesh_path, 0, 100, gPhysics, gScene, gCooking, gMaterial);
    sceneMeshManager->calculateBounds = true;
    sceneMeshManager->doubleNorms = true;
    sceneMeshManager->load_file();
    sceneMeshManager->drawMeshShape();
    vector<float> pos{0, 0, 0};
    vector<float> rot{0, 0, 0, 1};
    sceneMeshManager->generateObj(pos, rot);
}

void SceneManager::generate_objects()
{
    srand(time(NULL) % 1000);

    for (PxU32 i = 0; i < obj_per_sim; i++)
    {
        int objManagerPos = rand() % physx_objs.size();
        PxConvManager *objManager = physx_objs.at(objManagerPos);

        float y = - (rand() % 100);
        float x = (rand() % ((int)(sceneMeshManager->xmax * 100 - sceneMeshManager->xmin * 100))) + sceneMeshManager->xmin * 100;
        float z = (rand() % ((int)(sceneMeshManager->ymax * 100 - sceneMeshManager->ymin * 100))) + sceneMeshManager->ymin * 100;
        vector<float> pos{x, y, z};
        vector<float> rot{-0.7071068, 0, 0, 0.7071068};

        PxRigidDynamic *body = objManager->generateObj(pos, rot);
        sim_objects.push_back(make_pair(objManager, body));
    }
}

void SceneManager::destroy_meshes()
{
    if (sceneMeshManager != nullptr)
        free(sceneMeshManager);
    for (auto obj : sim_objects)
    {
        PxMeshManager::destroyObject(obj.second);
    }
    sim_objects.clear();
}

void SceneManager::run_physx_sim()
{
    for (PxU32 i = 0; i < 2000; i++)
    {
        gScene->simulate(3.0f / 60.0f);
        gScene->fetchResults(true);
        PxTransform tempTrans;
        tempTrans = sim_objects[0].second->getGlobalPose();
        if (i % 10 == 0)
        {
            // cout << tempTrans.p.x << "c " << tempTrans.p.y << "c " << tempTrans.p.z  << "c " << endl;
        }
    }
    fetch_results();
    destroy_meshes();
}

void SceneManager::fetch_results()
{
    PxTransform tempTrans;
    int i = 0;
    for (auto obj : sim_objects)
    {
        i++;
        tempTrans = obj.second->getGlobalPose();
        SceneManager::ObjectInfo *currInfo = new SceneManager::ObjectInfo(obj.first->get_id(), i);
        PxQuat quat = tempTrans.q.getNormalized();
        PxVec3 pxPos = tempTrans.p;
        Vector3f *pos = new Vector3f(pxPos.x, pxPos.y, pxPos.z);
        Quaterniond *rot = new Quaterniond(quat.w, quat.x, quat.y, quat.z);
        currInfo->set_pose(pos, rot);
        curr_objects.push_back(currInfo);
    }
}

void SceneManager::render_scene_depth_imgs()
{
    string mesh_path = scene_path + "/mesh.refined.obj";
    sceneAiMeshManager = new AiMeshManager(mesh_path, 0, 100);
    vector<float> pos{0, 0, 0};
    vector<float> rot{0, 0, 0, 1};
    sceneAiMeshManager->is_scene = true;
    sceneAiMeshManager->load_file();
    AtNode *sceneMesh = sceneAiMeshManager->drawObj(0, pos, rot);
    AtNode *sceneNode = AiNodeLookUpByName("body0_0");
    AiNodeSetBool(shader_obj_depth, "is_body", true);
    AiNodeSetPtr(sceneNode, "shader", shader_obj_depth);
    // AiNodeSetInt();
    AiNodeSetFlt(shader_bck_depth, "force_val", 20000);
    AiNodeSetPtr(options, "background", shader_bck_depth);

    AtNode *myfilter = AiNodeLookUpByName("myfilter");
    AiNodeDestroy(myfilter);

    myfilter = AiNode("null_filter");
    AiNodeSetFlt(myfilter, "width", 1);
    AiNodeSetStr(myfilter, "name", "myfilter");

    AiNodeSetInt(driver, "format", AI_TYPE_INT);
    AiNodeSetInt(options, "xres", 1920);
    AiNodeSetInt(options, "yres", 1080);

    AiNodeSetInt(options, "GI_diffuse_depth", 1);
    AiNodeSetInt(options, "AA_samples", 1);
    AiNodeSetInt(options, "GI_glossy_samples", 1);

    for (scene_count = 0; scene_count < cameraPoses.size(); scene_count++)
    {

        // cout << cameraPoses.at(scene_count) << endl;
        load_cam_mat();

        ostringstream out;
        out << std::internal << std::setfill('0') << std::setw(6) << scene_count;

        string buf = FILE_TEMP_PATH + "/scene_depth/img_" + out.str() + ".png";

        AiNodeSetStr(driver, "filename", buf.c_str());
        AiArraySetStr(outputs_array, 0, "RGBA RGBA myfilter mydriver");
        AiNodeSetArray(options, "outputs", outputs_array);
        AiRender(AI_RENDER_MODE_CAMERA);
    }
    scene_count = 0;

    AiMeshManager::destroyObject("body0_0");
}

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

void setBBox(BodyAnnotation *ann, cv::Mat mask)
{
    Rect Min_Rect = cv::boundingRect(mask);
    Min_Rect.x += Min_Rect.width / 2.f;
    Min_Rect.y += Min_Rect.height / 2.f;
    ann->bbox.push_back(Min_Rect.x);
    ann->bbox.push_back(Min_Rect.y);
    ann->bbox.push_back(Min_Rect.width);
    ann->bbox.push_back(Min_Rect.height);
}

void SceneManager::setAnnPose(BodyAnnotation *ann, Vector3f *pos, Quaterniond *q)
{
    // use camPos and camRot
    using namespace Eigen;
    // Vector3f bPos;
    // bPos << pos->x, pos->y, pos->z;

    // Matrix<float, 3, 3> camMat3 = Eigen::Matrix3f::Identity();
    // camMat3 = camRot.block(0, 0, 3, 3);
    // bPos = camMat3.inverse() * (bPos - camPos);

    Matrix<float, 4, 4> bRotMat = Eigen::Matrix4f::Identity();
    bRotMat.block(0, 0, 3, 3) = q->normalized().toRotationMatrix().cast<float>();
    bRotMat = camMat.transpose().inverse() * bRotMat;

    Matrix3f temp = bRotMat.block(0, 0, 3, 3);
    Quaternionf qr = Quaternionf(temp).normalized();

    ann->trans.push_back(bRotMat(0, 3) * 10);
    ann->trans.push_back(bRotMat(1, 3) * 10);
    ann->trans.push_back(bRotMat(2, 3) * 10);
    ann->quat.push_back(qr.w());
    ann->quat.push_back(qr.x());
    ann->quat.push_back(qr.y());
    ann->quat.push_back(qr.z());
}

void SceneManager::set_annotations(Mat seg, Mat segMasked)
{

    for (auto currBody : curr_objects)
    {
        float segBodySum = cv::sum(seg == (currBody->object_sim_id + 1) * 10)[0];
        if (segBodySum == 0)
            continue;

        float segBodySumMasked = cv::sum(segMasked == (currBody->object_sim_id + 1) * 10)[0];
        float percent = segBodySumMasked / segBodySum;

        if (percent <= 0.3 || segBodySumMasked / 255 < 2000)
            continue;

        BodyAnnotation *currAnn = new BodyAnnotation();
        currAnn->trans = std::vector<float>();
        currAnn->quat = std::vector<float>();

        currAnn->bbox = std::vector<float>();
        currAnn->id = currBody->shape_id;
        currAnn->sim_id = (currBody->object_sim_id + 1) * 10;
        setBBox(currAnn, seg == (currBody->object_sim_id + 1) * 10);
        setAnnPose(currAnn, currBody->get_pos(), currBody->get_rot());

        ostringstream out;
        out << std::internal << std::setfill('0') << std::setw(2) << currAnn->id;
        string buf = "obj_" + out.str();

        ANNOTATIONS_FILE << start_count << ", " << currAnn->bbox[0] << ", " << currAnn->bbox[1] << ", "
                         << currAnn->bbox[2] << ", " << currAnn->bbox[3] << ", " << buf << ", " << currAnn->quat[0] << ", "
                         << currAnn->quat[1] << ", " << currAnn->quat[2] << ", " << currAnn->quat[3] << ", "
                         << "0"
                         << ", "
                         << "0"
                         << ", " << currAnn->trans[0] << ", " << currAnn->trans[1] << ", " << currAnn->trans[2] << ", "
                         << currAnn->sim_id << "\n";

        //delete[] buf;
    }
}

void SceneManager::draw_bodies()
{
    sim_objs = {};
    for (int i = 0; i < 16; i++)
        sim_objs.push_back(NULL);

    for (auto body : curr_objects)
    {
        AiMeshManager *mesh = sim_objs.at(body->shape_id);
        if (sim_objs.at(body->shape_id) == NULL)
        {
            ostringstream out;
            out << std::internal << std::setfill('0') << std::setw(2) << (body->shape_id);
            string buf = FILE_OBJ_PATH + "/obj_" + out.str() + ".obj";
            mesh = new AiMeshManager(buf, body->shape_id, 0.1);
            mesh->load_file();
            sim_objs.at(body->shape_id) = mesh;
            //delete[] buf;
        }

        Vector3f *p = body->get_pos();
        Quaterniond *q = body->get_rot();
        vector<float> pos{p->x(), p->y(), p->z()};
        vector<float> rot{(float)q->x(), (float)q->y(), (float)q->z(), (float)q->w()};
        mesh->drawObj(body->object_sim_id, pos, rot);
    }
}

bool SceneManager::checkIfCenterOnImage(ObjectInfo *body)
{
    Quaterniond *q = body->get_rot();
    Matrix<float, 4, 4> bRotMat = Eigen::Matrix4f::Identity();
    bRotMat.block(0, 0, 3, 3) = q->normalized().toRotationMatrix().cast<float>();

    Vector3f *pos = body->get_pos();
    bRotMat(0, 3) = pos->x();
    bRotMat(1, 3) = pos->y();
    bRotMat(2, 3) = pos->z();
    bRotMat = camMat.transpose().inverse() * bRotMat;

    int width = 960;
    int height = 540;

    float x = width / 2.f + bRotMat(0, 3) * 756 / bRotMat(2, 3);
    float y = height / 2.f + bRotMat(1, 3) * 756 / bRotMat(2, 3);
    x += 20;
    y += 20;
    return (x > 40 && x < width && y > 40 && y < height);
}

bool SceneManager::render_bodies_depth()
{
    bool valid = false;
    for (auto body : curr_objects)
    {
        AtNode *curr = AiNodeLookUpByName(body->get_name().c_str());

        //TODO: This Check kciks out everything!
        bool closeToCamera = sqrt(pow(body->get_pos()->x() - camMat(3, 0), 2) + pow(body->get_pos()->y() - camMat(3, 1), 2) + pow(body->get_pos()->z() - camMat(3, 2), 2)) > 150;
        // cout << sqrt(pow(body->get_pos()->x() - camMat(3, 0), 2) + pow(body->get_pos()->y() - camMat(3, 1), 2) + pow(body->get_pos()->z() - camMat(3, 2), 2)) << endl;
        // cout << checkIfCenterOnImage(body) << endl;
        if (closeToCamera || !checkIfCenterOnImage(body))
        {
            AiNodeSetPtr(curr, "shader", shader_bck_depth);
            // AiNodeSetByte(curr, "visibility", (char)0);
        }
        else
        {
            AiNodeSetPtr(curr, "shader", shader_obj_depth);
            valid = true;
        }
    }

    AiNodeSetBool(shader_obj_depth, "is_body", true);
    AiNodeSetFlt(shader_bck_depth, "force_val", 30000);
    AiNodeSetPtr(options, "background", shader_bck_depth);

    AtNode *myfilter = AiNodeLookUpByName("myfilter");
    AiNodeDestroy(myfilter);

    myfilter = AiNode("null_filter");
    AiNodeSetFlt(myfilter, "width", 1);
    AiNodeSetStr(myfilter, "name", "myfilter");

    ostringstream out;
    out << std::internal << std::setfill('0') << std::setw(6) << scene_count;

    string buf = FILE_TEMP_PATH + "/" + "body_depth/img_" + out.str() + ".png";

    AiNodeSetStr(driver, "filename", buf.c_str());
    AiNodeSetInt(driver, "format", AI_TYPE_INT);
    AiNodeSetInt(options, "xres", 1920);
    AiNodeSetInt(options, "yres", 1080);
    AiNodeSetInt(options, "GI_diffuse_depth", 1);
    AiNodeSetInt(options, "AA_samples", 1);
    AiNodeSetInt(options, "GI_glossy_samples", 1);

    load_cam_mat();

    AiArraySetStr(outputs_array, 0, "RGBA RGBA myfilter mydriver");
    AiNodeSetArray(options, "outputs", outputs_array);
    if (valid)
        AiRender(AI_RENDER_MODE_CAMERA);
    // for (auto body : curr_objects)
    // {
    //     AtNode *curr = AiNodeLookUpByName(body->get_name().c_str());
    //     AiNodeSetByte(curr, "visibility", (unsigned char)244);
    // }
    return valid;
}

bool SceneManager::calculate_mask()
{
    ostringstream out;
    out << std::internal << std::setfill('0') << std::setw(6) << scene_count;
    string sbuf = FILE_TEMP_PATH + "/scene_depth/img_" + out.str() + ".png";
    string bbuf = FILE_TEMP_PATH + "/body_depth/img_" + out.str() + ".png";
    cvSceneD = cv::imread(sbuf, cv::IMREAD_ANYDEPTH);
    cvBodiesD = cv::imread(bbuf, cv::IMREAD_ANYDEPTH);
    if (false)
    {
        cv::imshow("scene depth", cvSceneD);
        cv::imshow("bodies depth", cvBodiesD);
        cv::waitKey();
    }
    cvMask = cvBodiesD < cvSceneD;

    Scalar maskMean = cv::mean(cvMask);
    return maskMean[0] >= 1.f;
}

void SceneManager::render_bodies_seg()
{
    for (auto body : curr_objects)
    {
        string sbuffer = "label_" + std::to_string(body->object_sim_id);

        AtNode *shader_obj_label;
        shader_obj_label = AiNodeLookUpByName(sbuffer.c_str());
        if (shader_obj_label == NULL)
        {
            shader_obj_label = AiNode("labelshader");
            AiNodeSetStr(shader_obj_label, "name", sbuffer.c_str());
            AiNodeSetInt(shader_obj_label, "id", (body->object_sim_id + 1) * 10);
        }
        AtNode *curr = AiNodeLookUpByName(body->get_name().c_str());
        AiNodeSetPtr(curr, "shader", shader_obj_label);
    }
    AtNode *shader_bck_label = AiNodeLookUpByName("label_background");
    if (shader_bck_label == NULL)
    {
        shader_bck_label = AiNode("labelshader");
        AiNodeSetStr(shader_bck_label, "name", "label_background");
        AiNodeSetInt(shader_bck_label, "id", 0);
    }
    AiNodeSetPtr(options, "background", shader_bck_label);

    AtNode *myfilter = AiNodeLookUpByName("myfilter");
    AiNodeDestroy(myfilter);

    myfilter = AiNode("null_filter");
    AiNodeSetFlt(myfilter, "width", 1);
    AiNodeSetStr(myfilter, "name", "myfilter");

    ostringstream out;
    out << std::internal << std::setfill('0') << std::setw(6) << scene_count;
    string buf = FILE_TEMP_PATH + "/body_label/img_" + out.str() + ".png";

    AiNodeSetStr(driver, "filename", buf.c_str());
    AiNodeSetInt(driver, "format", AI_TYPE_INT);
    AiNodeSetInt(options, "xres", 1920);
    AiNodeSetInt(options, "yres", 1080);
    AiNodeSetInt(options, "GI_diffuse_depth", 1);
    AiNodeSetInt(options, "AA_samples", 1);
    AiNodeSetInt(options, "GI_glossy_samples", 1);

    load_cam_mat();

    AiArraySetStr(outputs_array, 0, "RGBA RGBA myfilter mydriver");
    AiNodeSetArray(options, "outputs", outputs_array);
    AiRender(AI_RENDER_MODE_CAMERA);
}

void SceneManager::blend_depth()
{
    cv::Mat cvBodiesD, cvOut;

    ostringstream out;
    out << std::internal << std::setfill('0') << std::setw(6) << scene_count;

    string sbuf = FILE_TEMP_PATH + "/scene_depth/img_" + out.str() + ".png";
    string bbuf = FILE_TEMP_PATH + "/body_depth/img_" + out.str() + ".png";

    cvSceneD = cv::imread(sbuf, cv::IMREAD_ANYDEPTH);
    cvBodiesD = cv::imread(bbuf, cv::IMREAD_ANYDEPTH);
    cvOut = cv::min(cvSceneD, cvBodiesD);

    ostringstream out_start;
    out_start << std::internal << std::setfill('0') << std::setw(6) << start_count;

    string obuf = FILE_FINAL_PATH + "/final_imgs/img_" + out_start.str() + "_depth.png";
    cv::imwrite(obuf, cvOut);
}

void SceneManager::blend_seg()
{
    cv::Mat cvOut;

    ostringstream out;
    out << std::internal << std::setfill('0') << std::setw(6) << scene_count;

    ostringstream out_start;
    out_start << std::internal << std::setfill('0') << std::setw(6) << start_count;

    string bbuf = FILE_TEMP_PATH + "/body_label/img_" + out.str() + ".png";
    string obuf = FILE_FINAL_PATH + "/final_imgs/img_" + out_start.str() + "_seg.png";

    cvBodiesS = cv::imread(bbuf, cv::IMREAD_ANYDEPTH);

    cvBodiesS.copyTo(cvOut, cvMask);
    cv::imwrite(obuf, cvOut);

    set_annotations(cvBodiesS, cvOut);
}
void SceneManager::blend_rgb()
{

    std::vector<float> map_obj_ks = {0.01, 0.05, 0.05, 0.05, 0.03, 0.01, 0.03, 0.02, 0.03, 0.02, 0.02, 0.03, 0.03};
    for (auto body : curr_objects)
    {

        string buffer = "blend_" + std::to_string(body->shape_id);
        AtNode *curr = AiNodeLookUpByName(body->get_name().c_str());
        AtNode *blendTemp = AiNodeLookUpByName(buffer.c_str());

        if (blendTemp != nullptr)
        {
            AiNodeSetPtr(curr, "shader", blendTemp);
            AiNodeSetPtr(blendTemp, "mask", (void *)&cvMask);
            AiNodeSetPtr(blendTemp, "blend_image", (void *)&cvScene);
            continue;
        }
        AtNode *shaderMaterial = AiNode("standard");
        AtNode *image = AiNode("image");

        ostringstream out;
        out << std::internal << std::setfill('0') << std::setw(2) << (body->shape_id);
        string imgbuffer = "obj_" + out.str() + "_color.png";
        string imgPath = FILE_OBJ_PATH + "/" + imgbuffer;

        AiNodeSetStr(image, "filename", imgPath.c_str());
        AiNodeLink(image, "Kd_color", shaderMaterial);

        AtNode *shader_blend = AiNode("blendshader");
        AiNodeSetStr(shader_blend, "name", buffer.c_str());
        AiNodeSetPtr(shader_blend, "mask", (void *)&cvMask);
        AiNodeSetPtr(shader_blend, "blend_image", (void *)&cvScene);
        AiNodeSetBool(shader_blend, "force_scene", false);
        AiNodeSetFlt(shaderMaterial, "diffuse_roughness", 0.5);
        AiNodeSetFlt(shaderMaterial, "Ks", map_obj_ks[body->shape_id]);
        AiNodeLink(shaderMaterial, "Kd_bcolor", shader_blend);
        AiNodeSetPtr(curr, "shader", shader_blend);
    }

    if (false)
    {
        cv::imshow("mask", cvMask);
        cv::imshow("scene", cvScene);
        cv::waitKey();
    }
    AiNodeSetPtr(shader_blendBG, "mask", (void *)&cvMask);
    AiNodeSetPtr(shader_blendBG, "blend_image", (void *)&cvScene);
    AiNodeSetPtr(options, "background", shader_blendBG);

    AtNode *myfilter = AiNodeLookUpByName("myfilter");
    AiNodeDestroy(myfilter);

    myfilter = AiNode("gaussian_filter");
    AiNodeSetFlt(myfilter, "width", 1);
    AiNodeSetStr(myfilter, "name", "myfilter");

    ostringstream out;
    out << std::internal << std::setfill('0') << std::setw(6) << start_count;

    string buf = FILE_FINAL_PATH + "/final_imgs/img_" + out.str() + ".png";

    AiNodeSetStr(driver, "filename", buf.c_str());
    AiNodeSetInt(driver, "format", AI_TYPE_RGBA);
    AiNodeSetFlt(driver, "gamma", 1.0f);
    AiNodeSetInt(options, "xres", 960);
    AiNodeSetInt(options, "yres", 540);
    AiNodeSetInt(options, "AA_samples", 6);
    AiNodeSetInt(options, "GI_diffuse_depth", 6);
    AiNodeSetInt(options, "GI_glossy_samples", 6);

    load_cam_mat();

    AiArraySetStr(outputs_array, 0, "RGBA RGBA myfilter mydriver");
    AiNodeSetArray(options, "outputs", outputs_array);
    AiRender(AI_RENDER_MODE_CAMERA);
}

void SceneManager::load_cam_mat()
{

    //if(AiNodeLookUpByName("mycamera") == NULL)
    AiNodeSetStr(camera, "name", "mycamera");

    std::ifstream inFile;

    inFile.open(cameraPoses.at(scene_count));

    std::string line;
    int i = 0;
    while (std::getline(inFile, line))
    {
        std::vector<std::string> entries = split(line, ' ');
        for (int j = 0; j < entries.size(); j++)
        {
            camMat(i, j) = std::stof(entries.at(j));
            // cout << camMat(i, j) << " ";
        }
        // cout << endl;
        i++;
    }
    // Vector3f posMat;
    // posMat << camMat(0, 3), camMat(1, 3), camMat(2, 3);
    float pos0 = camMat(0, 3) * 100;
    float pos1 = camMat(1, 3) * 100;
    float pos2 = camMat(2, 3) * 100;
    camMat(0, 3) = 0;
    camMat(1, 3) = 0;
    camMat(2, 3) = 0;
    Matrix4f switchAxisMat;
    Matrix4f switchAxisMat2; //free(&temp);
    switchAxisMat << -1, 0, 0, 0,
        0, 0, -1, 0,
        0, -1, 0, 0,
        0, 0, 0, 1;

    switchAxisMat2 << -1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;

    camMat = (switchAxisMat * camMat);

    camMat(0, 3) = pos0;
    camMat(2, 3) = pos1;
    camMat(1, 3) = pos2;

    camMat = switchAxisMat2 * camMat.transpose();
    AtMatrix camMatArn = {{camMat(0, 0), camMat(0, 1), camMat(0, 2), camMat(0, 3)},
                          {camMat(1, 0), camMat(1, 1), camMat(1, 2), camMat(1, 3)},
                          {camMat(2, 0), camMat(2, 1), camMat(2, 2), camMat(2, 3)},
                          {camMat(3, 0), camMat(3, 1), camMat(3, 2), 1}};

    double fovx = 2 * atan(960 / (2 * intrinsic_vals[0]));
    double fovy = 2 * atan(540 / (2 * intrinsic_vals[1]));
    AiNodeSetMatrix(camera, "matrix", camMatArn);
    AtArray *fovArr = AiArray(2, 1, AI_TYPE_FLOAT, fovx * 180.f / 3.14f, fovy * 180.f / 3.14f);
    AiNodeSetArray(camera, "fov", fovArr);
    // TODO: 0.95 and 0.9 shifts shouldn't be fixed

    float min_x = ((intrinsic_vals[2] * 0.95) - (960 / 2)) / (960 / 2) - 1;
    float max_x = ((intrinsic_vals[2] * 0.95) - (960 / 2)) / (960 / 2) + 1;
    float min_y = (intrinsic_vals[3] * 0.9 - (540 / 2)) / (540 / 2) - 1;
    float max_y = (intrinsic_vals[3] * 0.9 - (540 / 2)) / (540 / 2) + 1;

    AiNodeSetPnt2(camera, "screen_window_min", min_x, min_y);
    AiNodeSetPnt2(camera, "screen_window_max", max_x, max_y);
}

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

void SceneManager::remove_bodies_ai()
{
    for (ObjectInfo *curr : curr_objects)
    {
        AiMeshManager::destroyObject(curr->get_name());
    }
}

void SceneManager::getFilesInAdirectory(string path, int type)
{
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

    if ((dir = opendir(path.c_str())) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            std::string eName = ent->d_name;

            string buf = path + "/" + ent->d_name;

            if (ent->d_name[0] == '.')
                continue;
            if (type == 0)
            {
                camImages.push_back((std::string)buf);
            }
            else
            {
                cameraPoses.push_back((std::string)buf);
            }
        }

        std::sort(cameraPoses.begin(), cameraPoses.end());
        std::sort(camImages.begin(), camImages.end());
        closedir(dir);
    }
}

void SceneManager::run(int iter)
{
    load_intrinsics();

    getFilesInAdirectory(scene_path + "/rgbd/pose", 1);
    getFilesInAdirectory(scene_path + "/rgbd/color", 0);
    render_scene_depth_imgs();

    for (int count = 0; count < 30; count++)
    {
        draw_scene();
        generate_objects();
        run_physx_sim();
        // fetch_results();
        // destroy_meshes(); // TODO: Destroy meshes frees space it should not free

        draw_bodies();
        string buf = FILE_FINAL_PATH + "/labels.csv";

        ANNOTATIONS_FILE.open(buf, std::ios_base::app);
        //start_count = 0;
        for (scene_count = 0; scene_count < cameraPoses.size(); scene_count++)
        {

            if (render_bodies_depth() && calculate_mask())
            {
                
                render_bodies_seg();
                blend_depth();
                blend_seg();

                cvScene = cv::imread(camImages.at(scene_count), cv::IMREAD_COLOR);
                cv::Mat intrinsic = (cv::Mat1d(3, 3) << intrinsic_vals[0], 0, intrinsic_vals[2], 0, intrinsic_vals[1], intrinsic_vals[3], 0, 0, 1);

                cv::Mat distCoeffs = (cv::Mat1d(1, 5) << intrinsic_vals[4], intrinsic_vals[5], 0, 0, intrinsic_vals[6]);
                cv::Mat temp = cvScene.clone();
                cv::undistort(cvScene, temp, intrinsic, distCoeffs);
                cvScene = temp;

                blend_rgb();
                start_count++;
            }
        }
        // destroy_meshes();
        remove_bodies_ai();
        ANNOTATIONS_FILE.close();
    }
}

SceneManager::ObjectInfo::ObjectInfo(int shape_id, int object_sim_id) : shape_id(shape_id), object_sim_id(object_sim_id){};

void SceneManager::ObjectInfo::set_pose(Vector3f *pos, Quaterniond *rot)
{
    this->pos = pos;
    this->rot = rot;
}

Vector3f *SceneManager::ObjectInfo::get_pos()
{
    return pos;
}

Quaterniond *SceneManager::ObjectInfo::get_rot()
{
    return rot;
}

string SceneManager::ObjectInfo::get_name()
{
    string ret = "body" + to_string(shape_id) + "_" + to_string(object_sim_id);
    return ret;
}

SceneManager::ObjectInfo::~ObjectInfo()
{
    free(pos);
    free(rot);
}
