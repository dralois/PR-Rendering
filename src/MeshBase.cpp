#include "MeshBase.h"

//---------------------------------------
// Try to load the mesh
//---------------------------------------
bool MeshBase::LoadFile(bool doubleNorms)
{
	Assimp::Importer importer;

	// If reading possible
	std::ifstream f(meshPath.c_str());
	if (!f.good())
		return false;

	// Read and load with assimp
	const aiScene* scene = importer.ReadFile(meshPath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

	// Error handling
	if (!scene)
	{
		std::cout << importer.GetErrorString() << std::endl;
		return false;
	}
	else if (!scene->HasMeshes())
	{
		std::cout << "File " << meshPath << " has no mesh" << endl;
		return false;
	}

	const aiMesh* mesh = scene->mMeshes[0];

	// Load vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		aiVector3D pos = mesh->mVertices[i];
		// Scale
		vecVertices.push_back(pos.x * scale);
		vecVertices.push_back((scale == 100 ? pos.z : pos.y) * scale);
		vecVertices.push_back((scale == 100 ? pos.y : pos.z) * scale);
	}

	// Load UVs
	if (mesh->HasTextureCoords(0))
	{
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			aiVector3D UVW = mesh->mTextureCoords[0][i];
			vecUVs.push_back(UVW.x);
			vecUVs.push_back(UVW.y);
		}
	}

	// Load normals
	if (mesh->HasNormals())
	{
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			aiVector3D n = mesh->mNormals[i];
			vecNormals.push_back(n.x);
			vecVertices.push_back(scale == 100 ? n.z : n.y);
			vecVertices.push_back(scale == 100 ? n.y : n.z);
		}
	}

	// Load faces
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		for(unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
		{
			vecIndices.push_back(mesh->mFaces[i].mIndices[j]);
		}
		// ??
		if (doubleNorms)
		{
			for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
			{
				vecIndices.push_back(mesh->mFaces[i].mIndices[mesh->mFaces[i].mNumIndices - j - 1]);
			}
		}
	}

	// Loading successfull
	importer.FreeScene();
	return true;
}

//---------------------------------------
// Default constructor
//---------------------------------------
MeshBase::MeshBase(string meshPath, string texturePath, int meshId, float scale) :
	meshPath(meshPath),
	texturePath(texturePath),
	meshId(meshId),
	scale(scale)
{
};

//---------------------------------------
// Without texture
//---------------------------------------
MeshBase::MeshBase(string meshPath, int meshId, float scale) :
	MeshBase(meshPath, "", meshId, scale)
{
};
