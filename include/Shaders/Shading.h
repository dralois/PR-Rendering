#pragma once

#pragma warning(push, 0)
#include <Renderfile.h>
#pragma warning(pop)

using namespace std;
using namespace Eigen;

//---------------------------------------
// Texture data wrapper for rendering
//---------------------------------------
struct OSLTexture
{
	string filePath;
	string colorSpace;
	string colorDepth;
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

	string name;
	vector<OSLTexture> textures;

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(PrettyWriter<stringstream>& writer) = 0;

public:

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void AddToJSON(PrettyWriter<stringstream>& writer) override
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
			AddString(writer, currTex.filePath);

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

	OSLShader(const string& name, const vector<OSLTexture>& textures) :
		name(name),
		textures(textures)
	{
	}
};
