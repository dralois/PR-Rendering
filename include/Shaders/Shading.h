#pragma once

#pragma warning(push, 0)
#include <boost/filesystem.hpp>

#include <Renderfile.h>
#pragma warning(pop)

//---------------------------------------
// Texture data wrapper for rendering
//---------------------------------------
struct OSLTexture
{
	boost::filesystem::path filePath;
	std::string colorSpace;
	std::string colorDepth;
};

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
	std::vector<OSLTexture> textures;

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(rapidjson::PrettyWriter<rapidjson::StringStream>& writer) = 0;

public:

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void AddToJSON(rapidjson::PrettyWriter<rapidjson::StringStream>& writer) override
	{
		writer.StartObject();
		writer.Key("name");
		AddString(writer, name);

		// Texture array
		writer.StartArray();
		for (auto currTex : textures)
		{
			writer.StartObject();

			writer.Key("filePath");
			AddString(writer, currTex.filePath.string());

			writer.Key("colorSpace");
			AddString(writer, currTex.colorSpace);

			writer.Key("colorDepth");
			AddString(writer, currTex.colorDepth);

			writer.EndObject();
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

	OSLShader(const std::string& name, const std::vector<OSLTexture>& textures) :
		name(name),
		textures(textures)
	{
	}
};
