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
		std::cout << "Please provide only the path to the config file." << std::endl;
		for (size_t i = 0; i < argc; i++)
		{
			std::cout << argv[i] << std::endl;
		}
		return -1;
	}

	// Config file path in args
	ModifiablePath configPath = boost::filesystem::absolute(argv[1]);

	// Config file must exist of course
	if (!boost::filesystem::exists(configPath))
	{
		std::cout << "Config file " << configPath << " not found, exiting." << std::endl;
		return -1;
	}

	// Create simulation manager
	SimManager simulation(configPath);

	// Run the simulation
	simulation.RunSimulation();

	return 0;
}
