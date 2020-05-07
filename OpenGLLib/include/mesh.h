#pragma once

#include <vector>
#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <shader.h>

using namespace std;

namespace Renderer
{
	//---------------------------------------
	// OpenGL Vertex: Position, Normal, UV
	//---------------------------------------
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
	};

	//---------------------------------------
	// OpenGL Texture: ID, type, path
	//---------------------------------------
	struct Texture
	{
		GLuint id;
		string type;
		string path;
	};

	//---------------------------------------
	// Mesh data, geometry and textures
	//---------------------------------------
	class Mesh
	{
	private:
		//---------------------------------------
		// Fields
		//---------------------------------------
		vector<Vertex> vertices;
		vector<GLuint> indices;
		vector<Texture> textures;
		GLuint glVertexArray, glVertexBuf, glIndexBuf;

		//---------------------------------------
		// Methods
		//---------------------------------------
		void X_SetupMesh();

	public:
		//---------------------------------------
		// Methods
		//---------------------------------------
		void Draw(const Shader& shader);

		//---------------------------------------
		// Constructors
		//---------------------------------------
		Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures);
	};
}
