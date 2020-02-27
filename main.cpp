#include <iostream>
#include "sim_manager.h"
#include <boost/filesystem.hpp>

// TODO
int main(int argc, char** argv)
{
    // Only one arg allowed
    if(argc != 2){
        std::cerr << "Please provide only the path to the config file." << std::endl;
        return -1;
    }

    // Config file path in args
    string config_path = argv[1];

    SimManager man;

    man.init_physx();
    man.init_arnold();
    man.load_config(config_path);

    boost::filesystem::path final_dir(man.get_final_path());
    boost::filesystem::path temp_dir(man.get_temp_path());

    if (!boost::filesystem::exists(final_dir)){
        boost::filesystem::create_directories(final_dir);
        boost::filesystem::create_directories(final_dir / "rgb");
        boost::filesystem::create_directories(final_dir / "depth");
        boost::filesystem::create_directories(final_dir / "segs");
    }

    std::cout << "Creating Temp Directories" << std::endl;

    if(!boost::filesystem::exists(temp_dir)){
        boost::filesystem::create_directories(temp_dir / "body_depth");
        boost::filesystem::create_directories(temp_dir / "body_label");
        boost::filesystem::create_directories(temp_dir / "scene_depth");
        boost::filesystem::create_directories(temp_dir / "rgb");
    }

    man.load_meshes();
    man.run_sim();
    std::cout << "Deleting Temp Directories" << std::endl;

    if(boost::filesystem::exists(temp_dir)){
        boost::filesystem::remove_all(temp_dir);
    }

    return 0;
}
