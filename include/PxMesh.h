#pragma once

#include "MeshBase.h"

#include <PxPhysicsAPI.h>

#define PX_RELEASE(x) if(x) { x->release(); x = NULL; }

using namespace physx;

//---------------------------------------
// Base class for PhysX meshes
//---------------------------------------
class PxMesh : public MeshBase
{
private:
	//---------------------------------------
	// Fields
	//---------------------------------------
	PxVec3* pVertices;
	PxU32* pIndices;

protected:
	//---------------------------------------
	// Fields
	//---------------------------------------
	PxScene* pPxScene;
	PxCooking* pPxCooking;
	PxMaterial* pPxMaterial;

	//---------------------------------------
	// Methods
	//---------------------------------------
	void ConvertBuffers();
	bool TryReadCookedFile(PxU8*& outBuffer, PxU32& outSize) const;
	void WriteCookedFile(PxU8* const buffer, PxU32 const size) const;

	//---------------------------------------
	// Properties
	//---------------------------------------
	inline PxVec3* GetVertices() { return pVertices; };
	inline PxU32* GetIndices() { return pIndices; };

public:
	//---------------------------------------
	// Fields
	//---------------------------------------
	PxShape* pShape;

	//---------------------------------------
	// Methods
	//---------------------------------------
	virtual bool CreateMesh() = 0;
	virtual PxRigidDynamic* CreateRigidbody(const vector<float>& pos, const vector<float>& quat) const = 0;

	PxRigidDynamic* InitRigidbody(const vector<float>& pos, const vector<float>& quat) const;
	static void DestroyRigidbody(PxRigidDynamic* body);

	//---------------------------------------
	// Constructors
	//---------------------------------------
	PxMesh(string path, int meshId, float scale, PxScene* scene, PxCooking* cooking, PxMaterial* material);
	~PxMesh();
};
