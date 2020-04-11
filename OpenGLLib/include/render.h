#pragma once

#ifndef WIN32
#define GLEW_STATIC
#endif
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <eigen3/Eigen/Dense>
#include <opencv2/opencv.hpp>

#include "camera.h"
#include "model.h"
#include "shader.h"

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
#if WIN32
		__declspec(dllexport) vector<tuple<cv::Mat, cv::Mat> > RenderScenes(const std::string& scenePath, vector<string>camPoses, float fx, float fy, float ox, float oy);
#else
		__attribute__((visibility("default"))) vector<tuple<cv::Mat, cv::Mat> > RenderScenes(const std::string& scenePath, vector<string>camPoses, float fx, float fy, float ox, float oy);
#endif

		//---------------------------------------
		// Constructors
		//---------------------------------------
#if WIN32
		__declspec(dllexport) Render(const std::string& shaderPath, int width, int height, float near, float far);
#else
		__attribute__((visibility("default"))) Render(const std::string& shaderPath, int width, int height, float near, float far);
#endif
	};
}
