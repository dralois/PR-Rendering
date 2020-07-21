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
	boost::filesystem::path cookPath(meshPath); cookPath.concat("px");
	boost::filesystem::ifstream cookedMesh(cookPath);
	// If cooked mesh not on disk
	if (!cookedMesh.good())
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
		PxTriangleMeshCookingResult::Enum result;
		PxDefaultFileOutputStream writeOutBuffer(cookPath.string().c_str());
		if (!pPxCooking->cookTriangleMesh(triangleDesc, writeOutBuffer, &result))
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
	pPxMesh = PxGetPhysics().createTriangleMesh(readInBuffer);
	// This mesh has to release the mesh
	firstInstance = true;
}

//---------------------------------------
// Exports the cooked mesh to a obj file
//---------------------------------------
void PxMeshTriangle::X_ExportMesh()
{
	// Mesh needs to exist
	if (!pPxMesh)
		return;

	// Save pointers to vertices and triangles
	int* trisBuff = (int*)pPxMesh->getTriangles();
	PxVec3* vertsBuff = (PxVec3*)pPxMesh->getVertices();
	PxU32 trisNum = pPxMesh->getNbTriangles();
	PxU32 vertsNum = pPxMesh->getNbVertices();

	// Copy vertices into vector
	for(int i = 0; i < vertsNum; i++)
	{
		vecVertices.push_back(vertsBuff[i].x);
		vecVertices.push_back(vertsBuff[i].y);
		vecVertices.push_back(vertsBuff[i].z);
	}

	// Copy indices into vector
	for(int i = 0; i < trisNum; i++)
	{
		vecIndices.push_back(trisBuff[(i * 3)]);
		vecIndices.push_back(trisBuff[(i * 3) + 1]);
		vecIndices.push_back(trisBuff[(i * 3) + 2]);
	}

#ifdef EXPORT_TO_FILE
	// Store to obj file
	X_StoreFile("_px");
#endif // EXPORT_TO_FILE
}

//---------------------------------------
// Creates physx mesh
//---------------------------------------
void PxMeshTriangle::X_CreateMesh()
{
	// Cook / load mesh
	X_CookMesh();

#ifdef PX_EXTRACT_INTERNAL
	// Export cooked mesh to file
	X_ExportMesh();
#endif // PX_EXTRACT_INTERNAL

	// Save extends
	maximum = pPxMesh->getLocalBounds().maximum;
	minimum = pPxMesh->getLocalBounds().minimum;
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
	pPxShape = PxRigidActorExt::createExclusiveShape(*pPxActor, meshGeom, *pPxMaterial);
	pPxShape->setName(GetName().c_str());
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
