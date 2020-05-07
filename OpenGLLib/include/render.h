#pragma once

#include <memory>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

#ifdef WIN32
#define EXPORT_THIS __declspec(dllexport)
#else
#define EXPORT_THIS __attribute__((visibility("default")))
#endif

namespace Renderer
{
	//---------------------------------------
	// OpenGL renderer, renders depth & rgb
	//---------------------------------------
	class Render
	{
	private:
		//---------------------------------------
		// Fields
		//---------------------------------------
		class Render_impl;
		std::unique_ptr<Render_impl> renderImpl;

	public:
		//---------------------------------------
		// Methods
		//---------------------------------------
		EXPORT_THIS std::vector<std::tuple<cv::Mat, cv::Mat>>
			RenderScenes(const std::string& scenePath, std::vector<std::string>camPoses,
				float fx, float fy, float ox, float oy);

		//---------------------------------------
		// Constructors
		//---------------------------------------
		EXPORT_THIS Render(const std::string& shaderPath,
			int width, int height, float near, float far);
		EXPORT_THIS Render& operator=(Render rhs);
		EXPORT_THIS Render(const Render& other);
		EXPORT_THIS ~Render();
	};
}
