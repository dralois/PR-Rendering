#include "AiMesh.h"

#include <iostream>

#pragma warning(push, 0)
#include <eigen3/Eigen/Dense>
#pragma warning(pop)

//---------------------------------------
// Convert vector to arnold array
//---------------------------------------
template <class T>
AtArray* AiMesh::X_VectorToAiArray(const vector<T>& input, const size_t size, const uint8_t type)
{
	return AiArrayConvert(size, 1, type, input.data());
}

//---------------------------------------
// Load & create the base node
//---------------------------------------
void AiMesh::X_CreateBaseNode()
{
	// Check if node has been loaded from file
	baseNode = AiNodeLookUpByName(GetBaseName().c_str());

	// If not loaded
	if (baseNode == NULL)
	{
		// Create base mesh node
		baseNode = AiNode("procedural", GetBaseName().c_str());
		AiNodeSetStr(baseNode, "filename", meshPath.c_str());
	}

	// Disable the base mesh node
	AiNodeSetDisabled(baseNode, true);
}

//---------------------------------------
// Create arnold mesh
//---------------------------------------
void AiMesh::CreateMesh(const vector<float>& posIn, const vector<float>& rotIn, float scale)
{
	// Try to find node
	AtNode* currMesh = AiNodeLookUpByName(GetName().c_str());

	// If mesh not created
	if (currMesh == NULL)
	{
		if(isScene)
		{
			// Set node, scene exists only once
			currMesh = baseNode;
		}
		else
		{
			// Clone node with new name
			currMesh = AiNodeClone(baseNode, AtString(GetName().c_str()));
		}
		// Enable the node
		AiNodeSetDisabled(currMesh, false);
	}

	// Create rotation matrix
	AtMatrix aiMatRot = AiM4Identity();
	Eigen::Quaternionf quatRot = Eigen::Quaternionf(&rotIn[0]);
	Eigen::Matrix3f matRot = quatRot.normalized().toRotationMatrix();
	// Column major -> Row major
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			aiMatRot[i][j] = matRot(j, i);
		}
	}

	// Create translation matrix
	AtVector vecPos(posIn[0], posIn[1], posIn[2]);
	AtMatrix aiMatPos = AiM4Translation(vecPos);

	// Create scaling matrix
	AtVector vecScale(0.01f * scale, 0.01f * scale, 0.01f * scale);
	AtMatrix aiMatScale = AiM4Scaling(vecScale);

	// Create and save object to world matrix in actual node
	AtMatrix aiMatTrans = AiM4Mult(AiM4Mult(aiMatPos, aiMatRot), aiMatScale);
	AiNodeSetMatrix(currMesh, "matrix", aiMatTrans);
}

//---------------------------------------
// Cleanup arnold mesh
//---------------------------------------
void AiMesh::DestroyMesh(const string& nodeName)
{
	// Find and destroy mesh node
	AtNode* node = AiNodeLookUpByName(nodeName.c_str());
	// Try to destroy it
	AiNodeDestroy(node);
}

//---------------------------------------
// Get name of the base node
//---------------------------------------
const string AiMesh::GetBaseName()
{
	size_t last = meshPath.find_last_of("/") + 1;
	string base = meshPath.substr(last, meshPath.length() - last);
	return base;
}

//---------------------------------------
// Constructor with texture
//---------------------------------------
AiMesh::AiMesh(const string& meshPath, const string& texturePath, int meshId, int objId) :
	MeshBase(meshPath, texturePath, meshId, objId)
{
	X_CreateBaseNode();
}

//---------------------------------------
// Constructor without texture
//---------------------------------------
AiMesh::AiMesh(const string& meshPath, int meshId, int objId) :
	MeshBase(meshPath, meshId, objId)
{
	X_CreateBaseNode();
}
