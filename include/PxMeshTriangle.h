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
	virtual bool CreateMesh() override;
	virtual PxRigidDynamic* CreateRigidbody(const vector<float>& pos, const vector<float>& quat) const override;

	//---------------------------------------
	// Constructors
	//---------------------------------------
	using PxMesh::PxMesh;
	~PxMeshTriangle();
};
