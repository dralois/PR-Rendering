#include "PxMeshTriangle.h"

//---------------------------------------
// Creates triangle based physx mesh
//---------------------------------------
bool PxMeshTriangle::CreateMesh()
{
	PxU8* meshBuff;
	PxU32 buffSize;
	// If cooked mesh on disk
	if (TryReadCookedFile(meshBuff, buffSize))
	{
		// Create the mesh with buffer
		PxDefaultMemoryInputData input(meshBuff, buffSize);
		pPxMesh = PxGetPhysics().createTriangleMesh(input);
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

		// Create triangle mesh object
		PxTriangleMeshDesc triangleDesc;
		triangleDesc.points.count = (PxU32)vecVertices.size() / 3;
		triangleDesc.points.stride = sizeof(PxVec3);
		triangleDesc.points.data = GetVertices();
		triangleDesc.triangles.count = (PxU32)vecIndices.size() / 3;
		triangleDesc.triangles.stride = sizeof(PxU32) * 3;
		triangleDesc.triangles.data = GetIndices();

		// Lower hierarchy for internal mesh
		PxDefaultMemoryOutputStream writeBuffer;

		// Cook mesh
		if (!pPxCooking->cookTriangleMesh(triangleDesc, writeBuffer))
		{
			return false;
		}

		// Write into file
		WriteCookedFile(writeBuffer.getData(), writeBuffer.getSize());

		// Create the mesh
		PxDefaultMemoryInputData input(writeBuffer.getData(), writeBuffer.getSize());
		pPxMesh = PxGetPhysics().createTriangleMesh(input);

	}

	// Create collision detection capable shape from mesh
	pShape = PxGetPhysics().createShape(PxTriangleMeshGeometry(pPxMesh), *pPxMaterial);

	return true;
}

//---------------------------------------
// Create and add triangle rigidbody
//---------------------------------------
PxRigidDynamic* PxMeshTriangle::CreateRigidbody(const vector<float>& pos, const vector<float>& quat) const
{
	// Create rigidbody
	PxRigidDynamic* body = InitRigidbody(pos, quat);
	// Update
	pShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
	body->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	// Attach rigidbody to shape
	body->attachShape(*pShape);
	// Add to scene and return
	pPxScene->addActor(*body);
	return body;
}

//---------------------------------------
// Cleanup triangle mesh manager
//---------------------------------------
PxMeshTriangle::~PxMeshTriangle()
{
	PX_RELEASE(pPxMesh);
}
