#pragma once

#pragma warning(push, 0)
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include <SceneManager.h>
#pragma warning(pop)

using namespace rapidjson;

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
	PxPvd* pPxPvd;
#endif
	PxPhysics* pPxPhysics;
	PxCooking* pPxCooking;
	PxMaterial* pPxMaterial;
	PxFoundation* pPxFoundation;
	PxDefaultCpuDispatcher* pPxDispatcher;
	PxDefaultAllocator pxAllocator;
	PxDefaultErrorCallback pxErrorCallback;

	// Meshes
	vector<PxMeshConvex*> vecpPxMesh;
	vector<RenderMesh*> vecpRenderMesh;

	// Other
	Document jsonConfig;
	Settings* pRenderSettings;
	vector<string> vecSceneFolders;

	//---------------------------------------
	// Methods
	//---------------------------------------

	void X_SaveSceneFolders(const string& path);
	void X_InitPhysx();
	void X_LoadConfig(const string& configPath);
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

	SimManager(const string& configPath);
	~SimManager();
};
