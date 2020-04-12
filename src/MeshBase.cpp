#include "MeshBase.h"

#pragma warning(push, 0)
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Exporter.hpp>
#pragma warning(pop)

//---------------------------------------
// Try to load the mesh
//---------------------------------------
bool MeshBase::X_LoadFile()
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
		std::cout << "Mesh load error:" << importer.GetErrorString() << std::endl;
		return false;
	}
	else if (!scene->HasMeshes())
	{
		std::cout << "Mesh load error:" << meshPath << " has no mesh" << endl;
		return false;
	}

	const aiMesh* mesh = scene->mMeshes[0];

	// Load vertices
	vecVertices.reserve(mesh->mNumVertices);
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		aiVector3D pos = mesh->mVertices[i];
		// Scale
		vecVertices.push_back(pos.x);
		vecVertices.push_back(pos.y);
		vecVertices.push_back(pos.z);
	}

	// Load UVs
	if (mesh->HasTextureCoords(0))
	{
		vecUVs.reserve(mesh->mNumVertices);
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
		vecNormals.reserve(mesh->mNumVertices);
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			aiVector3D n = mesh->mNormals[i];
			vecNormals.push_back(n.x);
			vecNormals.push_back(n.y);
			vecNormals.push_back(n.z);
		}
	}

	// Load index buffer
	vecIndices.reserve(mesh->mNumFaces * mesh->mFaces[0].mNumIndices);
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
		{
			vecIndices.push_back(mesh->mFaces[i].mIndices[j]);
		}
	}

	// Loading successfull
	importer.FreeScene();
	return true;
}

//---------------------------------------
// Stores a provided mesh
//---------------------------------------
void MeshBase::X_StoreFile(const vector<int>& idxs, int nIdxs, const vector<float>& verts, int nVerts, const string& ext) const
{
	Assimp::Exporter exporter;

	// Create scene
	aiScene scene;
	scene.mRootNode = new aiNode();

	// Create material
	scene.mMaterials = new aiMaterial * [1];
	scene.mMaterials[0] = nullptr;
	scene.mNumMaterials = 1;
	scene.mMaterials[0] = new aiMaterial();

	// Create mesh
	scene.mMeshes = new aiMesh * [1];
	scene.mMeshes[0] = nullptr;
	scene.mNumMeshes = 1;
	scene.mMeshes[0] = new aiMesh();
	scene.mMeshes[0]->mMaterialIndex = 0;

	// Assign it to the scene
	scene.mRootNode->mMeshes = new unsigned int[1];
	scene.mRootNode->mMeshes[0] = 0;
	scene.mRootNode->mNumMeshes = 1;
	auto pMesh = scene.mMeshes[0];

	// Save vertices
	pMesh->mVertices = new aiVector3D[nVerts];
	pMesh->mNumVertices = nVerts;
	for (int i = 0; i < nVerts; i++)
	{
		pMesh->mVertices[i] = aiVector3D(verts[(i * 3)], verts[(i * 3) + 1], verts[(i * 3) + 2]);
	}

	// Save faces
	pMesh->mFaces = new aiFace[nIdxs];
	pMesh->mNumFaces = nIdxs;
	for (int i = 0; i < nIdxs; i++)
	{
		aiFace& face = pMesh->mFaces[i];
		// Create index buffer
		face.mIndices = new unsigned int[3];
		face.mNumIndices = 3;
		// Save indices
		face.mIndices[0] = idxs[(i * 3)];
		face.mIndices[1] = idxs[(i * 3) + 1];
		face.mIndices[2] = idxs[(i * 3) + 2];
	}

	// Append prefix to path
	string savePath(meshPath);
	savePath.replace(meshPath.length() - 4, 4, ext + ".obj");

	// Export created mesh to path
	exporter.Export(&scene, "obj", savePath);

	std::cout << "Exported mesh:" << savePath << std::endl;
}

//---------------------------------------
// Base constructor
//---------------------------------------
MeshBase::MeshBase(const string& meshPath, const string& texturePath, int meshId) :
	meshId(meshId),
	objId(-1),
	meshPath(meshPath),
	texturePath(texturePath)
{
}

//---------------------------------------
// Constructor without texture
//---------------------------------------
MeshBase::MeshBase(const string& meshPath, int meshId) :
	MeshBase(meshPath, "", meshId)
{
}

//---------------------------------------
// Copy constructor
//---------------------------------------
MeshBase::MeshBase(const MeshBase& copy) :
	meshId(copy.meshId),
	objId(-1),
	meshPath(copy.meshPath),
	texturePath(copy.texturePath),
	metalness(copy.metalness),
	meshScale(copy.meshScale)
{
}

//---------------------------------------
// Mesh cleanup
//---------------------------------------
MeshBase::~MeshBase()
{
	// Cleanup vectors
	vecVertices.clear();
	vecIndices.clear();
	vecNormals.clear();
	vecUVs.clear();
}
