#pragma once

#include "PxMesh.h"

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
	virtual void X_ExportCookedMesh() override;
	virtual void X_CreateMesh() override;
	virtual void X_CreateShape() override;

public:
	//---------------------------------------
	// Constructors
	//---------------------------------------
	using PxMesh::PxMesh;
	~PxMeshTriangle();
};
