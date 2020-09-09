#pragma once

#include <string>
#include <vector>
#include <iostream>

#ifdef WIN32
#define EXPORT_THIS __declspec(dllexport)
#else
#define EXPORT_THIS __attribute__((visibility("default")))
#endif

namespace Blender
{
	//---------------------------------------
	// Blender Python API bridge
	//---------------------------------------
	class BlenderRenderer
	{
		private:
			//---------------------------------------
			// Fields
			//---------------------------------------
			class Renderer_impl;
			Renderer_impl* rendererImpl;

		public:
			//---------------------------------------
			// Methods
			//---------------------------------------
			EXPORT_THIS void LogPerformance(const std::string& what, int thread);
			EXPORT_THIS void ProcessRenderfile(const std::string& renderfile, float timeout, int thread);
			EXPORT_THIS void UnloadProcess(int thread);

			//---------------------------------------
			// Constructors
			//---------------------------------------
			EXPORT_THIS BlenderRenderer(int workerCount);
			EXPORT_THIS ~BlenderRenderer();
	};
}
