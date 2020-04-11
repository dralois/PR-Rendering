#include "PxMeshTriangle.h"

//---------------------------------------
// Cooks or loads a mesh
//---------------------------------------
void PxMeshTriangle::X_CookMesh()
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
		PxDefaultFileOutputStream writeOutBuffer((meshPath + "px").c_str());
		if (!pPxCooking->cookTriangleMesh(triangleDesc, writeOutBuffer, &result))
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
	pPxMesh = PxGetPhysics().createTriangleMesh(readInBuffer);
	// This mesh has to release the mesh
	firstInstance = true;
}

//---------------------------------------
// Exports the cooked mesh to a obj file
//---------------------------------------
void PxMeshTriangle::X_ExportCookedMesh()
{
	// Save pointers to vertices and triangles
	const void* trisBuff = pPxMesh->getTriangles();
	const PxVec3* vertsBuff = pPxMesh->getVertices();
	PxU32 trisNum = pPxMesh->getNbTriangles();
	PxU32 vertsNum = pPxMesh->getNbVertices();

	// Triangles and vertices buffers
	vector<int> tris((int*)trisBuff, (int*)trisBuff + (trisNum * 3));
	vector<float> verts((float*)vertsBuff, (float*)vertsBuff + (vertsNum * 3));

	// Create obj file
	X_StoreFile(tris, trisNum, verts, vertsNum, "_px");
}

//---------------------------------------
// Create shape and attach to actor
//---------------------------------------
void PxMeshTriangle::X_CreateShape()
{
	// Create mesh descriptor
	PxTriangleMeshGeometry meshGeom;
	meshGeom.triangleMesh = pPxMesh;
	meshGeom.meshFlags = PxMeshGeometryFlag::eDOUBLE_SIDED;
	meshGeom.scale = PxMeshScale(meshScale);

	// Create shape from the descriptor
	pPxShape = PxRigidActorExt::createExclusiveShape(*pPxActor, meshGeom, *pPxMaterial);
	pPxShape->setName(GetName().c_str());
}

//---------------------------------------
// Creates physx mesh
//---------------------------------------
void PxMeshTriangle::CreateMesh(float scale)
{
	// Cook / load mesh
	X_CookMesh();

#ifdef PX_EXPORT_TO_OBJ
	// Export cooked mesh to file
	X_ExportCookedMesh();
#endif

	// Save scale
	meshScale = scale;

	// Save extends
	maximum = pPxMesh->getLocalBounds().maximum * meshScale;
	minimum = pPxMesh->getLocalBounds().minimum * meshScale;
}

//---------------------------------------
// Create and add rigidbody
//---------------------------------------
void PxMeshTriangle::AddRigidActor(const PxTransform& pose, PxScene* scene)
{
	// Create statuc rigidbody & add to scene
	PxRigidActor* body = X_CreateRigidActor(pose, true);
	scene->addActor(*body);
}

//---------------------------------------
// Cleanup mesh
//---------------------------------------
PxMeshTriangle::~PxMeshTriangle()
{
	// Release mesh if this is the first instance
	if (firstInstance)
	{
		PX_RELEASE(pPxMesh);
	}
}
