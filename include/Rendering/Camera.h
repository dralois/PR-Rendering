#pragma once

#pragma warning(push, 0)
#include <Helpers/JSONUtils.h>
#include <Helpers/PathUtils.h>

#include <Rendering/Settings.h>
#include <Rendering/Intrinsics.h>
#include <Renderfile.h>
#pragma warning(pop)

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
	ModifiablePath sourceFile;

	Eigen::Vector3f aspectFOV;
	Eigen::Vector2f lensShift;
	ModifiablePath resultFile;
	Eigen::Vector2i resolution;
	bool dataOnly;
	int rayBounces;
	int aaSamples;
	std::string shadingOverride;

	int imageNum;

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(JSONWriterRef writer) override
	{
		writer.Key("fov");
		AddEigenVector<Eigen::Vector3f>(writer, aspectFOV);

		writer.Key("shift");
		AddEigenVector<Eigen::Vector2f>(writer, lensShift);

		writer.Key("resultFile");
		AddString(writer, resultFile.string());

		writer.Key("resolution");
		AddEigenVector<Eigen::Vector2i>(writer, resolution);

		writer.Key("dataOnly");
		writer.Bool(dataOnly);

		writer.Key("rayBounces");
		writer.Int(rayBounces);

		writer.Key("aaSamples");
		writer.Int(aaSamples);

		writer.Key("shadingOverride");
		AddString(writer, shadingOverride);
	}

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	inline Intrinsics GetIntrinsics() const { return cameraIntrinsics; }
	inline void SetIntrinsics(const Intrinsics& intr) { cameraIntrinsics = intr; }
	inline ModifiablePath GetSourceFile() const { return sourceFile; }
	inline Eigen::Vector2f GetFOV() const { return Eigen::Vector2f(aspectFOV.y(), aspectFOV.z()); }
	inline Eigen::Vector2f GetShift(Eigen::Vector2f shift) const { return lensShift; }

	inline void SetImageNum(int num) { imageNum = num; }
	inline int GetImageNum() const { return imageNum; }

	//---------------------------------------
	// Methods
	//---------------------------------------

	inline void SetupRendering(
		ReferencePath outputFile,
		Eigen::Vector2i renderResolution,
		bool rendersData,
		int sampleCount,
		int maxRayBounces,
		const std::string& shading
	)
	{
		resultFile = outputFile;
		resolution = renderResolution;
		dataOnly = rendersData;
		aaSamples = sampleCount;
		rayBounces = maxRayBounces;
		shadingOverride = shading;
	}

	inline void LoadIntrinsics(const Settings& settings)
	{
		// Load intrinsics (custom or provided ones)
		if (SafeGet<bool>(settings.GetJSONConfig(), "custom_intrinsics"))
		{
			// Store provided ones
			SetIntrinsics(settings.GetEngineSettings().CustomIntrinsics);
		}
		else
		{
			Intrinsics fromFile;
			// Load from file
			fromFile.LoadIntrinsics(settings.GetSceneRGBPath().append("_info.txt"));
			// Store in camera
			SetIntrinsics(fromFile);
		}
	}

	inline void LoadExtrinsics(ReferencePath extrFile)
	{
		Eigen::Matrix4f matTrans;

		// File must exist
		boost::filesystem::ifstream extrFileStream;
		if (!extrFileStream.good())
		{
			return;
		}
		else
		{
			extrFileStream.open(extrFile);
			sourceFile = ModifiablePath(extrFile);
		}

		// Load camera matrix
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				extrFileStream >> matTrans(i, j);
		extrFileStream.close();

		// Transform from scene -> Blender space
		SetTransform(matTrans);
		SetScale(Eigen::Vector3f(1.0f, -1.0f, -1.0f));

		// Calculate fov
		float fovx = 2.0f * atan(cameraIntrinsics.GetResolution().x() / (2.0f * cameraIntrinsics.GetFocalLenght().x()));
		float fovy = 2.0f * atan(cameraIntrinsics.GetResolution().y() / (2.0f * cameraIntrinsics.GetFocalLenght().y()));

		// Calculate aspect ratio
		float aspect = (cameraIntrinsics.GetFocalLenght().y() / cameraIntrinsics.GetFocalLenght().x()) *
			(static_cast<float>(cameraIntrinsics.GetResolution().x()) / static_cast<float>(cameraIntrinsics.GetResolution().y()));

		// Find max(width, height) (so shift is equal in x/y)
		float resMax = static_cast<float>(cameraIntrinsics.GetResolution().x() > cameraIntrinsics.GetResolution().y() ?
			cameraIntrinsics.GetResolution().x() : cameraIntrinsics.GetResolution().y());

		// Calculate lens shift (Blender specific)
		float shiftx = (cameraIntrinsics.GetPrincipalPoint().x() - (0.5f * cameraIntrinsics.GetResolution().x())) /
			cameraIntrinsics.GetResolution().x();
		shiftx *= -0.5f * (static_cast<float>(cameraIntrinsics.GetResolution().x()) / resMax);
		float shifty = (cameraIntrinsics.GetPrincipalPoint().y() - (0.5f * cameraIntrinsics.GetResolution().y())) /
			cameraIntrinsics.GetResolution().y();
		shifty *= -0.5f * (static_cast<float>(cameraIntrinsics.GetResolution().y()) / resMax);

		// Store in Camera
		aspectFOV = Eigen::Vector3f(aspect, fovx, fovy);
		lensShift = Eigen::Vector2f(shiftx, shifty);
	}

	//---------------------------------------
	// Constructors
	//---------------------------------------

	Camera() :
		cameraIntrinsics(),
		sourceFile("None"),
		aspectFOV(Eigen::Vector3f(1.5f, 0.6911f, 0.4711f)),
		lensShift(Eigen::Vector2f(0.0f, 0.0f)),
		resultFile(""),
		resolution(Eigen::Vector2i(0, 0)),
		dataOnly(false),
		rayBounces(-1),
		aaSamples(16),
		shadingOverride(""),
		imageNum(0)
	{
	}

	Camera(const Camera& copy) :
		RenderfileObject(copy),
		cameraIntrinsics(copy.cameraIntrinsics),
		sourceFile(copy.sourceFile),
		aspectFOV(copy.aspectFOV),
		lensShift(copy.lensShift),
		resultFile(copy.resultFile),
		resolution(copy.resolution),
		dataOnly(copy.dataOnly),
		rayBounces(copy.rayBounces),
		aaSamples(copy.aaSamples),
		shadingOverride(copy.shadingOverride),
		imageNum(copy.imageNum)
	{
	}

	Camera(Camera&& other) :
		RenderfileObject(std::move(other))
	{
		cameraIntrinsics = std::exchange(other.cameraIntrinsics, Intrinsics());
		sourceFile = std::exchange(other.sourceFile, "");
		aspectFOV = std::exchange(other.aspectFOV, Eigen::Vector3f());
		lensShift = std::exchange(other.lensShift, Eigen::Vector2f());
		resultFile = std::exchange(other.resultFile, ModifiablePath());
		resolution = std::exchange(other.resolution, Eigen::Vector2i());
		dataOnly = std::exchange(other.dataOnly, false);
		rayBounces = std::exchange(other.rayBounces, 0);
		aaSamples = std::exchange(other.aaSamples, 0);
		shadingOverride = std::exchange(other.shadingOverride, "");
		imageNum = std::exchange(other.imageNum, 0);
	}

	~Camera()
	{
	}

};
