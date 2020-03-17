#pragma once

#include <eigen3/Eigen/Dense>
#include <fstream>
#include "types.h"
#include <vector>

namespace Renderer
{
	class Data
	{
	public:
		Data(const std::string& path);

		Intrinsics intrinsics;

		void LoadViewMatrix();
		Eigen::Matrix4f LoadViewMatrix(const std::string cam_pose);

		bool LoadIntrinsics();
		void NextFrame();

		const Eigen::Matrix4f& GetPose() const;

		const std::string getCalibFile() const;
	private:
		const std::string calibFile{ "/frames/_info.txt" };
		const std::string calibFilePath{ "" };
		const std::string dataPath{ "" };

		float frame_id = 0;

		std::vector<Eigen::Matrix4f, Eigen::aligned_allocator<Eigen::Matrix4f>> poses_;
		const std::string pose_prefix_{ "/frames/frame-" };
		const std::string pose_suffix_{ ".pose.txt" };

		inline bool FileExists(const std::string& name)
		{
			std::ifstream f(name.c_str());
			return f.good();
		};

		void LoadPose(const std::string& pose_file, Eigen::Matrix4f& pose);
	};
}
