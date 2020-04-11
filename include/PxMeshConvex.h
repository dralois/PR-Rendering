#pragma once

#include "PxMesh.h"

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
	virtual void X_CookMesh() override;
	virtual void X_ExportCookedMesh() override;
	virtual void X_CreateShape() override;

public:
	//---------------------------------------
	// Methods
	//---------------------------------------
	virtual void CreateMesh(float scale) override;
	virtual void AddRigidActor(const PxTransform& pose, PxScene* scene) override;

	//---------------------------------------
	// Constructors
	//---------------------------------------
	using PxMesh::PxMesh;
	~PxMeshConvex();
};
