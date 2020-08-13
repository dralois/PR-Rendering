#pragma once

#include <memory>
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
			std::unique_ptr<Renderer_impl> rendererImpl;

		public:
			//---------------------------------------
			// Methods
			//---------------------------------------
			EXPORT_THIS void LogPerformance(const std::string& what);
			EXPORT_THIS void ProcessRenderfile(const std::string& renderfile);
			EXPORT_THIS void UnloadProcesses();

			//---------------------------------------
			// Constructors
			//---------------------------------------
			EXPORT_THIS BlenderRenderer();
			EXPORT_THIS BlenderRenderer& operator=(BlenderRenderer rhs);
			EXPORT_THIS BlenderRenderer(const BlenderRenderer& other);
			EXPORT_THIS ~BlenderRenderer();
	};
}
