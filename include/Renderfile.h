#pragma once

#include <string>

#pragma warning(push, 0)
#include <Eigen/Dense>

#include <rapidjson/prettywriter.h>
#include <rapidjson/stream.h>

#include <Helpers/JSONUtils.h>

#include <Transformable.h>
#pragma warning(pop)

//---------------------------------------
// Necessary for render related data
//---------------------------------------
class RenderfileData
{
public:
	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void AddToJSON(JSONWriterRef writer) = 0;
};

//---------------------------------------
// Necessary for renderable objects
//---------------------------------------
class RenderfileObject : public RenderfileData, public Transformable<Eigen::Matrix4f, Eigen::Vector3f, Eigen::Quaternionf>
{
private:
	//---------------------------------------
	// Methods
	//---------------------------------------

	inline void X_SetPosRotScale(
		const Eigen::Vector3f* pos,
		const Eigen::Quaternionf* rot,
		const Eigen::Vector3f* scale
	)
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
			scale ? *scale : static_cast<Eigen::Vector3f>(scaling.diagonal())
		);
		// Update transform
		SetTransform(trans.matrix());
		// Update actual values
		X_GetPosRotScale(meshPos, meshRot, meshScale);
	}

	inline void X_GetPosRotScale(
		Eigen::Vector3f& pos,
		Eigen::Quaternionf& rot,
		Eigen::Vector3f& scl
	) const
	{
		Eigen::Affine3f trans(GetTransform());
		// Compute current values
		Eigen::Affine3f::LinearMatrixType rotation, scale;
		trans.computeRotationScaling(&rotation, &scale);
		// Update internals
		pos = trans.translation();
		rot = Eigen::Quaternionf(rotation);
		scl = scale.diagonal();
	}

protected:
	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(JSONWriterRef writer) = 0;

	RenderfileObject():
		Transformable(
			Eigen::Matrix4f(),
			Eigen::Vector3f(),
			Eigen::Quaternionf(),
			Eigen::Vector3f()
		)
	{
		meshTrans.setIdentity();
		meshPos.setZero();
		meshRot.setIdentity();
		meshScale.setOnes();
	}

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	inline const Eigen::Matrix4f GetTransform() const { return meshTrans; }
	virtual const Eigen::Matrix4f GetTransform() override { return meshTrans; }
	virtual void SetTransform(Eigen::Matrix4f trans) override { meshTrans = trans; }

	inline const Eigen::Vector3f RenderfileObject::GetPosition() const
	{
		Eigen::Vector3f pos, scl;
		Eigen::Quaternionf rot;
		X_GetPosRotScale(pos, rot, scl);
		return pos;
	}
	virtual const Eigen::Vector3f RenderfileObject::GetPosition() override
	{
		X_GetPosRotScale(meshPos, meshRot, meshScale);
		return meshPos;
	}
	virtual void RenderfileObject::SetPosition(Eigen::Vector3f pos) override
	{
		X_SetPosRotScale(&pos, NULL, NULL);
	}

	inline const Eigen::Quaternionf RenderfileObject::GetRotation() const
	{
		Eigen::Vector3f pos, scl;
		Eigen::Quaternionf rot;
		X_GetPosRotScale(pos, rot, scl);
		return rot;
	}
	virtual const Eigen::Quaternionf RenderfileObject::GetRotation() override
	{
		X_GetPosRotScale(meshPos, meshRot, meshScale);
		return meshRot;
	}
	virtual void RenderfileObject::SetRotation(Eigen::Quaternionf rot) override
	{
		X_SetPosRotScale(NULL, &rot, NULL);
	}

	inline const Eigen::Vector3f RenderfileObject::GetScale() const
	{
		Eigen::Vector3f pos, scl;
		Eigen::Quaternionf rot;
		X_GetPosRotScale(pos, rot, scl);
		return scl;
	}
	virtual const Eigen::Vector3f RenderfileObject::GetScale() override
	{
		X_GetPosRotScale(meshPos, meshRot, meshScale);
		return meshScale;
	}
	virtual void RenderfileObject::SetScale(Eigen::Vector3f scale) override
	{
		X_SetPosRotScale(NULL, NULL, &scale);
	}

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void AddToJSON(JSONWriterRef writer) override
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
