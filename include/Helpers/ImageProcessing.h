#pragma once

#pragma warning(push, 0)
#include <boost/filesystem.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#pragma warning(pop)

//---------------------------------------
// Loads an image from path as rgb / depth
//---------------------------------------
static cv::Mat LoadImage(
	const boost::filesystem::path& path,
	bool grayscale
)
{
	return cv::imread(path.string(), grayscale ? cv::IMREAD_ANYDEPTH : cv::IMREAD_COLOR);
}

//---------------------------------------
// Computes blurriness of an image
//---------------------------------------
static float ComputeVariance(const boost::filesystem::path& path)
{
	cv::Mat gray;
	cv::Mat laplacianImage;
	cv::Scalar mean, deviation;
	// Convert to grayscale
	cv::Mat image = LoadImage(path, false);
	cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
	// Create laplacian and calculate deviation
	cv::Laplacian(gray, laplacianImage, CV_64F);
	cv::meanStdDev(laplacianImage, mean, deviation, cv::Mat());
	// Return variance
	return deviation.val[0] * deviation.val[0];
}

//---------------------------------------
// Computes objects mask & visibility
//---------------------------------------
static bool ComputeObjectsMask(
	const cv::Mat& bodiesDepth,
	const cv::Mat& sceneDepth,
	cv::Mat& maskOut
)
{
	// Create mask (value == 0 | 255) & compute mean
	maskOut = bodiesDepth < sceneDepth;
	float maskMean = cv::mean(maskOut)[0];
	// Return whether mask is not mostly empty
	return maskMean > 1.0f;
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
static cv::Mat BlendDepth(
	const cv::Mat& bodiesDepth,
	const cv::Mat& sceneDepth,
	const boost::filesystem::path& output
)
{
	// Blend depth from scene and objects
	cv::Mat blended = cv::min(sceneDepth, bodiesDepth);
	// Store image on disk & return it
	cv::imwrite(output.string(), blended);
	return blended;
}

//---------------------------------------
// Creates label image from depth images
//---------------------------------------
static cv::Mat BlendLabel(
	const cv::Mat& labeled,
	const cv::Mat& masked,
	const boost::filesystem::path& output
)
{
	// Mask the labeled image -> segmented
	cv::Mat segmented;
	labeled.copyTo(segmented, masked);
	// Store image on disk & return it
	cv::imwrite(output.string(), segmented);
	return segmented;
}

//---------------------------------------
// Computes bounding box of mask
//---------------------------------------
static cv::Rect ComputeBoundingBox(const cv::Mat& mask)
{
	// Calculate bounding box of mask
	cv::Rect minRect = cv::boundingRect(mask);
	// Shift origin & return it
	minRect.x += minRect.width / 2;
	minRect.y += minRect.height / 2;
	return minRect;
}
