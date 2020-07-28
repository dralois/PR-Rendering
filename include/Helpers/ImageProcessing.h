#pragma once

#pragma warning(push, 0)
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#include <Helpers/PathUtils.h>
#pragma warning(pop)

//---------------------------------------
// Computes how blurry an image in memory is
//---------------------------------------
static float ComputeVariance(
	const cv::Mat& image
)
{
	cv::Mat gray;
	cv::Mat laplacianImage;
	cv::Scalar mean, deviation;
	// Convert to grayscale
	cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
	// Create laplacian and calculate deviation
	cv::Laplacian(gray, laplacianImage, CV_64F);
	cv::meanStdDev(laplacianImage, mean, deviation, cv::Mat());
	// Return variance
	return deviation.val[0] * deviation.val[0];
}

//---------------------------------------
// Computes how blurry an image on disk is
//---------------------------------------
static float ComputeVariance(
	ReferencePath path
)
{
	// Load image from path & compute variance
	cv::Mat image = cv::imread(path.string());
	return ComputeVariance(image);
}

//---------------------------------------
// Computes objects mask & visibility
//---------------------------------------
static cv::Mat ComputeOcclusionMask(
	const cv::Mat& bodiesDepth,
	const cv::Mat& sceneDepth,
	bool& occluded
)
{
	// Create mask (value == 0 | 255) & compute mean
	cv::Mat maskOut = bodiesDepth < sceneDepth;
	float maskMean = cv::mean(maskOut)[0];
	// Return mask & whether it is not mostly empty
	occluded = maskMean < 1.0f;
	return maskOut;
}

//---------------------------------------
// Computes if object is mostly visible
//---------------------------------------
static bool ComputeObjectVisible(
	const cv::Mat& labeled,
	const cv::Mat& segmented,
	int bodyId
)
{
	// Determine unmasked amount
	float bodySum = cv::sum(labeled == bodyId)[0];
	// Stop if object not visible at all
	if (bodySum < FLT_EPSILON)
		return false;
	// Determine masked amount and coverage
	float maskSum = cv::sum(segmented == bodyId)[0];
	float percent = maskSum / bodySum;
	// Visible if at least 30% uncovered & 2000px big
	return (percent > 0.3f && static_cast<int>(maskSum) / 255 > 2000);
}

//---------------------------------------
// Blends depth images and write to file
//---------------------------------------
static cv::Mat ComputeDepthBlend(
	const cv::Mat& bodiesDepth,
	const cv::Mat& sceneDepth
)
{
	// Take minimal depth from scene and objects
	return cv::min(sceneDepth, bodiesDepth);
}

//---------------------------------------
// Creates label image from depth images
//---------------------------------------
static cv::Mat ComputeSegmentMask(
	const cv::Mat& labeled,
	const cv::Mat& masked
)
{
	// Mask the labeled image -> segmented
	cv::Mat segmented;
	labeled.copyTo(segmented, masked);
	return segmented;
}

//---------------------------------------
// Computes bounding box of mask
//---------------------------------------
static cv::Rect ComputeBoundingBox(
	const cv::Mat& mask
)
{
	// Calculate bounding box of mask
	cv::Rect minRect = cv::boundingRect(mask);
	// Shift origin & return it
	minRect.x += minRect.width / 2;
	minRect.y += minRect.height / 2;
	return minRect;
}

//---------------------------------------
// Converts packed depth to float
//---------------------------------------
static cv::Mat UnpackRGBDepth(
	const cv::Mat& packed
)
{
	cv::Mat unpacked;
	unpacked.create(packed.rows, packed.cols, CV_32FC1);
	const float depthScale = (256.0f * 256.0f * 256.0f) / (256.0f * 256.0f * 256.0f - 1.0f);
	// Unpack & convert each pixel to linear float depth
	for (int i = 0; i < packed.rows; i++)
	{
		for (int j = 0; j < packed.cols; j++)
		{
			cv::Vec3f depthPacked = cv::Vec3f(
				(float)packed.at<cv::Vec3b>(i, j)[0],
				(float)packed.at<cv::Vec3b>(i, j)[1],
				(float)packed.at<cv::Vec3b>(i, j)[2]
			);
			unpacked.at<float>(i, j) = depthScale *
				depthPacked.dot(cv::Vec3f(1.0f, 1.0f / 256.0f, 1.0f / (256.0f * 256.0f)));
		}
	}
	// Return unpacked depth map
	return unpacked;
}
