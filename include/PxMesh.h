#pragma once

#include "MeshBase.h"

#pragma warning(push, 0)
#include <PxPhysicsAPI.h>
#pragma warning(pop)

#include <iostream>
#include <fstream>

#define PX_RELEASE(x) if(x != NULL) { x->release(); x = NULL; }

//#define PX_EXPORT_TO_OBJ

using namespace physx;

//---------------------------------------
// Base class for physx meshes
//---------------------------------------
class PxMesh : public MeshBase
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------
	PxShape* pPxShape;
	PxRigidActor* pPxActor;
	const PxCooking* pPxCooking;
	const PxMaterial* pPxMaterial;

	bool firstInstance = false;
	PxVec3 maximum = PxVec3(1e8);
	PxVec3 minimum = PxVec3(-1e8);

	//---------------------------------------
	// Methods
	//---------------------------------------
	virtual void X_CookMesh() = 0;
	virtual void X_ExportCookedMesh() = 0;
	virtual void X_CreateShape() = 0;
	PxRigidActor* X_CreateRigidActor(const PxTransform& pose, bool isStatic);
	virtual void X_UpdateScale() override;

	//---------------------------------------
	// Properties
	//---------------------------------------
	inline PxVec3* X_GetVertices() { return (PxVec3*) &vecVertices[0]; };
	inline PxU32* X_GetIndices() { return (PxU32*) &vecIndices[0]; };

public:
	//---------------------------------------
	// Methods
	//---------------------------------------
	virtual void CreateMesh(float scale) = 0;
	virtual void AddRigidActor(const PxTransform& pose, PxScene* scene) = 0;
	void RemoveRigidActor(PxScene* scene);

	//---------------------------------------
	// Properties
	//---------------------------------------
	inline const PxVec3 GetMinimum() { return minimum; };
	inline const PxVec3 GetMaximum() { return maximum; };
	inline const PxTransform GetPose() { return pPxActor->getGlobalPose().getNormalized(); };
	inline void SetPose(const PxTransform& pose) { pPxActor->setGlobalPose(pose, true); };

	//---------------------------------------
	// Constructors
	//---------------------------------------
	PxMesh(const string& meshPath, int meshId, int objId,
		const PxCooking* cooking, const PxMaterial* material);
	PxMesh(const PxMesh& copy);
	~PxMesh();
};
