#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

//#define EXPORT_TO_FILE

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
	unsigned int meshId;
	int objId;
	string meshPath;
	string texturePath;

	float metalness = 0.1f;
	float meshScale = 1.0f;

	vector<float> vecVertices;
	vector<int> vecIndices;
	vector<float> vecNormals;
	vector<float> vecUVs;

	//---------------------------------------
	// Methods
	//---------------------------------------
	bool X_LoadFile();
	void X_StoreFile(const string& ext) const;
	virtual void X_ExportMesh() = 0;

public:
	//---------------------------------------
	// Methods
	//---------------------------------------
	virtual void CreateMesh() = 0;

	//---------------------------------------
	// Properties
	//---------------------------------------
	inline const string GetName() { return "mesh_" + to_string(meshId) + "_" + to_string(objId); };

	inline const int GetMeshId() { return meshId; };

	inline const int GetObjId() { return objId; };
	inline void SetObjId(int id) { objId = id; };

	inline const float GetMetallic() { return metalness; };
	inline void SetMetallic(float metallic) { metalness = metallic; };

	inline const float GetScale() { return meshScale; };
	virtual void SetScale(float scale) = 0;

	//---------------------------------------
	// Constructors
	//---------------------------------------
	MeshBase(const string& meshPath, const string& texturePath, int meshId);
	MeshBase(const string& meshPath, int meshId);
	MeshBase(const MeshBase& copy);
	~MeshBase();
};
