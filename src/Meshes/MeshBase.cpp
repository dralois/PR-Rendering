#include <Meshes/MeshBase.h>

#pragma warning(push, 0)
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Exporter.hpp>
#pragma warning(pop)

#pragma warning(disable:26451)

//---------------------------------------
// Try to load mesh from disk
//---------------------------------------
bool MeshBase::X_LoadFile()
{
	Assimp::Importer importer;
	// Removes degenerate faces, lines & points from mesh
	importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);

	// Mesh file must exist
	if (!boost::filesystem::exists(meshPath))
		return false;

	// Read and load mesh with assimp, improve mesh
	const aiScene* scene = importer.ReadFile(meshPath.string(),
		aiProcessPreset_TargetRealtime_MaxQuality & ~aiProcess_SplitLargeMeshes);

	// Error handling
	if (!scene)
	{
		std::cout << "\33[2K\r" << "Mesh load error:\t" << importer.GetErrorString() << std::endl;
		return false;
	}
	else if (!scene->HasMeshes())
	{
		std::cout << "\33[2K\r" << "Mesh load error:\t" << boost::filesystem::relative(meshPath) << " has no mesh" << std::endl;
		return false;
	}
	else
	{
		std::cout << "\33[2K\r" << "Imported mesh:\t" << boost::filesystem::relative(meshPath) << std::flush;
	}

	const aiMesh* mesh = scene->mMeshes[0];

	// Load vertices
	vecVertices.reserve(mesh->mNumVertices);
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		aiVector3D pos = mesh->mVertices[i];
		// Make Y forward & Z up
		vecVertices.push_back(pos.x);
		vecVertices.push_back(pos.z);
		vecVertices.push_back(-pos.y);
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
			// Make Y forward & Z up
			vecNormals.push_back(n.x);
			vecNormals.push_back(n.z);
			vecNormals.push_back(-n.y);
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
	if (vecUVs.size() > 0)
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
	ModifiablePath savePath(meshPath.parent_path());
	savePath.append(meshPath.stem().string());
	savePath.concat(ext + ".obj");

	// Export created mesh to path
	exporter.Export(&scene, "obj", savePath.string());
	std::cout << "\33[2K\r" << "Exported mesh:\t" << boost::filesystem::relative(savePath) << std::flush;
	exporter.FreeBlob();
}

//---------------------------------------
// Base constructor
//---------------------------------------
MeshBase::MeshBase(ReferencePath meshPath, ReferencePath texturePath, const std::string& meshClass, int meshId) :
	objId(-1),
	meshId(meshId),
	meshClass(meshClass),
	meshPath(meshPath),
	texturePath(texturePath)
{
}

//---------------------------------------
// Constructor without texture
//---------------------------------------
MeshBase::MeshBase(ReferencePath meshPath, const std::string& meshClass, int meshId) :
	MeshBase(meshPath, "", meshClass, meshId)
{
}

//---------------------------------------
// Copy constructor
//---------------------------------------
MeshBase::MeshBase(const MeshBase& copy) :
	objId(copy.objId),
	meshId(copy.meshId),
	meshClass(copy.meshClass),
	meshPath(copy.meshPath),
	texturePath(copy.texturePath)
{
}

//---------------------------------------
// Move constructor
//---------------------------------------
MeshBase::MeshBase(MeshBase&& other)
{
	objId = std::exchange(other.objId, -1);
	meshId = std::exchange(other.meshId, -1);
	meshClass = std::exchange(other.meshClass, "");
	std::swap(meshPath, other.meshPath);
	std::swap(texturePath, other.texturePath);
	std::swap(vecVertices, other.vecVertices);
	std::swap(vecIndices, other.vecIndices);
	std::swap(vecNormals, other.vecNormals);
	std::swap(vecUVs, other.vecUVs);
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

#pragma warning(default:26451)
