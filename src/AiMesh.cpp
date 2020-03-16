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
	// Cast to input struct, create name
	AiMeshInput* input = static_cast<AiMeshInput*>(data);
	string objName = "body" + std::to_string(meshId) + "_" + std::to_string(input->objSimId);

	// Destroy old mesh
	AtNode* node = AiNodeLookUpByName(objName.c_str());
	if (node != NULL)
	{
		return 1;
		//AiNodeDestroy(node);
	}

	// Create new mesh node
	AtNode* mesh = AiNode("polymesh");
	AiNodeSetStr(mesh, "name", objName.c_str());

	// Convert and set vertices and indices
	AtArray* vlist_array = X_VectorToAiArray(vecVertices, AI_TYPE_FLOAT);
	AiNodeSetArray(mesh, "vlist", vlist_array);
	AtArray* vidxs_array = X_VectorToAiArray(vecIndices, AI_TYPE_UINT);
	AiNodeSetArray(mesh, "vidxs", vidxs_array);

	// If not scan mesh
	if (!isScene)
	{
		// Convert and set UVs, and normals
		AtArray* uvlist_array = X_VectorToAiArray(vecUVs, AI_TYPE_FLOAT);
		AiNodeSetArray(mesh, "uvlist", uvlist_array);
		AtArray* nlist_array = X_VectorToAiArray(vecNormals, AI_TYPE_FLOAT);
		AiNodeSetArray(mesh, "nlist", nlist_array);
		AtArray* uvidxs_array = X_VectorToAiArray(vecIndices, AI_TYPE_UINT);
		AtArray* nidxs_array = X_VectorToAiArray(vecIndices, AI_TYPE_UINT);
		if (vecIndices.size() == vecUVs.size() / 2)
			AiNodeSetArray(mesh, "uvidxs", uvidxs_array);
		else
			AiArrayDestroy(uvidxs_array);
		if (vecIndices.size() == vecNormals.size() / 3)
			AiNodeSetArray(mesh, "nidxs", nidxs_array);
		else
			AiArrayDestroy(nidxs_array);
	}

	AtMatrix trans, prod;
	AiM4Identity(prod);
	Eigen::Quaterniond q;
	q.x() = input->rot[0];
	q.y() = input->rot[1];
	q.z() = input->rot[2];
	q.w() = input->rot[3];

	// Create rotation matrix
	Eigen::Matrix<float, 3, 3> bRotMat = q.normalized().toRotationMatrix().cast<float>();
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			prod[i][j] = bRotMat(j, i);
		}
	}

	// Create position vector
	AtPoint transV = { input->pos[0], input->pos[1], input->pos[2] };
	// Create and save object to world matrix
	AiM4Translation(trans, &transV);
	AiM4Mult(prod, prod, trans);
	AiNodeSetMatrix(mesh, "matrix", prod);

	// Return
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
