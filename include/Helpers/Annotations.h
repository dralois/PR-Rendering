#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iostream>

#pragma warning(push, 0)
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <Eigen/Dense>

#include <opencv2/opencv.hpp>

#include <Helpers/ImageProcessing.h>

#include <Rendering/Settings.h>
#include <Rendering/Camera.h>
#include <Meshes/RenderMesh.h>
#pragma warning(pop)

//---------------------------------------
// Handles annotations & formatting
//---------------------------------------
class AnnotationsManager
{
private:
	//---------------------------------------
	// Fields
	//---------------------------------------

	const Settings* settings;
	boost::filesystem::ofstream osAnnotations;

	//---------------------------------------
	// Methods
	//---------------------------------------

	inline std::string X_FormatInt(int num) const
	{
		std::ostringstream numStr;
		numStr << std::internal << std::setfill('0') << std::setw(6) << num;
		return numStr.str();
	}

	inline void X_TransformPose(
		const Camera& renderCam,
		Eigen::Vector3f& pos,
		Eigen::Quaternionf& rot
	) const
	{
		// Create transform for pose
		Eigen::Affine3f transObj;
		transObj = transObj.fromPositionOrientationScale(pos, rot, Eigen::Vector3f::Identity());

		// Camera space -> World space
		transObj = renderCam.GetTransform().transpose().inverse().matrix() * transObj.matrix();

		// Save scaled position & rotation as quaternion
		pos = Eigen::Vector3f(transObj.translation()) * 10.0f;
		rot = Eigen::Quaternionf(transObj.rotation().matrix());
	}

public:
	//---------------------------------------
	// Methods
	//---------------------------------------

	// Formatting

	inline boost::filesystem::path GetImagePath(
		const std::string& category,
		int id,
		bool temp
	) const
	{
		boost::filesystem::path bodyPath(temp ? settings->GetTemporaryPath() : settings->GetFinalPath());
		bodyPath.append(category);
		bodyPath.append("img_" + X_FormatInt(id) + ".png");
	}

	inline boost::filesystem::path GetSceneRGBPath() const
	{
		boost::filesystem::path scenePath(settings->GetScenePath());
		return scenePath.append("rgbd");
	}

	// Annotations

	inline void Begin()
	{
		// Build path
		boost::filesystem::path path(settings->GetFinalPath());
		path.append("labels.csv");
		// Make sure stream is ready
		if (osAnnotations.is_open())
			osAnnotations.close();
		// Open or create file
		osAnnotations.open(path, std::ios_base::app);
	}

	inline void Write(
		const RenderMesh* currBody,
		const cv::Mat& labeled,
		const cv::Mat& segmented,
		const Camera& renderCam,
		int currImage
	)
	{
		MeshBase* currMesh = (MeshBase*)currBody;

		// Only annotate properly visible objects
		if (!ComputeObjectVisible(labeled, segmented, currMesh->GetLabelId()))
			return;

		// Compute bounding box
		cv::Rect bbox = ComputeBoundingBox(labeled == currMesh->GetLabelId());

		// Compute world space pose of object
		Eigen::Vector3f pos(currBody->GetPosition());
		Eigen::Quaternionf rot(currBody->GetRotation());
		X_TransformPose(renderCam, pos, rot);

		// Add formatted info to annotation file
		osAnnotations << currImage << ", " << bbox.x << ", " << bbox.y << ", " << bbox.width << ", " << bbox.height << ", "
			<< "obj_" << X_FormatInt(currMesh->GetMeshId()) << ", "
			<< rot.coeffs()[0] << ", " << rot.coeffs()[1] << ", " << rot.coeffs()[2] << ", " << rot.coeffs()[3] << ", "
			<< "0" << ", " << "0" << ", "
			<< pos[0] << ", " << pos[1] << ", " << pos[2] << ", "
			<< currMesh->GetLabelId() << " ["
			<< renderCam.GetIntrinsics().GetFocalLenght().x() << ", " << renderCam.GetIntrinsics().GetFocalLenght().y() << ", "
			<< renderCam.GetIntrinsics().GetPrincipalPoint().x() << ", " << renderCam.GetIntrinsics().GetPrincipalPoint().y() << "]"
			<< "\n";
	}

	inline void End()
	{
		osAnnotations.close();
	}

	//---------------------------------------
	// Constructors
	//---------------------------------------

	AnnotationsManager(const Settings* settings) :
		settings(settings)
	{
	}
};
