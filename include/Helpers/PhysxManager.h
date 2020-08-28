#pragma once

#pragma warning(push, 0)
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

#if _DEBUG || DEBUG
	physx::PxPvd* pPxPvd;
#endif //_DEBUG || DEBUG
	physx::PxPhysics* pPxPhysics;
	physx::PxCooking* pPxCooking;
	physx::PxMaterial* pPxMaterial;
	physx::PxFoundation* pPxFoundation;
	physx::PxCudaContextManager* pPxCuda;
	physx::PxDefaultAllocator pxAllocator;
	physx::PxDefaultErrorCallback pxErrorCallback;

	//---------------------------------------
	// Constructors
	//---------------------------------------

	PxManager() :
#if _DEBUG || DEBUG
		pPxPvd(NULL),
#endif //_DEBUG || DEBUG
		pPxPhysics(NULL),
		pPxCooking(NULL),
		pPxMaterial(NULL),
		pPxFoundation(NULL),
		pPxCuda(NULL),
		pxAllocator(),
		pxErrorCallback()
	{
	}

public:
	//---------------------------------------
	// Singleton specifics
	//---------------------------------------

	PxManager(const PxManager&) = delete;
	PxManager(PxManager&&) = delete;
	PxManager& operator=(const PxManager&) = delete;
	PxManager& operator=(PxManager&&) = delete;

	//---------------------------------------
	// Properties
	//---------------------------------------

	// Singleton instance
	inline static PxManager& GetInstance()
	{
		static PxManager instance;
		return instance;
	}

	inline const physx::PxCooking* GetCooker() { return pPxCooking; }
	inline const physx::PxMaterial* GetMaterial() { return pPxMaterial; }
	inline physx::PxCudaContextManager* GetCudaManager() { return pPxCuda; }

	//---------------------------------------
	// Methods
	//---------------------------------------

	inline void InitPhysx()
	{
		using namespace physx;

		// Create foundation
		pPxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, pxAllocator, pxErrorCallback);
		pPxFoundation->setErrorLevel(PxErrorCode::eMASK_ALL);

#if _DEBUG || DEBUG
		// Setup Physx Visual Debugger
		pPxPvd = PxCreatePvd(*pPxFoundation);
		PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 100);
		pPxPvd->connect(*transport, PxPvdInstrumentationFlag::eDEBUG);
#endif // DEBUG || _DEBUG

		// Create API
#if _DEBUG || DEBUG
		pPxPhysics = PxCreateBasePhysics(PX_PHYSICS_VERSION, *pPxFoundation, PxTolerancesScale(), true, pPxPvd);
#else
		pPxPhysics = PxCreateBasePhysics(PX_PHYSICS_VERSION, *pPxFoundation, PxTolerancesScale(), true, NULL);
#endif // DEBUG || _DEBUG

		// Create cuda manager for GPU rigidbodies
		PxCudaContextManagerDesc cuda;
		cuda.interopMode = PxCudaInteropMode::NO_INTEROP;
		pPxCuda = PxCreateCudaContextManager(*pPxFoundation, cuda);
		// Make sure it worked
		if(pPxCuda)
		{
			if(!pPxCuda->contextIsValid())
			{
				PX_RELEASE(pPxCuda);
			}
		}

		// Create mesh cooking module
		PxCookingParams params(pPxPhysics->getTolerancesScale());
		params.meshPreprocessParams |= PxMeshPreprocessingFlag::eWELD_VERTICES;
		params.meshPreprocessParams |= PxMeshPreprocessingFlag::eFORCE_32BIT_INDICES;
		params.midphaseDesc = PxMeshMidPhase::eBVH34;
		params.meshWeldTolerance = 0.01f;
		params.buildGPUData = true;
		pPxCooking = PxCreateCooking(PX_PHYSICS_VERSION, *pPxFoundation, params);

		// Enable extensions
#if _DEBUG || DEBUG
		PxInitExtensions(*pPxPhysics, pPxPvd);
#else
		PxInitExtensions(*pPxPhysics, NULL);
#endif // DEBUG || _DEBUG

		// Create default material
		pPxMaterial = pPxPhysics->createMaterial(0.5f, 0.5f, 0.2f);
	}

	inline static physx::PxFilterFlags CCDFilterShader(
		physx::PxFilterObjectAttributes attributes0,
		physx::PxFilterData filterData0,
		physx::PxFilterObjectAttributes attributes1,
		physx::PxFilterData filterData1,
		physx::PxPairFlags& pairFlags,
		const void* constantBlock,
		physx::PxU32 constantBlockSize
	)
	{
		// Enable continuous collision detection (CCD)
		pairFlags = physx::PxPairFlag::eSOLVE_CONTACT;
		pairFlags |= physx::PxPairFlag::eDETECT_CCD_CONTACT;
		pairFlags |= physx::PxPairFlag::eDETECT_DISCRETE_CONTACT;
		return physx::PxFilterFlags();
	}

	inline void DeletePhysx()
	{
		using namespace physx;

		// Free physics
		PX_RELEASE(pPxMaterial);
		PxCloseExtensions();
		PX_RELEASE(pPxPhysics);
		PX_RELEASE(pPxCooking);
#if _DEBUG || DEBUG
		if (pPxPvd)
		{
			PxPvdTransport* pTransp = pPxPvd->getTransport();
			PX_RELEASE(pPxPvd);
			PX_RELEASE(pTransp);
		}
#endif // DEBUG || _DEBUG
		PX_RELEASE(pPxCuda);
		PX_RELEASE(pPxFoundation);
	}
};
