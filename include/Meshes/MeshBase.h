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
	int objId = -1;
	int meshId = -1;
	std::string meshClass = "none";

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
	virtual void X_ExtractMesh() = 0;

public:
	//---------------------------------------
	// Methods
	//---------------------------------------
	virtual void CreateMesh() = 0;

	//---------------------------------------
	// Properties
	//---------------------------------------
	inline int GetObjId() const { return objId; }
	inline void SetObjId(int id) { objId = id; }
	inline int GetMeshId() const { return meshId; }
	inline std::string GetMeshClass() const { return meshClass; }
	inline ModifiablePath GetMeshPath() const { return meshPath; }
	inline ModifiablePath GetTexturePath() const { return texturePath; }
	inline std::string GetName() const { return meshPath.stem().string(); }

	//---------------------------------------
	// Constructors
	//---------------------------------------
	MeshBase(ReferencePath meshPath, ReferencePath texturePath, const std::string& meshClass, int meshId);
	MeshBase(ReferencePath meshPath, const std::string& meshClass, int meshId);
	MeshBase(const MeshBase& copy);
	MeshBase(MeshBase&& other);
	virtual ~MeshBase();
};
