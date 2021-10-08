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
	std::string shaderType;
	OSLShader* oslShader;

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(JSONWriterRef writer) const override
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

	inline bool GetIndirect() const { return indirect; }
	inline void SetIndirect(bool isIndirect) { indirect = isIndirect; }

	inline std::string& GetShaderType() { return shaderType; }

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
		const std::string& meshClass,
		const std::string& meshShader,
		int meshId,
		bool indirect = false
	) :
		MeshBase(meshFile, textureFile, meshClass, meshId),
		RenderfileObject(),
		indirect(indirect),
		shaderType(meshShader),
		oslShader(NULL)
	{
	}

	RenderMesh(
		ReferencePath meshFile,
		const std::string& meshClass,
		const std::string& meshShader,
		int meshId,
		bool indirect = false
	) :
		RenderMesh(meshFile, "", meshClass, meshShader, meshId, indirect)
	{
	}

	RenderMesh(const RenderMesh& copy) :
		MeshBase(copy),
		RenderfileObject(copy),
		indirect(copy.indirect),
		shaderType(copy.shaderType),
		oslShader(NULL)
	{
		if (copy.oslShader)
		{
			oslShader = copy.oslShader->MakeCopy();
		}
	}

	RenderMesh(RenderMesh&& other) :
		MeshBase(std::move(other)),
		RenderfileObject(std::move(other))
	{
		indirect = std::exchange(other.indirect, false);
		shaderType = std::exchange(other.shaderType, "");
		oslShader = std::exchange(other.oslShader, nullptr);
	}

	~RenderMesh()
	{
		// Mesh is responsible for shader
		delete oslShader;
		oslShader = NULL;
	}
};
