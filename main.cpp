#pragma once

#include "SimManager.h"

#include <iostream>
#include <boost/filesystem.hpp>

// Entry point
int main(int argc, char** argv)
{
	// Only one input argument (path to the config file) allowed
	if (argc != 2)
	{
		std::cerr << "Please provide only the path to the config file." << std::endl;
		return -1;
	}

	// Config file path in args
	string config_path = argv[1];

	SimManager man;
	// Init PhysX and Arnold, load file
	man.InitPhysx();
	man.InitArnold();
	man.LoadConfig(config_path);

	// Save the paths
	boost::filesystem::path final_dir(man.GetFinalPath());
	boost::filesystem::path temp_dir(man.GetTemporaryPath());

	// Create final output directories
	if (!boost::filesystem::exists(final_dir))
	{
		boost::filesystem::create_directories(final_dir);
		boost::filesystem::create_directories(final_dir / "rgb");
		boost::filesystem::create_directories(final_dir / "depth");
		boost::filesystem::create_directories(final_dir / "segs");
	}

	std::cout << "Creating Temp Directories" << std::endl;

	// Create temporary output directories
	if (!boost::filesystem::exists(temp_dir))
	{
		boost::filesystem::create_directories(temp_dir / "body_depth");
		boost::filesystem::create_directories(temp_dir / "body_label");
		boost::filesystem::create_directories(temp_dir / "scene_depth");
		boost::filesystem::create_directories(temp_dir / "rgb");
	}

	// Load meshes and run simulation
	man.LoadMeshes();
	man.RunSimulation();

	std::cout << "Deleting Temp Directories" << std::endl;

	// Delete temporary output
	if (boost::filesystem::exists(temp_dir)) {
		boost::filesystem::remove_all(temp_dir);
	}

	return 0;
}
