#pragma once

#include <string>

#ifdef WIN32
#define EXPORT_THIS __declspec(dllexport)
#else
#define EXPORT_THIS __attribute__((visibility("default")))
#endif

namespace HDR
{
	//---------------------------------------
	// Light Estimator API bridge
	//---------------------------------------
	class LightEstimator
	{
	private:
		//---------------------------------------
		// Fields
		//---------------------------------------
		class Estimator_impl;
		Estimator_impl* estimatorImpl;

	public:
		//---------------------------------------
		// Methods
		//---------------------------------------
		EXPORT_THIS void Estimate(const std::string& path);

		//---------------------------------------
		// Constructors
		//---------------------------------------
		EXPORT_THIS LightEstimator();
		EXPORT_THIS ~LightEstimator();
	};
}
