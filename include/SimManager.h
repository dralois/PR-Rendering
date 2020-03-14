#pragma once

#include "SceneManager.h"

#pragma warning(push, 0)
#include <dirent.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#pragma warning(pop)

// Manages loading and simulation
class SimManager
{
private:
	//---------------------------------------
	// Fields
	//---------------------------------------

	// Meshes
	vector<PxMeshConvex*> vecMeshPhysX;
	vector<AiMesh*> vecMesh3D;
	// PhysX
#ifdef  _DEBUG
	PxPvd* pPxPvdServer;
#endif
	PxScene* pPxScene;
	PxCooking* pPxCooking;
	PxMaterial* pPxMaterial;
	PxDefaultCpuDispatcher* pPxDispatcher;
	PxDefaultAllocator pxAllocator;
	PxDefaultErrorCallback pxErrorCallback;
	// Arnold
	AtNode* aiRenderCamera;
	AtNode* aiRenderOptions;
	AtNode* aiOutputDriver;
	AtArray* aiOuputArray;
	AtNode* aiShaderObjectDepth;
	AtNode* aiShaderSceneDepth;
	AtNode* aiShaderBlend;
	// Other
	rapidjson::Document CONFIG_FILE;
	vector<string> vecSceneFolders;
	SceneManager* currScene;
	int sceneCount = 0;
	int imagesCount = 0;

	//---------------------------------------
	// Methods
	//---------------------------------------
	void X_SaveSceneFolders(string path);

public:
	//---------------------------------------
	// Methods
	//---------------------------------------
	void InitPhysx();
	void InitArnold();
	void LoadConfig(string config_path);
	void LoadMeshes();
	int RunSimulation();

	//---------------------------------------
	// Properties
	//---------------------------------------
	inline string GetFinalPath() { return CONFIG_FILE["final_imgs_path"].GetString(); };
	inline string GetTemporaryPath() { return CONFIG_FILE["temp_files_path"].GetString(); };

	//---------------------------------------
	// Construtors
	//---------------------------------------
	~SimManager();
};
