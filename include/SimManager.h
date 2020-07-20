#pragma once

#pragma warning(push, 0)
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include <Eigen/Dense>

#include <SceneManager.h>
#pragma warning(pop)

//---------------------------------------
// Manages loading and simulation
//---------------------------------------
class SimManager
{
private:
	//---------------------------------------
	// Fields
	//---------------------------------------

	// PhysX
#ifdef _DEBUG || DEBUG
	physx::PxPvd* pPxPvd;
#endif
	physx::PxPhysics* pPxPhysics;
	physx::PxCooking* pPxCooking;
	physx::PxMaterial* pPxMaterial;
	physx::PxFoundation* pPxFoundation;
	physx::PxDefaultCpuDispatcher* pPxDispatcher;
	physx::PxDefaultAllocator pxAllocator;
	physx::PxDefaultErrorCallback pxErrorCallback;

	// Meshes
	std::vector<PxMeshConvex*> vecpPxMesh;
	std::vector<RenderMesh*> vecpRenderMesh;

	// Other
	rapidjson::Document jsonConfig;
	Settings* pRenderSettings;
	std::vector<boost::filesystem::path> vecSceneFolders;

	//---------------------------------------
	// Methods
	//---------------------------------------

	void X_SaveSceneFolders(const boost::filesystem::path& path);
	void X_InitPhysx();
	void X_LoadConfig(const boost::filesystem::path& configPath);
	void X_LoadMeshes();

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	inline const Settings* GetSettings() const { return pRenderSettings; }

	//---------------------------------------
	// Methods
	//---------------------------------------

	int RunSimulation();

	//---------------------------------------
	// Construtors
	//---------------------------------------

	SimManager(const boost::filesystem::path& configPath);
	~SimManager();
};
