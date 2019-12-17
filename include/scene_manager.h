#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <eigen3/Eigen/Dense>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include "mesh_managers.h"

using namespace cv;
using namespace Eigen;

struct BodyAnnotation
{
    int id;
    int sim_id;
    char *name;
    std::vector<float> bbox;
    std::vector<float> trans;
    std::vector<float> quat;
};

/*
 * Manager class for a scene.
 * Responsible of automating the process of generating final blended
 * RGB images
 */
class SceneManager
{
private:
    PxPhysics *gPhysics;
    PxScene *gScene;
    PxCooking *gCooking;
    PxFoundation *gFoundation;

    AtNode *camera;
    AtNode *options;
    AtNode *driver;
    AtArray *outputs_array;
    AtNode *shader_obj_depth;
    AtNode *shader_bck_depth;
    AtNode *shader_blendBG;

    PxMaterial *gMaterial;

    vector<PxConvManager *> physx_objs;
    vector<AiMeshManager *> sim_objs;
    vector<string> cameraPoses;
    vector<string> camImages;
    int start_count;
    int obj_per_sim;
    string scene_path;

    Mat cvMask, cvScene, cvSceneD, cvBodiesS, cvBodiesD;
    Matrix4f camMat;
    Vector3f camPos;
    float intrinsic_vals[7];

    class ObjectInfo;
    vector<ObjectInfo *> curr_objects;
    vector<pair<PxConvManager *, PxRigidDynamic *> > sim_objects;
    PxTriManager *sceneMeshManager;
    AiMeshManager *sceneAiMeshManager;
    PxRigidDynamic *myScene;
    int scene_count;

    std::ofstream ANNOTATIONS_FILE;
    rapidjson::Document *CONFIG_FILE;

    void draw_scene();
    void generate_objects();
    void destroy_meshes();
    void run_physx_sim();
    void fetch_results();

    bool checkIfCenterOnImage(ObjectInfo *);
    void render_scene_depth_imgs();
    bool render_bodies_depth();
    // returns a bool stating if the final image should be rendered
    // or no depending on the visible parts of objects and distance
    void draw_bodies();
    bool calculate_mask();
    void render_bodies_seg();
    void blend_depth();
    void blend_seg();
    void blend_rgb();
    void set_cam_mat(string path);
    void load_cam_mat();
    void load_intrinsics();
    void render_image();
    void remove_bodies_ai();
    void getFilesInAdirectory(string, int);

    void setAnnPose(BodyAnnotation *ann, Vector3f *pos, Quaterniond *q);
    void set_annotations(Mat seg, Mat segMasked);

public:
    SceneManager(PxPhysics *gPhysics, PxScene *gScene, PxCooking *gCooking,
                 PxFoundation *gFoundation, PxMaterial *gMaterial, AtNode *camera,
                 AtNode *options, AtNode *driver, AtArray *outputs_array,
                 vector<PxConvManager *> physx_objs, vector<AiMeshManager *> sim_objs,
                 int start_count, int obj_per_sim, rapidjson::Document *CONFIG_FILE,
                 AtNode *shader_obj_depth, AtNode *shader_bck_depth, AtNode *shader_blendBG);
    void set_scene_path(string);
    void run(int iter);
};

class SceneManager::ObjectInfo
{
private:
    Vector3f *pos;
    Quaterniond *rot;

public:
    int shape_id;
    int object_sim_id;
    ObjectInfo(int, int);
    void set_pose(Vector3f *, Quaterniond *);
    Vector3f *get_pos();
    Quaterniond *get_rot();
    string get_name();
    ~ObjectInfo();
};
