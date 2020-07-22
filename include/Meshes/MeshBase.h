#pragma once

#include <vector>
#include <string>
#include <iostream>

#pragma warning(push, 0)
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#pragma warning(pop)

//#define EXPORT_TO_FILE

//---------------------------------------
// Base class for all meshes
//---------------------------------------
class MeshBase
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------
	int objId;
	unsigned int meshId;
	boost::filesystem::path meshPath;
	boost::filesystem::path texturePath;

	std::vector<float> vecVertices;
	std::vector<int> vecIndices;
	std::vector<float> vecNormals;
	std::vector<float> vecUVs;

	//---------------------------------------
	// Methods
	//---------------------------------------
	bool X_LoadFile();
	void X_StoreFile(const std::string& ext) const;
	virtual void X_ExportMesh() = 0;

public:
	//---------------------------------------
	// Methods
	//---------------------------------------
	virtual void CreateMesh() = 0;

	//---------------------------------------
	// Properties
	//---------------------------------------
	inline int GetMeshId() { return meshId; };
	inline int GetObjId() { return objId; };
	inline void SetObjId(int id) { objId = id; };
	inline int GetLabelId() { return (objId + 1) * 10; }
	inline const boost::filesystem::path& GetMeshPath() { return meshPath; }
	inline const boost::filesystem::path& GetTexturePath() { return texturePath; }
	inline const std::string GetName() { return "mesh_" + std::to_string(meshId) + "_" + std::to_string(objId); };

	//---------------------------------------
	// Constructors
	//---------------------------------------
	MeshBase(const boost::filesystem::path& meshPath, const boost::filesystem::path& texturePath, int meshId);
	MeshBase(const boost::filesystem::path& meshPath, int meshId);
	MeshBase(const MeshBase& copy);
	~MeshBase();
};
