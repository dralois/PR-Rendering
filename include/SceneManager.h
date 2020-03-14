#pragma once

#pragma warning(push, 0)
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#include <eigen3/Eigen/Dense>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include "PxPhysicsAPI.h"

#include "ai.h"
#pragma warning(pop)

#include "../render/include/render.h"

#include <AiMesh.h>
#include <PxMeshConvex.h>
#include <PxMeshTriangle.h>

using namespace Eigen;
using namespace physx;
using namespace std;

//---------------------------------------
// Camera intrinsics
//---------------------------------------
struct Camera
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
	int shapeId;
	int objId;
	char* name;
	std::vector<float> vecBBox;
	std::vector<float> vecPos;
	std::vector<float> vecRot;
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
	PxScene* pScene;
	PxCooking* pCooking;
	PxMaterial* pMaterial;
	PxRigidDynamic* pSceneRigidbody;
	vector<pair<PxMeshConvex*, PxRigidDynamic*>> dicObjsRigidbodies;

	// Rendering
	Render* pRenderer;
	AtNode* aiCamera;
	AtNode* aiOptions;
	AtNode* aiDriver;
	AtArray* aiArrOutputs;
	AtNode* aiShaderDepthObj;
	AtNode* aiShaderDepthScene;
	AtNode* aiShaderBlendImage;

	// Meshes
	vector<PxMeshConvex*> vecpPhysxObjs;
	vector<AiMesh*> vecpArnoldObjs;
	PxMeshTriangle* pPhysxScene;
	AiMesh* pArnoldScene;

	// Camera
	Camera camIntrinsicScene, camIntrinsicsRender;
	vector<string> vecCameraPoses;
	vector<string> vecCameraImages;
	Matrix4f camMat;
	Vector3f camPos;

	// Objects
	class ObjectInfo;
	vector<ObjectInfo*> vecpCurrObjs;

	// OpenCV
	cv::Mat cvMask, cvScene, cvRend, cvSceneD, cvBodiesS, cvBodiesD;

	// Files
	std::ofstream ANNOTATIONS_FILE;
	rapidjson::Document* CONFIG_FILE;

	// Other
	int startCount;
	int objsPerSim;
	int sceneCount;
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
	bool X_RenderObjsDepth() const;
	void X_RenderObjsLabel() const;

	// Helpers
	void X_GetFilesInDir(string dir, float varThreshold);
	float X_ComputeImageVariance(const cv::Mat& image) const;
	bool X_CheckIfImageCenter(const ObjectInfo& info) const;
	bool X_CalculateObjsMask();

	// Blending & Camera
	void X_LoadCamMat(float fx, float fy, float ox, float oy);
	void X_LoadCamIntrinsics();
	void X_BlendDepth();
	void X_BlendLabel();
	void X_BlendImage();

	// Annotation
	void X_SaveAnnotationPose(BodyAnnotation& ann, const Vector3f& pos, const Quaterniond& rot) const;
	void X_SaveAnnotations(const cv::Mat& seg, const cv::Mat& segMasked);

public:
	//---------------------------------------
	// Methods
	//---------------------------------------
	inline void SetScenePath(string path) { scenePath = path; };
	bool Run(int iterations, int maxCount);

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

//---------------------------------------
// Saves position and rotation
//---------------------------------------
class SceneManager::ObjectInfo
{
private:
	//---------------------------------------
	// Fields
	//---------------------------------------
	Vector3f pos;
	Quaterniond rot;

public:
	//---------------------------------------
	// Fields
	//---------------------------------------
	int shapeId;
	int objSimId;

	//---------------------------------------
	// Methods
	//---------------------------------------
	inline void SetPose(Vector3f pos, Quaterniond rot) { pos = pos; rot = rot; };
	inline Quaterniond GetRot() const { return rot; };
	inline Vector3f GetPos() const { return pos; };
	inline string GetName() const { return "body" + to_string(shapeId) + "_" + to_string(objSimId); };

	//---------------------------------------
	// Constructors
	//---------------------------------------
	ObjectInfo(int shapeId, int objSimId);
};
