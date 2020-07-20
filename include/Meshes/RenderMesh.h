#pragma once

#include <iostream>

#pragma warning(push, 0)
#include <Meshes/MeshBase.h>
#include <Shaders/Shading.h>
#include <Renderfile.h>
#pragma warning(pop)

//---------------------------------------
// Mesh object wrapper for rendering
//---------------------------------------
class RenderMesh : public RenderfileObject, MeshBase
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------

	OSLShader* oslShader;

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(rapidjson::PrettyWriter<rapidjson::StringStream>& writer) override
	{
		writer.Key("file");
		AddString(writer, GetMeshPath().string());
		// Mesh should always have a shader
		if (oslShader)
		{
			writer.Key("shader");
			oslShader->AddToJSON(writer);
		}
		else
		{
			std::cout << "Mesh " << GetMeshPath() << " has no shader!" << std::endl;
		}
	}

	void RenderMesh::X_ExportMesh()	{ /* Does not apply to render mesh */ }

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	inline const OSLShader* GetShader() { return oslShader; }
	inline void SetShader(OSLShader* shader)
	{
		delete oslShader;
		oslShader = shader;
	}

	//---------------------------------------
	// Methods
	//---------------------------------------

	void RenderMesh::CreateMesh() { /* Does not apply to render mesh */ }

	//---------------------------------------
	// Constructors
	//---------------------------------------

	RenderMesh(const boost::filesystem::path& meshFile, const boost::filesystem::path& textureFile, int meshId) :
		MeshBase(meshFile, textureFile, meshId),
		oslShader(NULL)
	{
	}

	RenderMesh(const boost::filesystem::path& meshFile, int meshId) :
		MeshBase(meshFile, meshId),
		oslShader(NULL)
	{
	}

	~RenderMesh()
	{
		// Shader is unique to mesh
		delete oslShader;
	}
};
