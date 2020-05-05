#pragma once

#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <Eigen/Dense>
#include <opencv2/opencv.hpp>

#include "camera.h"
#include "model.h"
#include "shader.h"

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
		int width, height;
		float nearClip, farClip;
		GLFWwindow* window;
		std::string shaderPath;
		Eigen::Matrix4f projection;

		//---------------------------------------
		// Methods
		//---------------------------------------
		void X_SetupGLFW();
		void X_Render(Model& model, Shader& shader, Eigen::Matrix4f& pose);

		//---------------------------------------
		// Properties
		//---------------------------------------
		cv::Mat X_GetRGB();
		cv::Mat X_GetDepth();

	public:
		//---------------------------------------
		// Methods
		//---------------------------------------
		EXPORT_THIS vector<tuple<cv::Mat, cv::Mat> > RenderScenes(const std::string& scenePath, vector<string>camPoses, float fx, float fy, float ox, float oy);

		//---------------------------------------
		// Constructors
		//---------------------------------------
		EXPORT_THIS Render(const std::string& shaderPath, int width, int height, float near, float far);
	};
}
