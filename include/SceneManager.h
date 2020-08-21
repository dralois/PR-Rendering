#pragma once

#include <vector>
#include <string>
#include <random>

#pragma warning(push, 0)
#include <boost/algorithm/string.hpp>

#include <Helpers/Annotations.h>
#include <Helpers/ImageProcessing.h>
#include <Helpers/JSONUtils.h>
#include <Helpers/PathUtils.h>
#include <Helpers/PhysxManager.h>

#include <BlenderLib/BlenderRenderer.h>

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

	// PhysX
	physx::PxScene* pPxScene;

	// Rendering
	float maxDist;
	Camera camBlueprint;
	std::vector<Light*> vecpLights;

	// Meshes (Blueprint)
	const std::vector<PxMeshConvex*> vecpPxMeshObjs;
	const std::vector<RenderMesh*> vecpRenderMeshObjs;

	// Meshes (Instances)
	std::vector<PxMeshConvex*> vecpPxMeshCurrObjs;
	std::vector<RenderMesh*> vecpRenderMeshCurrObjs;
	PxMeshTriangle* pPxMeshScene;
	RenderMesh* pRenderMeshScene;

	// Other
	Settings* pRenderSettings;
	AnnotationsManager* pAnnotations;
	Blender::BlenderRenderer* pBlender;
	std::default_random_engine randGen;

	//---------------------------------------
	// Methods
	//---------------------------------------

	// Simulation
	void X_PxSaveSimResults();
	void X_PxCreateScene();
	void X_PxCreateObjs();
	void X_PxRunSim(
		float timestep,
		int stepCount
	) const;

	// Renderfile creation
	void X_ConvertToRenderfile(
		JSONWriterRef writer,
		std::vector<RenderMesh*>& meshes,
		std::vector<Camera>& cams
	);
	void X_BuildSceneDepth(
		JSONWriterRef writer,
		std::vector<Camera>& cams,
		std::vector<Texture>& results,
		int start
	);
	void X_BuildObjectsDepth(
		JSONWriterRef writer,
		std::vector<Camera>& cams,
		std::vector<Texture>& results,
		int start
	);
	void X_BuildObjectsLabel(
		JSONWriterRef writer,
		std::vector<Camera>& cams,
		std::vector<Texture>& results,
		int start
	);
	void X_BuildObjectsPBR(
		JSONWriterRef writer,
		std::vector<Camera>& cams,
		std::vector<Texture>& results,
		int start
	);

	// Blender rendering
	std::vector<Mask> X_RenderDepthMasks(
		std::vector<Camera>& cams,
		int start
	);
	void X_RenderSegments(
		std::vector<Camera>& cams,
		std::vector<Mask>& masks,
		int start
	);
	void X_RenderPBRBlend(
		std::vector<Camera>& cams,
		std::vector<Mask>& masks,
		std::vector<SceneImage>& sceneRGBs,
		int start
	);

	// Other
	void X_CleanupScene();
	std::vector<SceneImage> X_GetImagesToProcess(
		ReferencePath dir,
		float varThreshold
	) const;

public:
	//---------------------------------------
	// Methods
	//---------------------------------------

	int ProcessNext(int imageCount);

	//---------------------------------------
	// Constructors
	//---------------------------------------

	SceneManager(
		Settings* settings,
		const std::vector<PxMeshConvex*>& vecPhysxObjs,
		const std::vector<RenderMesh*>& vecArnoldObjs
	);

	~SceneManager();
};
