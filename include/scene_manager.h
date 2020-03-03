#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <eigen3/Eigen/Dense>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include "mesh_managers.h"
#include "../render/include/render.h"

using namespace Eigen;

struct Camera{
    float fx;
    float fy;
    float ox;
    float oy;
};

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
    Render *render;
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

    cv::Mat cvMask, cvScene, cvRend, cvSceneD, cvBodiesS, cvBodiesD;
    Matrix4f camMat;
    Vector3f camPos;

    Camera intrinsic_scene, intrinsics_render_out;

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
    float computeVarianceOfLaplacian(const cv::Mat &image);
    void render_scene_depth_imgs();
    vector<tuple<cv::Mat, cv::Mat> > render_scenes_gl();
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
    void load_cam_mat(float fx, float fy, float ox, float oy);
    void load_intrinsics();
    void render_image();
    void remove_bodies_ai();
    void getFilesInAdirectory(string, float variance_threshold);

    void setAnnPose(BodyAnnotation &ann, Vector3f *pos, Quaterniond *q);
    void set_annotations(cv::Mat seg, cv::Mat segMasked);

public:
    SceneManager(PxPhysics *gPhysics, PxScene *gScene, PxCooking *gCooking,
                 PxFoundation *gFoundation, PxMaterial *gMaterial, AtNode *camera,
                 AtNode *options, AtNode *driver, AtArray *outputs_array,
                 vector<PxConvManager *> physx_objs, vector<AiMeshManager *> sim_objs,
                 int start_count, int obj_per_sim, rapidjson::Document *CONFIG_FILE,
                 AtNode *shader_obj_depth, AtNode *shader_bck_depth, AtNode *shader_blendBG);
    ~SceneManager();
    void set_scene_path(string);
    bool run(int iter, int max_count);
};

/*
* Transform struct for objects.
* Saves rotation and position as well as mesh IDs.
*/
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
