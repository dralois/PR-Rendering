#pragma once

#include <iostream>

#pragma warning(push, 0)
#include <boost/filesystem.hpp>

#include <SimManager.h>
#pragma warning(pop)

//---------------------------------------
// Entry point
//---------------------------------------
int main(int argc, char** argv)
{
	// Only one input argument (path to the config file) allowed
	if (argc != 2)
	{
		std::cerr << "Please provide only the path to the config file." << std::endl;
		for (size_t i = 0; i < argc; i++)
		{
			std::cerr << argv[i] << std::endl;
		}
		return -1;
	}

	// Config file path in args
	string config_path = argv[1];

	// Create simulation manager
	SimManager man(config_path);

	// Save the paths
	boost::filesystem::path final_dir(man.GetSettings()->GetFinalPath());
	boost::filesystem::path temp_dir(man.GetSettings()->GetTemporaryPath());

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

	// Run the simulation
	int exit = man.RunSimulation();

	std::cout << "Deleting Temp Directories" << std::endl;

	// Delete temporary output
	if (boost::filesystem::exists(temp_dir))
	{
		boost::filesystem::remove_all(temp_dir);
	}

	return exit;
}
