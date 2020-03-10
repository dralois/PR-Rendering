#include "AiMesh.h"

// Convert vector to arnold array
template <class T>
AtArray* AiMesh::X_ArrayConvertByVector(const std::vector<T>& input, const AtByte type)
{
	T* array = new T[input.size()];
	// Vector to array
	for (int i = 0; i < input.size(); i++)
	{
		array[i] = input.at(i);
	}
	// Array to arnold array
	AtArray* r = AiArrayConvert(input.size(), 1, type, array);
	// Cleanup
	delete array;
	return r;
}

// Create arnold mesh
AtNode* AiMesh::CreateMesh(const int obj_id, const vector<float>& pos, const vector<float>& quat)
{
	string buffer = "body" + std::to_string(mesh_id) + "_" + std::to_string(obj_id);
	// Destroy old mesh
	AtNode* node = AiNodeLookUpByName(buffer.c_str());
	if (node != nullptr)
		AiNodeDestroy(node);

	// Create new mesh node
	AtNode* mesh = AiNode("polymesh");
	AiNodeSetStr(mesh, "name", buffer.c_str());

	// Convert and set vertices and indices
	AtArray* vlist_array = X_ArrayConvertByVector(vecVertices, AI_TYPE_FLOAT);
	AiNodeSetArray(mesh, "vlist", vlist_array);
	AtArray* vidxs_array = X_ArrayConvertByVector(vecIndices, AI_TYPE_UINT);
	AiNodeSetArray(mesh, "vidxs", vidxs_array);

	// If not scan mesh
	if (!isScene)
	{
		// Convert and set UVs, and normals
		AtArray* uvlist_array = X_ArrayConvertByVector(vecUVs, AI_TYPE_FLOAT);
		AiNodeSetArray(mesh, "uvlist", uvlist_array);
		AtArray* nlist_array = X_ArrayConvertByVector(vecNormals, AI_TYPE_FLOAT);
		AiNodeSetArray(mesh, "nlist", nlist_array);
		AtArray* uvidxs_array = X_ArrayConvertByVector(vecIndices, AI_TYPE_UINT);
		AtArray* nidxs_array = X_ArrayConvertByVector(vecIndices, AI_TYPE_UINT);
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
	q.x() = quat[0];
	q.y() = quat[1];
	q.z() = quat[2];
	q.w() = quat[3];

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
	AtPoint transV = { pos.at(0), pos.at(1), pos.at(2) };

	// Create and save object to world matrix
	AiM4Translation(trans, &transV);
	AiM4Mult(prod, prod, trans);
	AiNodeSetMatrix(mesh, "matrix", prod);

	// Return converted mesh
	return mesh;
}

// Cleanup arnold mesh manager
bool AiMesh::DestroyMesh(const string node_name)
{
	// Find and destroy mesh node
	AtNode* node = AiNodeLookUpByName(node_name.c_str());
	// Not found, no success
	if (node == nullptr)
	{
		return false;
	}
	// Found: Destroy, success
	AiNodeDestroy(node);
	return true;
}
