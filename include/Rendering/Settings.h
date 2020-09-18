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
public:
	//---------------------------------------
	// Settings groups
	//---------------------------------------

	// Blender rendering
	struct RenderEngine
	{
		bool StoreBlend;
		std::string LogLevel;
		ModifiablePath PluginPath;
		std::vector<ModifiablePath> ShaderDirs;
		Intrinsics CustomIntrinsics;
		float RenderScale;
	};

	// Image filter paramters
	struct BlurDetection
	{
		float EdgeThreshold;
		float EdgeWeak;
		float EdgeStrong;
		float EdgeFactor;
		float FrequencyFactor;
	};

	// Simulation limits
	struct Simulation
	{
		int SimulationObjects;
		int SimulationSteps;
		int SceneIterations;
		int BatchSize;
		int SceneLimit;
		int TotalLimit;
	};

	// Object spawning & forces
	struct Spawning
	{
		Eigen::Vector3f SpawnMin;
		Eigen::Vector3f SpawnMax;
		Eigen::Vector3f VelocityMax;
		Eigen::Vector3f TorqueMax;
		float ApplyProbability;
	};

private:
	//---------------------------------------
	// Fields
	//---------------------------------------

	// Groups
	RenderEngine engineSettings;
	BlurDetection filterSettings;
	Simulation simSettings;
	Spawning spawnSettings;

	// Paths
	ModifiablePath meshesPath, tempPath, finalPath, scenePath;

	// Config file
	rapidjson::Document jsonConfig;

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	inline Settings::RenderEngine GetEngineSettings() const { return engineSettings; }
	inline Settings::BlurDetection GetFilterSettings() const { return filterSettings; }
	inline Settings::Simulation GetSimulationSettings() const { return simSettings; }
	inline Settings::Spawning GetSpawnSettings() const { return spawnSettings; }

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
		AddString(writer, engineSettings.LogLevel);

		writer.Key("storeBlend");
		writer.Bool(engineSettings.StoreBlend);

		writer.Key("pluginPath");
		AddString(writer, engineSettings.PluginPath.string());

		writer.Key("shaderDirs");
		writer.StartArray();
		for (const auto& curr : engineSettings.ShaderDirs)
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
		engineSettings(),
		filterSettings(),
		simSettings(),
		spawnSettings()
	{
		// Steal json document
		json.Swap(jsonConfig);

		// Init paths
		meshesPath = boost::filesystem::absolute(ModifiablePath(SafeGet<const char*>(jsonConfig, "meshes_path")));
		finalPath = boost::filesystem::absolute(ModifiablePath(SafeGet<const char*>(jsonConfig, "final_path")));
		tempPath = boost::filesystem::absolute(ModifiablePath(SafeGet<const char*>(jsonConfig, "temp_path")));

		// Init blur detection settings
		filterSettings.EdgeThreshold = SafeGet<float>(jsonConfig, "edge_threshold");
		filterSettings.EdgeWeak = SafeGet<float>(jsonConfig, "edge_weak");
		filterSettings.EdgeStrong = SafeGet<float>(jsonConfig, "edge_strong");
		filterSettings.EdgeFactor = SafeGet<float>(jsonConfig, "edge_factor");
		filterSettings.FrequencyFactor = SafeGet<float>(jsonConfig, "frequency_factor");

		// Init simulation settings
		simSettings.SceneIterations = SafeGet<int>(jsonConfig, "scene_iterations");
		simSettings.SimulationObjects = SafeGet<int>(jsonConfig, "simulation_objects");
		simSettings.SimulationSteps = SafeGet<int>(jsonConfig, "simulation_steps");
		simSettings.BatchSize = SafeGet<int>(jsonConfig, "batch_size");
		simSettings.SceneLimit = SafeGet<int>(jsonConfig, "scene_limit");
		simSettings.TotalLimit = SafeGet<int>(jsonConfig, "total_limit");

		// Init spawning settings
		rapidjson::Value minSpawn = SafeGetArray(jsonConfig, "spawn_min");
		rapidjson::Value maxSpawn = SafeGetArray(jsonConfig, "spawn_max");
		rapidjson::Value velocityMax = SafeGetArray(jsonConfig, "velocity_max");
		rapidjson::Value torqueMax = SafeGetArray(jsonConfig, "torque_max");
		spawnSettings.SpawnMin = SafeGetEigenVector<Eigen::Vector3f>(minSpawn);
		spawnSettings.SpawnMax = SafeGetEigenVector<Eigen::Vector3f>(maxSpawn);
		spawnSettings.VelocityMax = SafeGetEigenVector<Eigen::Vector3f>(velocityMax).cwiseAbs();
		spawnSettings.TorqueMax = SafeGetEigenVector<Eigen::Vector3f>(torqueMax).cwiseAbs();
		spawnSettings.ApplyProbability = SafeGet<float>(jsonConfig, "apply_probability");

		// Init render settings
		engineSettings.LogLevel = SafeGet<const char*>(jsonConfig, "log_level");
		engineSettings.StoreBlend = SafeGet<bool>(jsonConfig, "store_blend");
		engineSettings.PluginPath = boost::filesystem::absolute(ModifiablePath(SafeGet<const char*>(jsonConfig, "blenderseed_path")));
		engineSettings.ShaderDirs.push_back(boost::filesystem::absolute(ModifiablePath(SafeGet<const char*>(jsonConfig, "shaders_path"))));
		engineSettings.RenderScale = MAX(SafeGet<float>(jsonConfig, "render_scale"), 0.1);

		// Init custom intrinsics
		rapidjson::Value intrf = SafeGetArray(jsonConfig, "intrinsics_f");
		rapidjson::Value intro = SafeGetArray(jsonConfig, "intrinsics_o");
		rapidjson::Value intrr = SafeGetArray(jsonConfig, "intrinsics_r");
		engineSettings.CustomIntrinsics.SetFocalLenght(SafeGetEigenVector<Eigen::Vector2f>(intrf));
		engineSettings.CustomIntrinsics.SetPrincipalPoint(SafeGetEigenVector<Eigen::Vector2f>(intro));
		engineSettings.CustomIntrinsics.SetResolution(SafeGetEigenVector<Eigen::Vector2i>(intrr));

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
