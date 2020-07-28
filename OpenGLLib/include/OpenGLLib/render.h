#pragma once

#include <memory>
#include <vector>

#pragma warning(push, 0)
#include <boost/filesystem.hpp>

#include <opencv2/opencv.hpp>
#pragma warning(pop)

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
			RenderScenes(const boost::filesystem::path& scenePath,
				const std::vector<boost::filesystem::path>& camPoses,
				float fx, float fy, float ox, float oy, float w, float h);

		//---------------------------------------
		// Constructors
		//---------------------------------------
		EXPORT_THIS Render(const boost::filesystem::path& shaderPath,
			int width, int height, float near, float far);
		EXPORT_THIS Render& operator=(Render rhs);
		EXPORT_THIS Render(const Render& other);
		EXPORT_THIS ~Render();
	};
}
