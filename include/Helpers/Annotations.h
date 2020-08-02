#pragma once

#include <vector>

#pragma warning(push, 0)
#include <Eigen/Dense>

#include <Helpers/ImageProcessing.h>
#include <Helpers/PathUtils.h>

#include <Rendering/Settings.h>
#include <Rendering/Camera.h>
#include <Meshes/RenderMesh.h>
#pragma warning(pop)

//---------------------------------------
// Handles annotations
//---------------------------------------
class AnnotationsManager
{
private:
	//---------------------------------------
	// Fields
	//---------------------------------------

	boost::filesystem::ofstream osAnnotations;

	//---------------------------------------
	// Methods
	//---------------------------------------

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

	inline void Begin(
		const Settings* settings
	)
	{
		// Build path
		ModifiablePath path(settings->GetFinalPath());
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
		cv::Mat currObjectMask;
		if (!ComputeObjectVisible(labeled, segmented, currObjectMask, currMesh->GetLabelId()))
			return;

		// Compute bounding box
		cv::Rect bbox = ComputeBoundingBox(currObjectMask);

		// Compute world space pose of object
		Eigen::Vector3f pos(currBody->GetPosition());
		Eigen::Quaternionf rot(currBody->GetRotation());
		X_TransformPose(renderCam, pos, rot);

		// Add formatted info to annotation file
		osAnnotations << currImage << ", " << bbox.x << ", " << bbox.y << ", " << bbox.width << ", " << bbox.height << ", "
			<< "obj_" << FormatInt(currMesh->GetMeshId()) << ", "
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

	AnnotationsManager()
	{
	}
};
