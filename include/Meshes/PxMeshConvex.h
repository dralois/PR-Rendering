#pragma once

#pragma warning(push, 0)
#include <Meshes/PxMesh.h>
#pragma warning(pop)

//---------------------------------------
// Convex mesh, limited to 256 tris
//---------------------------------------
class PxMeshConvex : public PxMesh
{
private:
	//---------------------------------------
	// Fields
	//---------------------------------------

	physx::PxConvexMesh* pPxMesh = NULL;

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

	PxMeshConvex(ReferencePath meshPath, int meshId);
	PxMeshConvex(const PxMeshConvex& copy);
	PxMeshConvex(PxMeshConvex&& other);
	~PxMeshConvex();
};
