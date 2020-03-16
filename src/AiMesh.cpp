#include "AiMesh.h"

//---------------------------------------
// Convert vector to arnold array
//---------------------------------------
template <class T>
AtArray* AiMesh::X_VectorToAiArray(const std::vector<T>& input, const AtByte type)
{
	// Array to arnold array
	AtArray* returnArr = AiArrayConvert(input.size(), 1, type, &input[0]);
	return returnArr;
}

//---------------------------------------
// Create arnold mesh
//---------------------------------------
unsigned int AiMesh::CreateMesh(void* data)
{
	// Cast to input struct, define names
	AiMeshInput* input = static_cast<AiMeshInput*>(data);
	string baseName = "mesh_" + std::to_string(meshId);
	string currName = baseName + "_" + std::to_string(input->objSimId);

	// Find base mesh node
	AtNode* baseMesh = AiNodeLookUpByName(baseName.c_str());
	// Find actual mesh node
	AtNode* currMesh = AiNodeLookUpByName(currName.c_str());

	// Base mesh node not created
	if(baseMesh == NULL)
	{
		// Create base mesh node
		baseMesh = AiNode("polymesh");
		AiNodeSetStr(baseMesh, "name", baseName.c_str());

		// Convert and set vertices and indices
		AtArray* vlist_array = X_VectorToAiArray(vecVertices, AI_TYPE_FLOAT);
		AiNodeSetArray(baseMesh, "vlist", vlist_array);
		AtArray* vidxs_array = X_VectorToAiArray(vecIndices, AI_TYPE_UINT);
		AiNodeSetArray(baseMesh, "vidxs", vidxs_array);

		// If not scan mesh
		if (!isScene)
		{
			// Convert and set UVs & normals
			AtArray* uvlist_array = X_VectorToAiArray(vecUVs, AI_TYPE_FLOAT);
			AiNodeSetArray(baseMesh, "uvlist", uvlist_array);
			AtArray* nlist_array = X_VectorToAiArray(vecNormals, AI_TYPE_FLOAT);
			AiNodeSetArray(baseMesh, "nlist", nlist_array);
			// Convert and set UV & normal index buffers
			AtArray* uvidxs_array = X_VectorToAiArray(vecIndices, AI_TYPE_UINT);
			AiNodeSetArray(baseMesh, "uvidxs", uvidxs_array);
			AtArray* nidxs_array = X_VectorToAiArray(vecIndices, AI_TYPE_UINT);
			AiNodeSetArray(baseMesh, "nidxs", nidxs_array);
		}

		// Disable the base mesh node
		AiNodeSetDisabled(baseMesh, true);
	}

	// Clone or set the node (scene exists only once)
	if (currMesh == NULL && !isScene)
	{
		currMesh = AiNodeClone(baseMesh);
	}
	else if(isScene)
	{
		currMesh = baseMesh;
	}

	// Enable the mesh node
	AiNodeSetDisabled(currMesh, false);

	// Translation, rotation and transformation matrices
	AtMatrix aiMatPos, aiMatRot, aiMatTrans;
	AiM4Identity(aiMatPos);
	AiM4Identity(aiMatRot);
	AiM4Identity(aiMatTrans);

	// Create rotation matrix
	Eigen::Quaternionf rot = Eigen::Quaternion<float>(&input->rot[0]);
	Eigen::Matrix<float, 3, 3> matRot = rot.normalized().toRotationMatrix();
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			aiMatRot[i][j] = matRot(j, i);
		}
	}

	// Create translation matrix
	AtPoint pos = { input->pos[0], input->pos[1], input->pos[2] };
	AiM4Translation(aiMatPos, &pos);

	// Create and save object to world matrix in actual node
	AiM4Mult(aiMatTrans, aiMatRot, aiMatPos);
	AiNodeSetMatrix(currMesh, "matrix", aiMatTrans);

	// Done
	return 0;
}

//---------------------------------------
// Static function calling the member function
//---------------------------------------
unsigned int AiMesh::CreateMeshThread(void* data)
{
	// Cast to input struct
	AiMeshInput* input = static_cast<AiMeshInput*>(data);
	// Call actual member function
	return ((AiMesh*)input->pAiMesh)->CreateMesh(data);
}

//---------------------------------------
// Cleanup arnold mesh
//---------------------------------------
void AiMesh::DestroyMesh(const string nodeName)
{
	// Find and destroy mesh node
	AtNode* node = AiNodeLookUpByName(nodeName.c_str());
	// Try to destroy it
	AiNodeDestroy(node);
}

//---------------------------------------
// Constructor with texture
//---------------------------------------
AiMesh::AiMesh(string meshPath, string texturePath, int meshId, float scale) :
	MeshBase(meshPath, texturePath, meshId, scale)
{
	LoadFile(false);
}

//---------------------------------------
// Constructor without texture
//---------------------------------------
AiMesh::AiMesh(string meshPath, int meshId, float scale) :
	MeshBase(meshPath, meshId, scale)
{
	LoadFile(false);
}
