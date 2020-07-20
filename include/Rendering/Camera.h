#pragma once

#pragma warning(push, 0)
#include <Shaders/Shading.h>
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

	inline void SetFOV(Eigen::Vector2f fov) { fieldOfView = fov; }
	inline Eigen::Vector2f GetFOV() { return fieldOfView; }
	inline void SetShift(Eigen::Vector2f shift) { lensShift = shift; }
	inline Eigen::Vector2f GetShift(Eigen::Vector2f shift) { return shift; }
	inline void SetClipping(float near, float far) { clipPlanes = Eigen::Vector2f(near, far); }
	inline Eigen::Vector2f GetClipping() { return clipPlanes; }
	inline void SetResultFile(const std::string& result) { resultName = result; }
	inline const boost::filesystem::path& GetResultFile() { return resultName; }
	inline void SetDepthOnly(bool renderDepth) { depthOnly = renderDepth; }
	inline bool GetDepthOnly() { return depthOnly; }
	inline void SetEffect(OSLShader* effect) { cameraEffect = effect; }
	inline const OSLShader* GetEffect() { return cameraEffect; }

	//---------------------------------------
	// Constructors
	//---------------------------------------

	Camera() :
		fieldOfView(Eigen::Vector2f(0.6911f, 0.4711f)),
		lensShift(Eigen::Vector2f(0.0f, 0.0f)),
		clipPlanes(Eigen::Vector2f(0.1f, 10.0f)),
		resultName(""),
		depthOnly(false),
		cameraEffect(NULL)
	{
	}
};
