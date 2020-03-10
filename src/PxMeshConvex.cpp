#include "PxMeshConvex.h"

// Creates convex physx mesh
bool PxMeshConvex::CreateMesh()
{
	// Set as physx types
	ConvertBuffers();
	// Create convex mesh
	PxVec3* pVerts = GetVertices();
	PxConvexMeshDesc convDesc;
	convDesc.points.count = (PxU32)vecVertices.size() / (3);
	convDesc.points.stride = sizeof(PxVec3);
	convDesc.points.data = pVerts;
	convDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

	// Cook the mesh
	PxDefaultMemoryOutputStream writeBuffer;
	if (!gCooking->cookConvexMesh(convDesc, writeBuffer))
	{
		return false;
	}

	// Create the mesh
	PxDefaultMemoryInputData input(writeBuffer.getData(), writeBuffer.getSize());
	pMesh = gPhysics->createConvexMesh(input);

	// Create collision detection capable shape from mesh
	pShape = gPhysics->createShape(PxConvexMeshGeometry(pMesh), *gMaterial);
	pShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);

	return true;
}

// Create and add convex rigidbody
PxRigidDynamic* PxMeshConvex::CreateRigidbody(const vector<float>& pos, const vector<float>& quat)
{
	// Create rigidbody
	PxRigidDynamic* body = InitRigidbody(pos, quat);
	// Update
	PxRigidBodyExt::updateMassAndInertia(*body, 10.f);
	body->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
	// Add to scene and return
	gScene->addActor(*body);
	return body;
}

// Cleanup convex mesh manager
PxMeshConvex::~PxMeshConvex()
{
	pMesh->release();
}
