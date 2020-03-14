#include "PxMeshConvex.h"

//---------------------------------------
// Creates convex physx mesh
//---------------------------------------
bool PxMeshConvex::CreateMesh(bool saveBounds, bool doubleNorms)
{
	// Path to cooked mesh
	std::ifstream cookedMesh(meshPath + "px");
	// If cooked mesh not on disk
	if (!cookedMesh.good())
	{
		// Load mesh file
		LoadFile(doubleNorms);
		// Create convex mesh
		PxConvexMeshDesc convDesc;
		convDesc.points.count = (PxU32) (vecVertices.size() / 3);
		convDesc.points.stride = (PxU32) sizeof(PxVec3);
		convDesc.points.data = GetVertices();
		convDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

		// Cook the mesh
		PxDefaultFileOutputStream writeOutBuffer((meshPath + "px").c_str());
		if (!pPxCooking->cookConvexMesh(convDesc, writeOutBuffer))
		{
			return false;
		}
	}

	// Create buffer
	PxDefaultFileInputData readInBuffer((meshPath + "px").c_str());
	// Create the mesh from buffer
	pPxMesh = PxGetPhysics().createConvexMesh(readInBuffer);

	// Save extends
	if (saveBounds)
	{
		xMax = pPxMesh->getLocalBounds().maximum.x;
		yMax = pPxMesh->getLocalBounds().maximum.y;
		xMin = pPxMesh->getLocalBounds().minimum.x;
		yMin = pPxMesh->getLocalBounds().minimum.y;
	}

	// Create collision detection capable shape from mesh
	pShape = PxGetPhysics().createShape(PxConvexMeshGeometry(pPxMesh), *pPxMaterial);
	pShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);

	return true;
}

//---------------------------------------
// Create and add convex rigidbody
//---------------------------------------
PxRigidActor* PxMeshConvex::CreateRigidbody(const vector<float>& pos, const vector<float>& quat) const
{
	// Create rigidbody
	PxRigidDynamic* body = (PxRigidDynamic*) InitRigidbody(pos, quat, false);
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
