#pragma once

#include "PxMesh.h"

//---------------------------------------
// Triangle meshes, to be used sparingly
//---------------------------------------
class PxMeshTriangle : public PxMesh
{
private:
	//---------------------------------------
	// Fields
	//---------------------------------------
	PxTriangleMesh* pPxMesh;

public:
	//---------------------------------------
	// Methods
	//---------------------------------------
	virtual bool CreateMesh(bool saveBounds, float scale) override;
	virtual PxRigidActor* AddRigidActor(const PxVec3& pos, const PxQuat& rot) const override;

	//---------------------------------------
	// Constructors
	//---------------------------------------
	using PxMesh::PxMesh;
	PxMeshTriangle(const PxMeshTriangle& copy);
	~PxMeshTriangle();
};
