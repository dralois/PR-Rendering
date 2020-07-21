#include <Meshes/MeshBase.h>

#pragma warning(push, 0)
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Exporter.hpp>
#pragma warning(pop)

//---------------------------------------
// Try to load mesh from disk
//---------------------------------------
bool MeshBase::X_LoadFile()
{
	Assimp::Importer importer;

	// File must be readable
	boost::filesystem::fstream meshFile(meshPath);
	if (!meshFile.good())
		return false;

	// Read and load with assimp
	const aiScene* scene = importer.ReadFile(meshPath.string(), aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

	// Error handling
	if (!scene)
	{
		std::cout << "Mesh load error:" << importer.GetErrorString() << std::endl;
		return false;
	}
	else if (!scene->HasMeshes())
	{
		std::cout << "Mesh load error:" << meshPath.filename() << " has no mesh" << std::endl;
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
// Store internal mesh to disk
//---------------------------------------
void MeshBase::X_StoreFile(const std::string& ext) const
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
	int numVerts = vecVertices.size() / 3;
	pMesh->mVertices = new aiVector3D[numVerts];
	pMesh->mNumVertices = numVerts;
	for (int i = 0; i < numVerts; i++)
	{
		pMesh->mVertices[i] = aiVector3D(vecVertices[(i * 3)], vecVertices[(i * 3) + 1], vecVertices[(i * 3) + 2]);
	}

	// Save UVs if there are any
	if(vecUVs.size() > 0)
	{
		int numUVs = vecUVs.size() / 2;
		pMesh->mTextureCoords[0] = new aiVector3D[numUVs];
		pMesh->mNumUVComponents[0] = 2;
		for (unsigned int i = 0; i < numUVs; i++)
		{
			pMesh->mTextureCoords[0][i] = aiVector3D(vecUVs[(i * 2)], vecUVs[(i * 2) + 1], 0);
		}
	}

	// Save normals if there are any
	if (vecNormals.size() > 0)
	{
		int numNormals = vecNormals.size() / 3;
		pMesh->mNormals = new aiVector3D[numNormals];
		for (int i = 0; i < numNormals; i++)
		{
			pMesh->mNormals[i] = aiVector3D(vecNormals[(i * 3)], vecNormals[(i * 3) + 1], vecNormals[(i * 3) + 2]);
		}
	}

	// Save faces
	int numFaces = vecIndices.size() / 3;
	pMesh->mFaces = new aiFace[numFaces];
	pMesh->mNumFaces = numFaces;
	for (int i = 0; i < numFaces; i++)
	{
		aiFace& face = pMesh->mFaces[i];
		// Create index buffer
		face.mIndices = new unsigned int[3];
		face.mNumIndices = 3;
		// Save indices
		face.mIndices[0] = vecIndices[(i * 3)];
		face.mIndices[1] = vecIndices[(i * 3) + 1];
		face.mIndices[2] = vecIndices[(i * 3) + 2];
	}

	// Build path
	boost::filesystem::path savePath(meshPath.parent_path());
	savePath.append(meshPath.filename());
	savePath.concat(".obj");

	// Export created mesh to path
	exporter.Export(&scene, "obj", savePath.string());
	std::cout << "Exported mesh:" << savePath << std::endl;
}

//---------------------------------------
// Base constructor
//---------------------------------------
MeshBase::MeshBase(const boost::filesystem::path& meshPath, const boost::filesystem::path& texturePath, int meshId) :
	meshId(meshId),
	objId(-1),
	meshPath(meshPath),
	texturePath(texturePath)
{
}

//---------------------------------------
// Constructor without texture
//---------------------------------------
MeshBase::MeshBase(const boost::filesystem::path& meshPath, int meshId) :
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
	texturePath(copy.texturePath)
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
