#pragma once

#pragma warning(push, 0)
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#include <Helpers/PathUtils.h>
#pragma warning(pop)

//---------------------------------------
// Converts int to uchar vector
//---------------------------------------
static cv::Vec3b EncodeInt(
	int toEncode
)
{
	static std::hash<int> hash;
	// Hash for nicer color spread
	static int maxVal = (1 << 24) - 1;
	toEncode = (((int) hash(toEncode)) % maxVal) + 1;
	// Shift into uchars
	uchar encode8 = (uchar)toEncode;
	toEncode >>= 8;
	uchar encode16 = (uchar)toEncode;
	toEncode >>= 8;
	uchar encode24 = (uchar)toEncode;
	return cv::Vec3b(encode8, encode16, encode24);
}

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
	cv::Mat& objectMask,
	int bodyId
)
{
	// Single out current object
	const cv::Vec3b encodedId = EncodeInt(bodyId);
	// Determine masked amount visible
	cv::inRange(segmented, encodedId, encodedId, objectMask);
	const float maskSum = cv::sum(objectMask)[0];
	// Stop if object not visible at all
	if (maskSum < FLT_EPSILON)
		return false;
	// Determine unmasked amount & visibility
	cv::inRange(labeled, encodedId, encodedId, objectMask);
	const float bodySum = cv::sum(objectMask)[0];
	const float visibility = maskSum / bodySum;
	// Visible if at least 30% unoccluded & 2000px big
	return (visibility > 0.3f && static_cast<int>(maskSum) / 255 > 2000);
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
// Simple mask based blending operation
//---------------------------------------
static cv::Mat ComputeRGBBlend(
	const cv::Mat& bodiesRGB,
	const cv::Mat& sceneRGB,
	const cv::Mat& bodiesMask
)
{
	cv::Mat blendResult, maskResized;
	sceneRGB.copyTo(blendResult);
	cv::resize(bodiesMask, maskResized, cv::Size(bodiesRGB.cols, bodiesRGB.rows));
	bodiesRGB.copyTo(blendResult, maskResized);
	return blendResult;
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
static auto UnpackDepth = [&](cv::Mat& packed) -> cv::Mat
{
	cv::Mat unpacked = cv::Mat::zeros(packed.rows, packed.cols, CV_32FC1);
	// Unpack into one channel, convert no hit (0.0f) to inf
	unpacked.forEach<float>([&](float& val, const int pixel[]) -> void {
		float distance = packed.at<float>(pixel[0], pixel[1]);
		val = distance == 0.0f ? FLT_MAX : distance;
	});
	return unpacked;
};

//---------------------------------------
// Converts packed label to rgb
//---------------------------------------
static auto UnpackLabel = [&](cv::Mat& packed) -> cv::Mat
{
	cv::Mat unpacked = cv::Mat::zeros(packed.rows, packed.cols, CV_8UC3);
	// Unpack float and convert to rgb
	unpacked.forEach<cv::Vec3b>([&](cv::Vec3b& val, const int pixel[]) -> void {
		cv::Vec3f labelPacked = packed.at<cv::Vec3f>(pixel[0], pixel[1]) * 255.0f;
		val = cv::Vec3b((uchar)labelPacked[0], (uchar)labelPacked[1], (uchar)labelPacked[2]);
	});
	return unpacked;
};
