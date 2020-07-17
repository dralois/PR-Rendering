#pragma once

#pragma warning(push, 0)
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#include <Eigen/Dense>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include <PxPhysicsAPI.h>

#include <OpenGLLib/render.h>

#include <Meshes/RenderMesh.h>
#include <Meshes/PxMeshConvex.h>
#include <Meshes/PxMeshTriangle.h>

#include <Rendering/Camera.h>
#include <Rendering/Light.h>
#include <Rendering/Settings.h>
#pragma warning(pop)

using namespace Eigen;
using namespace physx;
using namespace std;

//---------------------------------------
// Focal lenght & principal point
//---------------------------------------
struct Intrinsics
{
	float fx{ 0 };
	float fy{ 0 };
	float ox{ 0 };
	float oy{ 0 };
	int w{ 0 };
	int h{ 0 };
};

//---------------------------------------
// Annotation info
//---------------------------------------
struct BodyAnnotation
{
	int meshId{ 0 };
	int labelId{ 0 };
	std::vector<float> vecBBox{ };
	std::vector<float> vecPos{ };
	std::vector<float> vecRot{ };
};

//---------------------------------------
// Spawned object info
//---------------------------------------
struct ObjectInfo
{
	int meshId{ 0 };
	int objId{ 0 };
	string objName{ "" };
	Vector3f pos{ };
	Quaternionf rot{ };
	Eigen::Matrix4f mat{ };
};

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
	PxScene* pPxScene;
	PxCpuDispatcher* pPxDispatcher;
	const PxCooking* pPxCooking;
	const PxMaterial* pPxMaterial;

	// Rendering
	Renderer::Render* pRenderer;
	Settings renderSettings;
	vector<Light> vecLights;
	Camera renderCam;
	Intrinsics intrOriginal, intrCustom;
	bool useCustomIntr = false;
	vector<string> vecCameraPoses;
	vector<string> vecCameraImages;

	// Meshes (Blueprint)
	const vector<PxMeshConvex*> vecpPxMeshObjs;
	const vector<RenderMesh*> vecpAiMeshObjs;

	// Meshes (Instances)
	vector<PxMeshConvex*> vecpPxMeshCurrObjs;
	vector<RenderMesh*> vecpRenderMeshCurrObjs;
	PxMeshTriangle* pPxMeshScene;
	RenderMesh* pRenderMeshScene;

	// OpenCV
	cv::Mat cvMask, cvScene, cvRend, cvSceneD, cvBodiesS, cvBodiesD;

	// Other
	ofstream osAnnotationsFile;
	const Settings* pRenderSettings;

	//---------------------------------------
	// Methods
	//---------------------------------------

	// PhysX
	void X_PxCreateScene();
	void X_PxCreateObjs();
	void X_PxRunSim(float timestep, int stepCount) const;
	void X_PxSaveSimResults();
	
	// Rendering
	vector<tuple<cv::Mat, cv::Mat>> X_RenderSceneDepth() const;
	bool X_RenderObjsDepth();
	void X_RenderObjsLabel();
	void X_RenderImageBlend();

	// Image processing
	float X_CvComputeImageVariance(const cv::Mat& image) const;
	bool X_CvComputeObjsMask(int currPose);
	void X_CvBlendDepth(int currImage, int currPose);
	void X_CvBlendLabel(int currImage, int currPose);

	// Annotation
	void X_SaveAnnotationPose(BodyAnnotation& ann, const Vector3f& pos, const Quaternionf& rot) const;
	void X_SaveAnnotations(const cv::Mat& seg, const cv::Mat& segMasked, int currImage);

	// Other
	void X_GetImagesToProcess(const string& dir, float varThreshold);
	bool X_CheckIfImageCenter(const ObjectInfo& info) const;
	void X_LoadCameraExtrinsics(const Intrinsics& intr, int currPose);
	void X_LoadCameraIntrinsics();
	void X_CleanupScene();

public:
	//---------------------------------------
	// Methods
	//---------------------------------------

	int Run(int imageCount);

	//---------------------------------------
	// Constructors
	//---------------------------------------

	SceneManager(PxCpuDispatcher* pPxDispatcher, const PxCooking* pPxCooking, const PxMaterial* pPxMaterial,
		const vector<PxMeshConvex*> vecPhysxObjs, const vector<RenderMesh*> vecArnoldObjs, const Settings* settings);
	~SceneManager();
};
