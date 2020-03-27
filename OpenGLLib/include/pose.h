#pragma once

#include <fstream>

#include <eigen3/Eigen/Dense>

#include "types.h"
#include "camera.h"

namespace Renderer
{
	//---------------------------------------
	// Pose utility functions
	//---------------------------------------
	class Pose
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
}
