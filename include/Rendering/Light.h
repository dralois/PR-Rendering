#pragma once

#pragma warning(push, 0)
#include <Renderfile.h>
#pragma warning(pop)

using namespace std;
using namespace Eigen;

//---------------------------------------
// Generic light params wrapper
//---------------------------------------
class LightParamsBase : RenderfileData
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------

	string type;

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(PrettyWriter<std::stringstream>& writer) = 0;

public:
	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void AddToJSON(PrettyWriter<std::stringstream>& writer) override
	{
		writer.Key("type");
		AddString(writer, type);

		// Add specific params
		writer.Key("params");
		writer.StartObject();
		X_AddToJSON(writer);
		writer.EndObject();
	}

	//---------------------------------------
	// Constructors
	//---------------------------------------

	LightParamsBase(const string& type) :
		type(type)
	{
	}
};

//---------------------------------------
// Generic light object wrapper for rendering
//---------------------------------------
class Light : public RenderfileObject
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------

	Vector3f color;
	float intensity;
	float exposure;
	bool castsIndirect;
	LightParamsBase* params;

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(PrettyWriter<stringstream>& writer) override
	{
		writer.Key("color");
		AddEigenVector<Vector3f>(writer, color);

		writer.Key("intensity");
		AddFloat(writer, intensity);

		writer.Key("exposure");
		AddFloat(writer, exposure);

		writer.Key("castsIndirect");
		writer.Bool(castsIndirect);

		// Specific params & type
		params->AddToJSON(writer);
	}

public:
	//---------------------------------------
	// Constructors
	//---------------------------------------

	Light(LightParamsBase* params, Vector3f color, float intensity, float exposure, bool castsIndirect = true) :
		params(params),
		color(color),
		intensity(intensity),
		exposure(exposure),
		castsIndirect(castsIndirect)
	{
	}
};

//---------------------------------------
// Point light params wrapper
//---------------------------------------
class PointLightParams : LightParamsBase
{
protected:
	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(PrettyWriter<std::stringstream>& writer)
	{
	}

public:
	//---------------------------------------
	// Constructors
	//---------------------------------------

	PointLightParams() :
		LightParamsBase("POINT")
	{
	}
};

//---------------------------------------
// Point light params wrapper
//---------------------------------------
class SpotLightParams : LightParamsBase
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------

	float spotAngle;

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(PrettyWriter<std::stringstream>& writer)
	{
		writer.Key("spotAngle");
		AddFloat(writer, spotAngle);
	}

public:
	//---------------------------------------
	// Constructors
	//---------------------------------------

	SpotLightParams(float spotAngle) :
		LightParamsBase("SPOT"),
		spotAngle(spotAngle)
	{
	}
};

//---------------------------------------
// Sun light params wrapper
//---------------------------------------
class SunLightParams : LightParamsBase
{
protected:
	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(PrettyWriter<std::stringstream>& writer)
	{
	}

public:
	//---------------------------------------
	// Constructors
	//---------------------------------------

	SunLightParams() :
		LightParamsBase("SUN")
	{
	}
};

//---------------------------------------
// Area light params wrapper
//---------------------------------------
class AreaLightParams : LightParamsBase
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------

	string shape;
	Vector2f size;

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(PrettyWriter<std::stringstream>& writer)
	{
		writer.Key("shape");
		AddString(writer, shape);

		writer.Key("size");
		AddEigenVector<Vector2f>(writer, size);
	}

public:
	//---------------------------------------
	// Constructors
	//---------------------------------------

	AreaLightParams(const string& shape, Vector2f size) :
		LightParamsBase("AREA"),
		shape(shape),
		size(size)
	{
	}
};
