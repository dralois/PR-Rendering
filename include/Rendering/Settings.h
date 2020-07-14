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
	bool depthOnly;
	string outputDir;
	string pluginDir;
	vector<string> shaderDirs;

public:

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

		writer.Key("depthOnly");
		writer.Bool(depthOnly);

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

	Settings(const string& logLevel, bool storeBlend, Vector2i resolution, bool depthOnly,
		const string& outputDir, const string& pluginDir, const vector<string>& shaderDirs) :
		logLevel(logLevel),
		storeBlend(storeBlend),
		resolution(resolution),
		depthOnly(depthOnly),
		outputDir(outputDir),
		pluginDir(pluginDir),
		shaderDirs(shaderDirs)
	{
	}

	Settings(Vector2i resolution, const string& outputDir, const string& pluginDir, const vector<string>& shaderDirs) :
		Settings("error", false, resolution, false, outputDir, pluginDir, shaderDirs)
	{
	}
};