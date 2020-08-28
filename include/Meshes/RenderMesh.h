#pragma once

#include <iostream>

#pragma warning(push, 0)
#include <Meshes/MeshBase.h>

#include <Rendering/Shader.h>
#include <Renderfile.h>
#pragma warning(pop)

//---------------------------------------
// Mesh object wrapper for rendering
//---------------------------------------
class RenderMesh : public RenderfileObject, public MeshBase
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------

	bool indirect;
	OSLShader* oslShader;

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(JSONWriterRef writer) override
	{
		writer.Key("objectID");
		writer.Int(GetObjId());

		writer.Key("file");
		AddString(writer, GetMeshPath().string());

		writer.Key("indirect");
		writer.Bool(indirect);

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

	virtual void X_ExtractMesh() override { /* Does not apply to render mesh */ }

public:
	//---------------------------------------
	// Properties
	//---------------------------------------

	inline const OSLShader* GetShader() const { return oslShader; }
	inline void SetShader(OSLShader* shader)
	{
		delete oslShader;
		oslShader = shader;
	}

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void CreateMesh() override { /* Does not apply to render mesh */ }

	//---------------------------------------
	// Constructors
	//---------------------------------------

	RenderMesh(
		ReferencePath meshFile,
		ReferencePath textureFile,
		int meshId,
		bool indirect = false
	) :
		MeshBase(meshFile, textureFile, meshId),
		RenderfileObject(),
		indirect(indirect),
		oslShader(NULL)
	{
	}

	RenderMesh(
		ReferencePath meshFile,
		int meshId,
		bool indirect = false
	) :
		RenderMesh(meshFile, "", meshId, indirect)
	{
	}

	RenderMesh(const RenderMesh& copy) :
		MeshBase(copy),
		RenderfileObject(copy),
		indirect(copy.indirect),
		oslShader(NULL)
	{
		if(copy.oslShader)
		{
			oslShader = copy.oslShader->MakeCopy();
		}
	}

	RenderMesh(RenderMesh&& other) :
		MeshBase(std::move(other)),
		RenderfileObject(std::move(other))
	{
		indirect = std::exchange(other.indirect, false);
		oslShader = std::exchange(other.oslShader, nullptr);
	}

	~RenderMesh()
	{
		// Mesh is responsible for shader
		delete oslShader;
		oslShader = NULL;
	}
};
