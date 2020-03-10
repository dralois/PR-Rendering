#include "PxMesh.h"

// Physx vertices
PxVec3* PxMesh::GetVertices()
{
	return pVerts;
}

// Physx indices
PxU32* PxMesh::GetIndices()
{
	return pIndices;
}

// Sets vertices and indices as physx types
void PxMesh::ConvertBuffers()
{
	pVerts = new PxVec3[vecVertices.size() / 3];
	pIndices = new PxU32[vecIndices.size()];

	for (int i = 0; i * 3 < vecVertices.size(); i++)
	{
		pVerts[i] = PxVec3(vecVertices.at(i * 3), vecVertices.at(i * 3 + 1), vecVertices.at(i * 3 + 2));
	}

	for (int i = 0; i < vecIndices.size(); i++)
	{
		pIndices[i] = vecIndices.at(i);
	}
}

// Create rigidbody
PxRigidDynamic* PxMesh::InitRigidbody(const vector<float>& pos, const vector<float>& quat) const
{
	// Create rigidbody at provided position and rotation
	PxQuat currQ(quat.at(0), quat.at(1), quat.at(2), quat.at(3));
	PxVec3 posVec(pos.at(0), pos.at(1), pos.at(2));
	PxTransform currT(posVec, currQ);
	PxRigidDynamic* body = gPhysics->createRigidDynamic(currT);
	// Attach rigidbody to shape
	body->attachShape(*pShape);
	return body;
}

// Cleanup rigidbody
void PxMesh::DestroyRigidbody(PxRigidDynamic* curr)
{
	curr->release();
}

// New physx mesh manager
PxMesh::PxMesh(string path,
							int _id,
							float scale,
							PxPhysics* gPhysics,
							PxScene* gScene,
							PxCooking* gCooking,
							PxMaterial* gMaterial) :
							MeshBase(path, _id, scale),
	gPhysics(gPhysics),
	gScene(gScene),
	gCooking(gCooking),
	gMaterial(gMaterial)
{
	// Setup vertices and indices for physx
	ConvertBuffers();
};

// Cleanup mesh manager
PxMesh::~PxMesh()
{
	pShape->release();
	delete[] pVerts;
	delete[] pIndices;
}
