#pragma once

#pragma warning(push, 0)
#include <Meshes/PxMesh.h>
#pragma warning(pop)

//---------------------------------------
// Triangle mesh, to be used sparingly
//---------------------------------------
class PxMeshTriangle : public PxMesh
{
private:
	//---------------------------------------
	// Fields
	//---------------------------------------

	physx::PxTriangleMesh* pPxMesh = NULL;

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual bool X_IsStatic() override;
	virtual void X_CookMesh() override;
	virtual void X_ExportMesh() override;
	virtual void X_CreateMesh() override;
	virtual void X_CreateShape() override;

public:
	//---------------------------------------
	// Constructors
	//---------------------------------------

	PxMeshTriangle(ReferencePath meshPath, int meshId);
	PxMeshTriangle(const PxMeshTriangle& copy);
	PxMeshTriangle(PxMeshTriangle&& other);
	~PxMeshTriangle();
};
