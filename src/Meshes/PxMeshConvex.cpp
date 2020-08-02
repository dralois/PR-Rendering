#include <Meshes/PxMeshConvex.h>

using namespace physx;

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
	ModifiablePath cookPath(meshPath); cookPath.concat("px");
	boost::filesystem::ifstream cookedMesh(cookPath);
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
		PxDefaultFileOutputStream writeOutBuffer(cookPath.string().c_str());
		if (!PxManager::GetInstance().GetCooker()->cookConvexMesh(convDesc, writeOutBuffer, &result))
		{
			std::cout << GetName() << " cooking error:" << result << std::endl;
		}
		else
		{
			std::cout << GetName() << " mesh cook result:" << result << ", saved at:" << cookPath << std::endl;
		}
	}

	// Create buffer
	PxDefaultFileInputData readInBuffer(cookPath.string().c_str());
	// Create the mesh from buffer
	pPxMesh = PxGetPhysics().createConvexMesh(readInBuffer);
	// This mesh has to release the mesh
	firstInstance = true;
}

//---------------------------------------
// Exports the cooked mesh to a obj file
//---------------------------------------
void PxMeshConvex::X_ExportMesh()
{
	// Mesh needs to exist
	if (!pPxMesh)
		return;

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
			vecVertices.push_back(vertsBuff[faceIndices[j]].x);
			vecVertices.push_back(vertsBuff[faceIndices[j]].y);
			vecVertices.push_back(vertsBuff[faceIndices[j]].z);
		}
		// Save indices
		for (PxU32 j = 2; j < face.mNbVerts; j++)
		{
			vecIndices.push_back((int) offset);
			vecIndices.push_back((int) offset + j);
			vecIndices.push_back((int) offset + j - 1);
		}
		// Update counter
		offset += face.mNbVerts;
	}

#if EXPORT_TO_FILE
	// Store to obj file
	X_StoreFile("_px");
#endif // EXPORT_TO_FILE
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
	meshGeom.scale = PxMeshScale(GetScale());

	// Create shape from the descriptor
	pPxShape = PxRigidActorExt::createExclusiveShape(*pPxActor, meshGeom, PxManager::GetInstance().GetMaterial());
	pPxShape->setName(GetName().c_str());
}

//---------------------------------------
// Creates physx mesh
//---------------------------------------
void PxMeshConvex::X_CreateMesh()
{
	// Cook / load mesh
	X_CookMesh();

#if PX_EXTRACT_INTERNAL
	// Export cooked mesh to file
	X_ExportMesh();
#endif // PX_EXTRACT_INTERNAL

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
