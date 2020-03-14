#include "MeshBase.h"

//---------------------------------------
// Try to load the mesh
//---------------------------------------
bool MeshBase::LoadFile()
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
			if (pos.x < xMin)
			{
				xMin = pos.x;
			}
			else if (pos.x > xMax)
			{
				xMax = pos.x;
			}
			if (pos.y < yMin)
			{
				yMin = pos.y;
			}
			else if (pos.y > yMax)
			{
				yMax = pos.y;
			}
		}
	}

	// Tolerance (?)
	xMax -= 0.5;
	xMin += 0.5;
	yMax -= 0.5;
	yMin += 0.5;

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
		// Not in use?
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
