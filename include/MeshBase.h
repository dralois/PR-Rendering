#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#pragma warning(push, 0)
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <eigen3/Eigen/Dense>
#pragma warning(pop)

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
	float scale;
	string meshPath;
	string texturePath;
	vector<float> vecVertices;
	vector<int> vecIndices;
	vector<float> vecUVs;
	vector<float> vecNormals;
	float xMin = 1e8;
	float yMin = 1e8;
	float xMax = -1e8;
	float yMax = -1e8;

	//---------------------------------------
	// Methods
	//---------------------------------------
	bool LoadFile(bool doubleNorms);

public:
	//---------------------------------------
	// Properties
	//---------------------------------------
	inline int GetMeshId() { return meshId; };
	inline float GetXMax() { return xMax; };
	inline float GetXMin() { return xMin; };
	inline float GetYMax() { return yMax; };
	inline float GetYMin() { return yMin; };

	//---------------------------------------
	// Constructors
	//---------------------------------------
	MeshBase(string meshPath, string texturePath, int meshId, float scale);
	MeshBase(string meshPath, int meshId, float scale);
};
