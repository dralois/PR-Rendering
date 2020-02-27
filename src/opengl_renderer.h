#ifndef OPENGL_RENDERER_H
#define OPENGL_RENDERER_H

#pragma once
// GLEW
#define GLEW_STATIC
#include <GL/glew.h>


// GLFW
#include <GLFW/glfw3.h>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <glm/common.hpp>
#include <opencv2/core.hpp>

#include <iostream>

struct OpenglObj{
public:
    std::vector<glm::vec4> vertices_;
    std::vector<GLushort> elements_;
};

class OpenglRenderer
{
private:
    void initFrameBuffer();
    int window_height_, window_width_;
public:
    OpenglRenderer(int window_width, int window_height);
    OpenglObj::OpenglObj loadObj(std::string path);
};

#endif // OPENGL_RENDERER_H
