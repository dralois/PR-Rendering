#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <eigen3/Eigen/Dense>

using namespace std;

static const aiScene* pScene;

// Base mesh class
class MeshBase
{
private:
	// Fields
	string path;
	string texture_path;

protected:
	// Fields
	int mesh_id;
	float scale;
	vector<float> vecVertices;
	vector<int> vecIndices;
	vector<float> vecUVs;
	vector<float> vecNormals;

public:
	// Fields
	float xmin;
	float ymin;
	float xmax;
	float ymax;
	bool doubleNorms = false;
	bool calculateBounds = false;

	// Methods
	bool LoadFile();

	// Properties
	int GetID();

	// Constructors
	MeshBase(string path, string texture_path, int mesh_id, float scale);
	MeshBase(string path, int mesh_id, float scale);
};
