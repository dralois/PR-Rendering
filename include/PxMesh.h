#pragma once

#include "MeshBase.h"

#pragma warning(push, 0)
#include <PxPhysicsAPI.h>
#pragma warning(pop)

#define PX_RELEASE(x) if(x != NULL) { x->release(); x = NULL; }

#define PX_EXPORT_TO_OBJ

using namespace physx;

//---------------------------------------
// Base class for PhysX meshes
//---------------------------------------
class PxMesh : public MeshBase
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------
	PxScene* pPxScene;
	PxCooking* pPxCooking;
	PxMaterial* pPxMaterial;

	//---------------------------------------
	// Properties
	//---------------------------------------
	inline PxVec3* X_GetVertices() { return (PxVec3*) &vecVertices[0]; };
	inline PxU32* X_GetIndices() { return (PxU32*) &vecIndices[0]; };

public:
	//---------------------------------------
	// Fields
	//---------------------------------------
	PxShape* pPxShape;

	//---------------------------------------
	// Methods
	//---------------------------------------
	virtual bool CreateMesh(bool saveBounds, float scale) = 0;
	virtual PxRigidActor* AddRigidActor(const PxVec3& pos, const PxQuat& rot) const = 0;
	PxRigidActor* CreateRigidActor(const PxVec3& pos, const PxQuat& rot, bool isStatic) const;
	static void DestroyRigidbody(PxRigidActor* body);

	//---------------------------------------
	// Constructors
	//---------------------------------------
	PxMesh(const string& meshPath, int meshId, int objId, PxScene* scene, PxCooking* cooking, PxMaterial* material);
	PxMesh(const PxMesh& copy);
	~PxMesh();
};
