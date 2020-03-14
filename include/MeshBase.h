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

	//---------------------------------------
	// Methods
	//---------------------------------------
	bool LoadFile();

public:
	//---------------------------------------
	// Fields
	//---------------------------------------
	float xMin = 1e8;
	float yMin = 1e8;
	float xMax = -1e8;
	float yMax = -1e8;
	bool doubleNorms = false;
	bool calculateBounds = false;

	//---------------------------------------
	// Properties
	//---------------------------------------
	inline int GetID() { return meshId; };

	//---------------------------------------
	// Constructors
	//---------------------------------------
	MeshBase(string meshPath, string texturePath, int meshId, float scale);
	MeshBase(string meshPath, int meshId, float scale);
};
