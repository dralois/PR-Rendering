#include <mesh.h>

#include <sstream>

namespace Renderer
{
	//---------------------------------------
	// Draw mesh with given shader
	//---------------------------------------
	void Mesh::Draw(const Shader& shader)
	{
		GLuint diffuseNr = 1;
		GLuint specularNr = 1;
		// For each texture
		for (GLuint i = 0; i < textures.size(); i++)
		{
			// Activate texture slot
			glActiveTexture(GL_TEXTURE0 + i);

			stringstream ss;
			string name = textures[i].type;
			// Increase texture count
			if (name == "texture_diffuse")
			{
				ss << name << diffuseNr++;
			}
			else if (name == "texture_specular")
			{
				ss << name << specularNr++;
			}
			// Assumption: This texture always exists in shader
			else
			{
				ss << "texture_diffuse";
			}

			// Set texture uniform
			glUniform1i(glGetUniformLocation(shader.GetProgram(), ss.str().c_str()), i);
			// Bind texture
			glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
		}

		// Bind mesh and draw it
		glBindVertexArray(this->glVertexArray);
		glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// Reset texture slots
		for (GLuint i = 0; i < this->textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	//---------------------------------------
	// Create mesh in OpenGL
	//---------------------------------------
	void Mesh::X_SetupMesh()
	{
		// Create buffers
		glGenVertexArrays(1, &glVertexArray);
		glGenBuffers(1, &glVertexBuf);
		glGenBuffers(1, &glIndexBuf);

		// Bind vertices
		glBindVertexArray(glVertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, glVertexBuf);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		// Bind indices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glIndexBuf);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

		// Create and enable layout
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));

		// Make sure vertices can't be modified
		glBindVertexArray(0);
	}

	//---------------------------------------
	// Create mesh from vertices and textures
	//---------------------------------------
	Mesh::Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures) :
		vertices(vertices),
		indices(indices),
		textures(textures)
	{
		X_SetupMesh();
	}
}
