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

#include <dirent.h>
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
	float fx;
	float fy;
	float ox;
	float oy;
};

//---------------------------------------
// Annotation info
//---------------------------------------
struct BodyAnnotation
{
	int meshId;
	int labelId;
	char* name;
	std::vector<float> vecBBox;
	std::vector<float> vecPos;
	std::vector<float> vecRot;
};

//---------------------------------------
// Spawned object info
//---------------------------------------
struct ObjectInfo
{
	int meshId;
	int objId;
	string objName;
	Vector3f pos;
	Quaternionf rot;
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
	PxCooking* pPxCooking;
	PxMaterial* pPxMaterial;
	PxRigidDynamic* pPxSceneRigidbody;

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
	vector<PxMeshConvex*> vecpPxMeshObjs;
	vector<AiMesh*> vecpAiMeshObjs;

	// Meshes (Copies)
	vector<pair<PxMeshConvex*, PxRigidDynamic*>> vecpPxMeshCurrObjs;
	vector<AiMesh*> vecpAiMeshCurrObj;
	PxMeshTriangle* pPxMeshScene;
	AiMesh* pAiMeshScene;

	// Camera
	Intrinsics intrCameraScene, intrCameraRender;
	vector<string> vecCameraPoses;
	vector<string> vecCameraImages;
	Matrix4f matCamera;
	Vector3f posCamera;

	// Objects
	vector<ObjectInfo> vecCurrObjs;

	// OpenCV
	cv::Mat cvMask, cvScene, cvRend, cvSceneD, cvBodiesS, cvBodiesD;

	// Files
	std::ofstream ANNOTATIONS_FILE;
	rapidjson::Document* CONFIG_FILE;

	// Other
	int imageCount;
	int objsPerSim;
	int poseCount;
	string scenePath;

	//---------------------------------------
	// Methods
	//---------------------------------------

	// PhysX
	void X_PxCreateScene();
	void X_PxCreateObjs();
	void X_PxRunSim();
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
	void X_LoadCamMat(float fx, float fy, float ox, float oy);
	void X_LoadCamIntrinsics();

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
	SceneManager(PxScene* pPxScene, PxCooking* pPxCooking, PxMaterial* pPxMaterial,
							AtNode* aiCamera, AtNode* aiOptions, AtNode* aiDriver, AtArray* aiOutputArray,
							vector<PxMeshConvex*> vecPhysxObjs, vector<AiMesh*> vecArnoldObjs,
							int startCount, int objPerSim, rapidjson::Document* CONFIG_FILE,
							AtNode* aiShaderObjDepth, AtNode* aiShaderSceneDepth, AtNode* aiShaderBlend);
	~SceneManager();
};
