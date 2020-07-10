#pragma once

#include <fstream>

#pragma warning(push, 0)
#include <Eigen/Dense>
#pragma warning(pop)

namespace Renderer
{
	//---------------------------------------
	// Focal lenght, principal point & dimensions
	//---------------------------------------
	struct Intrinsics
	{
		float fx{ 0 };
		float fy{ 0 };
		float ox{ 0 };
		float oy{ 0 };
		int w{ 0 };
		int h{ 0 };
	};

	//---------------------------------------
	// Helper class for camera matrices
	//---------------------------------------
	class Camera
	{
	private:
		//---------------------------------------
		// Loads pose from file into matrix
		//---------------------------------------
		static void X_LoadPose(const std::string& pose_file, Eigen::Matrix4f& pose)
		{
			std::ifstream file(pose_file);
			if (file.is_open())
			{
				for (int i = 0; i < 4; i++)
					for (int j = 0; j < 4; j++)
						file >> pose(i, j);
				file.close();
			}
		}

	public:
		//---------------------------------------
		// Creates perspective camera matrix
		//---------------------------------------
		template<class T>
		static Eigen::Matrix<T, 4, 4> Perspective(const Intrinsics& intr, float n, float f)
		{
			Eigen::Matrix<T, 4, 4> res = Eigen::Matrix<T, 4, 4>::Zero();
			// Build perspective matrix
			const float a = 2.0f * intr.fx / intr.w;
			const float b = 2.0f * intr.fy / intr.h;
			const float c = -(2.0f * (intr.ox / intr.w) - 1.0f);
			const float d = -(2.0f * (intr.oy / intr.h) - 1.0f);
			const float e = -(f + n) / (f - n);
			const float g = -2.0f * f * n / (f - n);
			res <<
				a, 0, c, 0,
				0, b, d, 0,
				0, 0, e, g,
				0, 0, -1, 0;
			// Return it
			return res;
		}

		//---------------------------------------
		// Creates a look-at camera matrix
		//---------------------------------------
		template<class T>
		static Eigen::Matrix<T, 4, 4> LookAt(const Eigen::Matrix<T, 3, 1>& eye,
																				const Eigen::Matrix<T, 3, 1>& center,
																				const Eigen::Matrix<T, 3, 1>& up)
		{
			// Calculate necessary vectors
			const Eigen::Matrix<T, 3, 1> f = (center - eye).normalized();
			Eigen::Matrix<T, 3, 1> u = up.normalized();
			const Eigen::Matrix<T, 3, 1> s = f.cross(u).normalized();
			u = s.cross(f);
			// Calculate look-at matrix
			Eigen::Matrix<T, 4, 4> res;
			res <<
				s.x(), s.y(), s.z(), -s.dot(eye),
				u.x(), u.y(), u.z(), -u.dot(eye),
				-f.x(), -f.y(), -f.z(), f.dot(eye),
				0, 0, 0, 1;
			// Return it
			return res;
		};

		//---------------------------------------
		// Create camera matrix from pose file
		//---------------------------------------
		static Eigen::Matrix4f LoadViewMatrix(const std::string& cam_pose)
		{
			Eigen::Matrix4f camera_pose;
			Eigen::Vector3f camera_direction;
			Eigen::Vector3f camera_right;
			Eigen::Vector3f camera_up;
			Eigen::Vector3f camera_eye;
			Eigen::Vector3f camera_center;
			Eigen::Matrix4f view_pose;
			// Load the file
			X_LoadPose(cam_pose, camera_pose);
			// Create necessary vectors
			camera_direction = camera_pose.block<3, 3>(0, 0) * Eigen::Vector3f(0, 0, 1);
			camera_right = camera_pose.block<3, 3>(0, 0) * Eigen::Vector3f(1, 0, 0);
			camera_up = camera_right.cross(camera_direction);
			camera_eye = camera_pose.block<3, 1>(0, 3);
			camera_center = camera_eye + 1 * camera_direction;
			// Convert to and return camera matrix
			view_pose = Camera::LookAt(camera_eye, camera_center, camera_up);
			return view_pose;
		}
	};
};
