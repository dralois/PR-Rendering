#pragma once

#include <PxMesh.h>

//---------------------------------------
// Convex mesh, limited to 256 tris
//---------------------------------------
class PxMeshConvex : public PxMesh
{
private:
	//---------------------------------------
	// Fields
	//---------------------------------------
	PxConvexMesh* pPxMesh = NULL;

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
	~PxMeshConvex();
};
