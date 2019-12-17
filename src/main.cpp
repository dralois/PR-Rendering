#include <iostream>
#include "sim_manager.h"

int main(int argc, char** argv)
{

    if(argc != 2){
        std::cerr << "Please provide only the path to the config file." << std::endl;
        return -1;
    }

    string config_path = argv[1];

    SimManager *man = new SimManager();

    man->init_physx();
    man->init_arnold();
    man->load_config(config_path);
    man->load_meshes();
    man->run_sim();

    return 0;
}
