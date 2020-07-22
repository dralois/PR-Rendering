#pragma once

#pragma warning(push, 0)
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <PxPhysicsAPI.h>
#pragma warning(pop)

#define PX_RELEASE(x) if(x != NULL) { x->release(); x = NULL; }

//---------------------------------------
// Physx helper singleton
//---------------------------------------
class PxManager
{
private:
	//---------------------------------------
	// Fields
	//---------------------------------------

#ifdef _DEBUG || DEBUG
	physx::PxPvd* pPxPvd;
#endif // _DEBUG || DEBUG
	physx::PxPhysics* pPxPhysics;
	physx::PxCooking* pPxCooking;
	physx::PxMaterial* pPxMaterial;
	physx::PxFoundation* pPxFoundation;
	static physx::PxDefaultAllocator pxAllocator;
	static physx::PxDefaultErrorCallback pxErrorCallback;

	//---------------------------------------
	// Constructors
	//---------------------------------------

	PxManager()
	{
		using namespace physx;

		// Create foundation
		pPxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, pxAllocator, pxErrorCallback);
		pPxFoundation->setErrorLevel(PxErrorCode::eMASK_ALL);

#ifdef _DEBUG || DEBUG
		// Setup Physx Visual Debugger
		pPxPvd = PxCreatePvd(*pPxFoundation);
		PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 100);
		pPxPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
#endif // DEBUG || _DEBUG

		// Create API
#ifdef _DEBUG || DEBUG
		pPxPhysics = PxCreateBasePhysics(PX_PHYSICS_VERSION, *pPxFoundation, PxTolerancesScale(), true, pPxPvd);
#else
		pPxPhysics = PxCreateBasePhysics(PX_PHYSICS_VERSION, *pPxFoundation, PxTolerancesScale(), true, NULL);
#endif // DEBUG || _DEBUG

		// Create mesh cooking module
		PxCookingParams params(pPxPhysics->getTolerancesScale());
		params.meshPreprocessParams |= PxMeshPreprocessingFlag::eWELD_VERTICES;
		params.meshPreprocessParams |= PxMeshPreprocessingFlag::eFORCE_32BIT_INDICES;
		params.meshWeldTolerance = 0.01f;
		params.midphaseDesc = PxMeshMidPhase::eBVH34;
		pPxCooking = PxCreateCooking(PX_PHYSICS_VERSION, *pPxFoundation, params);

		// Enable extensions
#ifdef _DEBUG || DEBUG
		PxInitExtensions(*pPxPhysics, pPxPvd);
#else
		PxInitExtensions(*pPxPhysics, NULL);
#endif // DEBUG || _DEBUG

		// Create default material
		pPxMaterial = pPxPhysics->createMaterial(0.6f, 0.6f, 0.0f);
	}

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	// Singleton instance
	inline static PxManager& GetInstance()
	{
		static PxManager* instance = new PxManager();
		return *instance;
	}

	inline const physx::PxCooking* GetCooker() { return pPxCooking; }
	inline const physx::PxMaterial& GetMaterial() { return *pPxMaterial; }

	//---------------------------------------
	// Methods
	//---------------------------------------

	inline void DeletePhysx()
	{
		using namespace physx;

		// Free physics
		PX_RELEASE(pPxMaterial);
		PxCloseExtensions();
		PX_RELEASE(pPxPhysics);
		PX_RELEASE(pPxCooking);
#ifdef _DEBUG || DEBUG
		if (pPxPvd)
		{
			PxPvdTransport* transp = pPxPvd->getTransport();
			PX_RELEASE(pPxPvd);
			PX_RELEASE(transp);
		}
#endif // DEBUG || _DEBUG
		PX_RELEASE(pPxFoundation);
	}
};
