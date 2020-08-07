#include <model.h>

#include <iostream>
#include <sstream>

#pragma warning(push, 0)
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <opencv2/opencv.hpp>
#pragma warning(pop)

namespace Renderer
{
	//---------------------------------------
	// Draw model with given shader
	//---------------------------------------
	void Model::Draw(Shader shader)
	{
		// Issue draw for each mesh
		for (GLuint i = 0; i < meshes.size(); i++)
		{
			meshes[i].Draw(shader);
		}
	}

	//---------------------------------------
	// Creates a texture and returns slot
	//---------------------------------------
	GLint Model::X_TextureFromFile(const boost::filesystem::path& texPath)
	{
		GLuint textureId;
		int width, height;

		std::cout << "Bind texture:" << texPath.filename() << std::endl;

		// Load texture from file
		cv::Mat image = cv::imread(texPath.string());

		// Create and bind texture slot
		glDeleteTextures(1, &textureId);
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		// Bind actual texture & create mip maps
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, image.ptr());
		glGenerateMipmap(GL_TEXTURE_2D);

		// Set up sampler
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Make sure it cant be modified
		glBindTexture(GL_TEXTURE_2D, 0);

		// Return the slot
		return textureId;
	}

	//---------------------------------------
	// Load a mesh from given path
	//---------------------------------------
	void Model::X_LoadModel()
	{
		// Read file
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(modelPath.string(), aiProcess_Triangulate | aiProcess_FlipUVs);

		// Error handling
		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "Mesh load error:" << importer.GetErrorString() << std::endl;
			return;
		}
		else if (!scene->HasMeshes())
		{
			std::cout << "Mesh load error:" << modelPath.filename() << " has no mesh" << std::endl;
			return;
		}

		// Process mesh recursively
		X_ProcessNode(scene->mRootNode, scene);

		// Cleanup
		importer.FreeScene();
	}

	//---------------------------------------
	// Load all meshes from an assimp node
	//---------------------------------------
	void Model::X_ProcessNode(aiNode* node, const aiScene* scene)
	{
		// Save meshes of current node
		for (GLuint i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(X_ProcessMesh(mesh, scene));
		}
		// Save meshes of child nodes
		for (GLuint i = 0; i < node->mNumChildren; i++)
		{
			X_ProcessNode(node->mChildren[i], scene);
		}
	}

	//---------------------------------------
	// Load a mesh into memory
	//---------------------------------------
	Mesh Model::X_ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;
		std::vector<Texture> textures;

		// Create & save vertices
		for (GLuint i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector;
			// Positions
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;
			// Normals
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;
			// UVs
			if (mesh->mTextureCoords[0])
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else
			{
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			}
			// Save in vector
			vertices.push_back(vertex);
		}

		// Create & save indices
		for (GLuint i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (GLuint j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		// Create & save materials
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			// It is assumed, that the names are by convention
			std::vector<Texture> diffuseMaps = X_LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			std::vector<Texture> specularMaps = X_LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		}

		// If no materials / textures
		if (textures.empty())
		{
			Texture tex;
			// Create and bind external texture
			tex.id = X_TextureFromFile(texturePath);
			tex.type = "texture_external";
			tex.file = texturePath;
			textures.push_back(tex);
		}

		// Create and return the mesh
		return Mesh(vertices, indices, textures);
	}

	//---------------------------------------
	// Load all textures from a given material
	//---------------------------------------
	std::vector<Texture> Model::X_LoadMaterialTextures(
		aiMaterial* mat,
		aiTextureType type,
		const std::string& typeName
	)
	{
		std::vector<Texture> textures;

		// For each texture
		for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
		{
			// Save info
			aiString fileName;
			mat->GetTexture(type, i, &fileName);

			// Check if texture has been loaded already
			GLboolean skip = false;
			for (GLuint j = 0; j < loadedTextures.size(); j++)
			{
				// If so, skip
				if (loadedTextures[j].file.filename() == fileName.C_Str())
				{
					textures.push_back(loadedTextures[j]);
					skip = true;
					break;
				}
			}

			// If new texture
			if (!skip)
			{
				Texture texture;
				// Build path
				boost::filesystem::path texFile(texturePath.parent_path());
				texFile.append(fileName.C_Str());
				// Create and bind it
				texture.id = X_TextureFromFile(texFile);
				texture.type = typeName;
				texture.file = texFile;
				textures.push_back(texture);
				// Save in vector
				loadedTextures.push_back(texture);
			}
		}

		// Return all textures
		return textures;
	}

	//---------------------------------------
	// New model from given path
	//---------------------------------------
	Model::Model(
		const boost::filesystem::path& modelPath,
		const boost::filesystem::path& texturePath
	) :
		modelPath(modelPath),
		texturePath(texturePath)
	{
		X_LoadModel();
	}
}
