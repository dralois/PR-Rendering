#pragma once

#include <string>
#include <vector>

#pragma warning(push, 0)
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <assimp/scene.h>

#include <mesh.h>
#pragma warning(pop)

using namespace std;

namespace Renderer
{
	//---------------------------------------
	// Model consisting of meshes and textures
	//---------------------------------------
	class Model
	{
	private:
		//---------------------------------------
		// Fields
		//---------------------------------------
		string path;
		string model;
		string texture;
		vector<Mesh> meshes;
		vector<Texture> loadedTextures;

		//---------------------------------------
		// Methods
		//---------------------------------------
		void X_LoadModel();
		void X_ProcessNode(aiNode* node, const aiScene* scene);
		Mesh X_ProcessMesh(aiMesh* mesh, const aiScene* scene);
		vector<Texture> X_LoadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
		GLint X_TextureFromFile(const char* file);

	public:
		//---------------------------------------
		// Methods
		//---------------------------------------
		void Draw(Shader shader);

		//---------------------------------------
		// Constructors
		//---------------------------------------
		Model(const std::string& path, const std::string& model, const std::string& texture);
	};
}
