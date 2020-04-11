#pragma once

#pragma warning(push, 0)
#include <eigen3/Eigen/Dense>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#include <eigen3/Eigen/Dense>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include "PxPhysicsAPI.h"

#include <ai.h>
#pragma warning(pop)

#include "../OpenGLLib/include/render.h"

#include <AiMesh.h>
#include <PxMeshConvex.h>
#include <PxMeshTriangle.h>

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
	AtNode* aiCamera;
	AtNode* aiOptions;
	AtNode* aiDriver;
	AtArray* aiArrOutputs;
	AtNode* aiShaderDepthObj;
	AtNode* aiShaderDepthScene;
	AtNode* aiShaderBlendImage;

	// Meshes (Reference)
	const vector<PxMeshConvex*> vecpPxMeshObjs;
	const vector<AiMesh*> vecpAiMeshObjs;

	// Meshes (Copies)
	vector<PxMeshConvex*> vecpPxMeshCurrObjs;
	vector<AiMesh*> vecpAiMeshCurrObjs;
	PxMeshTriangle* pPxMeshScene;
	AiMesh* pAiMeshScene;

	// Camera
	Intrinsics intrOriginal, intrCustom;
	vector<string> vecCameraPoses;
	vector<string> vecCameraImages;
	Matrix4f matCamera;
	float nearClip, farClip;
	bool useCustomIntr = false;

	// Objects
	vector<ObjectInfo> vecCurrObjs;

	// OpenCV
	cv::Mat cvMask, cvScene, cvRend, cvSceneD, cvBodiesS, cvBodiesD;

	// Files
	std::ofstream ANNOTATIONS_FILE;
	const rapidjson::Document* CONFIG_FILE;

	// Other
	int poseCount;
	int imageCount;
	int objsPerSim;
	string scenePath;

	//---------------------------------------
	// Methods
	//---------------------------------------

	// PhysX
	void X_PxCreateScene();
	void X_PxCreateObjs();
	void X_PxRunSim(float timestep, int stepCount) const;
	void X_PxSaveSimResults();
	void X_PxDestroy();

	// Arnold
	void X_AiCreateObjs();
	void X_AiDestroy();

	// Rendering
	vector<tuple<cv::Mat, cv::Mat>> X_RenderSceneDepth() const;
	bool X_RenderObjsDepth();
	void X_RenderObjsLabel();
	void X_RenderImageBlend();

	// Helpers
	void X_GetImagesToProcess(const string& dir, float varThreshold);
	bool X_CheckIfImageCenter(const ObjectInfo& info) const;
	void X_LoadCameraExtrinsics(const Intrinsics& intr);
	void X_LoadCameraIntrinsics();

	// Image processing
	float X_CvComputeImageVariance(const cv::Mat& image) const;
	bool X_CvComputeObjsMask();
	void X_CvBlendDepth();
	void X_CvBlendLabel();

	// Annotation
	void X_SaveAnnotationPose(BodyAnnotation& ann, const Vector3f& pos, const Quaternionf& rot) const;
	void X_SaveAnnotations(const cv::Mat& seg, const cv::Mat& segMasked);

public:
	//---------------------------------------
	// Methods
	//---------------------------------------
	bool Run(int sceneIters, int maxImages);

	//---------------------------------------
	// Properties
	//---------------------------------------
	inline void SetScenePath(const string& path) { scenePath = path; };

	//---------------------------------------
	// Constructors
	//---------------------------------------
	SceneManager(PxCpuDispatcher* pPxDispatcher, const PxCooking* pPxCooking, const PxMaterial* pPxMaterial,
		AtNode* aiCamera, AtNode* aiOptions, AtNode* aiDriver, AtArray* aiOutputArray,
		const vector<PxMeshConvex*> vecPhysxObjs, const vector<AiMesh*> vecArnoldObjs,
		int startCount, int objPerSim, const rapidjson::Document* CONFIG_FILE,
		AtNode* aiShaderObjDepth, AtNode* aiShaderSceneDepth, AtNode* aiShaderBlend);
	~SceneManager();
};
