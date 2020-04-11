#include "AiMesh.h"

#include <iostream>

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
// Changes the scale of the shape
// FIXME: Doesn't work yet
//---------------------------------------
void AiMesh::X_UpdateScale()
{
	return;

	// Try to find node
	AtNode* currMesh = AiNodeLookUpByName(GetName().c_str());

	// TODO
	if(currMesh)
	{
		AtMatrix matOld = AiNodeGetMatrix(currMesh, "matrix");
		AtMatrix scale = AiM4Scaling(AtVector(meshScale, meshScale, meshScale));
		matOld = AiM4Mult(matOld, scale);
		AiNodeSetMatrix(currMesh, "matrix", matOld);
	}
}

//---------------------------------------
// Create arnold mesh
//---------------------------------------
void AiMesh::CreateMesh(const Matrix4f& mat)
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

	// Convert
	AtMatrix aiMatTrans = AiM4Identity();
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			aiMatTrans[i][j] = mat(j, i);

	// Save
	AiNodeSetMatrix(currMesh, "matrix", aiMatTrans);
}

//---------------------------------------
// Cleanup arnold mesh
//---------------------------------------
void AiMesh::DestroyMesh()
{
	// Find and destroy this mesh node
	AtNode* node = AiNodeLookUpByName(GetName().c_str());
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
	AiMesh::AiMesh(meshPath, "", meshId, objId)
{
}

//---------------------------------------
// Cleanup mesh
//---------------------------------------
AiMesh::~AiMesh()
{
	DestroyMesh();
}
