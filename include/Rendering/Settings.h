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
	Eigen::Vector2i resolution;
	string outputDir;
	string pluginDir;
	vector<string> shaderDirs;

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	inline void SetLogLevel(const string& level) { logLevel = level; }
	inline const string& GetLogLevel() { return logLevel; }
	inline void SetOutputDir(const string& dir) { outputDir = dir; }
	inline const string& GetOutputDir() { return outputDir; }
	inline void SetPluginDir(const string& dir) { pluginDir = dir; }
	inline const string& GetPluginDir() { return pluginDir; }
	inline void SetShaderDirs(const vector<string>& dirs) { shaderDirs = dirs; }
	inline const vector<string>& GetShaderDirs() { return shaderDirs; }
	inline void SetStoreBlend(bool saveScene) { storeBlend = saveScene; }
	inline bool GetStoreBlend() { return storeBlend; }

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
		RenderfileData::AddEigenVector<Vector2i>(writer, resolution);

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

	Settings() :
		logLevel("error"),
		storeBlend(false),
		resolution(Vector2i(1920, 1080)),
		outputDir(""),
		pluginDir(""),
		shaderDirs(NULL)
	{
	}
};