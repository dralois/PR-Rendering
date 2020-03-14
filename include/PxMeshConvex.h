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
	float metallic = 0.1;

public:
	//---------------------------------------
	// Methods
	//---------------------------------------
	virtual bool CreateMesh() override;
	virtual PxRigidDynamic* CreateRigidbody(const vector<float>& pos, const vector<float>& quat) const override;

	//---------------------------------------
	// Properties
	//---------------------------------------
	inline void SetMetallic(float metallic_) { metallic = metallic_; }
	inline float GetMetallic() { return metallic; }

	//---------------------------------------
	// Constructors
	//---------------------------------------
	using PxMesh::PxMesh;
	~PxMeshConvex();
};
