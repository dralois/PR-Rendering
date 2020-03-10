#include "MeshBase.h"

// Mesh ID
int MeshBase::GetID()
{
	return mesh_id;
}

// Try to load the mesh
bool MeshBase::LoadFile()
{
	xmin = ymin = 1e8;
	xmax = ymax = 0;

	Assimp::Importer importer;

	// If reading possible
	std::ifstream f(path.c_str());
	if (!f.good())
		return false;

	// Read and load with assimp
	pScene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

	if (!pScene)
	{
		std::cout << importer.GetErrorString() << std::endl;
		return false;
	}

	const aiMesh* mesh = pScene->mMeshes[0];

	// Load vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		aiVector3D pos = mesh->mVertices[i];
		// Scale
		vecVertices.push_back(pos.x * scale);
		if (scale == 100)
		{
			vecVertices.push_back(pos.z * scale);
			vecVertices.push_back(pos.y * scale);
		}
		else
		{
			vecVertices.push_back(pos.y * scale);
			vecVertices.push_back(pos.z * scale);
		}
		// Update bounds
		if (calculateBounds)
		{
			if (pos.x < xmin)
			{
				xmin = pos.x;
			}
			else if (pos.x > xmax)
			{
				xmax = pos.x;
			}
			if (pos.y < ymin)
			{
				ymin = pos.y;
			}
			else if (pos.y > ymax)
			{
				ymax = pos.y;
			}
		}
	}

	// ?
	xmax -= 0.5;
	xmin += 0.5;
	ymax -= 0.5;
	ymin += 0.5;

	// Load UVs
	if (mesh->mTextureCoords[0] != nullptr)
	{
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			aiVector3D UVW = mesh->mTextureCoords[0][i];
			vecUVs.push_back(UVW.x);
			vecUVs.push_back(UVW.y);
		}
	}

	// Load normals
	if (&(mesh->mNormals[0]) != nullptr)
	{
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			aiVector3D n = mesh->mNormals[i];
			vecNormals.push_back(n.x);
			vecNormals.push_back(n.z);
			vecNormals.push_back(n.y);
		}
	}

	// Load faces
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		vecIndices.push_back(mesh->mFaces[i].mIndices[0]);
		vecIndices.push_back(mesh->mFaces[i].mIndices[1]);
		vecIndices.push_back(mesh->mFaces[i].mIndices[2]);
		if (doubleNorms)
		{
			vecIndices.push_back(mesh->mFaces[i].mIndices[2]);
			vecIndices.push_back(mesh->mFaces[i].mIndices[1]);
			vecIndices.push_back(mesh->mFaces[i].mIndices[0]);
		}
	}

	// Loading successfull
	importer.FreeScene();
	return true;
}

// Default constructor
MeshBase::MeshBase(string path, string texture_path, int mesh_id, float scale) :
	path(path),
	texture_path(texture_path),
	mesh_id(mesh_id),
	scale(scale)
{
};

// Without texture
MeshBase::MeshBase(string path, int mesh_id, float scale) :
	MeshBase(path, "", mesh_id, scale)
{
};
