#pragma once

#include <vector>
#include <iostream>

#pragma warning(push, 0)
#include <Helpers/PathUtils.h>
#pragma warning(pop)

#define EXPORT_TO_FILE 0

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
	ModifiablePath meshPath;
	ModifiablePath texturePath;

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
	inline int GetMeshId() const { return meshId; }
	inline int GetObjId() const { return objId; }
	inline void SetObjId(int id) { objId = id; }
	inline ReferencePath GetMeshPath() const { return meshPath; }
	inline ReferencePath GetTexturePath() const { return texturePath; }
	inline std::string GetName() const { return "mesh_" + std::to_string(meshId) + "_" + std::to_string(objId); }

	//---------------------------------------
	// Constructors
	//---------------------------------------
	MeshBase(ReferencePath meshPath, ReferencePath texturePath, int meshId);
	MeshBase(ReferencePath meshPath, int meshId);
	MeshBase(MeshBase&& other) noexcept = default;
	MeshBase(const MeshBase& copy);
	~MeshBase();
};
