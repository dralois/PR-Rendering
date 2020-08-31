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
	ModifiablePath basePath;
	cv::Mat objectMask;

	const char sep = ';';
	const char end = '\n';

	//---------------------------------------
	// Methods
	//---------------------------------------

	inline void WriteHeader()
	{
		// Description line
		osAnnotations << "BB x" << sep << "BB y" << sep << "BB w" << sep << "BB h" << sep
			<< "Object Name" << sep << "Mesh ID" << sep << "Object ID" << sep
			<< "Pos x" << sep << "Pos y" << sep << "Pos z" << sep
			<< "Rot w" << sep << "Rot x" << sep << "Rot y" << sep << "Rot z" << sep
			<< "Intr fx" << sep << "Intr fy" << sep << "Intr ox" << sep << "Intr oy" << sep
			<< end;
	}

public:
	//---------------------------------------
	// Methods
	//---------------------------------------

	inline void Begin(
		int currImage
	)
	{
		// Build path
		ModifiablePath path(basePath);
		path /= "labels_" + FormatInt(currImage) + ".csv";
		// Make sure stream is ready
		if (osAnnotations.is_open())
			osAnnotations.close();
		// Open or create file, clear existing
		osAnnotations.open(path, std::ios_base::trunc);
		// Add header
		WriteHeader();
	}

	inline void Write(
		const RenderMesh* currBody,
		const cv::Mat& labeled,
		const cv::Mat& segmented,
		const Camera& renderCam
	)
	{
		// Only annotate properly visible objects
		if (!ComputeObjectVisible(labeled, segmented, objectMask, currBody->GetObjId()))
			return;

		// Compute bounding box
		cv::Rect bbox = ComputeBoundingBox(objectMask);

		// FIXME position wrong?
		// Compute world space pose of object
		Eigen::Vector3f pos(currBody->GetPosition());
		Eigen::Quaternionf rot(currBody->GetRotation());

		// Add formatted info to annotation file
		osAnnotations << bbox.x << sep << bbox.y << sep << bbox.width << sep << bbox.height << sep
			<< currBody->GetName() << sep << currBody->GetMeshId() << sep << currBody->GetObjId() << sep
			<< pos[0] << sep << pos[1] << sep << pos[2] << sep
			<< rot.coeffs()[3] << sep << rot.coeffs()[0] << sep << rot.coeffs()[1] << sep << rot.coeffs()[2] << sep
			<< renderCam.GetIntrinsics().GetFocalLenght().x() << sep << renderCam.GetIntrinsics().GetFocalLenght().y() << sep
			<< renderCam.GetIntrinsics().GetPrincipalPoint().x() << sep << renderCam.GetIntrinsics().GetPrincipalPoint().y()
			<< end;
	}

	inline void End()
	{
		if(osAnnotations.good())
		{
			if (osAnnotations.is_open())
			{
				osAnnotations.close();
			}
		}
	}

	//---------------------------------------
	// Constructors
	//---------------------------------------

	AnnotationsManager(
		const Settings& settings
	)
	{
		// Get path for annotation files
		basePath = settings.GetFinalPath() / "annotations";
		// Create mask storage
		objectMask.create(settings.GetRenderResolution().y(), settings.GetRenderResolution().x(), CV_8UC1);
	}

	~AnnotationsManager()
	{
		End();
	}
};
