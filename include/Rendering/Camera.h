#pragma once

#pragma warning(push, 0)
#include <Helpers/JSONUtils.h>
#include <Helpers/PathUtils.h>

#include <Rendering/Settings.h>
#include <Rendering/Intrinsics.h>
#include <Rendering/Shader.h>
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
	ModifiablePath resultFile;
	bool depthOnly;
	OSLShader* cameraEffect;

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(JSONWriterRef writer) override
	{
		writer.Key("fov");
		AddEigenVector<Eigen::Vector2f>(writer, fieldOfView);

		writer.Key("shift");
		AddEigenVector<Eigen::Vector2f>(writer, lensShift);

		writer.Key("nearZ");
		AddFloat(writer, clipPlanes.x());

		writer.Key("resultFile");
		AddString(writer, resultFile.string());

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
	inline void SetResultFile(ReferencePath result) { resultFile = result; }
	inline const ReferencePath GetResultFile() const { return resultFile; }
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

	inline void LoadIntrinsics(const Settings* settings)
	{
		// Load intrinsics (custom or provided ones)
		if (SafeGet<bool>(settings->GetJSONConfig(), "custom_intrinsics"))
		{
			SetIntrinsics(settings->GetIntrinsics());
		}
		else
		{
			Intrinsics fromFile;
			// Load from file
			fromFile.LoadIntrinsics(
				settings->GetSceneRGBPath().append("_info.txt"),
				settings->GetRenderResolution()
			);
			// Store in camera
			SetIntrinsics(fromFile);
		}
	}

	inline void LoadExtrinsics(ReferencePath extrFile)
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
		resultFile(),
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
