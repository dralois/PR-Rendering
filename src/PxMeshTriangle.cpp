#include "PxMeshTriangle.h"

//---------------------------------------
// Creates triangle based physx mesh
//---------------------------------------
bool PxMeshTriangle::CreateMesh(bool saveBounds, bool doubleNorms)
{
	// Path to cooked mesh
	std::ifstream cookedMesh(meshPath + "px");
	// If cooked mesh not on disk
	if (!cookedMesh.good())
	{
		// Load mesh file
		LoadFile(doubleNorms);
		// Create triangle mesh object
		PxTriangleMeshDesc triangleDesc;
		triangleDesc.points.count = (PxU32)(vecVertices.size() / 3);
		triangleDesc.points.stride = (PxU32)sizeof(PxVec3);
		triangleDesc.points.data = GetVertices();
		triangleDesc.triangles.count = (PxU32)(vecIndices.size() / 3);
		triangleDesc.triangles.stride = (PxU32)(sizeof(PxU32) * 3);
		triangleDesc.triangles.data = GetIndices();

		// Cook the mesh
		PxDefaultFileOutputStream writeOutBuffer((meshPath + "px").c_str());
		// Cook mesh
		if (!pPxCooking->cookTriangleMesh(triangleDesc, writeOutBuffer))
		{
			return false;
		}
	}

	// Create buffer
	PxDefaultFileInputData readInBuffer((meshPath + "px").c_str());
	// Create the mesh from buffer
	pPxMesh = PxGetPhysics().createTriangleMesh(readInBuffer);

	// Save extends
	if (saveBounds)
	{
		xMax = pPxMesh->getLocalBounds().maximum.x;
		yMax = pPxMesh->getLocalBounds().maximum.y;
		xMin = pPxMesh->getLocalBounds().minimum.x;
		yMin = pPxMesh->getLocalBounds().minimum.y;
	}

	// Create collision detection capable shape from mesh
	pShape = PxGetPhysics().createShape(PxTriangleMeshGeometry(pPxMesh), *pPxMaterial);
	return true;
}

//---------------------------------------
// Create and add triangle rigidbody
//---------------------------------------
PxRigidActor* PxMeshTriangle::CreateRigidbody(const vector<float>& pos, const vector<float>& quat) const
{
	// Create rigidbody
	PxRigidStatic* body = (PxRigidStatic*) InitRigidbody(pos, quat, true);
	// Update
	pShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
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
