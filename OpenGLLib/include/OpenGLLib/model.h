#pragma once

#include <string>
#include <vector>

#pragma warning(push, 0)
#include <GL/glew.h>

#include <glm/glm.hpp>

#include <assimp/scene.h>

#include <mesh.h>
#pragma warning(pop)

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
		boost::filesystem::path model;
		boost::filesystem::path texture;
		std::vector<Mesh> meshes;
		std::vector<Texture> loadedTextures;

		//---------------------------------------
		// Methods
		//---------------------------------------
		void X_LoadModel();
		void X_ProcessNode(aiNode* node, const aiScene* scene);
		Mesh X_ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> X_LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
		GLint X_TextureFromFile(const boost::filesystem::path& file);

	public:
		//---------------------------------------
		// Methods
		//---------------------------------------
		void Draw(Shader shader);

		//---------------------------------------
		// Constructors
		//---------------------------------------
		Model(const boost::filesystem::path& modelPath,
			const boost::filesystem::path& texturePath);
	};
}
