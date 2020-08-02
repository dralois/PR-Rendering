#include <render.h>

#pragma warning(push, 0)
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <Eigen/Dense>

#include <camera.h>
#include <model.h>
#include <shader.h>
#pragma warning(pop)

namespace Renderer
{
	//---------------------------------------
	// Implementation class
	//---------------------------------------
	class Render::Render_impl
	{
	private:
		//---------------------------------------
		// Fields
		//---------------------------------------
		int width, height;
		float nearClip, farClip;
		GLFWwindow* window;
		boost::filesystem::path shaderPath;

		//---------------------------------------
		// Setup render window
		//---------------------------------------
		void Render_impl::X_SetupGLFW()
		{
			// Set all the required options for GLFW
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		}

		//---------------------------------------
		// Render a model
		//---------------------------------------
		void Render_impl::X_Render(Model& model, Shader& shader, Eigen::Matrix4f& pose, Eigen::Matrix4f& proj)
		{
			shader.Use();
			// Bind uniforms
			glUniformMatrix4fv(glGetUniformLocation(shader.GetProgram(), "matP"), 1, GL_FALSE, proj.data());
			glUniformMatrix4fv(glGetUniformLocation(shader.GetProgram(), "matV"), 1, GL_FALSE, pose.data());
			// Draw the model
			model.Draw(shader);
		}

		//---------------------------------------
		// Fetch rgb render results
		//---------------------------------------
		cv::Mat Render_impl::X_GetRGB()
		{
			// Setup buffers
			cv::Mat image(height, width, CV_8UC3);
			GLubyte* data = new GLubyte[height * width * 3];

			// Read result
			glReadBuffer(GL_FRONT);
			glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, &data[0]);

			// Convert OpenGL RGB -> OpenCV BGR
			image.forEach<cv::Vec3b>([&](cv::Vec3b& val, const int pixel[]) -> void {
				const int x = pixel[1];
				const int y = height - pixel[0] - 1;
				const int idx = ((y * width) + x) * 3;
				const uchar* rgb = data + idx;
				val = cv::Vec3b(rgb[2], rgb[1], rgb[0]);
			});

			// Return image
			return image;
		}

		//---------------------------------------
		// Fetch depth render results
		//---------------------------------------
		cv::Mat Render_impl::X_GetDepth(float fov)
		{
			// Setup buffers
			cv::Mat image(height, width, CV_32FC1);
			GLfloat* data = new GLfloat[height * width];

			// Read results
			glReadBuffer(GL_FRONT);
			glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, &data[0]);

			// Calculate constants
			const float centerX = static_cast<float>(width) / 2.0f - 0.5f;
			const float centerY = static_cast<float>(height) / 2.0f - 0.5f;
			const float focalLength = (static_cast<float>(height) / 2.0f) / tan(fov / 2.0f);

			// Parallel compute of actual distance
			image.forEach<float>([&](float& val, const int pixel[]) -> void {
				const int x = pixel[1];
				const int y = height - pixel[0] - 1;
				const float depth = data[((y * width) + x)];
				// Distance to near plane
				const float distToPlane = nearClip / (farClip - depth * (farClip - nearClip)) * farClip;
				// True distance camera - pixel at far plane
				const float diagonal = cv::norm(cv::Vec3f(
					static_cast<float>(x) - centerX,
					static_cast<float>(y) - centerY,
					focalLength));
				// Compute distance camera - pixel
				val = distToPlane * (diagonal / focalLength);
			});

			// Return image
			return image;
		}

	public:
		//---------------------------------------
		// Render all scenes with given intrinsics
		//---------------------------------------
		std::vector<std::tuple<cv::Mat, cv::Mat>> Render_impl::RenderScenes(
			const boost::filesystem::path& scenePath,
			const std::vector<boost::filesystem::path>& camPoses,
			float fx, float fy,
			float ox, float oy,
			float w, float h
		)
		{
			std::vector<std::tuple<cv::Mat, cv::Mat> > renderings;

			// Setup shader & load model
			Shader shader(boost::filesystem::path(shaderPath).append("textured3D.vs"),
				boost::filesystem::path(shaderPath).append("textured3D.frag"));
			Model model(boost::filesystem::path(scenePath).append("mesh.refined.obj"),
				boost::filesystem::path(scenePath).append("mesh.refined_0.png"));

			// Build intrinsics
			Intrinsics camRender;
			camRender.fx = fx;
			camRender.fy = fy;
			camRender.ox = ox;
			camRender.oy = oy;
			camRender.w = w;
			camRender.h = h;

			// And corresponding matrix & fov
			Eigen::Matrix4f camProj = Camera::Perspective<Eigen::Matrix4f::Scalar>(camRender, nearClip, farClip);
			float fov = 2.0f * atan(h / (2.0f * fy));

			// Make window visible
			glfwSetWindowOpacity(window, 1.0f);

			// For each pose
			for (auto currPose : camPoses)
			{
				cv::Mat col, dep;

				// Load camera matrix
				Eigen::Matrix4f camMat = Camera::LoadViewMatrix(currPose);

				glfwPollEvents();
				// Clear buffer
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				// Render pose
				X_Render(model, shader, camMat, camProj);

				// Fetch results
				glfwSwapBuffers(window);
				col = X_GetRGB();
				dep = X_GetDepth(fov);

				// Save in vector
				renderings.push_back(std::make_tuple(col, dep));
			}

			// Make window invisible
			glfwSetWindowOpacity(window, 0.0f);

			// Return renderings
			return renderings;
		}

		//---------------------------------------
		// Create new OpenGL renderer
		//---------------------------------------
		Render_impl::Render_impl(const boost::filesystem::path& shaderPath,
			int width, int height, float near, float far) :
			shaderPath(shaderPath),
			width(width),
			height(height),
			nearClip(near),
			farClip(far)
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
	};

	//---------------------------------------
	// Forward render command
	//---------------------------------------
	std::vector<std::tuple<cv::Mat, cv::Mat>> Render::RenderScenes(
		const boost::filesystem::path& scenePath,
		const std::vector<boost::filesystem::path>& camPoses,
		float fx, float fy,
		float ox, float oy,
		float w, float h
	)
	{
		return renderImpl->RenderScenes(scenePath, camPoses, fx, fy, ox, oy, w, h);
	}

	//---------------------------------------
	// Forward render creation
	//---------------------------------------
	Render::Render(const boost::filesystem::path& shaderPath,
		int width, int height, float near, float far) :
		renderImpl(new Render_impl(shaderPath, width, height, near, far))
	{
	}

	//---------------------------------------
	// Copy constructor
	//---------------------------------------
	Render::Render(const Render& other) :
		renderImpl(new Render_impl(*other.renderImpl))
	{
	}

	//---------------------------------------
	// Assignment operator
	//---------------------------------------
	Render& Render::operator=(Render rhs)
	{
		swap(renderImpl, rhs.renderImpl);
		return *this;
	}

	//---------------------------------------
	// Destructor
	//---------------------------------------
	Render::~Render() = default;
}
