#pragma once

#include <string>

#pragma warning(push, 0)
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

	virtual void X_AddToJSON(JSONWriterRef writer) const = 0;

public:
	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual LightParamsBase* MakeCopy() const = 0;

	virtual void AddToJSON(JSONWriterRef writer) const override
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

	virtual ~LightParamsBase()
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

	virtual void X_AddToJSON(JSONWriterRef writer) const override
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

	Light(
		LightParamsBase* params,
		const rapidjson::Value& origin
	) :
		params(params),
		color(Eigen::Vector3f().setOnes()),
		intensity(1.0f),
		exposure(0.0f),
		castsIndirect(true)
	{
		const rapidjson::Value* posVal;
		const rapidjson::Value* colVal;
		const rapidjson::Value* expVal;
		const rapidjson::Value* intVal;
		const rapidjson::Value* indVal;

		// Fetch & store values from document if they exists

		if (SafeHasMember(origin, "position", posVal))
		{
			SetPosition(SafeGetEigenVector<Eigen::Vector3f>(*posVal));
		}

		if (SafeHasMember(origin, "color", colVal))
		{
			color = SafeGetEigenVector<Eigen::Vector3f>(*colVal);
		}

		if (SafeHasMember(origin, "exposure", expVal))
		{
			exposure = SafeGetValue<float>(*expVal);
		}

		if (SafeHasMember(origin, "intensity", intVal))
		{
			intensity = SafeGetValue<float>(*intVal);
		}

		if (SafeHasMember(origin, "castsIndirect", indVal))
		{
			castsIndirect = SafeGetValue<bool>(*indVal);
		}
	}

	Light(const Light& copy) :
		RenderfileObject(copy),
		color(copy.color),
		intensity(copy.intensity),
		exposure(copy.exposure),
		castsIndirect(copy.castsIndirect),
		params(NULL)
	{
		if (copy.params)
		{
			params = copy.params->MakeCopy();
		}
	}

	Light(Light&& other) :
		RenderfileObject(std::move(other))
	{
		color = std::exchange(other.color, Eigen::Vector3f().setZero());
		intensity = std::exchange(other.intensity, 0.0f);
		exposure = std::exchange(other.exposure, 0.0f);
		castsIndirect = std::exchange(other.castsIndirect, false);
		params = std::exchange(other.params, nullptr);
	}

	~Light()
	{
		// Light cleans up internals
		delete params;
		params = NULL;
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

	virtual void X_AddToJSON(JSONWriterRef writer) const
	{
	}

public:
	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual LightParamsBase* MakeCopy() const override
	{
		return new PointLightParams(*this);
	}

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

	virtual void X_AddToJSON(JSONWriterRef writer) const
	{
		writer.Key("spotAngle");
		AddFloat(writer, spotAngle);
	}

public:
	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual LightParamsBase* MakeCopy() const override
	{
		return new SpotLightParams(*this);
	}

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

	virtual void X_AddToJSON(JSONWriterRef writer) const
	{
	}

public:
	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual LightParamsBase* MakeCopy() const override
	{
		return new SunLightParams(*this);
	}

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

	virtual void X_AddToJSON(JSONWriterRef writer) const
	{
		writer.Key("shape");
		AddString(writer, shape);

		writer.Key("size");
		AddEigenVector<Eigen::Vector2f>(writer, size);
	}

public:
	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual LightParamsBase* MakeCopy() const override
	{
		return new AreaLightParams(*this);
	}

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
