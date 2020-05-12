#pragma once

#pragma warning(push, 0)
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

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
	PxPvd* pPxPvd;
#endif
	PxPhysics* pPxPhysics;
	PxCooking* pPxCooking;
	PxMaterial* pPxMaterial;
	PxFoundation* pPxFoundation;
	PxDefaultCpuDispatcher* pPxDispatcher;
	PxDefaultAllocator pxAllocator;
	PxDefaultErrorCallback pxErrorCallback;

	// Arnold
	AtNode* aiRenderCamera;
	AtNode* aiRenderOptions;
	AtNode* aiOutputDriver;
	AtArray* aiOutputArray;
	AtNode* aiShaderObjectDepth;
	AtNode* aiShaderSceneDepth;
	AtNode* aiShaderBlend;

	// Meshes
	vector<PxMeshConvex*> vecpPxMesh;
	vector<AiMesh*> vecpAiMesh;

	// Other
	rapidjson::Document CONFIG_FILE;
	vector<string> vecSceneFolders;
	int sceneCount = 0;
	int imagesCount = 0;

	//---------------------------------------
	// Methods
	//---------------------------------------
	void X_SaveSceneFolders(const string& path);
	void X_InitPhysx();
	void X_InitArnold();
	void X_LoadConfig(const string& configPath);
	void X_LoadMeshes();

public:
	//---------------------------------------
	// Methods
	//---------------------------------------
	int RunSimulation();

	//---------------------------------------
	// Properties
	//---------------------------------------
	inline const string GetFinalPath() { return CONFIG_FILE["final_imgs_path"].GetString(); };
	inline const string GetTemporaryPath() { return CONFIG_FILE["temp_files_path"].GetString(); };

	//---------------------------------------
	// Construtors
	//---------------------------------------
	SimManager(const string& configPath);
	~SimManager();
};
