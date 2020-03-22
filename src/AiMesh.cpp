#include "AiMesh.h"

#include <iostream>

#pragma warning(push, 0)
#include <eigen3/Eigen/Dense>
#pragma warning(pop)

//---------------------------------------
// Convert vector to arnold array
//---------------------------------------
template <class T>
AtArray* AiMesh::X_VectorToAiArray(const vector<T>& input, const size_t size, const AtByte type)
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

	// Return if loaded
	if (baseNode != NULL)
		return;

	// Load the mesh from file
	LoadFile(false);

	// Create base mesh node
	baseNode = AiNode("polymesh");
	AiNodeSetStr(baseNode, "name", GetBaseName().c_str());

	// Convert and set vertices and indices
	AtArray* vlist_array = X_VectorToAiArray(vecVertices, vecIndices.size(), AI_TYPE_POINT);
	AiNodeSetArray(baseNode, "vlist", vlist_array);
	AtArray* idxs_array = X_VectorToAiArray(vecIndices, vecIndices.size(), AI_TYPE_UINT);
	AiNodeSetArray(baseNode, "vidxs", idxs_array);

	// If not a scan mesh
	// FIXME: UVs & normals not in use
	if (!isScene && false)
	{
		// Sanity check: Sizes must match
		if (vecUVs.size() / 2 == vecIndices.size() && vecNormals.size() / 3 == vecIndices.size())
		{
			// Convert and set UVs & normals
			AtArray* uvlist_array = X_VectorToAiArray(vecUVs, vecIndices.size(), AI_TYPE_POINT2);
			AiNodeSetArray(baseNode, "uvlist", uvlist_array);
			AtArray* nlist_array = X_VectorToAiArray(vecNormals, vecIndices.size(), AI_TYPE_VECTOR);
			AiNodeSetArray(baseNode, "nlist", nlist_array);
			// Convert and set UV & normal index buffers
			AiNodeSetArray(baseNode, "uvidxs", AiArrayCopy(idxs_array));
			AiNodeSetArray(baseNode, "nidxs", AiArrayCopy(idxs_array));
		}
	}

	// Disable the base mesh node
	AiNodeSetDisabled(baseNode, true);
}

//---------------------------------------
// Create arnold mesh
//---------------------------------------
void AiMesh::CreateMesh(const vector<float>& posIn, const vector<float>& rotIn)
{
	// Try to find node
	AtNode* currMesh = AiNodeLookUpByName(GetName().c_str());

	// If mesh not created
	if (currMesh == NULL)
	{
		// Clone or set the node (scene exists only once)
		if(isScene)
		{
			currMesh = baseNode;
		}
		else
		{
			currMesh = AiNodeClone(baseNode);
			AiNodeSetStr(currMesh, "name", GetName().c_str());
		}
	}

	// Translation, rotation and transformation matrices
	AtMatrix aiMatPos, aiMatRot, aiMatTrans;
	AiM4Identity(aiMatPos);
	AiM4Identity(aiMatRot);
	AiM4Identity(aiMatTrans);

	// Create rotation matrix
	Eigen::Quaternionf rot = Eigen::Quaternion<float>(&rotIn[0]);
	Eigen::Matrix<float, 3, 3> matRot = rot.normalized().toRotationMatrix();
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			aiMatRot[i][j] = matRot(j, i);
		}
	}

	// Create translation matrix
	AtPoint pos = { posIn[0], posIn[1], posIn[2] };
	AiM4Translation(aiMatPos, &pos);

	// Create and save object to world matrix in actual node
	AiM4Mult(aiMatTrans, aiMatRot, aiMatPos);
	AiNodeSetMatrix(currMesh, "matrix", aiMatTrans);

	// Finally enable the mesh node
	AiNodeSetDisabled(currMesh, false);
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
// Destroys the base node of this mesh
//---------------------------------------
void AiMesh::DestroyBaseNode()
{
	AiNodeDestroy(baseNode);
}

//---------------------------------------
// Restores the base node of this mesh
//---------------------------------------
void AiMesh::RestoreBaseNode()
{
	baseNode = AiNodeLookUpByName(GetBaseName().c_str());
}

//---------------------------------------
// Constructor with texture
//---------------------------------------
AiMesh::AiMesh(const string& meshPath, const string& texturePath, int meshId, int objId, float scale) :
	MeshBase(meshPath, texturePath, meshId, objId, scale)
{
	X_CreateBaseNode();
}

//---------------------------------------
// Constructor without texture
//---------------------------------------
AiMesh::AiMesh(const string& meshPath, int meshId, int objId, float scale) :
	MeshBase(meshPath, meshId, objId, scale)
{
	X_CreateBaseNode();
}
