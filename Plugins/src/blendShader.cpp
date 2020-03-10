#pragma once

#ifndef ai_common_mtds
#include <ai.h>
#endif
#include <string.h>
#include <iostream>
#include <math.h>

#include <opencv2/highgui.hpp>

AI_SHADER_NODE_EXPORT_METHODS(BlendShader);

enum BlendParams
{
	p_mask,
	p_blend_image,
	p_rend_image,
	p_force_scene,
	p_Kd_bcolor
};

node_parameters
{
		AiParameterPtr("mask", NULL);
		AiParameterPtr("blend_image", NULL);
		AiParameterPtr("rend_image", NULL);
		AiParameterBool("force_scene", false);
		AiParameterRGB("Kd_bcolor", 0.f, 0.f, 0.f);
}

node_initialize
{
}

node_update
{
}

node_finish
{
}

bool comp(float a, float b)
{
	return (a < b);
}

shader_evaluate
{
	using namespace cv;

// Get images
Mat mask = *(Mat*)AiShaderEvalParamPtr(p_mask);
Mat blend_image = *(Mat*)AiShaderEvalParamPtr(p_blend_image);
Mat rend_image = *(Mat*)AiShaderEvalParamPtr(p_rend_image);

AtRGB rgba;

// Pixel X/Y
int x = sg->x;
int y = sg->y;

// Get pixels from blended and rendered images
Vec3b pxBlend = blend_image.at<Vec3b>(y, x);
Vec3b pxRend = rend_image.at<Vec3b>(y, x);

// Get color
AtRGB bodyC = AiShaderEvalParamRGB(p_Kd_bcolor);

float avg = 0;
// For debugging: 
if (false && (float)pxRend[0] <= 0 && (float)pxRend[1] <= 0 && (float)pxRend[2] <= 0)
{
	// If forced draw
	if (!AiShaderEvalParamBool(p_force_scene))
	{
		int count = 0;
		// 9 px radius
		for (int i = -3; i < 4; i++)
		{
			for (int j = -3; j < 4; j++)
			{
				int currY = y + i;
				int currX = x + j;
				// Check bounds
				if (currX >= 0 && currY >= 0 && currX < blend_image.cols && currY < blend_image.rows)
				{
					// Add to average
					Vec3b curr = blend_image.at<Vec3b>(currY, currX);
					avg += curr[0] + curr[1] + curr[2];
					count += 1;
				}
			}
		}
		// Calculate average
		if (count > 0)
		{
			avg = avg / (1.2f * count * 255);
		}
	}

	// Clamp
	if (avg > 1.3)
		avg = 1.3;
	if (avg < 0.35)
		avg = 0.35;

	int count = 0;
	float mask_sum = 0;
	// 2 px radius
	for (int i = -1; i < 2; i++)
	{
		for (int j = -1; j < 2; j++)
		{
			int currY = i + y * 2;
			int currX = j + x * 2;
			// Bounds check
			if (currX >= 0 && currY >= 0 && currX < mask.cols && currY < mask.rows)
			{
				// Add to mask
				mask_sum += float(mask.at<uchar>(currY, currX) > 0);
				++count;
			}
		}
	}

	// Calculate influence
	float body_ratio = 0;
	if (count > 0)
	{
			body_ratio = mask_sum / count;
	}

	// If forced draw
	if (AiShaderEvalParamBool(p_force_scene))
	{
		AtColor opac;
		// Determine transparency
		if (body_ratio <= 0.5)
		{
			opac.r = 1;
			opac.g = 1;
			opac.b = 1;
		}
		else
		{
			opac.r = 0;
			opac.g = 0;
			opac.b = 0;
		}
		// Set transparency
		sg->out_opacity = opac;
		body_ratio = 0;
	}

	//blend scene with rgb
	rgba.r = bodyC.r * body_ratio * 1.5 * avg + (((float)pxBlend[2]) / 255.0) * (1.0 - body_ratio);
	rgba.g = bodyC.g * body_ratio * 1.5 * avg + (((float)pxBlend[1]) / 255.0) * (1.0 - body_ratio);
	rgba.b = bodyC.b * body_ratio * 1.5 * avg + (((float)pxBlend[0]) / 255.0) * (1.0 - body_ratio);
}
// Currently used mode
else
{
	if (float(mask.at<uchar>(2 * y, 2 * x)) > 0)
	{
		// float l_body = std::max(bodyC.r, bodyC.g, bodyC.b) + std::min(bodyC.r, bodyC.g, bodyC.b) / 2.f;
		int count = 0;
		float l_diff = 0;
		/*
		// light diff averaging from neighbors
		for (int i = -1; i < 2; i++)
		{
			for (int j = -1; j < 2; j++)
			{
				int currY = i + y;
				int currX = j + x;
				if (currX >= 0 && currY >= 0 && currX < rend_image.cols && currY < rend_image.rows)
				{
					Vec3b r_ = rend_image.at<Vec3b>(currY, currX);
					Vec3b p_ = blend_image.at<Vec3b>(currY, currX);

					float l_rend = (std::max({r_[0], r_[1], r_[2]}, comp) + std::min({r_[0], r_[1], r_[2]}, comp)) / 2.f;
					float l_scene = (std::max({p_[0], p_[1], p_[2]}, comp) + std::min({p_[0], p_[1], p_[2]}, comp)) / 2.f;
					l_diff += (l_rend - l_scene) / 255.f;
					++count;
				}
			}
		}
		l_diff = l_diff / count;
		*/
		rgba.r = (bodyC.r + l_diff);
		rgba.g = (bodyC.g + l_diff);
		rgba.b = (bodyC.b + l_diff);
	}
	else
	{
		rgba.r = (((float)pxBlend[2]) / 255.0);
		rgba.g = (((float)pxBlend[1]) / 255.0);
		rgba.b = (((float)pxBlend[0]) / 255.0);
	}
}
// Write out color
sg->out.RGB = rgba;
}
