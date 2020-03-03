#include "sim_manager.h"
#include <dirent.h>

// ?
namespace d
{
#include "../Plugins/src/depthShader.cpp"
extern AtNodeMethods *DepthMethods;
} // namespace d

// ?
namespace l
{
#include "../Plugins/src/labelShader.cpp"
extern AtNodeMethods *LabelMethods;
} // namespace l

// ?
namespace b
{
#include "../Plugins/src/blendShader.cpp"
extern AtNodeMethods *BlendMethods;
} // namespace b

// ?
namespace n
{
#include "../Plugins/src/nullFilter.cpp"
extern AtNodeMethods *CustomNullFilterMtd;
} // namespace n


string SimManager::get_final_path(){
    return CONFIG_FILE["final_imgs_path"].GetString();
}

string SimManager::get_temp_path(){
    return CONFIG_FILE["temp_files_path"].GetString();
}

SimManager::~SimManager(){
    // free physics
    if(gPhysics != nullptr){
        free(gFoundation);
        delete gPhysics;
        delete gDispatcher;
        free(gCooking);
        free(gScene);
        free(gMaterial);
    }

    // free arnold
    if(camera != nullptr)
    {
        free(camera);
        free(options);
        free(driver);
        delete outputs_array;
        free(shader_obj_depth);
        free(shader_obj_depth);
        free(shader_bck_depth);
        free(shader_blendBG);
    }

    if(curr_scene != nullptr)
        delete curr_scene;
}

void SimManager::init_physx()
{
    gAllocator;
    gErrorCallback;

    // Singletons
    gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, NULL);

    // Set up scene
    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -6.81f, 0.0f);
    gDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = gDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    gScene = gPhysics->createScene(sceneDesc);

    // Set up mesh cooking
    gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(PxTolerancesScale()));

    // Set up visual debugger
    PxPvdSceneClient *pvdClient = gScene->getScenePvdClient();
    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }

    // Default physics material
    gMaterial = gPhysics->createMaterial(0.6f, 0.6f, 0.f);
}

// Loads json config file
void SimManager::load_config(string config_path)
{
    cout << "reading config file:\t" << config_path << endl;
    using namespace rapidjson;

    // Open und parse config file
    char buffer[65536];
    FILE *pFile = fopen(config_path.c_str(), "rb");
    FileReadStream is(pFile, buffer, sizeof(buffer));
    CONFIG_FILE.ParseStream<0, UTF8<>, FileReadStream>(is);
}

// Sets up arnold for rendering
void SimManager::init_arnold()
{
    // Set up arnold
    AiBegin();
    AiMsgSetConsoleFlags(AI_LOG_ALL);
    const char *SHADERS_PATH = "libPlugins.so";

    // Set up shaders
    AiNodeEntryInstall(AI_NODE_SHADER, AI_TYPE_RGBA, "depthshader", SHADERS_PATH, d::DepthMethods, AI_VERSION);
    AiNodeEntryInstall(AI_NODE_SHADER, AI_TYPE_RGBA, "blendshader", SHADERS_PATH, b::BlendMethods, AI_VERSION);
    AiNodeEntryInstall(AI_NODE_SHADER, AI_TYPE_RGBA, "labelshader", SHADERS_PATH, l::LabelMethods, AI_VERSION);
    AiNodeEntryInstall(AI_NODE_FILTER, AI_TYPE_INT, "null_filter", SHADERS_PATH, n::CustomNullFilterMtd, AI_VERSION);

    // Create initialize camera
    camera = AiNode("persp_camera");
    options = AiUniverseGetOptions();
    AiNodeSetInt(options, "AA_samples", 4);
    AiNodeSetInt(options, "GI_diffuse_depth", 6);
    AiNodeSetPtr(options, "camera", camera);
    // ?
    driver = AiNode("driver_png");
    AiNodeSetStr(driver, "name", "mydriver");
    outputs_array = AiArrayAllocate(1, 1, AI_TYPE_STRING);
    // Initialize Shaders
    shader_obj_depth = AiNode("depthshader");
    AiNodeSetBool(shader_obj_depth, "is_body", true);
    shader_bck_depth = AiNode("depthshader");
    shader_blendBG = AiNode("blendshader");
}

// Extracts integers from a string?
int extractIntegerWords(string str)
{
    int found;
    int ret = 0;
    for (char c : str) {
        if (c <= '9' && c >= '0'){
            ret = ret*10 + (c - '0');
        }
    }
    return ret;
}

// Loads all required meshes (physx, rendering)
void SimManager::load_meshes()
{
    string mesh_path = CONFIG_FILE["models"].GetString();

    DIR *dir;
    if ((dir = opendir(mesh_path.c_str())) != NULL)
    {
        for (int i = 0; i < CONFIG_FILE["objs"].Size(); i++) // Uses SizeType instead of size_t
        {
            string obj_path = mesh_path + "/" + CONFIG_FILE["objs"][i].GetString() + ".obj";
            ifstream f(obj_path.c_str());

            if(!f.good()){
                cerr << "Did not find obj " << CONFIG_FILE["objs"][i].GetString() << "... Skipping." << endl;
                continue;
            }

            cout << "Loading obj " << CONFIG_FILE["objs"][i].GetString() << endl;
            PxConvManager *curr = new PxConvManager(obj_path, i + 1, CONFIG_FILE["scale"].GetFloat(), gPhysics, gScene, gCooking, gMaterial);
            curr->load_file();
            curr->drawMeshShape();
            curr->setMetallic(CONFIG_FILE["metallic"][i].GetFloat());
            physx_objs.push_back(curr);

            AiMeshManager *ai_mesh_manager = new AiMeshManager(obj_path, i+1, CONFIG_FILE["scale"].GetFloat());
        }
        closedir(dir);
    }
}

// Determines scenes to process
void SimManager::get_scene_folders(string path)
{
    scene_folders = std::vector<std::string>();

    DIR *dir;
    struct dirent *ent;

    // Search provided path
    if ((dir = opendir(path.c_str())) != NULL)
    {
        // While folders available
        while ((ent = readdir(dir)) != NULL)
        {
            if (ent->d_name[0] == '.')
                continue;
            // Save folder in vector
            std::string eName = ent->d_name;
            string buf = path + "/" + eName;
            scene_folders.push_back(buf);
        }
        // Finally sort vector and close dir
        std::sort(scene_folders.begin(), scene_folders.end());
        closedir(dir);
    }
}

// Runs simulation and rendering
int SimManager::run_sim()
{
    // Fetch 3R scan scenes
    get_scene_folders(CONFIG_FILE["3RScan_path"].GetString());
    SceneManager curr(gPhysics, gScene, gCooking, gFoundation, gMaterial, camera, options, driver, outputs_array,
                      physx_objs, sim_objs, 0, CONFIG_FILE["objects_per_sim"].GetInt(), &CONFIG_FILE,
            shader_obj_depth, shader_bck_depth, shader_blendBG);

    AtNode *light  = AiNode("point_light");
    AtNode *light1 = AiNode("point_light");
    AtNode *light2 = AiNode("point_light");
    AtNode *light3 = AiNode("point_light");

    AtNode *light4 = AiNode("point_light");

    AtNode *light5 = AiNode("point_light");

    AtNode *light6 = AiNode("point_light");
    AiNodeSetStr(light, "name", "mylight");
    // position the light (alternatively use 'matrix')

    AiNodeSetPnt(light, "position", -1000.f, 100.f, -1000.f);
    AiNodeSetFlt(light, "intensity", 2.f); // alternatively, use 'exposure'
    AiNodeSetFlt(light, "radius", 10.f);   // for soft shadows
    AiNodeSetInt(light, "decay_type", 0);
    AiNodeSetPnt(light1, "position", -100.f, 100.f, 100.f);
    AiNodeSetFlt(light1, "intensity", 2.1f); // alternatively, use 'exposure'
    AiNodeSetFlt(light1, "radius", 10.f);    // for soft shadows
    AiNodeSetInt(light1, "decay_type", 0);
    AiNodeSetPnt(light2, "position", 1000.f, 100.f, -1000.f);
    AiNodeSetFlt(light2, "intensity", 1.9f); // alternatively, use 'exposure'
    AiNodeSetFlt(light2, "radius", 10.f);    // for soft shadows
    AiNodeSetInt(light2, "decay_type", 0);
    AiNodeSetPnt(light3, "position", 1000.f, 100.f, 1000.f);
    AiNodeSetFlt(light3, "intensity", 2.f); // alternatively, use 'exposure'
    AiNodeSetFlt(light3, "radius", 10.f);   // for soft shadows
    AiNodeSetInt(light3, "decay_type", 0);
    AiNodeSetPnt(light4, "position", 1000.f, 1000.f, 1000.f);
    AiNodeSetFlt(light4, "intensity", 2.f); // alternatively, use 'exposure'
    AiNodeSetFlt(light4, "radius", 10.f);   // for soft shadows
    AiNodeSetInt(light4, "decay_type", 0);
    AiNodeSetPnt(light5, "position", 1000.f, 1000.f, -1000.f);
    AiNodeSetFlt(light5, "intensity", 2.f); // alternatively, use 'exposure'
    AiNodeSetFlt(light5, "radius", 10.f);   // for soft shadows
    AiNodeSetInt(light5, "decay_type", 0);
    AiNodeSetPnt(light6, "position", -1000.f, 1000.f, 1000.f);
    AiNodeSetFlt(light6, "intensity", 2.f); // alternatively, use 'exposure'
    AiNodeSetFlt(light6, "radius", 10.f);   // for soft shadows
    AiNodeSetInt(light6, "decay_type", 0);

    int max_count = CONFIG_FILE["max_images"].GetInt();
    int iter_per_scene = CONFIG_FILE["iter_per_scene"].GetInt();

    //for(int i = 100; i < scene_folders.size(); i++)
    //{
    //    string folder = scene_folders[i];
    for (string folder : scene_folders)
    {
        curr.set_scene_path(folder);
        bool count_samples = curr.run(iter_per_scene, max_count);
        if(!count_samples)
            break;
    }

    return 0;
}
