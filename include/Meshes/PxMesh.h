#pragma once

#pragma warning(push, 0)
#include <PxPhysicsAPI.h>

#include <Meshes/MeshBase.h>
#include <Transformable.h>
#pragma warning(pop)

//#define PX_EXTRACT_INTERNAL

#define PX_RELEASE(x) if(x != NULL) { x->release(); x = NULL; }

using namespace physx;

//---------------------------------------
// Base class for physx meshes
//---------------------------------------
class PxMesh : public MeshBase, Transformable<PxTransform, PxVec3, PxQuat>
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------

	PxShape* pPxShape;
	PxRigidActor* pPxActor;
	const PxCooking* pPxCooking;
	const PxMaterial* pPxMaterial;

	bool firstInstance = false;
	PxVec3 maximum = PxVec3(1e8);
	PxVec3 minimum = PxVec3(-1e8);

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

	inline PxVec3* X_GetVertices() { return (PxVec3*) &vecVertices[0]; };
	inline PxU32* X_GetIndices() { return (PxU32*) &vecIndices[0]; };

public:
	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void CreateMesh() override;
	void AddRigidActor(PxScene* scene);
	void RemoveRigidActor(PxScene* scene);

	//---------------------------------------
	// Properties
	//---------------------------------------

	inline const PxVec3 GetMinimum() { return minimum.multiply(meshScale); };
	inline const PxVec3 GetMaximum() { return maximum.multiply(meshScale); };
	virtual const PxTransform GetTransform() override;
	virtual void SetTransform(PxTransform trans) override;
	virtual const PxVec3 GetPosition() override;
	virtual void SetPosition(PxVec3 pos) override;
	virtual const PxQuat GetRotation() override;
	virtual void SetRotation(PxQuat rot) override;
	virtual const PxVec3 GetScale() override;
	virtual void SetScale(PxVec3 scale) override;

	//---------------------------------------
	// Constructors
	//---------------------------------------

	PxMesh(const string& meshPath, int meshId, const PxCooking* cooking, const PxMaterial* material);
	PxMesh(const PxMesh& copy);
	~PxMesh();

};
