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
	boost::filesystem::path configPath(argv[1]);

	// Create simulation manager
	SimManager man(configPath);

	// Save the paths
	boost::filesystem::path finalDir(man.GetSettings()->GetFinalPath());
	boost::filesystem::path tempDir(man.GetSettings()->GetTemporaryPath());

	// Create final output directories
	if (!boost::filesystem::exists(finalDir))
	{
		boost::filesystem::create_directories(finalDir);
		boost::filesystem::create_directories(finalDir / "rgb");
		boost::filesystem::create_directories(finalDir / "depth");
		boost::filesystem::create_directories(finalDir / "segs");
	}

	std::cout << "Creating Temp Directories" << std::endl;

	// Create temporary output directories
	if (!boost::filesystem::exists(tempDir))
	{
		boost::filesystem::create_directories(tempDir / "body_depth");
		boost::filesystem::create_directories(tempDir / "body_label");
		boost::filesystem::create_directories(tempDir / "scene_depth");
		boost::filesystem::create_directories(tempDir / "rgb");
	}

	// Run the simulation
	int exit = man.RunSimulation();

	std::cout << "Deleting Temp Directories" << std::endl;

	// Delete temporary output
	if (boost::filesystem::exists(tempDir))
	{
		boost::filesystem::remove_all(tempDir);
	}

	return exit;
}
