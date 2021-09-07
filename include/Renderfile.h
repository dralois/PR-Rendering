#pragma once

#include <string>

#pragma warning(push, 0)
#include <Eigen/Dense>

#include <Helpers/JSONUtils.h>

#include <Transformable.h>
#pragma warning(pop)

#define RENDER_TIMEOUT 30.0f

#define RENDERFILE_SINGLE(renderer, thread, builder, meshes, scene, cams, lights, out) \
{\
	rapidjson::StringBuffer renderstring;\
	JSONWriter writer(renderstring);\
	builder(writer, meshes, scene, cams, lights, out);\
	std::string renderfile(renderstring.GetString());\
	renderer->ProcessRenderfile(renderfile, cams.size() * RENDER_TIMEOUT, thread);\
}

#define RENDERFILE_DEPTH(renderer, thread, builder, meshes, scene, cams, lights, out, maxDist) \
{\
	rapidjson::StringBuffer renderstring;\
	JSONWriter writer(renderstring);\
	builder(writer, meshes, scene, cams, lights, out, maxDist);\
	std::string renderfile(renderstring.GetString());\
	renderer->ProcessRenderfile(renderfile, cams.size() * RENDER_TIMEOUT, thread);\
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
		const Eigen::Vector3f* scl
	)
	{
		Eigen::Matrix4f mat;

		// Update internals
		meshPos = pos ? *pos : meshPos;
		meshRot = rot ? *rot : meshRot;
		meshScl = scl ? *scl : meshScl;

		// Convert to matrices
		Eigen::Matrix3f rotMat = meshRot.normalized().toRotationMatrix();
		Eigen::Matrix3f sclMat = meshScl.asDiagonal();

		// Set position
		mat.block<3, 1>(0, 3) = meshPos;
		// Set rotation & scale
		mat.block<3, 3>(0, 0) = rotMat * sclMat;
		// Make affine
		mat.block<1, 3>(3, 0).setZero();
		mat.coeffRef(3, 3) = 1.0f;

		// Update transform
		meshTrans = mat;
	}

protected:
	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(JSONWriterRef writer) = 0;

	RenderfileObject() :
		Transformable(
			Eigen::Matrix4f().setIdentity(),
			Eigen::Vector3f().setZero(),
			Eigen::Quaternionf().Identity(),
			Eigen::Vector3f().setOnes()
		)
	{
	}

	RenderfileObject(const RenderfileObject& copy):
		Transformable(copy)
	{
	}

	RenderfileObject(RenderfileObject&& other) :
		Transformable(std::move(other))
	{
	}

	virtual ~RenderfileObject()
	{
	}

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	// Transform
	inline const Eigen::Matrix4f GetTransform() const { return meshTrans; }
	virtual const Eigen::Matrix4f GetTransform() override { return meshTrans; }
	virtual void SetTransform(Eigen::Matrix4f trans) override
	{
		// Set transform
		meshTrans = trans;
		Eigen::Affine3f affine(trans);
		// Compute current values
		Eigen::Affine3f::LinearMatrixType rotation, scale;
		affine.computeRotationScaling(&rotation, &scale);
		// Update internals
		meshPos = affine.translation().eval();
		meshRot = Eigen::Quaternionf(rotation);
		meshScl = scale.diagonal();
	}

	// Position
	inline const Eigen::Vector3f GetPosition() const
	{
		return meshPos;
	}
	virtual const Eigen::Vector3f GetPosition() override
	{
		return meshPos;
	}
	virtual void SetPosition(Eigen::Vector3f pos) override
	{
		X_SetPosRotScale(&pos, NULL, NULL);
	}

	// Rotation
	inline const Eigen::Quaternionf GetRotation() const
	{
		return meshRot;
	}
	virtual const Eigen::Quaternionf GetRotation() override
	{
		return meshRot;
	}
	virtual void SetRotation(Eigen::Quaternionf rot) override
	{
		X_SetPosRotScale(NULL, &rot, NULL);
	}

	// Scale
	inline const Eigen::Vector3f GetScale() const
	{
		return meshScl;
	}
	virtual const Eigen::Vector3f GetScale() override
	{
		return meshScl;
	}
	virtual void SetScale(Eigen::Vector3f scale) override
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
		AddEigenVector<Eigen::Vector4f>(writer, (Eigen::Vector4f() << GetRotation().w(), GetRotation().vec()).finished());
		writer.Key("scale");
		AddEigenVector<Eigen::Vector3f>(writer, GetScale());

		// Add rest of the object
		X_AddToJSON(writer);

		writer.EndObject();
	}
};
