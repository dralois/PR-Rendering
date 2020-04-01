#pragma once

#include "SceneManager.h"

#pragma warning(push, 0)
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
	vector<PxMeshConvex*> vecpPxMesh;
	vector<AiMesh*> vecpAiMesh;

	// PhysX
#ifdef  _DEBUG
	PxPvd* pPxPvdServer;
#endif
	PxScene* pPxScene;
	PxCooking* pPxCooking;
	PxMaterial* pPxMaterial;
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

	// Other
	rapidjson::Document CONFIG_FILE;
	vector<string> vecSceneFolders;
	int sceneCount = 0;
	int imagesCount = 0;

	//---------------------------------------
	// Methods
	//---------------------------------------
	void X_SaveSceneFolders(const string& path);

public:
	//---------------------------------------
	// Methods
	//---------------------------------------
	void InitPhysx();
	void InitArnold();
	void LoadConfig(const string& configPath);
	void LoadMeshes();
	int RunSimulation();

	//---------------------------------------
	// Properties
	//---------------------------------------
	inline const string GetFinalPath() { return CONFIG_FILE["final_imgs_path"].GetString(); };
	inline const string GetTemporaryPath() { return CONFIG_FILE["temp_files_path"].GetString(); };

	//---------------------------------------
	// Construtors
	//---------------------------------------
	~SimManager();
};
