#include <AiMesh.h>

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

#ifdef AI_EXTRACT_INTERNAL
	// Export loaded base node to file
	X_ExportMesh();
#endif // AI_EXPORT_TO_OBJ

	// Disable the base mesh node
	AiNodeSetDisabled(baseNode, true);
}

//---------------------------------------
// Export loaded node to obj file
//---------------------------------------
void AiMesh::X_ExportMesh()
{
	// Clone and save vertices into vector
	AtArray* aiVerts = AiArrayCopy((AiNodeGetArray(baseNode, "vlist")));
	int vertsNum = AiArrayGetNumElements(aiVerts);
	float* pVerts = (float*)AiArrayMap(aiVerts);
	vecVertices = vector<float>(pVerts, pVerts + (vertsNum * 3));

	// Clone and save indices into vector
	AtArray* aiIndices = AiArrayCopy((AiNodeGetArray(baseNode, "vidxs")));
	int indicesNum = AiArrayGetNumElements(aiIndices);
	unsigned int* pIndices = (unsigned int*)AiArrayMap(aiIndices);
	vecIndices = vector<int>(pIndices, pIndices + indicesNum);

	// Clone and save normals into vector
	AtArray* aiNormals = AiArrayCopy((AiNodeGetArray(baseNode, "nlist")));
	int normalsNum = AiArrayGetNumElements(aiNormals);
	float* pNormals = (float*)AiArrayMap(aiNormals);
	vecNormals = vector<float>(pNormals, pNormals + (normalsNum * 3));

	// Clone UVs + indices
	AtArray* aiUVs = AiArrayCopy((AiNodeGetArray(baseNode, "uvlist")));
	float* pUVs = (float*)AiArrayMap(aiUVs);
	// Save UVs according to indices in vector
	vecUVs.resize(vertsNum * 2);
	for(int i = 0; i < indicesNum; i++)
	{
		int idx = vecIndices[i];
		vecUVs[idx] = *(pUVs + i);
		vecUVs[idx + 1] = *(pUVs + i + 1);
	}

#ifdef EXPORT_TO_FILE
	// Store to obj file
	X_StoreFile("_ai");
#endif // EXPORT_TO_FILE

	// Cleanup arrays
	AiArrayUnmap(aiVerts);
	AiArrayDestroy(aiVerts);
	AiArrayUnmap(aiIndices);
	AiArrayDestroy(aiIndices);
	AiArrayUnmap(aiNormals);
	AiArrayDestroy(aiNormals);
	AiArrayUnmap(aiUVs);
	AiArrayDestroy(aiUVs);
}

//---------------------------------------
// Create arnold mesh
//---------------------------------------
void AiMesh::CreateMesh()
{
	// Try to find node
	meshNode = AiNodeLookUpByName(GetName().c_str());

	// If mesh not created
	if (!meshNode)
	{
		if(isScene)
		{
			// Set node, scene exists only once
			meshNode = baseNode;
		}
		else
		{
			// Clone node with new name
			meshNode = AiNodeClone(baseNode, AtString(GetName().c_str()));
		}
		// Enable the node
		AiNodeSetDisabled(meshNode, false);
	}
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
// Set scale of arnold mesh
// TODO
//---------------------------------------
void AiMesh::SetScale(float scale)
{
	// Save
	meshScale = scale;

	// Set scaling
	AiNodeSetMatrix(meshNode, "matrix", AiM4Scaling(AtVector(meshScale, meshScale, meshScale)));
}

//---------------------------------------
// Get transform of arnold mesh
// TODO
//---------------------------------------
const Matrix4f AiMesh::GetTransform()
{
	return meshTrans;
}

//---------------------------------------
// Set transform of arnold mesh
//---------------------------------------
void AiMesh::SetTransform(Matrix4f trans)
{
	// Save
	meshTrans = trans;

	// Convert to arnold matrix (transposed)
	AtMatrix aiMatTrans = AiM4Identity();
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			aiMatTrans[i][j] = meshTrans(j, i);

	// Save in mesh node
	AiNodeSetMatrix(meshNode, "matrix", aiMatTrans);
}

//---------------------------------------
// Base constructor
//---------------------------------------
AiMesh::AiMesh(const string& meshPath, const string& texturePath, int meshId) :
	MeshBase(meshPath, texturePath, meshId)
{
	X_CreateBaseNode();
}

//---------------------------------------
// Constructor without texture
//---------------------------------------
AiMesh::AiMesh(const string& meshPath, int meshId) :
	AiMesh::AiMesh(meshPath, "", meshId)
{
}

//---------------------------------------
// Arnold mesh cleanup
//---------------------------------------
AiMesh::~AiMesh()
{
	// Destroy this mesh node
	if (meshNode)
	{
		AiNodeDestroy(meshNode);
	}
}
