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

	PxTriangleMesh* pPxMesh = NULL;

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

	using PxMesh::PxMesh;
	~PxMeshTriangle();
};
