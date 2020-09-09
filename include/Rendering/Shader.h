#pragma once

#pragma warning(push, 0)
#include <Helpers/JSONUtils.h>
#include <Helpers/PathUtils.h>

#include <Rendering/Texture.h>
#include <Renderfile.h>
#pragma warning(pop)

//---------------------------------------
// Base shader data wrapper for rendering
//---------------------------------------
class OSLShader : public RenderfileData
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------

	std::string name;
	std::vector<Texture> textures;

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(JSONWriterRef writer) = 0;

public:

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual OSLShader* MakeCopy() const = 0;

	virtual void AddToJSON(JSONWriterRef writer) override
	{
		writer.StartObject();
		writer.Key("name");
		AddString(writer, name);

		// Texture array
		writer.Key("textures");
		writer.StartArray();
		for (auto& currTex : textures)
		{
			currTex.AddToJSON(writer);
		}
		writer.EndArray();

		// Special params
		writer.Key("params");
		writer.StartObject();
		X_AddToJSON(writer);
		writer.EndObject();

		writer.EndObject();
	}

	//---------------------------------------
	// Constructors
	//---------------------------------------

	OSLShader(
		const std::string& name,
		const std::vector<Texture>& textures
	) :
		name(name),
		textures(textures)
	{
	}

	OSLShader(
		const std::string& name
	) :
		OSLShader(name, {})
	{
	}
};
