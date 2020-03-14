#pragma once

#include "MeshBase.h"

#pragma warning(push, 0)
#include <PxPhysicsAPI.h>
#pragma warning(pop)

#define PX_RELEASE(x) if(x) { x->release(); x = NULL; }

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
	inline PxVec3* GetVertices() { return (PxVec3*) &vecVertices[0]; };
	inline PxU32* GetIndices() { return (PxU32*) &vecIndices[0]; };

public:
	//---------------------------------------
	// Fields
	//---------------------------------------
	PxShape* pShape;

	//---------------------------------------
	// Methods
	//---------------------------------------
	virtual bool CreateMesh(bool saveBounds, bool doubleNorms) = 0;
	virtual PxRigidActor* CreateRigidbody(const vector<float>& pos, const vector<float>& quat) const = 0;
	PxRigidActor* InitRigidbody(const vector<float>& pos, const vector<float>& quat, bool isStatic) const;
	static void DestroyRigidbody(PxRigidActor* body);

	//---------------------------------------
	// Constructors
	//---------------------------------------
	PxMesh(string path, int meshId, float scale, PxScene* scene, PxCooking* cooking, PxMaterial* material);
	~PxMesh();
};
