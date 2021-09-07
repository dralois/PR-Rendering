#pragma once

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
			<< "Object Class" << sep << "Object Name" << sep << "Object Instance ID" << sep
			<< "Pos x" << sep << "Pos y" << sep << "Pos z" << sep
			<< "Rot w" << sep << "Rot x" << sep << "Rot y" << sep << "Rot z" << sep
			<< "Scl x" << sep << "Scl y" << sep << "Scl z" << sep
			<< "Intr fx" << sep << "Intr fy" << sep << "Intr ox" << sep << "Intr oy" << sep
			<< "Learn 3D" << sep
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
		const RenderMesh& currBody,
		const cv::Mat& labeled,
		const cv::Mat& segmented,
		const Camera& renderCam
	)
	{
		// Only annotate properly visible objects
		if (!ComputeObjectVisible(labeled, segmented, objectMask, currBody.GetObjId()))
			return;

		// Compute bounding box
		cv::Rect bbox = ComputeBoundingBox(objectMask);

		// Compute camera space pose of object
		Eigen::Matrix4f bodyTrans = currBody.GetTransform();
		Eigen::Affine3f bodyToCam(renderCam.ToCameraSpace(bodyTrans));
		// Compute current values
		Eigen::Affine3f::LinearMatrixType rotation, scale;
		bodyToCam.computeRotationScaling(&rotation, &scale);
		// Update internals
		Eigen::Vector3f pos(bodyToCam.translation().eval());
		Eigen::Quaternionf rot(rotation);
		Eigen::Vector3f scl(scale.diagonal());

		// Add formatted info to annotation file
		osAnnotations << bbox.x << sep << bbox.y << sep << bbox.width << sep << bbox.height << sep
			<< currBody.GetMeshClass() << sep << currBody.GetName() << sep << currBody.GetObjId() << sep
			<< pos[0] << sep << pos[1] << sep << pos[2] << sep
			<< rot.coeffs()[3] << sep << rot.coeffs()[0] << sep << rot.coeffs()[1] << sep << rot.coeffs()[2] << sep
			<< scl[0] << sep << scl[1] << sep << scl[2] << sep
			<< renderCam.GetIntrinsics().GetFocalLenght().x() << sep << renderCam.GetIntrinsics().GetFocalLenght().y() << sep
			<< renderCam.GetIntrinsics().GetPrincipalPoint().x() << sep << renderCam.GetIntrinsics().GetPrincipalPoint().y() << sep
			<< "1" << sep
			<< end;
	}

	inline void End()
	{
		// Properly close file
		if (osAnnotations.good())
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
		ReferencePath storePath,
		Eigen::Vector2i resolution
	):
		basePath(storePath)
	{
		// Create mask buffer
		objectMask.create(resolution.y(), resolution.x(), CV_8UC1);
	}

	~AnnotationsManager()
	{
		End();
	}
};
