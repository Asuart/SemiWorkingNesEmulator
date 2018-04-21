#pragma once
#include <GL/glew.h>
#include <map>
#include <string>

using namespace std;

class ShaderLib {
	map<string, GLuint> shaders;

	string SearchShader(string name);
	GLuint LoadShader(string name, string path = "C:/Users/Asuart/Desktop/Shaders/");
public:
	GLuint GetShader(string name);

	ShaderLib();
	~ShaderLib();
};

static ShaderLib ShaderLibrary;