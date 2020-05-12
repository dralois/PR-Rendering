#pragma once

#include <string>

#pragma warning(push, 0)
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
		bool X_LoadFile(const std::string& vertPath, std::string& vertCode, const std::string& fragPath, std::string& fragCode);

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
		Shader(const std::string& vertPath, const std::string& fragPath);
	};
}
