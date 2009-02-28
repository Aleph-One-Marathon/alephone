#include <iostream>

#include "OGL_Shader.h"
#include "FileHandler.h"

std::map<std::string, Shader> Shader::Shaders;

class XML_ShaderParser: public XML_ElementParser {

	std::string _vert, _frag, _name;
public:

	virtual bool HandleAttribute(const char *Tag, const char *Value);
	virtual bool AttributesDone();
	XML_ShaderParser(): XML_ElementParser("shader") {}
};

bool XML_ShaderParser::HandleAttribute(const char *Tag, const char *Value) {

	if(StringsEqual(Tag,"name")) {
		_name = Value;
	} else if(StringsEqual(Tag,"vert")) {
		_vert = Value;	
	} else if(StringsEqual(Tag,"frag")) {
		_frag = Value;
	}
	
	return true;
};

bool XML_ShaderParser::AttributesDone() {
	Shader::Shaders[_name] = Shader(_vert, _frag);
	return true;
}

static XML_ShaderParser ShaderParser;
XML_ElementParser *Shader_GetParser() {return &ShaderParser;}

GLcharARB* parseFile(const std::string& name) {

	FileSpecifier fileSpec;
	if (!fileSpec.SetNameWithPath(name.c_str()))
	{
		return NULL;
	}

	FILE *file = fopen(fileSpec.GetPath(), "r");
	if(!file) {
		fprintf(stderr, "%s not found\n", name.c_str());
		return NULL;
	}
	
	fseek(file, 0, SEEK_END);
	size_t n = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	GLcharARB* str = new GLcharARB[n + 1];
	fread(str, 1, n, file);
	fclose(file);
	str[n] = 0;

	return str;
}


GLhandleARB parseShader(GLcharARB* str, GLenum shaderType) {

	GLint status;
	GLhandleARB shader = glCreateShaderObjectARB(shaderType);

	glShaderSourceARB(shader, 1, (const GLcharARB**) &str, NULL);

	glCompileShaderARB(shader);
	glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &status);

	if(status) {
		return shader;
	} else {
		glDeleteObjectARB(shader);
		return NULL;		
	}
}

Shader::Shader(const std::string& vert, const std::string& frag) : _programObj(NULL), _loaded(false) {
	_vert = parseFile(vert);
	_frag = parseFile(frag);
}

void Shader::init() {

	_loaded = true;

	_programObj = glCreateProgramObjectARB();

	assert(_vert);
	GLhandleARB vertexShader = parseShader(_vert, GL_VERTEX_SHADER_ARB);
	assert(vertexShader);
	glAttachObjectARB(_programObj, vertexShader);
	glDeleteObjectARB(vertexShader);
	delete [] _vert; _vert = NULL;

	assert(_frag);
	GLhandleARB fragmentShader = parseShader(_frag, GL_FRAGMENT_SHADER_ARB);
	assert(fragmentShader);
	glAttachObjectARB(_programObj, fragmentShader);
	glDeleteObjectARB(fragmentShader);
	delete [] _frag; _frag = NULL;
	
	glLinkProgramARB(_programObj);

	assert(_programObj);

	glUseProgramObjectARB(_programObj);

	glUniform1iARB(glGetUniformLocationARB(_programObj, "texture0"), 0);
	glUniform1iARB(glGetUniformLocationARB(_programObj, "texture1"), 1);
	glUniform1iARB(glGetUniformLocationARB(_programObj, "texture2"), 2);
	glUniform1iARB(glGetUniformLocationARB(_programObj, "texture3"), 3);	
	glUniform1fARB(glGetUniformLocationARB(_programObj, "time"), 0.0);


	glUseProgramObjectARB(NULL);

	assert(glGetError() == GL_NO_ERROR);
}

void Shader::setFloat(const char* name, float f) {

	glUseProgramObjectARB(_programObj);
	glUniform1fARB(glGetUniformLocationARB(_programObj, name), f);
	glUseProgramObjectARB(NULL);	
}

Shader::~Shader() {
	if(_programObj) { glDeleteObjectARB(_programObj); }
}

void Shader::enable() {
	if(!_loaded) { init(); }
	glUseProgramObjectARB(_programObj);
}

void Shader::disable() {
	glUseProgramObjectARB(NULL);
}
