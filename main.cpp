#include <iostream>

#pragma warning(push, 0)
#include <boost/filesystem.hpp>

#include <Helpers/JSONUtils.h>
#include <Helpers/PathUtils.h>

#include <Rendering/Settings.h>

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
		std::cout << "Please provide only the path to the config file." << std::endl;
		for (size_t i = 0; i < argc; i++)
		{
			std::cout << argv[i] << std::endl;
		}
		return -1;
	}

	// Config file path in args
	ModifiablePath configPath = boost::filesystem::weakly_canonical(argv[1]);

	// Config file must exist of course
	if (!boost::filesystem::exists(configPath))
	{
		std::cout << "Config file " << configPath << " not found, exiting." << std::endl;
		return -1;
	}

	// Parse config file
	rapidjson::Document json;
	if (CanReadJSONFile(configPath.string(), json))
	{
		// Create simulation manager
		SimManager simulation(new Settings(MOVE_DOC(json), configPath.parent_path()));

		// Run the simulation
		simulation.RunSimulation();

		return 0;
	}
	else
	{
		std::cout << "Config file broken, exiting." << std::endl;
		return -1;
	}
}
