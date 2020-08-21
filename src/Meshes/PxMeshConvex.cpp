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
	// If cooked mesh not on disk
	if (!boost::filesystem::exists(cookPath))
	{
		// Load mesh file
		X_LoadFile();

		// Create convex mesh
		PxConvexMeshDesc convDesc;
		convDesc.points.count = vecVertices.size() / 3;
		convDesc.points.stride = sizeof(float) * 3;
		convDesc.points.data = X_GetVertices();
		convDesc.indices.count = vecIndices.size();
		convDesc.indices.stride = sizeof(int);
		convDesc.indices.data = X_GetIndices();
		convDesc.flags |= PxConvexFlag::eCOMPUTE_CONVEX;
		convDesc.flags |= PxConvexFlag::eSHIFT_VERTICES;
		convDesc.flags |= PxConvexFlag::eGPU_COMPATIBLE;

		// Cook the mesh
		PxConvexMeshCookingResult::Enum result;
		PxDefaultFileOutputStream writeOutBuffer(cookPath.string().c_str());
		if (!PxManager::GetInstance().GetCooker()->cookConvexMesh(convDesc, writeOutBuffer, &result))
		{
			std::cout << "\r\33[2K" << GetName() << " cooking error:\t" << result << std::endl;
		}
		else
		{
			std::cout << "\r\33[2K" << GetName() << " cooked to:\t" << boost::filesystem::relative(cookPath) << std::flush;
		}
	}
	else
	{
		std::cout << "\r\33[2K" << GetName() << " loaded from:\t" << boost::filesystem::relative(cookPath) << std::flush;
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
void PxMeshConvex::X_ExtractMesh()
{
	// Mesh needs to exist
	if (!pPxMesh)
		return;

	// Delete potentially loaded mesh
	vecVertices.clear();
	vecIndices.clear();
	vecNormals.clear();
	vecUVs.clear();

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
			vecIndices.push_back((int)offset);
			vecIndices.push_back((int)offset + j);
			vecIndices.push_back((int)offset + j - 1);
		}
		// Update counter
		offset += face.mNbVerts;
	}
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
	pPxShape = PxRigidActorExt::createExclusiveShape(*pPxActor, meshGeom, *PxManager::GetInstance().GetMaterial());
	pPxShape->setName(GetName().c_str());
}

//---------------------------------------
// Creates physx mesh
//---------------------------------------
void PxMeshConvex::X_CreateMesh()
{
	// Cook / load mesh (only first instance)
	X_CookMesh();

#if PX_EXTRACT_INTERNAL
	// Export cooked mesh to file (if first instance)
	if (firstInstance)
	{
		X_ExtractMesh();
	}
#endif // PX_EXTRACT_INTERNAL

#if EXPORT_TO_FILE
	// Store to obj file (if first instance)
	if (firstInstance)
	{
		X_StoreFile("_px");
	}
#endif // EXPORT_TO_FILE

	// Save extends
	bounds = pPxMesh->getLocalBounds();
}

//---------------------------------------
// Default constructor
//---------------------------------------
PxMeshConvex::PxMeshConvex(ReferencePath meshPath, int meshId) :
	PxMesh(meshPath, meshId),
	pPxMesh(NULL)
{
}

//---------------------------------------
// Copy constructor
//---------------------------------------
PxMeshConvex::PxMeshConvex(const PxMeshConvex& copy) :
	PxMesh(copy),
	pPxMesh(copy.pPxMesh)
{
}

//---------------------------------------
// Move constructor
//---------------------------------------
PxMeshConvex::PxMeshConvex(PxMeshConvex&& other) :
	PxMesh(std::move(other))
{
	pPxMesh = std::exchange(other.pPxMesh, nullptr);
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
