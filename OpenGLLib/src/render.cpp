#include "../include/render.h"

namespace Renderer
{
	//---------------------------------------
	// Render all scenes with given intrinsics
	//---------------------------------------
	vector<tuple<cv::Mat, cv::Mat> > Render::RenderScenes(const std::string& scenePath, vector<string>camPoses,
		float fx, float fy, float ox, float oy)
	{
		vector<tuple<cv::Mat, cv::Mat> > renderings;

		// Setup shader & load model
		Shader shader(shaderPath + "/textured3D.vs", shaderPath + "/textured3D.frag");
		Model model(scenePath, "mesh.refined.obj", "mesh.refined_0.png");

		// Calculate intrinsics
		Intrinsics camRender;
		camRender.fx = fx * 2.0;
		camRender.fy = fy * 2.0;
		camRender.ox = width / 2.0 + (ox - (width / 4)) * 2.0;
		camRender.oy = height / 2.0 + (oy - (height / 4)) * 2.0;
		camRender.imgWidth = width;
		camRender.imgHeight = height;
		// An corresponding matrix
		projection = Camera::Perspective<Eigen::Matrix4f::Scalar>(camRender, NearPlane, FarPlane);

		// Make window visible
		glfwSetWindowOpacity(window, 1.0f);

		// For each pose
		for (string& currPose : camPoses)
		{
			cv::Mat col, dep;

			// Load camera matrix
			Eigen::Matrix4f camMat = Pose::LoadViewMatrix(currPose);

			glfwPollEvents();
			// Clear buffer
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Render pose
			X_Render(model, shader, camMat);

			// Fetch results
			glfwSwapBuffers(window);
			col = X_GetRGB();
			dep = X_GetDepth();

			// Save in vector
			renderings.push_back(make_tuple(col, dep));
		}

		// Make window invisible
		glfwSetWindowOpacity(window, 0.0f);

		// Return renderings
		return renderings;
	}

	//---------------------------------------
	// Render a model
	//---------------------------------------
	void Render::X_Render(Model& model, Shader& shader, Eigen::Matrix4f& pose)
	{
		shader.Use();
		// Bind mvp matrix
		Eigen::Matrix4f model_view_projection = projection * pose;
		glUniformMatrix4fv(glGetUniformLocation(shader.GetProgram(), "model_view_projection"), 1, GL_FALSE, model_view_projection.data());
		// Draw the model
		model.Draw(shader);
	}

	//---------------------------------------
	// Setup render window
	//---------------------------------------
	void Render::X_SetupGLFW()
	{
		// Set all the required options for GLFW
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	}

	//---------------------------------------
	// Fetch rgb render results
	//---------------------------------------
	cv::Mat  Render::X_GetRGB()
	{
		// Setup buffers
		cv::Mat image(height, width, CV_8UC3);
		GLuint* data = new GLuint[height * width * 3];

		// Read result
		glReadBuffer(GL_FRONT);
		glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_INT, &data[0]);

		// For each pixel
		const int channels = 3;
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				// Save as OpenCV pixel
				for (int c = 0; c < channels; c++)
					image.at<cv::Vec3b>(height - i - 1, j)[2 - c] =
					(int)data[int(channels * i * width + channels * j + c)];
			}
		}

		// Return OpenCV image
		return image;
	}

	//---------------------------------------
	// Fetch depth render results
	//---------------------------------------
	cv::Mat Render::X_GetDepth()
	{
		// Setup buffers
		cv::Mat image(height, width, CV_16UC1);
		GLfloat* data = new GLfloat[height * width];

		// Read results
		glReadBuffer(GL_FRONT);
		glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, &data[0]);

		// For each pixel
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				// Read pixel
				const float depthBufferValue = data[static_cast<int>(i * width + j)];
				// Convert from [0,1] to z distance
				const float zn = (2 * depthBufferValue - 1);
				const float ze = (2 * FarPlane * NearPlane) /
					(FarPlane + NearPlane + zn * (NearPlane - FarPlane));
				// Save as OpenCV pixel
				image.at<unsigned short>(height - i - 1, j) = 1000 * ze;
			}
		}

		// Return OpenCV image
		return image;
	}

	//---------------------------------------
	// Create new OpenGL renderer
	//---------------------------------------
	Render::Render(const std::string& shaderPath, int width, int height) :
		shaderPath(shaderPath),
		width(width),
		height(height)
	{
		// Initialize & setup glfw
		if (!glfwInit())
		{
			std::cerr << "Could not init glfw!" << std::endl;
			exit(-1);
		}
		else
		{
			X_SetupGLFW();
		}

		// Create window
		window = glfwCreateWindow(width, height, "Evaluation", nullptr, nullptr);
		if (nullptr == window)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			exit(-1);
		}
		else
		{
			glfwMakeContextCurrent(window);
			glfwSetWindowOpacity(window, 0.0f);
		}

		// Initialize OpenGL
		glewExperimental = GL_TRUE;
		if (GLEW_OK != glewInit())
		{
			std::cout << "Failed to initialize GLEW" << std::endl;
			exit(-1);
		}

		// Enable depth testing
		glEnable(GL_DEPTH_TEST);
	}
}