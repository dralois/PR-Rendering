#pragma once

#pragma warning(push, 0)
#include <Helpers/PhysxManager.h>

#include <Meshes/MeshBase.h>
#include <Transformable.h>
#pragma warning(pop)

#define PX_EXTRACT_INTERNAL 0

//---------------------------------------
// Base class for physx meshes
//---------------------------------------
class PxMesh : public MeshBase, public Transformable<physx::PxTransform, physx::PxVec3, physx::PxQuat>
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------

	physx::PxShape* pPxShape;
	physx::PxRigidActor* pPxActor;

	bool firstInstance = false;
	physx::PxVec3 maximum = physx::PxVec3(1e8);
	physx::PxVec3 minimum = physx::PxVec3(-1e8);

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual bool X_IsStatic() = 0;
	virtual void X_CookMesh() = 0;
	virtual void X_ExportMesh() = 0;
	virtual void X_CreateMesh() = 0;
	virtual void X_CreateShape() = 0;

	//---------------------------------------
	// Properties
	//---------------------------------------

	inline physx::PxVec3* X_GetVertices() { return (physx::PxVec3*) &vecVertices[0]; };
	inline physx::PxU32* X_GetIndices() { return (physx::PxU32*) &vecIndices[0]; };

public:
	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void CreateMesh() override;
	void AddRigidActor(physx::PxScene* scene);
	void RemoveRigidActor(physx::PxScene* scene);

	//---------------------------------------
	// Properties
	//---------------------------------------

	inline const physx::PxVec3 GetMinimum() { return minimum.multiply(meshScale); };
	inline const physx::PxVec3 GetMaximum() { return maximum.multiply(meshScale); };
	virtual const physx::PxTransform GetTransform() override;
	virtual void SetTransform(physx::PxTransform trans) override;
	virtual const physx::PxVec3 GetPosition() override;
	virtual void SetPosition(physx::PxVec3 pos) override;
	virtual const physx::PxQuat GetRotation() override;
	virtual void SetRotation(physx::PxQuat rot) override;
	virtual const physx::PxVec3 GetScale() override;
	virtual void SetScale(physx::PxVec3 scale) override;

	//---------------------------------------
	// Constructors
	//---------------------------------------

	PxMesh(ReferencePath meshPath, int meshId);
	PxMesh(const PxMesh& copy);
	~PxMesh();
};
