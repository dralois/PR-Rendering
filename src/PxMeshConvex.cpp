#include "PxMeshConvex.h"

//---------------------------------------
// Convex meshes may be dynamic
//---------------------------------------
bool PxMeshConvex::X_IsStatic()
{
	return false;
}

//---------------------------------------
// Cooks or loads a mesh
//---------------------------------------
void PxMeshConvex::X_CookMesh()
{
	// Return if mesh already loaded
	if (pPxMesh)
		return;

	// Path to cooked mesh
	std::ifstream cookedMesh(meshPath + "px");
	// If cooked mesh not on disk
	if (!cookedMesh.good())
	{
		// Load mesh file
		X_LoadFile();

		// Create convex mesh
		PxConvexMeshDesc convDesc;
		convDesc.points.count = vecVertices.size() / 3;
		convDesc.points.stride = sizeof(PxVec3);
		convDesc.points.data = X_GetVertices();
		convDesc.flags |= PxConvexFlag::eCOMPUTE_CONVEX;

		// Cook the mesh
		PxConvexMeshCookingResult::Enum result;
		PxDefaultFileOutputStream writeOutBuffer((meshPath + "px").c_str());
		if (!pPxCooking->cookConvexMesh(convDesc, writeOutBuffer, &result))
		{
			std::cout << GetName() << " cooking error:" << result << std::endl;
		}
		else
		{
			std::cout << GetName() << " mesh cook result:" << result << ", saved at:" << meshPath + "px" << std::endl;
		}
	}

	// Create buffer
	PxDefaultFileInputData readInBuffer((meshPath + "px").c_str());
	// Create the mesh from buffer
	pPxMesh = PxGetPhysics().createConvexMesh(readInBuffer);
	// This mesh has to release the mesh
	firstInstance = true;
}

//---------------------------------------
// Exports the cooked mesh to a obj file
//---------------------------------------
void PxMeshConvex::X_ExportCookedMesh()
{
	// Mesh needs to exist
	if (!pPxMesh)
		return;

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
	X_StoreFile(tris, tris.size() / 3, verts, verts.size() / 3, "_px");

	// Cleanup
	tris.clear();
	verts.clear();
}

//---------------------------------------
// Create shape and attach to actor
//---------------------------------------
void PxMeshConvex::X_CreateShape()
{
	// Mesh needs to exist
	if (!pPxMesh)
		return;

	// Create mesh descriptor
	PxConvexMeshGeometry meshGeom;
	meshGeom.convexMesh = pPxMesh;
	meshGeom.meshFlags = PxConvexMeshGeometryFlag::eTIGHT_BOUNDS;
	meshGeom.scale = PxMeshScale(meshScale);

	// Create shape from the descriptor
	pPxShape = PxRigidActorExt::createExclusiveShape(*pPxActor, meshGeom, *pPxMaterial);
	pPxShape->setName(GetName().c_str());
}

//---------------------------------------
// Creates physx mesh
//---------------------------------------
void PxMeshConvex::X_CreateMesh()
{
	// Cook / load mesh
	X_CookMesh();

#ifdef PX_EXPORT_TO_OBJ
	// Export cooked mesh to file
	X_ExportCookedMesh();
#endif

	// Save extends
	maximum = pPxMesh->getLocalBounds().maximum;
	minimum = pPxMesh->getLocalBounds().minimum;
}

//---------------------------------------
// Cleanup physx mesh
//---------------------------------------
PxMeshConvex::~PxMeshConvex()
{
	// Release mesh if this is the first instance
	if (firstInstance)
	{
		PX_RELEASE(pPxMesh);
	}
}
