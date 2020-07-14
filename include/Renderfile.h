#pragma once

#include <string>
#include <vector>

#pragma warning(push, 0)
#include <Eigen/Dense>

#include <rapidjson/prettywriter.h>

#include <Transformable.h>
#pragma warning(pop)

using namespace rapidjson;

//---------------------------------------
// Necessary for render related data
//---------------------------------------
class RenderfileData
{
public:
	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void AddToJSON(PrettyWriter<std::stringstream>& writer) = 0;

	//---------------------------------------
	// Static Methods
	//---------------------------------------

	inline static void AddString(PrettyWriter<std::stringstream>& writer, const std::string& toAdd)
	{
		writer.String(toAdd.c_str(), static_cast<SizeType>(toAdd.length()));
	}

	inline static void AddFloat(PrettyWriter<std::stringstream>& writer, float toAdd)
	{
		writer.Double(static_cast<double>(toAdd));
	}

	template<typename VectorType>
	inline static void AddEigenVector(PrettyWriter<std::stringstream>& writer, VectorType toAdd)
	{
		writer.StartArray();
		for (int i = 0; i < toAdd.rows(); i++)
		{
			writer.Double(static_cast<double>(toAdd[i]));
		}
		writer.EndArray();
	}
};

//---------------------------------------
// Necessary for renderable objects
//---------------------------------------
class RenderfileObject : RenderfileData, Transformable<Eigen::Matrix4f>
{
protected:
	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(PrettyWriter<std::stringstream>& writer) = 0;

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	virtual const Eigen::Matrix4f GetTransform() override { return meshTrans; }
	virtual void SetTransform(Eigen::Matrix4f trans) override { meshTrans = trans; }

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void AddToJSON(PrettyWriter<std::stringstream>& writer) override
	{
		writer.StartObject();

		// Decompose transform
		Eigen::Affine3f trans = Eigen::Affine3f(meshTrans);
		Affine3f::LinearMatrixType rotation, scaling;
		trans.computeRotationScaling(&rotation, &scaling);
		Quaternionf rotQuat = Quaternionf(rotation);

		// Write to json
		writer.Key("position");
		RenderfileData::AddEigenVector<Vector3f>(writer, trans.translation());
		writer.Key("rotation");
		RenderfileData::AddEigenVector<Vector4f>(writer, rotQuat.coeffs());
		writer.Key("scale");
		RenderfileData::AddEigenVector<Vector3f>(writer, scaling.diagonal());

		// Lastly add rest of the object
		X_AddToJSON(writer);

		writer.EndObject();
	}
};
