#pragma once

#pragma warning(push, 0)
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#include <Eigen/Dense>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include <Helpers/Annotations.h>
#include <Helpers/ImageProcessing.h>
#include <Helpers/JSONUtils.h>
#include <Helpers/PhysxManager.h>

#include <OpenGLLib/render.h>

#include <BlenderLib/BlenderRenderer.h>

#include <Meshes/RenderMesh.h>
#include <Meshes/PxMeshConvex.h>
#include <Meshes/PxMeshTriangle.h>

#include <Rendering/Camera.h>
#include <Rendering/Light.h>
#include <Rendering/Settings.h>
#pragma warning(pop)

typedef std::vector<std::tuple<cv::Mat, cv::Mat>> RenderResult;

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

	// Renderers
	Renderer::Render* pRenderer;
	Blender::BlenderRenderer* pBlender;

	// Rendering
	Settings renderSettings;
	Camera renderCam;
	std::vector<Light> vecLights;

	// Inputs
	std::vector<boost::filesystem::path> vecCameraPoses;
	std::vector<boost::filesystem::path> vecCameraImages;

	// Meshes (Blueprint)
	const std::vector<PxMeshConvex*> vecpPxMeshObjs;
	const std::vector<RenderMesh*> vecpRenderMeshObjs;

	// Meshes (Instances)
	std::vector<PxMeshConvex*> vecpPxMeshCurrObjs;
	std::vector<RenderMesh*> vecpRenderMeshCurrObjs;
	PxMeshTriangle* pPxMeshScene;
	RenderMesh* pRenderMeshScene;

	// Other
	const Settings* pRenderSettings;
	AnnotationsManager* pAnnotations;

	//---------------------------------------
	// Methods
	//---------------------------------------

	// Simulation
	void X_PxCreateScene();
	void X_PxCreateObjs();
	void X_PxRunSim(float timestep, int stepCount) const;
	void X_PxSaveSimResults();

	// Rendering
	RenderResult X_RenderSceneDepth() const;
	void X_RenderObjsDepth();
	void X_RenderObjsLabel();
	void X_RenderImageBlend();

	// Other
	void X_GetImagesToProcess(const boost::filesystem::path& dir, float varThreshold);
	void X_CleanupScene();

public:
	//---------------------------------------
	// Methods
	//---------------------------------------

	int Run(int imageCount);

	//---------------------------------------
	// Constructors
	//---------------------------------------

	SceneManager(const Settings* settings,
		const std::vector<Light>& vecLights,
		const std::vector<PxMeshConvex*>& vecPhysxObjs,
		const std::vector<RenderMesh*>& vecArnoldObjs);
	~SceneManager();
};
