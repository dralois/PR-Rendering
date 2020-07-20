#pragma once

#pragma warning(push, 0)
#include <boost/filesystem.hpp>

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
	std::string logLevel;
	bool storeBlend;
	Eigen::Vector2i resolution;
	boost::filesystem::path outputDir;
	boost::filesystem::path pluginDir;
	std::vector<boost::filesystem::path> shaderDirs;

	// Config
	int iterCount;
	int objPerSim;
	int maxImages;
	const rapidjson::Document& jsonConfig;

	// Paths
	boost::filesystem::path meshesPath, tempPath, finalPath, scenePath;

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	inline void SetOutputDir(const boost::filesystem::path& dir) { outputDir = dir; }
	inline const boost::filesystem::path& GetOutputDir() const { return outputDir; }
	inline void SetScenePath(const boost::filesystem::path& path) { scenePath = path; }
	inline const boost::filesystem::path& GetScenePath() const { return scenePath; }
	
	inline Eigen::Vector2i GetResolution() const { return resolution; }
	inline bool GetStoreBlend() const { return storeBlend; }
	inline const boost::filesystem::path& GetLogLevel() const { return logLevel; }
	inline const boost::filesystem::path& GetPluginDir() const { return pluginDir; }
	inline const std::vector<boost::filesystem::path>& GetShaderDirs() const { return shaderDirs; }

	inline int GetIterationCount() const { return iterCount; }
	inline int GetObjectsPerSimulation() const { return objPerSim; }
	inline int GetMaxImageCount() const { return maxImages; }

	inline const boost::filesystem::path& GetMeshesPath() const { return meshesPath; }
	inline const boost::filesystem::path& GetTemporaryPath() const { return tempPath; }
	inline const boost::filesystem::path& GetFinalPath() const { return finalPath; }

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
		AddEigenVector<Eigen::Vector2i>(writer, resolution);

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
		jsonConfig(NULL)
	{
	}

	Settings(const rapidjson::Document& jsonConfig) :
		jsonConfig(jsonConfig)
	{
		// Init paths
		meshesPath = boost::filesystem::path(jsonConfig["meshes_path"].GetString());
		finalPath = boost::filesystem::path(jsonConfig["final_path"].GetString());
		tempPath = boost::filesystem::path(jsonConfig["temp_path"].GetString());
		// Init simulation stuff
		iterCount = jsonConfig["scene_iterations"].GetInt();
		objPerSim = jsonConfig["simulation_objects"].GetInt();
		maxImages = jsonConfig["max_images"].GetInt();
		// Init render stuff
		logLevel = jsonConfig["log_level"].GetString();
		storeBlend = jsonConfig["store_blend"].GetBool();
		pluginDir = boost::filesystem::path(jsonConfig["plugin_bl"].GetString());
		shaderDirs.push_back(boost::filesystem::path(jsonConfig["shaders_bl"].GetString()));
		resolution = Eigen::Vector2i(jsonConfig["render_width"].GetInt(), jsonConfig["render_height"].GetInt());
	}
};