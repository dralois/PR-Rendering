#include "../include/render.h"

#include "../include/util.h"
#include "../include/model.h"

float RenderData::near_ = 0.1f;
float RenderData::far_ = 10.0f;

Render::Render(const std::string shader_path): data(shader_path), shader_path(shader_path) {

    if (!glfwInit()){
        std::cerr << "Could not init glfw!" << std::endl;
        exit(-1);
    }

    initGLFW();

    // Create a GLFWwindow object that we can use for GLFW's functions
    window = glfwCreateWindow(window_width_, window_height_, "Evaluation", nullptr, nullptr);
    if (nullptr == window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate( );
        exit(-1);
    }
    // glfwSetUserPointer(window_, this);
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    if (GLEW_OK != glewInit()) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        exit(-1);
    }

    glEnable(GL_DEPTH_TEST);
}

vector<tuple<cv::Mat, cv::Mat> > Render::render_scenes(const std::string scene_path, vector<string>cam_poses, float fx, float fy, float ox, float oy)
{
    // Setup and compile our shaders
    Shader shader(shader_path + "/textured3D.vs", shader_path + "/textured3D.frag");
    // Load models
    Model model(scene_path + "/mesh.refined.obj");

    Intrinsics cam;
    //std::cout << fx << "\t " << fy << "\t " << ox << "\t " << oy << std::endl;
    cam.f_x = fx * 2.;
    cam.f_y = fy * 2.;
    cam.c_x = 1920 / 2. + (ox - 480) * 2.; // ox * 2.;
    cam.c_y = 1080 / 2. + (oy - 270) * 2.; // oy * 2.;
    cam.image_height = window_height_;
    cam.image_width = window_width_;

    //std::cout << cam.f_x << "\t " << cam.f_y << "\t " << cam.c_x  << "\t " << cam.c_y << std::endl;
    vector<tuple<cv::Mat, cv::Mat> > renderings;

    for(string &cam_pose: cam_poses){
        cv::Mat col, dep;

        Eigen::Matrix4f pose = data.LoadViewMatrix(cam_pose);
        projection = C3DV_camera::perspective<Eigen::Matrix4f::Scalar>(cam, RenderData::near_, RenderData::far_);

        // Check and call events
        glfwPollEvents();

        // Clear the colorbuffer
        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        // let's render
        render(model, shader, pose);

        glfwSwapBuffers(window);
        col = getRGB();
        dep = getDepth();

        renderings.push_back(make_tuple(col, dep));
    }

    return renderings;
}

void Render::render(Model& model, Shader& shader, Eigen::Matrix4f& pose) {
    shader.Use();
    Eigen::Matrix4f model_view_projection = projection * pose;
    glUniformMatrix4fv( glGetUniformLocation(shader.Program, "model_view_projection" ), 1, GL_FALSE, model_view_projection.data());
    Eigen::Matrix4f model_matrix{Eigen::Matrix4f::Identity()};
    model.Draw(shader);
}

void Render::initGLFW() {
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
}

cv::Mat  Render::getRGB() {
    cv::Mat image(window_height_, window_width_, CV_8UC3);

    std::vector<GLuint> data(window_width_ * window_height_ * 3);


    glReadPixels(0, 0, window_width_, window_height_, GL_RGB, GL_UNSIGNED_INT, data.data());
    const int channels = 3;
    for (int i = 0; i < window_height_; i++) {
        for (int j = 0; j < window_width_; j++) {
            for (int c = 0; c < channels; c++)
                image.at<cv::Vec3b>(window_height_ - i - 1, j)[2 - c] = (int) data[int(
                        channels * i * window_width_ + channels * j + c)];
        }
    }
    return image;
}

cv::Mat Render::getDepth() {
    cv::Mat image(window_height_, window_width_, CV_16UC1);
    GLfloat *data = new GLfloat[window_height_ * window_width_];
    glReadPixels(0, 0, window_width_, window_height_, GL_DEPTH_COMPONENT, GL_FLOAT, &data[0]);
    for (int i = 0; i < window_height_; i++) {
        for (int j = 0; j < window_width_; j++) {
            const float depthBufferValue = data[static_cast<int>(i * window_width_ + j)];
            //std::cout << i << ", " << j << " : " << depthBufferValue << std::endl;
            const float value = RenderData::near_ + depthBufferValue * (RenderData::far_ - RenderData::near_);
            // solve non-linearity

            const float zn = (2 * depthBufferValue - 1);
            const float ze = (2 * RenderData::far_* RenderData::near_) /
            (RenderData::far_ + RenderData::near_ + zn*(RenderData::near_ - RenderData::far_));
            image.at<unsigned short>(window_height_ - i - 1, j) = 1000 * ze;
        }
    }
    return image;
}
