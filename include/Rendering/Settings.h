#pragma once

#pragma warning(push, 0)
#include <Renderfile.h>
#pragma warning(pop)

using namespace std;
using namespace Eigen;

//---------------------------------------
// Settings data wrapper for rendering
//---------------------------------------
class Settings : public RenderfileData
{
private:
	//---------------------------------------
	// Fields
	//---------------------------------------

	string logLevel;
	bool storeBlend;
	Vector2i resolution;
	string outputDir;
	string pluginDir;
	vector<string> shaderDirs;

	int iterCount;
	int objPerSim;
	int maxImages;
	const Document& jsonConfig;

	string meshesPath, tempPath, finalPath, scenePath;

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	inline void SetOutputDir(const string& dir) { outputDir = dir; }
	inline const string& GetOutputDir() const { return outputDir; }
	inline void SetScenePath(const string& path) { scenePath = path; }
	inline const string& GetScenePath() const { return scenePath; }
	
	inline Vector2i GetResolution() const { return resolution; }
	inline bool GetStoreBlend() const { return storeBlend; }
	inline const string& GetLogLevel() const { return logLevel; }
	inline const string& GetPluginDir() const { return pluginDir; }
	inline const vector<string>& GetShaderDirs() const { return shaderDirs; }

	inline int GetIterationCount() const { return iterCount; }
	inline int GetObjectsPerSimulation() const { return objPerSim; }
	inline int GetMaxImageCount() const { return maxImages; }

	inline const string& GetMeshesPath() const { return meshesPath; }
	inline const string& GetTemporaryPath() const { return tempPath; }
	inline const string& GetFinalPath() const { return finalPath; }

	inline const Document& GetJSONConfig() const { return jsonConfig; }

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void AddToJSON(PrettyWriter<std::stringstream>& writer) override
	{
		writer.StartObject();

		writer.Key("logLevel");
		AddString(writer, logLevel);

		writer.Key("storeBlend");
		writer.Bool(storeBlend);

		writer.Key("resolution");
		AddEigenVector<Vector2i>(writer, resolution);

		writer.Key("outputDir");
		AddString(writer, outputDir);

		writer.Key("pluginDir");
		AddString(writer, pluginDir);

		writer.Key("shaderDirs");
		writer.StartArray();
		for (string curr : shaderDirs)
		{
			AddString(writer, curr);
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

	Settings(const Document& jsonConfig) :
		jsonConfig(jsonConfig)
	{
		// Init paths
		meshesPath = jsonConfig["meshes_path"].GetString();
		finalPath = jsonConfig["final_path"].GetString();
		tempPath = jsonConfig["temp_path"].GetString();
		// Init simulation stuff
		iterCount = jsonConfig["scene_iterations"].GetInt();
		objPerSim = jsonConfig["simulation_objects"].GetInt();
		maxImages = jsonConfig["max_images"].GetInt();
		// Init render stuff
		logLevel = jsonConfig["log_level"].GetString();
		storeBlend = jsonConfig["store_blend"].GetBool();
		pluginDir = jsonConfig["plugin_bl"].GetString();
		shaderDirs.push_back(jsonConfig["shaders_bl"].GetString());
		resolution = Vector2i(jsonConfig["render_width"].GetInt(), jsonConfig["render_height"].GetInt());
	}
};