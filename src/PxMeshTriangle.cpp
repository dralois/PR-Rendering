#include "PxMeshTriangle.h"

// Creates triangle based physx mesh
bool PxMeshTriangle::CreateMesh()
{
	// Set as physx types
	ConvertBuffers();
	PxVec3* pVerts = GetVertices();
	PxU32* pIndices = GetIndices();

	// Create triangle mesh object
	PxTriangleMeshDesc triangleDesc;
	triangleDesc.points.count = (PxU32)vecVertices.size() / 3;
	triangleDesc.points.stride = sizeof(PxVec3);
	triangleDesc.points.data = pVerts;

	triangleDesc.triangles.count = (PxU32)vecIndices.size() / 3;
	triangleDesc.triangles.stride = sizeof(PxU32) * 3;
	triangleDesc.triangles.data = pIndices;

	// Lower hierarchy for internal mesh
	PxDefaultMemoryOutputStream writeBuffer;

	// Cook mesh
	if (!gCooking->cookTriangleMesh(triangleDesc, writeBuffer))
	{
		return false;
	}

	// Create the mesh
	PxDefaultMemoryInputData input(writeBuffer.getData(), writeBuffer.getSize());
	pMesh = gPhysics->createTriangleMesh(input);
	PxTriangleMeshGeometry geom(pMesh);

	// Create collision detection capable shape from mesh
	pShape = gPhysics->createShape(geom, *gMaterial);
	pShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);

	return true;
}

// Create and add triangle rigidbody
PxRigidDynamic* PxMeshTriangle::CreateRigidbody(const vector<float>& pos, const vector<float>& quat)
{
	// Create rigidbody
	PxRigidDynamic* body = InitRigidbody(pos, quat);
	// Update
	pShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
	body->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	// Add to scene and return
	gScene->addActor(*body);
	return body;
}

// Cleanup triangle mesh manager
PxMeshTriangle::~PxMeshTriangle()
{
	pMesh->release();
}
