#pragma once

#pragma warning(push, 0)
#include <Helpers/JSONUtils.h>
#include <Helpers/PathUtils.h>

#include <Rendering/Intrinsics.h>
#include <Renderfile.h>
#pragma warning(pop)

//---------------------------------------
// Settings data wrapper for rendering
//---------------------------------------
class Settings : public RenderfileData
{
private:
	//---------------------------------------
	// Fields
	//---------------------------------------

	// Blender rendering
	bool storeBlend;
	std::string logLevel;
	Eigen::Vector2i renderResolution;
	ModifiablePath outputDir;
	ModifiablePath pluginPath;
	std::vector<ModifiablePath> shaderDirs;
	Intrinsics customIntrinsics;

	// Simulation
	int iterCount;
	int objPerSim;
	int maxImages;

	// Paths
	ModifiablePath meshesPath, tempPath, finalPath, scenePath;

	// Config file
	const rapidjson::Document& jsonConfig;

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	// Blender rendering
	inline bool GetStoreBlend() const { return storeBlend; }
	inline Eigen::Vector2i GetRenderResolution() const { return renderResolution; }
	inline void SetRenderResolution(Eigen::Vector2i res) { renderResolution = res; }
	inline ReferencePath GetLogLevel() const { return logLevel; }
	inline ReferencePath GetPluginDir() const { return pluginPath; }
	inline const std::vector<ModifiablePath>& GetShaderDirs() const { return shaderDirs; }
	inline const Intrinsics& GetIntrinsics() const { return customIntrinsics; }

	// Simulation
	inline int GetIterationCount() const { return iterCount; }
	inline int GetObjectsPerSimulation() const { return objPerSim; }
	inline int GetMaxImageCount() const { return maxImages; }

	// Paths
	inline void SetOutputDir(ReferencePath dir) { outputDir = dir; }
	inline ReferencePath GetOutputDir() const { return outputDir; }
	inline void SetScenePath(ReferencePath path) { scenePath = path; }
	inline ReferencePath GetScenePath() const { return scenePath; }
	inline ReferencePath GetMeshesPath() const { return meshesPath; }
	inline ReferencePath GetTemporaryPath() const { return tempPath; }
	inline ReferencePath GetFinalPath() const { return finalPath; }

	inline ModifiablePath GetImagePath(
		const std::string& category,
		int imgNum,
		bool isFinal = false
	) const
	{
		ModifiablePath bodyPath(isFinal ? GetFinalPath() :  GetTemporaryPath());
		bodyPath.append(category);
		bodyPath.append("img_" + FormatInt(imgNum) + ".png");
		return bodyPath;
	}

	inline ModifiablePath GetSceneRGBPath() const
	{
		ModifiablePath scenePath(GetScenePath());
		return scenePath.append("rgbd");
	}

	// Config file
	inline const rapidjson::Document& GetJSONConfig() const { return jsonConfig; }

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void AddToJSON(JSONWriterRef writer) override
	{
		writer.StartObject();

		writer.Key("logLevel");
		AddString(writer, logLevel);

		writer.Key("storeBlend");
		writer.Bool(storeBlend);

		writer.Key("resolution");
		AddEigenVector<Eigen::Vector2i>(writer, renderResolution);

		writer.Key("pluginPath");
		AddString(writer, pluginPath.string());

		writer.Key("shaderDirs");
		writer.StartArray();
		for (auto curr : shaderDirs)
		{
			AddString(writer, curr.string());
		}
		writer.EndArray();

		writer.EndObject();
	}

	//---------------------------------------
	// Constructors
	//---------------------------------------

	Settings():
		jsonConfig(NULL),
		customIntrinsics()
	{
	}

	Settings(
		const rapidjson::Document& jsonConfig
	) :
		jsonConfig(jsonConfig),
		customIntrinsics()
	{
		// Init paths
		meshesPath = ModifiablePath(SafeGet<const char*>(jsonConfig, "meshes_path"));
		finalPath = ModifiablePath(SafeGet<const char*>(jsonConfig, "final_path"));
		tempPath = ModifiablePath(SafeGet<const char*>(jsonConfig, "temp_path"));
		// Init simulation settings
		iterCount = SafeGet<int>(jsonConfig, "scene_iterations");
		objPerSim = SafeGet<int>(jsonConfig, "simulation_objects");
		maxImages = SafeGet<int>(jsonConfig, "max_images");
		// Init render settings
		logLevel = SafeGet<const char*>(jsonConfig, "log_level");
		storeBlend = SafeGet<bool>(jsonConfig, "store_blend");
		pluginPath = ModifiablePath(SafeGet<const char*>(jsonConfig, "plugin_bl"));
		shaderDirs.push_back(ModifiablePath(SafeGet<const char*>(jsonConfig, "shaders_bl")));
		renderResolution = Eigen::Vector2i(SafeGet<int>(jsonConfig, "render_width"), SafeGet<int>(jsonConfig, "render_height"));
		// Init custom intrinsics
		customIntrinsics.SetFocalLenght(Eigen::Vector2f(SafeGet<float>(jsonConfig, "fx"), SafeGet<float>(jsonConfig, "fy")));
		customIntrinsics.SetPrincipalPoint(Eigen::Vector2f(SafeGet<float>(jsonConfig, "ox"), SafeGet<float>(jsonConfig, "oy")));
		customIntrinsics.SetWidth(SafeGet<int>(jsonConfig, "width"));
		customIntrinsics.SetHeight(SafeGet<int>(jsonConfig, "height"));
	}
};
