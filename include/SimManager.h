#pragma once

#pragma warning(push, 0)
#include <rapidjson/filereadstream.h>

#include <Eigen/Dense>

#include <Helpers/JSONUtils.h>
#include <Helpers/PathUtils.h>
#include <Helpers/PhysxManager.h>

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

	// Meshes
	std::vector<PxMeshConvex*> vecpPxMesh;
	std::vector<RenderMesh*> vecpRenderMesh;

	// Config
	Settings* pRenderSettings;
	rapidjson::Document jsonConfig;
	std::vector<ModifiablePath> vecSceneFolders;

	//---------------------------------------
	// Methods
	//---------------------------------------

	void X_SaveSceneFolders(ReferencePath path);
	void X_LoadConfig(ReferencePath configPath);
	void X_LoadMeshes();

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	inline const Settings* GetSettings() const { return pRenderSettings; }

	//---------------------------------------
	// Methods
	//---------------------------------------

	void RunSimulation();

	//---------------------------------------
	// Construtors
	//---------------------------------------

	SimManager(ReferencePath configPath);
	~SimManager();
};
