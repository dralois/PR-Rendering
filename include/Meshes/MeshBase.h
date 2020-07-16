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
	int objId;
	unsigned int meshId;
	string meshPath;
	string texturePath;

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
	inline int GetMeshId() { return meshId; };
	inline int GetObjId() { return objId; };
	inline void SetObjId(int id) { objId = id; };
	inline const string& GetMeshPath() { return meshPath; }
	inline const string& GetTexturePath() { return texturePath; }
	inline const string GetName() { return "mesh_" + to_string(meshId) + "_" + to_string(objId); };

	//---------------------------------------
	// Constructors
	//---------------------------------------
	MeshBase(const string& meshPath, const string& texturePath, int meshId);
	MeshBase(const string& meshPath, int meshId);
	MeshBase(const MeshBase& copy);
	~MeshBase();
};
