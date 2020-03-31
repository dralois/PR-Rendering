#include "PxMesh.h"

//---------------------------------------
// Create rigidbody
//---------------------------------------
PxRigidActor* PxMesh::CreateRigidActor(const PxVec3& pos, const PxQuat& rot, bool isStatic) const
{
	// Create transform with provided position and rotation
	PxTransform currT(pos, rot);
	// Create and return rigidbody
	if (isStatic)
	{
		PxRigidStatic* body = PxGetPhysics().createRigidStatic(currT);
		return body;
	}
	else
	{
		PxRigidDynamic* body = PxGetPhysics().createRigidDynamic(currT);
		return body;
	}
}

//---------------------------------------
// Cleanup rigidbody
//---------------------------------------
void PxMesh::DestroyRigidbody(PxRigidActor* curr)
{
	PX_RELEASE(curr);
}

//---------------------------------------
// New physx mesh manager
//---------------------------------------
PxMesh::PxMesh(const string& meshPath, int meshId, int objId,
	PxScene* scene, PxCooking* cooking, PxMaterial* material) :
	MeshBase(meshPath, meshId, objId),
	pPxScene(scene),
	pPxCooking(cooking),
	pPxMaterial(material),
	pPxShape(NULL)
{
}

//---------------------------------------
// Copy constructor, increases reference count
//---------------------------------------
PxMesh::PxMesh(const PxMesh& copy):
	pPxCooking(copy.pPxCooking),
	pPxMaterial(copy.pPxMaterial),
	pPxScene(copy.pPxScene),
	pPxShape(copy.pPxShape),
	MeshBase(copy)
{
	pPxShape->acquireReference();
}

//---------------------------------------
// Cleanup mesh manager
//---------------------------------------
PxMesh::~PxMesh()
{
	PX_RELEASE(pPxShape);
}
