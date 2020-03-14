#include "PxMeshConvex.h"

//---------------------------------------
// Creates convex physx mesh
//---------------------------------------
bool PxMeshConvex::CreateMesh()
{
	PxU8* meshBuff;
	PxU32 buffSize;
	// If cooked mesh on disk
	if (TryReadCookedFile(meshBuff, buffSize))
	{
		// Create the mesh with buffer
		PxDefaultMemoryInputData input(meshBuff, buffSize);
		pPxMesh = PxGetPhysics().createConvexMesh(input);
		// Cleanup
		delete[] meshBuff;
	}
	// Not cooked and saved yet
	else
	{
		// Load mesh
		LoadFile();

		// Convert to physx types
		ConvertBuffers();

		// Create convex mesh
		PxConvexMeshDesc convDesc;
		convDesc.points.count = (PxU32)vecVertices.size() / (3);
		convDesc.points.stride = sizeof(PxVec3);
		convDesc.points.data = GetVertices();
		convDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

		// Cook the mesh
		PxDefaultMemoryOutputStream writeBuffer;
		if (!pPxCooking->cookConvexMesh(convDesc, writeBuffer))
		{
			return false;
		}

		// Write into file
		WriteCookedFile(writeBuffer.getData(), writeBuffer.getSize());

		// Create the mesh
		PxDefaultMemoryInputData input(writeBuffer.getData(), writeBuffer.getSize());
		pPxMesh = PxGetPhysics().createConvexMesh(input);
	}

	// Create collision detection capable shape from mesh
	pShape = PxGetPhysics().createShape(PxConvexMeshGeometry(pPxMesh), *pPxMaterial);
	pShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);

	return true;
}

//---------------------------------------
// Create and add convex rigidbody
//---------------------------------------
PxRigidDynamic* PxMeshConvex::CreateRigidbody(const vector<float>& pos, const vector<float>& quat) const
{
	// Create rigidbody
	PxRigidDynamic* body = InitRigidbody(pos, quat);
	// Update
	PxRigidBodyExt::updateMassAndInertia(*body, 10.f);
	body->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
	// Attach rigidbody to shape
	body->attachShape(*pShape);
	// Add to scene and return
	pPxScene->addActor(*body);
	return body;
}

//---------------------------------------
// Cleanup convex mesh manager
//---------------------------------------
PxMeshConvex::~PxMeshConvex()
{
	PX_RELEASE(pPxMesh);
}
