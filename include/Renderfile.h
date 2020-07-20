#pragma once

#include <string>
#include <vector>

#pragma warning(push, 0)
#include <Eigen/Dense>

#include <rapidjson/prettywriter.h>
#include <rapidjson/stream.h>

#include <Transformable.h>
#pragma warning(pop)

//---------------------------------------
// Static Methods
//---------------------------------------

static void AddString(rapidjson::PrettyWriter<rapidjson::StringStream>& writer, const std::string& toAdd)
{
	writer.String(toAdd.c_str(), static_cast<rapidjson::SizeType>(toAdd.length()));
}

static void AddFloat(rapidjson::PrettyWriter<rapidjson::StringStream>& writer, float toAdd)
{
	writer.Double(static_cast<double>(toAdd));
}

template<typename VectorType>
static void AddEigenVector(rapidjson::PrettyWriter<rapidjson::StringStream>& writer, VectorType toAdd)
{
	writer.StartArray();
	for (int i = 0; i < toAdd.rows(); i++)
	{
		writer.Double(static_cast<double>(toAdd[i]));
	}
	writer.EndArray();
}

//---------------------------------------
// Necessary for render related data
//---------------------------------------
class RenderfileData
{
public:
	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void AddToJSON(rapidjson::PrettyWriter<rapidjson::StringStream>& writer) = 0;
};

//---------------------------------------
// Necessary for renderable objects
//---------------------------------------
class RenderfileObject : RenderfileData, Transformable<Eigen::Matrix4f, Eigen::Vector3f, Eigen::Quaternionf>
{
private:
	//---------------------------------------
	// Methods
	//---------------------------------------

	void X_SetPosRotScale(Eigen::Vector3f* pos, Eigen::Quaternionf* rot, Eigen::Vector3f* scale)
	{
		Eigen::Affine3f trans(GetTransform());
		// Compute current values
		Eigen::Affine3f::VectorType position = trans.translation();
		Eigen::Affine3f::LinearMatrixType rotation, scaling;
		trans.computeRotationScaling(&rotation, &scaling);
		// Update with new / current values
		trans.fromPositionOrientationScale(
			pos ? *pos : position,
			rot ? *rot : Eigen::Quaternionf(rotation),
			scale ? *scale : scaling.diagonal()
		);
		// Update transform
		SetTransform(trans.matrix());
	}

	void X_GetPosRotScale()
	{
		Eigen::Affine3f trans(GetTransform());
		// Compute current values
		Eigen::Affine3f::LinearMatrixType rotation, scale;
		trans.computeRotationScaling(&rotation, &scale);
		// Store internally
		meshPos = trans.translation();
		meshRot = Eigen::Quaternionf(rotation);
		meshScale = scale.diagonal();
	}

protected:
	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(rapidjson::PrettyWriter<rapidjson::StringStream>& writer) = 0;

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	const Eigen::Matrix4f GetTransform() const { return meshTrans; }
	virtual const Eigen::Matrix4f GetTransform() override { return meshTrans; }
	virtual void SetTransform(Eigen::Matrix4f trans) override { meshTrans = trans; }

	virtual const Eigen::Vector3f RenderfileObject::GetPosition() override
	{
		X_GetPosRotScale();
		return meshPos;
	}
	virtual void RenderfileObject::SetPosition(Eigen::Vector3f pos) override
	{
		X_SetPosRotScale(&pos, NULL, NULL);
	}

	virtual const Eigen::Quaternionf RenderfileObject::GetRotation() override
	{
		X_GetPosRotScale();
		return meshRot;
	}
	virtual void RenderfileObject::SetRotation(Eigen::Quaternionf rot) override
	{
		X_SetPosRotScale(NULL, &rot, NULL);
	}

	virtual const Eigen::Vector3f RenderfileObject::GetScale() override
	{
		X_GetPosRotScale();
		return meshScale;
	}
	virtual void RenderfileObject::SetScale(Eigen::Vector3f scale) override
	{
		X_SetPosRotScale(NULL, NULL, &scale);
	}

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void AddToJSON(rapidjson::PrettyWriter<rapidjson::StringStream>& writer) override
	{
		writer.StartObject();

		// Add transform
		writer.Key("position");
		AddEigenVector<Eigen::Vector3f>(writer, GetPosition());
		writer.Key("rotation");
		AddEigenVector<Eigen::Vector4f>(writer, GetRotation().coeffs());
		writer.Key("scale");
		AddEigenVector<Eigen::Vector3f>(writer, GetScale());

		// Add rest of the object
		X_AddToJSON(writer);

		writer.EndObject();
	}
};
