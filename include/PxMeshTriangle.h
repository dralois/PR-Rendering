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
	virtual bool CreateMesh(bool saveBounds, bool doubleNorms) override;
	virtual PxRigidActor* CreateRigidbody(const vector<float>& pos, const vector<float>& quat) const override;

	//---------------------------------------
	// Constructors
	//---------------------------------------
	using PxMesh::PxMesh;
	~PxMeshTriangle();
};
