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
#define PIOVER2 (1.5707963267948966f)

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
	float aspectRatio;
	Eigen::Vector2f lensShift;
	Eigen::Vector2f clipPlanes;
	ModifiablePath resultFile;
	bool dataOnly;
	int rayBounces;
	int aaSamples;
	std::string shadingOverride;
	OSLShader* cameraEffect;

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(JSONWriterRef writer) override
	{
		writer.Key("fov");
		AddEigenVector<Eigen::Vector2f>(writer, fieldOfView);

		writer.Key("aspect");
		AddFloat(writer, aspectRatio);

		writer.Key("shift");
		AddEigenVector<Eigen::Vector2f>(writer, lensShift);

		writer.Key("nearZ");
		AddFloat(writer, clipPlanes.x());

		writer.Key("resultFile");
		AddString(writer, resultFile.string());

		writer.Key("dataOnly");
		writer.Bool(dataOnly);

		writer.Key("rayBounces");
		writer.Int(rayBounces);

		writer.Key("aaSamples");
		writer.Int(aaSamples);

		writer.Key("shadingOverride");
		AddString(writer, shadingOverride);

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
	inline Eigen::Vector2f GetFOV() const { return fieldOfView; }
	inline float GetAspectRatio() const { return aspectRatio; }
	inline Eigen::Vector2f GetShift(Eigen::Vector2f shift) const { return shift; }
	inline void SetClipping(float near, float far) { clipPlanes = Eigen::Vector2f(abs(near), abs(far)); }
	inline Eigen::Vector2f GetClipping() const { return clipPlanes; }
	inline void SetResultFile(ReferencePath result) { resultFile = result; }
	inline const ReferencePath GetResultFile() const { return resultFile; }
	inline void SetDataOnly(bool rendersData) { dataOnly = rendersData; }
	inline bool GetDataOnly() const { return dataOnly; }
	inline int GetRayBounces() const { return rayBounces; }
	inline void SetRayBounces(int bounces) { rayBounces = bounces; }
	inline int GetAASamples() const { return aaSamples; }
	inline void SetAASamples(int samples) { aaSamples = samples; }
	inline const std::string& GetShadingOverride() const { return shadingOverride; }
	inline void SetShadingOverride(const std::string& so) { shadingOverride = so; }

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
		float fovx = 2.0f * atan(cameraIntrinsics.GetWidth() / (2.0f * cameraIntrinsics.GetFocalLenght().x()));
		float fovy = 2.0f * atan(cameraIntrinsics.GetHeight() / (2.0f * cameraIntrinsics.GetFocalLenght().y()));

		// Calculate aspect ratio
		float aspect = (cameraIntrinsics.GetFocalLenght().y() / cameraIntrinsics.GetFocalLenght().x()) *
			(static_cast<float>(cameraIntrinsics.GetWidth()) / static_cast<float>(cameraIntrinsics.GetHeight()));

		// Find max(width, height) (so shift is equal in x/y)
		float resMax = static_cast<float>(cameraIntrinsics.GetWidth() > cameraIntrinsics.GetHeight() ?
			cameraIntrinsics.GetWidth() : cameraIntrinsics.GetHeight());

		// Calculate lens shift (Blender specific)
		float shiftx = (cameraIntrinsics.GetPrincipalPoint().x() - (0.5f * cameraIntrinsics.GetWidth())) / cameraIntrinsics.GetWidth();
		shiftx *= -0.5f * (static_cast<float>(cameraIntrinsics.GetWidth()) / resMax);
		float shifty = (cameraIntrinsics.GetPrincipalPoint().y() - (0.5f * cameraIntrinsics.GetHeight())) / cameraIntrinsics.GetHeight();
		shifty *= -0.5f * (static_cast<float>(cameraIntrinsics.GetHeight()) / resMax);

		// Store in Camera
		fieldOfView = Eigen::Vector2f(fovx, fovy);
		aspectRatio = aspect;
		lensShift = Eigen::Vector2f(shiftx, shifty);

		SetTransform(matTrans);
		// Transform from OpenGL -> Blender coordinate system
		SetScale(Eigen::Vector3f(1.0f, -1.0f, -1.0f));
	}

	//---------------------------------------
	// Constructors
	//---------------------------------------

	Camera() :
		cameraIntrinsics(),
		fieldOfView(Eigen::Vector2f(0.6911f, 0.4711f)),
		lensShift(Eigen::Vector2f(0.0f, 0.0f)),
		clipPlanes(Eigen::Vector2f(0.1f, 10.0f)),
		resultFile(""),
		aspectRatio(1.777f),
		dataOnly(false),
		rayBounces(-1),
		aaSamples(16),
		shadingOverride(""),
		cameraEffect(NULL)
	{
	}

	~Camera()
	{
		// Camera is responsible for shader
		delete cameraEffect;
	}
};
