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
	~PxMeshTriangle();
};
