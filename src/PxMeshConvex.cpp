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
		convDesc.points.count = vecVertices.size() / 3;
		convDesc.points.stride = sizeof(PxVec3);
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

#ifdef PX_EXPORT_TO_OBJ
	// Triangles and vertices buffers
	vector<int> tris;
	vector<float> verts;

	// Get polygons and buffers
	PxU32 polyNum = pPxMesh->getNbPolygons();
	PxU32 vertsNum = pPxMesh->getNbVertices();
	const PxVec3* vertsBuff = pPxMesh->getVertices();
	const PxU8* idxBuff = pPxMesh->getIndexBuffer();

	// Triangulate
	PxU32 offset = 0;
	for (PxU32 i = 0; i < polyNum; i++)
	{
		// Get face
		PxHullPolygon face;
		pPxMesh->getPolygonData(i, face);
		// Save vertices
		const PxU8* faceIndices = idxBuff + face.mIndexBase;
		for (PxU32 j = 0; j < face.mNbVerts; j++)
		{
			verts.push_back(vertsBuff[faceIndices[j]].x);
			verts.push_back(vertsBuff[faceIndices[j]].y);
			verts.push_back(vertsBuff[faceIndices[j]].z);
		}
		// Save indices
		for (PxU32 j = 2; j < face.mNbVerts; j++)
		{
			tris.push_back(PxU32(offset));
			tris.push_back(PxU32(offset + j));
			tris.push_back(PxU32(offset + j - 1));
		}
		// Update counter
		offset += face.mNbVerts;
	}

	// Create obj file
	StoreFile(tris, tris.size() / 3, verts, verts.size() / 3, "_px");

	// Cleanup
	tris.clear();
	verts.clear();

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
	PxRigidDynamic* body = (PxRigidDynamic*)InitRigidbody(pos, quat, false);
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
