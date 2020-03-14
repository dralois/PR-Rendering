#include "PxMesh.h"

//---------------------------------------
// Create rigidbody
//---------------------------------------
PxRigidActor* PxMesh::InitRigidbody(const vector<float>& pos, const vector<float>& rot, bool isStatic) const
{
	// Create transform with provided position and rotation
	PxQuat pxRot(rot.at(0), rot.at(1), rot.at(2), rot.at(3));
	PxVec3 pxPos(pos.at(0), pos.at(1), pos.at(2));
	PxTransform currT(pxPos, pxRot);
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
PxMesh::PxMesh(string path,
							int meshId,
							float scale,
							PxScene* scene,
							PxCooking* cooking,
							PxMaterial* material) :
	MeshBase(path, meshId, scale),
	pPxScene(scene),
	pPxCooking(cooking),
	pPxMaterial(material),
	pShape(NULL)
{
};

//---------------------------------------
// Cleanup mesh manager
//---------------------------------------
PxMesh::~PxMesh()
{
	PX_RELEASE(pShape);
}
