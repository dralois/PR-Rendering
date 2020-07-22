#pragma once

#pragma warning(push, 0)
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include <Eigen/Dense>

#include <Helpers/JSONUtils.h>
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
	rapidjson::Document jsonConfig;
	Settings* pRenderSettings;
	std::vector<boost::filesystem::path> vecSceneFolders;

	//---------------------------------------
	// Methods
	//---------------------------------------

	void X_SaveSceneFolders(const boost::filesystem::path& path);
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
