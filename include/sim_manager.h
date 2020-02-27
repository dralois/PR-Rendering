#include "scene_manager.h"
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

/*
 * Simulation manager class.
 * Responsible of managing the whole process of loading files
 * of scenes and objects and passing it to scene managers.
 */
class SimManager
{
private:
    vector<PxConvManager *> physx_objs;
    vector<AiMeshManager *> sim_objs;

    PxPhysics *gPhysics;
    PxScene *gScene;
    PxCooking *gCooking;
    PxFoundation *gFoundation;
    PxMaterial *gMaterial;
    PxDefaultCpuDispatcher *gDispatcher;
    PxDefaultAllocator gAllocator;
    PxDefaultErrorCallback gErrorCallback;

    AtNode *camera;
    AtNode *options;
    AtNode *driver;
    AtArray *outputs_array;
    AtNode *shader_obj_depth;
    AtNode *shader_bck_depth;
    AtNode *shader_blendBG;

    vector<string> scene_folders;
    SceneManager *curr_scene;

    int scene_count = 0;
    int images_count = 0;

    void get_scene_folders(string path);
    rapidjson::Document CONFIG_FILE;

public:
    ~SimManager();

    void init_physx();
    void init_arnold();
    void load_config(string config_path);
    void load_meshes();
    int run_sim();

    string get_final_path();
    string get_temp_path();
};
