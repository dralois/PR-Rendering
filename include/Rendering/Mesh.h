#pragma once

#include <iostream>

#pragma warning(push, 0)
#include <Shaders/Shading.h>
#include <Renderfile.h>
#pragma warning(pop)

using namespace std;
using namespace Eigen;

//---------------------------------------
// Mesh object wrapper for rendering
//---------------------------------------
class Mesh : public RenderfileObject
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------

	string meshFile;
	OSLShader* shader;

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(PrettyWriter<stringstream>& writer) override
	{
		writer.Key("file");
		RenderfileData::AddString(writer, meshFile);

		// Mesh should always have a shader
		if (shader)
		{
			writer.Key("shader");
			shader->AddToJSON(writer);
		}
		else
		{
			cout << "Mesh " << meshFile << " has no shader!" << endl;
		}
	}

public:
	//---------------------------------------
	// Constructors
	//---------------------------------------

	Mesh(const string& meshFile, OSLShader* shader) :
		meshFile(meshFile),
		shader(shader)
	{
	}
};
