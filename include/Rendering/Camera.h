#pragma once

#pragma warning(push, 0)
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <Shaders/Shading.h>
#include <Renderfile.h>
#pragma warning(pop)

#define PI (3.1415926535897931f)

//---------------------------------------
// Camera object wrapper for rendering
//---------------------------------------
class Camera : public RenderfileObject
{
private:
	//---------------------------------------
	// Fields
	//---------------------------------------

	Intrinsics cameraIntrinsics;
	Eigen::Vector2f fieldOfView;
	Eigen::Vector2f lensShift;
	Eigen::Vector2f clipPlanes;
	boost::filesystem::path resultName;
	bool depthOnly;
	OSLShader* cameraEffect;

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(rapidjson::PrettyWriter<rapidjson::StringStream>& writer) override
	{
		writer.Key("fov");
		AddEigenVector<Eigen::Vector2f>(writer, fieldOfView);

		writer.Key("shift");
		AddEigenVector<Eigen::Vector2f>(writer, lensShift);

		writer.Key("nearZ");
		AddFloat(writer, clipPlanes.x());

		writer.Key("result");
		AddString(writer, resultName.string());

		writer.Key("depthOnly");
		writer.Bool(depthOnly);

		// Camera does not always have an effect set
		if (cameraEffect)
		{
			writer.Key("shader");
			cameraEffect->AddToJSON(writer);
		}
	}

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	inline const Intrinsics& GetIntrinsics() const { return cameraIntrinsics; }
	inline void SetIntrinsics(Intrinsics intr) { cameraIntrinsics = intr; }
	inline void SetFOV(Eigen::Vector2f fov) { fieldOfView = fov; }
	inline Eigen::Vector2f GetFOV() const { return fieldOfView; }
	inline void SetShift(Eigen::Vector2f shift) { lensShift = shift; }
	inline Eigen::Vector2f GetShift(Eigen::Vector2f shift) const { return shift; }
	inline void SetClipping(float near, float far) { clipPlanes = Eigen::Vector2f(near, far); }
	inline Eigen::Vector2f GetClipping() const { return clipPlanes; }
	inline void SetResultFile(const std::string& result) { resultName = result; }
	inline const boost::filesystem::path& GetResultFile() const { return resultName; }
	inline void SetDepthOnly(bool renderDepth) { depthOnly = renderDepth; }
	inline bool GetDepthOnly() const { return depthOnly; }

	inline const OSLShader* GetEffect() const { return cameraEffect; }
	inline void SetEffect(OSLShader* effect)
	{
		delete cameraEffect;
		cameraEffect = effect;
	}

	//---------------------------------------
	// Methods
	//---------------------------------------

	inline void LoadExtrinsics(const boost::filesystem::path& extrFile)
	{
		Eigen::Matrix4f matTrans;

		// File must exist
		boost::filesystem::ifstream extrFileStream;
		extrFileStream.open(extrFile);
		if (!extrFileStream.is_open())
			return;

		// Load camera matrix
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				extrFileStream >> matTrans(i, j);
		extrFileStream.close();

		// Calculate fov
		float fovx = 2.0f * atan(cameraIntrinsics.GetWidth() / (4.0f * cameraIntrinsics.GetFocalLenght().x())) * (180.0f / PI);
		float fovy = 2.0f * atan(cameraIntrinsics.GetHeight() / (4.0f * cameraIntrinsics.GetFocalLenght().y())) * (180.0f / PI);

		// Calculate lens shift
		float shiftx = ((4.0f * cameraIntrinsics.GetPrincipalPoint().x()) - cameraIntrinsics.GetWidth()) / cameraIntrinsics.GetWidth();
		float shifty = ((4.0f * cameraIntrinsics.GetPrincipalPoint().y()) - cameraIntrinsics.GetHeight()) / cameraIntrinsics.GetHeight();

		// Store in Camera
		SetFOV(Eigen::Vector2f(fovx, fovy));
		SetShift(Eigen::Vector2f(shiftx, shifty));
		SetTransform(matTrans);
	}

	//---------------------------------------
	// Constructors
	//---------------------------------------

	Camera() :
		cameraIntrinsics(),
		fieldOfView(Eigen::Vector2f(0.6911f, 0.4711f)),
		lensShift(Eigen::Vector2f(0.0f, 0.0f)),
		clipPlanes(Eigen::Vector2f(0.1f, 10.0f)),
		resultName(""),
		depthOnly(false),
		cameraEffect(NULL)
	{
	}

	~Camera()
	{
		// Camera is responsible for shader
		delete cameraEffect;
	}
};
