#pragma once

#include "SceneManager.h"

#include <dirent.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

// Manages loading and simulation
class SimManager
{
private:
	// Meshes
	vector<PxMeshConvex*> vecMeshPhysX;
	vector<AiMesh*> vecMesh3D;
	// PhysX
	PxPhysics* gPhysics;
	PxScene* gScene;
	PxCooking* gCooking;
	PxFoundation* gFoundation;
	PxMaterial* gMaterial;
	PxDefaultCpuDispatcher* gDispatcher;
	PxDefaultAllocator gAllocator;
	PxDefaultErrorCallback gErrorCallback;
	// Arnold
	AtNode* renderCamera;
	AtNode* renderOptions;
	AtNode* outputDriver;
	AtArray* ouputArray;
	AtNode* shaderObjectDepth;
	AtNode* shaderSceneDepth;
	AtNode* shaderBlend;
	// Other
	rapidjson::Document CONFIG_FILE;
	vector<string> vecSceneFolders;
	SceneManager* currScene;
	int sceneCount = 0;
	int imagesCount = 0;

	// Methods
	void X_GetSceneFolders(string path);

public:
	~SimManager();

	// Methods
	void InitPhysx();
	void InitArnold();
	void LoadConfig(string config_path);
	void LoadMeshes();
	int RunSimulation();

	// Properties
	string GetFinalPath();
	string GetTemporaryPath();
};
