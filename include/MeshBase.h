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

	float meshScale = 1.0f;
	float metalness = 0.1f;

	vector<float> vecVertices;
	vector<int> vecIndices;
	vector<float> vecUVs;
	vector<float> vecNormals;

	//---------------------------------------
	// Methods
	//---------------------------------------
	bool X_LoadFile();
	void X_StoreFile(const vector<int>& idxs, int nIdxs, const vector<float>& verts, int nVerts, const string& ext) const;
	virtual void X_UpdateScale() = 0;

public:
	//---------------------------------------
	// Properties
	//---------------------------------------
	inline const string GetName() { return "mesh_" + to_string(meshId) + "_" + to_string(objId); };

	inline const int GetMeshId() { return meshId; };

	inline const int GetObjId() { return objId; };
	inline void SetObjId(int objId_) { objId = objId_; };

	inline const float GetMetallic() { return metalness; };
	inline void SetMetallic(float metallic) { metalness = metallic; };

	inline const float GetScale() { return meshScale; };
	inline void SetScale(float scale) { meshScale = scale; X_UpdateScale(); };

	//---------------------------------------
	// Constructors
	//---------------------------------------
	MeshBase(const string& meshPath, const string& texturePath, int meshId, int objId);
	MeshBase(const string& meshPath, int meshId, int objId);
};
