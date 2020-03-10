#pragma once

#include "PxMesh.h"

class PxMeshTriangle : public PxMesh
{
private:
	// Fields
	PxTriangleMesh* pMesh;

public:
	// Methods
	virtual bool CreateMesh() override;
	virtual PxRigidDynamic* CreateRigidbody(const vector<float>& pos, const vector<float>& quat) override;

	// Constructors
	using PxMesh::PxMesh;
	~PxMeshTriangle();
};
