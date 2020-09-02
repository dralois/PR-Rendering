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
	int antiAliasingFactor;
	ModifiablePath pluginPath;
	std::vector<ModifiablePath> shaderDirs;
	Intrinsics customIntrinsics;

	// Simulation
	int iterCount;
	int objPerSim;
	int stepsPerSim;
	int batchSize;
	int maxImages;

	// Random forces
	float applyProbability;
	Eigen::Vector3f maxVelocity;
	Eigen::Vector3f maxTorque;

	// Paths
	ModifiablePath meshesPath, tempPath, finalPath, scenePath;

	// Config file
	rapidjson::Document* jsonConfig;

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	// Blender rendering
	inline bool GetStoreBlend() const { return storeBlend; }
	inline Eigen::Vector2i GetRenderResolution() const { return renderResolution; }
	inline int GetAntiAliasingFactor() const { return antiAliasingFactor; }
	inline ModifiablePath GetLogLevel() const { return logLevel; }
	inline ModifiablePath GetPluginDir() const { return pluginPath; }
	inline const std::vector<ModifiablePath>& GetShaderDirs() const { return shaderDirs; }
	inline const Intrinsics& GetIntrinsics() const { return customIntrinsics; }

	// Simulation
	inline int GetIterationCount() const { return iterCount; }
	inline int GetObjectsPerSimulation() const { return objPerSim; }
	inline int GetStepsPerSimulation() const { return stepsPerSim; }
	inline int GetRenderBatchSize() const { return batchSize; }
	inline int GetMaxImageCount() const { return maxImages; }

	// Random forces
	inline float GetApplyProbability() const { return applyProbability; }
	inline Eigen::Vector3f GetMaxVelocity() const { return maxVelocity; }
	inline Eigen::Vector3f GetMaxTorque() const { return maxTorque; }

	// Paths
	inline void SetScenePath(ReferencePath path) { scenePath = path; }
	inline ModifiablePath GetScenePath() const { return scenePath; }
	inline ModifiablePath GetMeshesPath() const { return meshesPath; }
	inline ModifiablePath GetTemporaryPath() const { return tempPath; }
	inline ModifiablePath GetFinalPath() const { return finalPath; }

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
	inline const rapidjson::Document& GetJSONConfig() const { return *jsonConfig; }

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

	Settings(
		rapidjson::Document* jsonConfig
	) :
		jsonConfig(jsonConfig),
		customIntrinsics()
	{
		// Init paths
		meshesPath = boost::filesystem::absolute(ModifiablePath(SafeGet<const char*>(*jsonConfig, "meshes_path")));
		finalPath = boost::filesystem::absolute(ModifiablePath(SafeGet<const char*>(*jsonConfig, "final_path")));
		tempPath = boost::filesystem::absolute(ModifiablePath(SafeGet<const char*>(*jsonConfig, "temp_path")));

		// Init simulation settings
		iterCount = SafeGet<int>(*jsonConfig, "scene_iterations");
		objPerSim = SafeGet<int>(*jsonConfig, "simulation_objects");
		stepsPerSim = SafeGet<int>(*jsonConfig, "simulation_steps");
		batchSize = SafeGet<int>(*jsonConfig, "batch_size");
		maxImages = SafeGet<int>(*jsonConfig, "max_images");

		// Init distribution settings
		applyProbability = SafeGet<float>(*jsonConfig, "apply_probability");
		rapidjson::Value velocityMax = SafeGetArray<float>(*jsonConfig, "velocity_max");
		rapidjson::Value torqueMax = SafeGetArray<float>(*jsonConfig, "torque_max");
		maxVelocity = SafeGetEigenVector<Eigen::Vector3f>(velocityMax);
		maxTorque = SafeGetEigenVector<Eigen::Vector3f>(torqueMax);

		// Init render settings
		logLevel = SafeGet<const char*>(*jsonConfig, "log_level");
		storeBlend = SafeGet<bool>(*jsonConfig, "store_blend");
		pluginPath = boost::filesystem::absolute(ModifiablePath(SafeGet<const char*>(*jsonConfig, "blenderseed_path")));
		shaderDirs.push_back(boost::filesystem::absolute(ModifiablePath(SafeGet<const char*>(*jsonConfig, "shaders_path"))));
		rapidjson::Value res = SafeGetArray<int>(*jsonConfig, "render_resolution");
		renderResolution = SafeGetEigenVector<Eigen::Vector2i>(res);
		antiAliasingFactor = MAX(SafeGet<int>(*jsonConfig, "anti_aliasing"), 1);

		// Init custom intrinsics
		rapidjson::Value intrf = SafeGetArray<float>(*jsonConfig, "intrinsics_f");
		rapidjson::Value intro = SafeGetArray<float>(*jsonConfig, "intrinsics_o");
		rapidjson::Value intrr = SafeGetArray<int>(*jsonConfig, "intrinsics_r");
		customIntrinsics.SetFocalLenght(SafeGetEigenVector<Eigen::Vector2f>(intrf));
		customIntrinsics.SetPrincipalPoint(SafeGetEigenVector<Eigen::Vector2f>(intro));
		customIntrinsics.SetResolution(SafeGetEigenVector<Eigen::Vector2i>(intrr));

		// Cleanup
		SafeDeleteArray(velocityMax);
		SafeDeleteArray(torqueMax);
		SafeDeleteArray(intrf);
		SafeDeleteArray(intro);
		SafeDeleteArray(intrr);
		SafeDeleteArray(res);
	}
};
