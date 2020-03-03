#pragma once

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>
#include <eigen3/Eigen/Dense>

#include "shader.h"
#include "data.h"
#include "types.h"
#include "model.h"

struct RenderData {
    static float near_;
    static float far_;
    GLfloat deltaTime = 0.0f;
    GLfloat lastFrame = 0.0f;
};

class Render {
public:
    Render(const std::string shader_path);

    vector<tuple<cv::Mat, cv::Mat> > render_scenes(const std::string scene_path, vector<string>cam_poses, float fx, float fy, float ox, float oy);
private:
	Data data;
    std::string shader_path{""};
	RenderData render_data;
	Eigen::Matrix4f projection;
    GLFWwindow *window;

    const int window_width_ = 1920;
    const int window_height_ = 1080;

    int SCREEN_WIDTH, SCREEN_HEIGHT;

    static void KeyCallback( GLFWwindow *window, int key, int scancode, int action, int mode);
    void initGLFW();
    void render(Model& model, Shader& shader, Eigen::Matrix4f& pose);

    cv::Mat getRGB();
    cv::Mat getDepth();
};
