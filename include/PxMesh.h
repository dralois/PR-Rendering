#pragma once

#include "MeshBase.h"

#include <PxPhysicsAPI.h>

using namespace physx;

// Base class for PhysX meshes
class PxMesh : public MeshBase
{
private:
	// Fields
	PxVec3* pVerts;
	PxU32* pIndices;

protected:
	// Fields
	PxPhysics* gPhysics;
	PxScene* gScene;
	PxCooking* gCooking;
	PxMaterial* gMaterial;

	// Methods
	void ConvertBuffers();

	// Properties
	PxVec3* GetVertices();
	PxU32* GetIndices();

public:
	// Fields
	PxShape* pShape;

	// Methods
	virtual bool CreateMesh() = 0;
	virtual PxRigidDynamic* CreateRigidbody(const vector<float>& pos, const vector<float>& quat) = 0;
	PxRigidDynamic* InitRigidbody(const vector<float>& pos, const vector<float>& quat) const;
	static void DestroyRigidbody(PxRigidDynamic* body);

	// Constructors
	PxMesh(string path, int mesh_id, float scale, PxPhysics* physics, PxScene* scene, PxCooking* cooking, PxMaterial* material);
	~PxMesh();
};
