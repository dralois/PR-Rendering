#pragma once

#include <vector>
#include <string>

using namespace std;

//---------------------------------------
// Base class for all meshes
//---------------------------------------
class MeshBase
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------
	int meshId;
	int objId;
	string meshPath;
	string texturePath;

	vector<float> vecVertices;
	vector<int> vecIndices;
	vector<float> vecUVs;
	vector<float> vecNormals;

	float metalness = 0.1f;

	float xMin = 1e8;
	float yMin = 1e8;
	float xMax = -1e8;
	float yMax = -1e8;

	//---------------------------------------
	// Methods
	//---------------------------------------
	bool LoadFile(float scale);
	void StoreFile(const vector<int>& idxs, int nIdxs, const vector<float>& verts, int nVerts, const string& ext) const;

public:
	//---------------------------------------
	// Properties
	//---------------------------------------
	inline const string GetName() { return "mesh_" + to_string(meshId) + "_" + to_string(objId); };
	inline int GetObjId() { return objId; };
	inline void SetObjId(int objId_) { objId = objId_; };
	inline int GetMeshId() { return meshId; };
	inline float GetXMax() { return xMax; };
	inline float GetXMin() { return xMin; };
	inline float GetYMax() { return yMax; };
	inline float GetYMin() { return yMin; };
	inline float GetMetallic() { return metalness; }
	inline void SetMetallic(float metallic) { metalness = metallic; }

	//---------------------------------------
	// Constructors
	//---------------------------------------
	MeshBase(const string& meshPath, const string& texturePath, int meshId, int objId);
	MeshBase(const string& meshPath, int meshId, int objId);
};
