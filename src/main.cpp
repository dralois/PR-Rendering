#include <iostream>
#include "sim_manager.h"

int main(int argc, char** argv)
{
    // Only one arg allowed
    if(argc != 2){
        std::cerr << "Please provide only the path to the config file." << std::endl;
        return -1;
    }

    // Config file path in args
    string config_path = argv[1];

    // Create simulator
    SimManager *man = new SimManager();

    // Initialize physx for simulation and arnold for rendering
    man->init_physx();
    man->init_arnold();
    // Load json config file and (physx, arnold) meshes
    man->load_config(config_path);
    man->load_meshes();
    // Run the simulation and render images
    man->run_sim();

    return 0;
}
