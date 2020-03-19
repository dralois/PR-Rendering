#pragma once

#include <eigen3/Eigen/Core>

namespace Renderer
{
	//---------------------------------------
	// Focal lenght, principal point & dimensions
	//---------------------------------------
	struct Intrinsics
	{
		double fx{ 0 };
		double fy{ 0 };
		double ox{ 0 };
		double oy{ 0 };
		int imgWidth{ 0 };
		int imgHeight{ 0 };
	};

	class Camera
	{
	public:
		//---------------------------------------
		// Creates perspective camera matrix
		//---------------------------------------
		template<class T>
		static Eigen::Matrix<T, 4, 4> Perspective(const Intrinsics& intrinsics, double n, double f)
		{
			assert(f > n);
			Eigen::Matrix<T, 4, 4> res = Eigen::Matrix<T, 4, 4>::Zero();
			// Build perspective matrix
			const float a = 2 * intrinsics.fx / intrinsics.imgWidth;
			const float b = 2 * intrinsics.fy / intrinsics.imgHeight;
			const float c = -(2 * (intrinsics.ox / intrinsics.imgWidth) - 1);
			const float d = -(2 * (intrinsics.oy / intrinsics.imgHeight) - 1);
			const float e = -(f + n) / (f - n);
			const float g = -2 * f * n / (f - n);
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
		static Eigen::Matrix<T, 4, 4> LookAt(Eigen::Matrix<T, 3, 1> const& eye,
																				Eigen::Matrix<T, 3, 1> const& center,
																				Eigen::Matrix<T, 3, 1> const& up)
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
	};
};
