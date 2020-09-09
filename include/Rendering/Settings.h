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
	float renderScale;
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
	Eigen::Vector3f spawnMin;
	Eigen::Vector3f spawnMax;
	Eigen::Vector3f maxVelocity;
	Eigen::Vector3f maxTorque;
	float applyProbability;

	// Paths
	ModifiablePath meshesPath, tempPath, finalPath, scenePath;

	// Config file
	rapidjson::Document jsonConfig;

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	// Blender rendering
	inline bool GetStoreBlend() const { return storeBlend; }
	inline float GetRenderScale() const { return renderScale; }
	inline ModifiablePath GetLogLevel() const { return logLevel; }
	inline ModifiablePath GetPluginDir() const { return pluginPath; }
	inline std::vector<ModifiablePath> GetShaderDirs() const { return shaderDirs; }
	inline Intrinsics GetIntrinsics() const { return customIntrinsics; }

	// Simulation
	inline int GetIterationCount() const { return iterCount; }
	inline int GetObjectsPerSimulation() const { return objPerSim; }
	inline int GetStepsPerSimulation() const { return stepsPerSim; }
	inline int GetRenderBatchSize() const { return batchSize; }
	inline int GetMaxImageCount() const { return maxImages; }

	// Random forces & spawning
	inline Eigen::Vector3f GetSpawnMin() const { return spawnMin; }
	inline Eigen::Vector3f GetSpawnMax() const { return spawnMax; }
	inline Eigen::Vector3f GetMaxVelocity() const { return maxVelocity; }
	inline Eigen::Vector3f GetMaxTorque() const { return maxTorque; }
	inline float GetApplyProbability() const { return applyProbability; }

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
		ModifiablePath bodyPath(isFinal ? GetFinalPath() : GetTemporaryPath());
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
	inline rapidjson::Document& GetJSONConfig() { return jsonConfig; }

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
		rapidjson::Document&& json
	) :
		customIntrinsics()
	{
		// Steal json document
		json.Swap(jsonConfig);

		// Init paths
		meshesPath = boost::filesystem::absolute(ModifiablePath(SafeGet<const char*>(jsonConfig, "meshes_path")));
		finalPath = boost::filesystem::absolute(ModifiablePath(SafeGet<const char*>(jsonConfig, "final_path")));
		tempPath = boost::filesystem::absolute(ModifiablePath(SafeGet<const char*>(jsonConfig, "temp_path")));

		// Init simulation settings
		iterCount = SafeGet<int>(jsonConfig, "scene_iterations");
		objPerSim = SafeGet<int>(jsonConfig, "simulation_objects");
		stepsPerSim = SafeGet<int>(jsonConfig, "simulation_steps");
		batchSize = SafeGet<int>(jsonConfig, "batch_size");
		maxImages = SafeGet<int>(jsonConfig, "max_images");

		// Init distribution settings
		rapidjson::Value minSpawn = SafeGetArray(jsonConfig, "spawn_min");
		rapidjson::Value maxSpawn = SafeGetArray(jsonConfig, "spawn_max");
		rapidjson::Value velocityMax = SafeGetArray(jsonConfig, "velocity_max");
		rapidjson::Value torqueMax = SafeGetArray(jsonConfig, "torque_max");
		spawnMin = SafeGetEigenVector<Eigen::Vector3f>(minSpawn);
		spawnMax = SafeGetEigenVector<Eigen::Vector3f>(maxSpawn);
		maxVelocity = SafeGetEigenVector<Eigen::Vector3f>(velocityMax).cwiseAbs();
		maxTorque = SafeGetEigenVector<Eigen::Vector3f>(torqueMax).cwiseAbs();
		applyProbability = SafeGet<float>(jsonConfig, "apply_probability");

		// Init render settings
		logLevel = SafeGet<const char*>(jsonConfig, "log_level");
		storeBlend = SafeGet<bool>(jsonConfig, "store_blend");
		pluginPath = boost::filesystem::absolute(ModifiablePath(SafeGet<const char*>(jsonConfig, "blenderseed_path")));
		shaderDirs.push_back(boost::filesystem::absolute(ModifiablePath(SafeGet<const char*>(jsonConfig, "shaders_path"))));
		renderScale = MAX(SafeGet<float>(jsonConfig, "render_scale"), 0.1);

		// Init custom intrinsics
		rapidjson::Value intrf = SafeGetArray(jsonConfig, "intrinsics_f");
		rapidjson::Value intro = SafeGetArray(jsonConfig, "intrinsics_o");
		rapidjson::Value intrr = SafeGetArray(jsonConfig, "intrinsics_r");
		customIntrinsics.SetFocalLenght(SafeGetEigenVector<Eigen::Vector2f>(intrf));
		customIntrinsics.SetPrincipalPoint(SafeGetEigenVector<Eigen::Vector2f>(intro));
		customIntrinsics.SetResolution(SafeGetEigenVector<Eigen::Vector2i>(intrr));

		// Cleanup
		SafeDeleteArray(minSpawn);
		SafeDeleteArray(maxSpawn);
		SafeDeleteArray(velocityMax);
		SafeDeleteArray(torqueMax);
		SafeDeleteArray(intrf);
		SafeDeleteArray(intro);
		SafeDeleteArray(intrr);
	}

	~Settings()
	{
		// Cleanup json document
		jsonConfig.GetAllocator().Clear();
	}

	// No copy / move allowed
	Settings(const Settings& copy) = delete;
	Settings(Settings&& other) = delete;
};
