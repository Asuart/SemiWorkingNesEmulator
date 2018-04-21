#include "ShaderLib.h"
#include <iostream>
#include <fstream>
using namespace std;

std::string readFile(const char *filePath) {
	std::string content;
	std::ifstream fileStream(filePath, std::ios::in);

	if (!fileStream.is_open()) {
		std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
		return "";
	}

	std::string line = "";
	while (!fileStream.eof()) {
		std::getline(fileStream, line);
		content.append(line + "\n");
	}

	fileStream.close();
	return content;
}

string ShaderLib::SearchShader(string name) {
	// TODO: search shader by name
	return  "TODO:";
}

GLuint ShaderLib::LoadShader(string name, string path) {
	cout << "Compiling shader: " << name << endl;

	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	std::string vertShaderStr = readFile((path + name + ".vs").c_str());
	std::string fragShaderStr = readFile((path + name + ".fs").c_str());

	const char *vertShaderSrc = vertShaderStr.c_str();
	const char *fragShaderSrc = fragShaderStr.c_str();

	GLint success;
	GLchar infoLog[512];

	// Compile vertex shader
	std::cout << "Compiling vertex shader." << std::endl;
	glShaderSource(vertShader, 1, &vertShaderSrc, NULL);
	glCompileShader(vertShader);

	// Check vertex shader
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// Compile fragment shader
	std::cout << "Compiling fragment shader." << std::endl;
	glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
	glCompileShader(fragShader);

	// Check fragment shader
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	std::cout << "Linking program" << std::endl;
	GLuint program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		std::cout << "Linking failed\n" << infoLog << std::endl;
	}

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	shaders.insert(make_pair(name, program));

	return program;
}

GLuint ShaderLib::GetShader(string name) {
	auto it = shaders.find(name);
	if (it != shaders.end()) {
		return it->second;
	}
	else {
		return LoadShader(name);
	}
}

ShaderLib::ShaderLib() {

}


ShaderLib::~ShaderLib() {
}
