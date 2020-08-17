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
	cv::Mat objectMask;

public:
	//---------------------------------------
	// Methods
	//---------------------------------------

	inline void Begin(
		const Settings& settings
	)
	{
		// Build path
		ModifiablePath path(settings.GetFinalPath());
		path.append("labels.csv");
		// Make sure stream is ready
		if (osAnnotations.is_open())
			osAnnotations.close();
		// Open or create file
		osAnnotations.open(path, std::ios_base::app);
		// Create mask storage
		objectMask.create(settings.GetRenderResolution().y(), settings.GetRenderResolution().x(), CV_8UC1);
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
		if (!ComputeObjectVisible(labeled, segmented, objectMask, currMesh->GetObjId()))
			return;

		// Compute bounding box
		cv::Rect bbox = ComputeBoundingBox(objectMask);

		// Compute world space pose of object
		Eigen::Vector3f pos(currBody->GetPosition());
		Eigen::Quaternionf rot(currBody->GetRotation());

		// Add formatted info to annotation file
		osAnnotations << FormatInt(currImage) << "; "
			<< bbox.x << "; " << bbox.y << "; " << bbox.width << "; " << bbox.height << "; "
			<< currMesh->GetName() << "; " << currMesh->GetMeshId() << "; " << currMesh->GetObjId() << "; "
			<< pos[0] << "; " << pos[1] << "; " << pos[2] << "; "
			<< rot.coeffs()[3] << "; " << rot.coeffs()[0] << "; " << rot.coeffs()[1] << "; " << rot.coeffs()[2] << "["
			<< renderCam.GetIntrinsics().GetFocalLenght().x() << "; " << renderCam.GetIntrinsics().GetFocalLenght().y() << "; "
			<< renderCam.GetIntrinsics().GetPrincipalPoint().x() << "; " << renderCam.GetIntrinsics().GetPrincipalPoint().y() << "]"
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
