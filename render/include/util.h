/*******************************************************
 * Copyright (c) 2020, Johanna Wald
 * All rights reserved.
 *
 * This file is distributed under the GNU Lesser General Public License v3.0.
 * The complete license agreement can be obtained at:
 * http://www.gnu.org/licenses/lgpl-3.0.html
 ********************************************************/

#pragma once

#include "types.h"
#include <eigen3/Eigen/Core>

namespace Renderer
{
	template<class T>
	Eigen::Matrix<T, 4, 4> perspective(const Intrinsics& intrinsics, double n, double f)
	{
		assert(f > n);
		Eigen::Matrix<T, 4, 4> res = Eigen::Matrix<T, 4, 4>::Zero();
		// this has to be fixed.
		const float a = 2 * intrinsics.f_x / intrinsics.image_width;
		const float b = 2 * intrinsics.f_y / intrinsics.image_height;
		const float c = -(2 * (intrinsics.c_x / intrinsics.image_width) - 1);
		const float d = -(2 * (intrinsics.c_y / intrinsics.image_height) - 1);
		const float e = -(f + n) / (f - n);
		const float g = -2 * f * n / (f - n);
		res << a, 0, c, 0,
			0, b, d, 0,
			0, 0, e, g,
			0, 0, -1, 0;
		return res;
	}

	template<class T>
	Eigen::Matrix<T, 4, 4> lookAt(Eigen::Matrix<T, 3, 1> const& eye, 
																Eigen::Matrix<T, 3, 1> const& center,
																Eigen::Matrix<T, 3, 1> const& up)
	{
		const Eigen::Matrix<T, 3, 1> f = (center - eye).normalized();
		Eigen::Matrix<T, 3, 1> u = up.normalized();
		const Eigen::Matrix<T, 3, 1> s = f.cross(u).normalized();
		u = s.cross(f);

		Eigen::Matrix<T, 4, 4> res;
		res << s.x(), s.y(), s.z(), -s.dot(eye),
			u.x(), u.y(), u.z(), -u.dot(eye),
			-f.x(), -f.y(), -f.z(), f.dot(eye),
			0, 0, 0, 1;

		return res;
	};
};
