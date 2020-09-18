#pragma once

#pragma warning(push, 0)
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <Helpers/PathUtils.h>
#pragma warning(pop)

//---------------------------------------
// Converts int to uchar vector
//---------------------------------------
static cv::Vec3b EncodeInt(
	int toEncode
)
{
	// 254 shades of grey
	return cv::Vec3b(toEncode, toEncode, toEncode);
}

//---------------------------------------
// Computes if image is blurry and outputs results
//---------------------------------------
static bool ComputeIsBlurry(
	const cv::Mat& image,
	float edgeWeakThreshold,
	float edgeStrongThreshold,
	float edgeMinThreshold,
	float& edgeResult,
	float& freqencyResult
)
{
	cv::Mat gray, edges, laplace;
	cv::Scalar meanEdge, meanLaplace, devEdge, devLaplace;
	// Convert to grayscale
	cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
	// Edge detection & laplacian (frequency)
	cv::Canny(gray, edges, edgeWeakThreshold, edgeStrongThreshold);
	cv::Laplacian(gray, laplace, CV_64F);
	// Calculate mean and standard deviation
	cv::meanStdDev(edges, meanEdge, devEdge);
	cv::meanStdDev(laplace, meanLaplace, devLaplace);
	// Store edge mean and frequency deviation
	edgeResult = static_cast<float>(fabs(meanEdge.val[0]));
	freqencyResult = static_cast<float>(fabs(devLaplace.val[0]));
	// Image is blurry if not enough edges were detected
	return edgeResult < edgeMinThreshold;
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
	// Determine masked amount visible
	objectMask = segmented == bodyId;
	const float maskSum = cv::sum(objectMask)[0];
	// Stop if object not visible at all
	if (maskSum < FLT_EPSILON)
		return false;
	// Determine unmasked amount & visibility
	objectMask = labeled == bodyId;
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
	const cv::Mat& bodiesAO,
	const cv::Mat& sceneRGB,
	const cv::Mat& bodiesMask
)
{
	cv::Mat blendResult;
	// Add PBR object image to blend
	bodiesRGB.copyTo(blendResult, bodiesMask);
	// Add ambient occlusion
	blendResult.forEach<cv::Vec3b>([&](cv::Vec3b& val, const int pixel[]) -> void {
		val *= bodiesAO.at<float>(pixel[0], pixel[1]);
	});
	// Add scene RGB image to blend
	sceneRGB.copyTo(blendResult, 255 - bodiesMask);
	// Return blended image
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
	cv::Mat segmented(labeled.rows, labeled.cols, CV_8UC1);
	segmented.setTo(cv::Vec<uchar, 1>(255));
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
static auto UnpackDepth = [](cv::Mat& packed) -> cv::Mat
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
// Converts packed label to single channel
//---------------------------------------
static auto UnpackLabel = [](cv::Mat& packed) -> cv::Mat
{
	cv::Mat unpacked(packed.rows, packed.cols, CV_8UC1);
	// Unpack float and convert to rgb
	unpacked.forEach<uchar>([&](uchar& val, const int pixel[]) -> void {
		float labelPacked = packed.at<cv::Vec3f>(pixel[0], pixel[1])[0] * 255.0f;
		val = labelPacked > FLT_EPSILON ? (uchar)labelPacked : val;
	});
	return unpacked;
};

//---------------------------------------
// Converts packed ao to [0-1]
//---------------------------------------
static auto UnpackAO = [](cv::Mat& packed) -> cv::Mat
{
	cv::Mat unpacked = cv::Mat::ones(packed.rows, packed.cols, CV_32FC1);
	// Load rgb and convert to float [0-1]
	unpacked.forEach<float>([&](float& val, const int pixel[]) -> void {
		val = cv::norm(packed.at<cv::Vec3b>(pixel[0], pixel[1]), cv::NormTypes::NORM_L1) / 765.0f;
	});
	return unpacked;
};
