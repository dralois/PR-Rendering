#pragma once

#include "PxMesh.h"

class PxMeshConvex : public PxMesh
{
private:
	// Fields
	PxConvexMesh* pMesh;
	float metallic = 0.1;

public:
	// Methods
	virtual bool CreateMesh() override;
	virtual PxRigidDynamic* CreateRigidbody(const vector<float>& pos, const vector<float>& quat) override;

	// Properties
	inline void SetMetallic(float metallic_) { metallic = metallic_; }
	inline float GetMetallic() { return metallic; }

	// Constructors
	using PxMesh::PxMesh;
	~PxMeshConvex();
};
