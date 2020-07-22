#pragma once

#pragma warning(push, 0)
#include <boost/filesystem.hpp>

#include <rapidjson/document.h>

#include <Helpers/JSONUtils.h>

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
	boost::filesystem::path outputDir;
	boost::filesystem::path pluginDir;
	std::vector<boost::filesystem::path> shaderDirs;
	Intrinsics customIntrinsics;

	// Simulation
	int iterCount;
	int objPerSim;
	int maxImages;

	// Paths
	boost::filesystem::path meshesPath, tempPath, finalPath, scenePath;

	// Config file
	const rapidjson::Document& jsonConfig;

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	// Blender rendering
	inline bool GetStoreBlend() const { return storeBlend; }
	inline Eigen::Vector2i GetRenderResolution() const { return renderResolution; }
	inline const boost::filesystem::path& GetLogLevel() const { return logLevel; }
	inline const boost::filesystem::path& GetPluginDir() const { return pluginDir; }
	inline const std::vector<boost::filesystem::path>& GetShaderDirs() const { return shaderDirs; }
	inline const Intrinsics& GetIntrinsics() const { return customIntrinsics; }

	// Simulation
	inline int GetIterationCount() const { return iterCount; }
	inline int GetObjectsPerSimulation() const { return objPerSim; }
	inline int GetMaxImageCount() const { return maxImages; }

	// Paths
	inline void SetOutputDir(const boost::filesystem::path& dir) { outputDir = dir; }
	inline const boost::filesystem::path& GetOutputDir() const { return outputDir; }
	inline void SetScenePath(const boost::filesystem::path& path) { scenePath = path; }
	inline const boost::filesystem::path& GetScenePath() const { return scenePath; }
	inline const boost::filesystem::path& GetMeshesPath() const { return meshesPath; }
	inline const boost::filesystem::path& GetTemporaryPath() const { return tempPath; }
	inline const boost::filesystem::path& GetFinalPath() const { return finalPath; }

	// Config file
	inline const rapidjson::Document& GetJSONConfig() const { return jsonConfig; }

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void AddToJSON(rapidjson::PrettyWriter<rapidjson::StringStream>& writer) override
	{
		writer.StartObject();

		writer.Key("logLevel");
		AddString(writer, logLevel);

		writer.Key("storeBlend");
		writer.Bool(storeBlend);

		writer.Key("resolution");
		AddEigenVector<Eigen::Vector2i>(writer, renderResolution);

		writer.Key("outputDir");
		AddString(writer, outputDir.string());

		writer.Key("pluginDir");
		AddString(writer, pluginDir.string());

		writer.Key("shaderDirs");
		writer.StartArray();
		for (auto curr : shaderDirs)
		{
			AddString(writer, curr.string());
		}
		writer.EndArray();
	}

	//---------------------------------------
	// Constructors
	//---------------------------------------

	Settings():
		jsonConfig(NULL),
		customIntrinsics()
	{
	}

	Settings(const rapidjson::Document& jsonConfig) :
		jsonConfig(jsonConfig),
		customIntrinsics()
	{
		// Init paths
		meshesPath = boost::filesystem::path(SafeGet<const char*>(jsonConfig, "meshes_path"));
		finalPath = boost::filesystem::path(SafeGet<const char*>(jsonConfig, "final_path"));
		tempPath = boost::filesystem::path(SafeGet<const char*>(jsonConfig, "temp_path"));
		// Init simulation settings
		iterCount = SafeGet<int>(jsonConfig, "scene_iterations");
		objPerSim = SafeGet<int>(jsonConfig, "simulation_objects");
		maxImages = SafeGet<int>(jsonConfig, "max_images");
		// Init render settings
		logLevel = SafeGet<const char*>(jsonConfig, "log_level");
		storeBlend = SafeGet<bool>(jsonConfig, "store_blend");
		pluginDir = boost::filesystem::path(SafeGet<const char*>(jsonConfig, "plugin_bl"));
		shaderDirs.push_back(boost::filesystem::path(SafeGet<const char*>(jsonConfig, "shaders_bl")));
		renderResolution = Eigen::Vector2i(SafeGet<int>(jsonConfig, "render_width"), SafeGet<int>(jsonConfig, "render_height"));
		// Init custom intrinsics
		customIntrinsics.SetFocalLenght(Eigen::Vector2f(SafeGet<float>(jsonConfig, "fx"), SafeGet<float>(jsonConfig, "fy")));
		customIntrinsics.SetPrincipalPoint(Eigen::Vector2f(SafeGet<float>(jsonConfig, "ox"), SafeGet<float>(jsonConfig, "oy")));
		customIntrinsics.SetWidth(SafeGet<int>(jsonConfig, "width"));
		customIntrinsics.SetHeight(SafeGet<int>(jsonConfig, "height"));
	}
};
