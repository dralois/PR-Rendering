#pragma once

#include <string>

#pragma warning(push, 0)
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <GL/glew.h>
#pragma warning(pop)

namespace Renderer
{
	//---------------------------------------
	// GLSL shader
	//---------------------------------------
	class Shader
	{
	private:
		//---------------------------------------
		// Fields
		//---------------------------------------
		GLuint shaderProgram;

		//---------------------------------------
		// Methods
		//---------------------------------------
		bool X_LoadFile(const boost::filesystem::path& vertPath, std::string& vertCode,
			const boost::filesystem::path& fragPath, std::string& fragCode);

	public:
		//---------------------------------------
		// Methods
		//---------------------------------------
		void Use();

		//---------------------------------------
		// Properties
		//---------------------------------------
		inline GLuint GetProgram() const { return shaderProgram; };

		//---------------------------------------
		// Constructors
		//---------------------------------------
		Shader(const boost::filesystem::path& vertPath, const boost::filesystem::path& fragPath);
	};
}
