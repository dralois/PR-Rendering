#pragma once

#pragma warning(push, 0)
#include <Shaders/Shading.h>
#include <Renderfile.h>
#pragma warning(pop)

using namespace std;
using namespace Eigen;

//---------------------------------------
// Camera object wrapper for rendering
//---------------------------------------
class Camera : public RenderfileObject
{
private:
	//---------------------------------------
	// Fields
	//---------------------------------------

	Vector2f fieldOfView;
	Vector2f lensShift;
	Vector2f clipPlanes;
	string resultName;
	bool depthOnly;
	OSLShader* cameraEffect;

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(PrettyWriter<std::stringstream>& writer) override
	{
		writer.Key("fov");
		RenderfileData::AddEigenVector<Vector2f>(writer, fieldOfView);

		writer.Key("shift");
		RenderfileData::AddEigenVector<Vector2f>(writer, lensShift);

		writer.Key("nearZ");
		RenderfileData::AddFloat(writer, clipPlanes.x());

		writer.Key("result");
		RenderfileData::AddString(writer, resultName);

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

	inline void SetFOV(Vector2f fov) { fieldOfView = fov; }
	inline Vector2f GetFOV() { return fieldOfView; }
	inline void SetShift(Vector2f shift) { lensShift = shift; }
	inline Vector2f GetShift(Vector2f shift) { return shift; }
	inline void SetClipping(float near, float far) { clipPlanes = Vector2f(near, far); }
	inline Vector2f GetClipping() { return clipPlanes; }
	inline void SetResultFile(const string& result) { resultName = result; }
	inline const string& GetResultFile() { return resultName; }
	inline void SetDepthOnly(bool renderDepth) { depthOnly = renderDepth; }
	inline bool GetDepthOnly() { return depthOnly; }
	inline void SetEffect(OSLShader* effect) { cameraEffect = effect; }
	inline const OSLShader* GetEffect() { return cameraEffect; }

	//---------------------------------------
	// Constructors
	//---------------------------------------

	Camera() :
		fieldOfView(Vector2f(0.6911f, 0.4711f)),
		lensShift(Vector2f(0.0f, 0.0f)),
		clipPlanes(Vector2f(0.1f, 10.0f)),
		resultName(""),
		depthOnly(false),
		cameraEffect(NULL)
	{
	}
};
