#pragma once

#include <vector>
#include <string>
#include <random>
#include <thread>

#pragma warning(push, 0)
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>

#include <Helpers/Annotations.h>
#include <Helpers/ImageProcessing.h>
#include <Helpers/JSONUtils.h>
#include <Helpers/PathUtils.h>
#include <Helpers/PhysxManager.h>

#include <BlenderLib/BlenderRenderer.h>

#include <HDRLib/LightEstimator.h>

#include <Meshes/RenderMesh.h>
#include <Meshes/PxMeshConvex.h>
#include <Meshes/PxMeshTriangle.h>

#include <Shaders/All.h>

#include <Rendering/Camera.h>
#include <Rendering/Intrinsics.h>
#include <Rendering/Light.h>
#include <Rendering/Settings.h>
#include <Rendering/Shader.h>
#include <Rendering/Texture.h>
#pragma warning(pop)

//---------------------------------------
// Simulates and renderes a scene
//---------------------------------------
class SceneManager
{
private:
	//---------------------------------------
	// Fields
	//---------------------------------------

	// Rendering
	Camera camBlueprint;

	// Meshes (Blueprint)
	const std::vector<PxMeshConvex*> vecpPxMeshObjs;
	const std::vector<RenderMesh*> vecpRenderMeshObjs;

	// Other
	const Settings& renderSettings;

	// Multithreading
	int imgCountDepth;
	int imgCountUnoccluded;
	int imgCountScene;

	//---------------------------------------
	// Methods
	//---------------------------------------

	// Scene mesh

	RenderMesh X_CreateSceneMesh() const;
	
	PxMeshTriangle X_PxCreateSceneMesh() const;

	// Simulation

	physx::PxScene* X_PxCreateSimulation(
		PxMeshTriangle& sceneMesh,
		float& maxDist
	) const;

	std::vector<PxMeshConvex> X_PxCreateObjs(
		std::default_random_engine& generator,
		PxMeshTriangle& sceneMesh,
		physx::PxScene* simulation
	) const;

	std::vector<RenderMesh> X_PxSaveSimResults(
		std::vector<PxMeshConvex>& simulationObjs
	) const;

	void X_PxRunSim(
		physx::PxScene* simulation,
		float timestep,
		int stepCount
	) const;

	// Renderfile creation

	void X_ConvertToRenderfile(
		JSONWriterRef writer,
		std::vector<RenderMesh>& meshes,
		std::vector<Camera>& cams,
		std::vector<Light>& lights
	) const;

	void X_BuildSceneDepth(
		JSONWriterRef writer,
		RenderMesh& sceneMesh,
		std::vector<RenderMesh>& meshes,
		std::vector<Camera>& cams,
		std::vector<Light>& lights,
		std::vector<Texture>& results,
		float maxDist
	) const;

	void X_BuildObjectsDepth(
		JSONWriterRef writer,
		RenderMesh& sceneMesh,
		std::vector<RenderMesh>& meshes,
		std::vector<Camera>& cams,
		std::vector<Light>& lights,
		std::vector<Texture>& results,
		float maxDist
	) const;

	void X_BuildObjectsLabel(
		JSONWriterRef writer,
		RenderMesh& sceneMesh,
		std::vector<RenderMesh>& meshes,
		std::vector<Camera>& cams,
		std::vector<Light>& lights,
		std::vector<Texture>& results
	) const;

	void X_BuildObjectsPBR(
		JSONWriterRef writer,
		RenderMesh& sceneMesh,
		std::vector<RenderMesh>& meshes,
		std::vector<Camera>& cams,
		std::vector<Light>& lights,
		std::vector<Texture>& results
	) const;

	void X_BuildObjectsAO(
		JSONWriterRef writer,
		RenderMesh& sceneMesh,
		std::vector<RenderMesh>& meshes,
		std::vector<Camera>& cams,
		std::vector<Light>& lights,
		std::vector<Texture>& results
	) const;

	// Blender rendering

	std::vector<Mask> X_RenderDepthMasks(
		Blender::BlenderRenderer* renderer,
		int threadID,
		RenderMesh& sceneMesh,
		std::vector<RenderMesh>& meshes,
		std::vector<Camera>& cams,
		std::vector<Light>& lights,
		boost::mutex* syncPoint,
		float maxDist
	) const;

	void X_RenderSegments(
		Blender::BlenderRenderer* renderer,
		int threadID,
		AnnotationsManager* annotations,
		RenderMesh& sceneMesh,
		std::vector<RenderMesh>& meshes,
		std::vector<Camera>& cams,
		std::vector<Light>& lights,
		std::vector<Mask>& masks
	) const;

	void X_RenderPBRBlend(
		Blender::BlenderRenderer* renderer,
		int threadID,
		RenderMesh& sceneMesh,
		std::vector<RenderMesh>& meshes,
		std::vector<Camera>& cams,
		std::vector<Light>& lights,
		std::vector<Mask>& masks,
		std::vector<SceneImage>& sceneRGBs
	) const;

	// Other

	void X_CleanupScene(
		physx::PxScene* simulation,
		AnnotationsManager* annotations,
		Blender::BlenderRenderer* renderer,
		int threadID
	) const;

	std::vector<Light> X_PlaceLights(
		Eigen::Vector3f min,
		Eigen::Vector3f max
	) const;

	void X_ComputeImagesToProcess(
		ReferencePath dir
	) const;

	void X_EstimateLighting(
		ReferencePath dir
	) const;

	std::vector<SceneImage> X_GetImagesToProcess(
		ReferencePath dir
	) const;

	void X_ProcessThread(
		Blender::BlenderRenderer* renderer,
		boost::mutex* syncPoint,
		int threadID
	);

public:
	//---------------------------------------
	// Methods
	//---------------------------------------

	int ProcessNext(int imageCount);

	//---------------------------------------
	// Constructors
	//---------------------------------------

	SceneManager(
		const Settings& settings,
		const std::vector<PxMeshConvex*>& vecPxMeshObjs,
		const std::vector<RenderMesh*>& vecRenderMeshObjs
	);
};
