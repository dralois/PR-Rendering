#pragma once

#include "PxMesh.h"

//---------------------------------------
// Convex meshes, limited to 255 tris
//---------------------------------------
class PxMeshConvex : public PxMesh
{
private:
	//---------------------------------------
	// Fields
	//---------------------------------------
	PxConvexMesh* pPxMesh;

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
	PxMeshConvex(const PxMeshConvex& copy);
	~PxMeshConvex();
};
