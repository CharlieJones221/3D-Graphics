#pragma once

/*
	Keith Helpers

	

Requires

GLFW: https://www.glfw.org/ - this provides a simple API for creating windows,
contexts and surfaces, receiving input and events.

GLEW: http://glew.sourceforge.net/ - The OpenGL Extension Wrangler Library.


*/

// Glew is a library that handles OpenGL extensions for us
#include <glew.h>
// GLFW is a library to handle OpenGL window setup as well as input
#include <GLFW/glfw3.h>

// GLM is a maths library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// FreeImage is used to load image files
#include <FreeImage.h>

// Assimp is a 3D model loader library
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Common C++ files used 
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
//#include <map>

// IMGUI UI library
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace KeithHelpers
{

	// Loads a whole file into a string e.g. for shader use
	std::string stringFromFile(const std::string& filepath);

	// Check program linked without error (i.e. no errors in the shaders)
	bool LinkProgramShaders(GLuint shaderProgram);

	// Load and compile a shader of shaderType from file shaderFilename. Returns 0 on error.
	GLuint LoadAndCompileShader(GLenum shaderType, const std::string& shaderFilename);

	void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
		const char* message, const void* userParam);

	// Helper to output a glm::vec3
	inline std::string ToString(glm::vec3 v) {
		return "Pos x:" + std::to_string(v.x) +
			" y:" + std::to_string(v.y) +
			" z:" + std::to_string(v.z);
	};

	// Helper to output a glm::vec4 (as a colour)
	inline std::string ToString(glm::vec4 col)
	{
		return "R: " + std::to_string(col.r) +
			"G: " + std::to_string(col.g) +
			"B: " + std::to_string(col.b) +
			"A: " + std::to_string(col.a);
	}

}