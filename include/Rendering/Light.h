#pragma once

#include <string>

#pragma warning(push, 0)
#include <Eigen/Dense>

#include <Helpers/PathUtils.h>
#include <Helpers/JSONUtils.h>

#include <Renderfile.h>
#pragma warning(pop)

//---------------------------------------
// Generic light params wrapper
//---------------------------------------
class LightParamsBase : public RenderfileData
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------

	std::string type;

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(JSONWriterRef writer) = 0;

public:
	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void AddToJSON(JSONWriterRef writer) override
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

	LightParamsBase(
		const std::string& type
	) :
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

	Eigen::Vector3f color;
	float intensity;
	float exposure;
	bool castsIndirect;
	LightParamsBase* params;

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(JSONWriterRef writer) override
	{
		writer.Key("color");
		AddEigenVector<Eigen::Vector3f>(writer, color);

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

	Light(
		LightParamsBase* params,
		Eigen::Vector3f color,
		float intensity,
		float exposure,
		bool castsIndirect = true
	) :
		params(params),
		color(color),
		intensity(intensity),
		exposure(exposure),
		castsIndirect(castsIndirect)
	{
	}

	~Light()
	{
		// Light cleans up internals
		delete params;
	}
};

//---------------------------------------
// Point light params wrapper
//---------------------------------------
class PointLightParams : public LightParamsBase
{
protected:
	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(JSONWriterRef writer)
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
class SpotLightParams : public LightParamsBase
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------

	float spotAngle;

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(JSONWriterRef writer)
	{
		writer.Key("spotAngle");
		AddFloat(writer, spotAngle);
	}

public:
	//---------------------------------------
	// Constructors
	//---------------------------------------

	SpotLightParams(
		float spotAngle
	) :
		LightParamsBase("SPOT"),
		spotAngle(spotAngle)
	{
	}
};

//---------------------------------------
// Sun light params wrapper
//---------------------------------------
class SunLightParams : public LightParamsBase
{
protected:
	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(JSONWriterRef writer)
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
class AreaLightParams : public LightParamsBase
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------

	std::string shape;
	Eigen::Vector2f size;

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(JSONWriterRef writer)
	{
		writer.Key("shape");
		AddString(writer, shape);

		writer.Key("size");
		AddEigenVector<Eigen::Vector2f>(writer, size);
	}

public:
	//---------------------------------------
	// Constructors
	//---------------------------------------

	AreaLightParams(
		const std::string& shape,
		Eigen::Vector2f size
	) :
		LightParamsBase("AREA"),
		shape(shape),
		size(size)
	{
	}
};
