#include <shader.h>

#include <fstream>
#include <sstream>
#include <iostream>

namespace Renderer
{
	//---------------------------------------
	// Try to load a shader from disk
	//---------------------------------------
	bool Shader::X_LoadFile(const std::string& vertPath, std::string& vertCode, const std::string& fragPath, std::string& fragCode)
	{
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		vShaderFile.exceptions(std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::badbit);
		// Try to load file
		try
		{
			// Open file
			vShaderFile.open(vertPath);
			fShaderFile.open(fragPath);
			std::stringstream vShaderStream, fShaderStream;
			// Read contents into streams
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			// Close file handlers
			vShaderFile.close();
			fShaderFile.close();
			// Convert stream into string
			vertCode = vShaderStream.str();
			fragCode = fShaderStream.str();
			return true;
		}
		catch (std::ifstream::failure e)
		{
			// Print error
			std::cout << "Shader load error:" << e.what() << std::endl;
			return false;
		}
	}

	//---------------------------------------
	// Activate shader for drawing
	//---------------------------------------
	void Shader::Use()
	{
		glUseProgram(shaderProgram);
	}

	//---------------------------------------
	// Create shader from provided files
	//---------------------------------------
	Shader::Shader(const std::string& vertPath, const std::string& fragPath)
	{
		GLint success;
		GLuint vertex, fragment;
		GLchar infoLog[512];

		// Load shader file
		std::string vertCode, fragCode;
		if (!X_LoadFile(vertPath, vertCode, fragPath, fragCode))
			return;

		// Convert
		const GLchar* vShaderCode = vertCode.c_str();
		const GLchar* fShaderCode = fragCode.c_str();

		// Compile & create vertex shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		// Print compile errors if any
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cout << "Shader vertex create error:" << infoLog << std::endl;
		}

		// Compile & create fragment shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		// Print compile errors if any
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			std::cout << "Shader fragment create error:" << infoLog << std::endl;
		}

		// Create shader program
		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertex);
		glAttachShader(shaderProgram, fragment);
		glLinkProgram(shaderProgram);
		// Print linking errors if any
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
			std::cout << "Shader program link error:" << infoLog << std::endl;
		}

		// Cleanup
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}
}
