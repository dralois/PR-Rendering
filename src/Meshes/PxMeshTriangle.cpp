#include <Meshes/PxMeshTriangle.h>

using namespace physx;

//---------------------------------------
// Triangle meshes can not be dynamic
//---------------------------------------
bool PxMeshTriangle::X_IsStatic()
{
	return true;
}

//---------------------------------------
// Cooks or loads a mesh
//---------------------------------------
void PxMeshTriangle::X_CookMesh()
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

		// Create triangle mesh object
		PxTriangleMeshDesc triangleDesc;
		triangleDesc.points.count = vecVertices.size() / 3;
		triangleDesc.points.stride = sizeof(float) * 3;
		triangleDesc.points.data = X_GetVertices();
		triangleDesc.triangles.count = vecIndices.size() / 3;
		triangleDesc.triangles.stride = sizeof(int) * 3;
		triangleDesc.triangles.data = X_GetIndices();

		// Cook the mesh
#pragma warning(disable:26812)
		PxTriangleMeshCookingResult::Enum result;
#pragma warning(default:26812)
		PxDefaultFileOutputStream writeOutBuffer(cookPath.string().c_str());
		if (!PxManager::GetInstance().GetCooker()->cookTriangleMesh(triangleDesc, writeOutBuffer, &result))
		{
			std::cout << "\33[2K\r" << GetName() << " cooking error:\t" << result << std::endl;
		}
		else
		{
			std::cout << "\33[2K\r" << GetName() << " cooked to:\t" << boost::filesystem::relative(cookPath) << std::flush;
		}
	}
	else
	{
		std::cout << "\33[2K\r" << GetName() << " loaded from:\t" << boost::filesystem::relative(cookPath) << std::flush;
	}

	// Create buffer
	PxDefaultFileInputData readInBuffer(cookPath.string().c_str());
	// Create the mesh from buffer
	pPxMesh = PxGetPhysics().createTriangleMesh(readInBuffer);
	// This mesh has to release the mesh
	firstInstance = true;
}

//---------------------------------------
// Exports the cooked mesh to a obj file
//---------------------------------------
void PxMeshTriangle::X_ExtractMesh()
{
	// Mesh needs to exist
	if (!pPxMesh)
		return;

	// Delete potentially loaded mesh
	vecVertices.clear();
	vecIndices.clear();
	vecNormals.clear();
	vecUVs.clear();

	// Save pointers to vertices and triangles
	int* trisBuff = (int*)pPxMesh->getTriangles();
	PxVec3* vertsBuff = (PxVec3*)pPxMesh->getVertices();
	PxU32 trisNum = pPxMesh->getNbTriangles();
	PxU32 vertsNum = pPxMesh->getNbVertices();

	// Copy vertices into vector
	for (int i = 0; i < vertsNum; i++)
	{
		vecVertices.push_back(vertsBuff[i].x);
		vecVertices.push_back(vertsBuff[i].y);
		vecVertices.push_back(vertsBuff[i].z);
	}

	// Copy indices into vector
	for (int i = 0; i < trisNum; i++)
	{
		vecIndices.push_back(trisBuff[(i * 3)]);
		vecIndices.push_back(trisBuff[(i * 3) + 1]);
		vecIndices.push_back(trisBuff[(i * 3) + 2]);
	}
}

//---------------------------------------
// Creates physx mesh
//---------------------------------------
void PxMeshTriangle::X_CreateMesh()
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
// Create shape and attach to actor
//---------------------------------------
void PxMeshTriangle::X_CreateShape()
{
	// Mesh needs to exist
	if (!pPxMesh)
		return;

	// Create mesh descriptor
	PxTriangleMeshGeometry meshGeom;
	meshGeom.triangleMesh = pPxMesh;
	meshGeom.meshFlags = PxMeshGeometryFlag::eDOUBLE_SIDED;
	meshGeom.scale = PxMeshScale(GetScale());

	// Create shape from the descriptor
	pPxShape = PxRigidActorExt::createExclusiveShape(*pPxActor, meshGeom, *PxManager::GetInstance().GetMaterial());
	pPxShape->setName(GetName().c_str());
}

//---------------------------------------
// Default constructor
//---------------------------------------
PxMeshTriangle::PxMeshTriangle(
	ReferencePath meshPath,
	const std::string& meshClass,
	int meshId
) :
	PxMesh(meshPath, meshClass, meshId),
	pPxMesh(NULL)
{
}

//---------------------------------------
// Copy constructor
//---------------------------------------
PxMeshTriangle::PxMeshTriangle(const PxMeshTriangle& copy) :
	PxMesh(copy),
	pPxMesh(copy.pPxMesh)
{
}

//---------------------------------------
// Move constructor
//---------------------------------------
PxMeshTriangle::PxMeshTriangle(PxMeshTriangle&& other) :
	PxMesh(std::move(other))
{
	pPxMesh = std::exchange(other.pPxMesh, nullptr);
}

//---------------------------------------
// Cleanup physx mesh
//---------------------------------------
PxMeshTriangle::~PxMeshTriangle()
{
	// Release mesh if this is the first instance
	if (firstInstance)
	{
		PX_RELEASE(pPxMesh);
	}
}
