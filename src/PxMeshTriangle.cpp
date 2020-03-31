#include "PxMeshTriangle.h"

#include <fstream>

//---------------------------------------
// Creates triangle based physx mesh
//---------------------------------------
bool PxMeshTriangle::CreateMesh(bool saveBounds, float scale)
{
	// Path to cooked mesh
	std::ifstream cookedMesh(meshPath + "px");
	// If cooked mesh not on disk
	if (!cookedMesh.good())
	{
		// Load mesh file
		LoadFile(scale);
		// Create triangle mesh object
		PxTriangleMeshDesc triangleDesc;
		triangleDesc.points.count = vecVertices.size() / 3;
		triangleDesc.points.stride = sizeof(float) * 3;
		triangleDesc.points.data = X_GetVertices();
		triangleDesc.triangles.count = vecIndices.size() / 3;
		triangleDesc.triangles.stride = sizeof(int) * 3;
		triangleDesc.triangles.data = X_GetIndices();

		// Cook the mesh
		PxDefaultFileOutputStream writeOutBuffer((meshPath + "px").c_str());
		if (!pPxCooking->cookTriangleMesh(triangleDesc, writeOutBuffer))
		{
			return false;
		}
	}

	// Create buffer
	PxDefaultFileInputData readInBuffer((meshPath + "px").c_str());
	// Create the mesh from buffer
	pPxMesh = PxGetPhysics().createTriangleMesh(readInBuffer);

#ifdef PX_EXPORT_TO_OBJ
	// Save pointers to vertices and triangles
	const void* trisBuff = pPxMesh->getTriangles();
	const PxVec3* vertsBuff = pPxMesh->getVertices();
	PxU32 trisNum = pPxMesh->getNbTriangles();
	PxU32 vertsNum = pPxMesh->getNbVertices();

	// Triangles and vertices buffers
	vector<int> tris((int*)trisBuff, (int*)trisBuff + (trisNum * 3));
	vector<float> verts((float*)vertsBuff, (float*)vertsBuff + (vertsNum * 3));

	// Create obj file
	StoreFile(tris, trisNum, verts, vertsNum, "_px");
#endif

	// Save extends
	if (saveBounds)
	{
		xMax = pPxMesh->getLocalBounds().maximum.x;
		yMax = pPxMesh->getLocalBounds().maximum.y;
		xMin = pPxMesh->getLocalBounds().minimum.x;
		yMin = pPxMesh->getLocalBounds().minimum.y;
	}

	// Create collision detection capable shape from mesh
	pPxShape = PxGetPhysics().createShape(PxTriangleMeshGeometry(pPxMesh), *pPxMaterial);
	return true;
}

//---------------------------------------
// Create and add triangle rigidbody
//---------------------------------------
PxRigidActor* PxMeshTriangle::AddRigidActor(const PxVec3& pos, const PxQuat& rot) const
{
	// Create rigidbody
	PxRigidStatic* body = (PxRigidStatic*)CreateRigidActor(pos, rot, true);
	// Update
	pPxShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
	// Attach rigidbody to shape
	body->attachShape(*pPxShape);
	// Add to scene and return
	pPxScene->addActor(*body);
	return body;
}

//---------------------------------------
// Copy constructor, increases reference count
//---------------------------------------
PxMeshTriangle::PxMeshTriangle(const PxMeshTriangle& copy):
	pPxMesh(copy.pPxMesh),
	PxMesh(copy)
{
	pPxMesh->acquireReference();
}

//---------------------------------------
// Cleanup triangle mesh manager
//---------------------------------------
PxMeshTriangle::~PxMeshTriangle()
{
	PX_RELEASE(pPxMesh);
}
