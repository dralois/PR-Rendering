#pragma once

#include <vector>
#include <string>

#pragma warning(push, 0)
#include <boost/filesystem.hpp>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <shader.h>
#pragma warning(pop)

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
		std::string type;
		boost::filesystem::path file;
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
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;
		std::vector<Texture> textures;
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
		Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures);
	};
}
