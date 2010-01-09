/*
 OGL_SHADER.CPP
 
 Copyright (C) 2009 by Clemens Unterkofler and the Aleph One developers
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 This license is contained in the file "COPYING",
 which is included with this source code; it is available online at
 http://www.gnu.org/licenses/gpl.html
 
 Implements OpenGL vertex/fragment shader class
 */
#include <iostream>

// gl_ClipVertex workaround
// In Mac OS X 10.4, setting gl_ClipVertex causes a black screen.
// Unfortunately, it's required for proper 5-D space on other
// systems. This workaround comments out its use under 10.4.
#if (defined(__APPLE__) && defined(__MACH__))
#include <sys/utsname.h>

// On Tiger, uname -r starts with "8."
inline bool DisableClipVertex() {
	struct utsname uinfo;
	uname(&uinfo);
	if (uinfo.release[0] == '8' && uinfo.release[1] == '.')
		return true;
	return false;
}
#else
inline bool DisableClipVertex() { return false; }
#endif

#include "OGL_Shader.h"
#include "FileHandler.h"
#include "OGL_Setup.h"

static std::map<std::string, std::string> defaultVertexPrograms;
static std::map<std::string, std::string> defaultFragmentPrograms;
void initDefaultPrograms();

std::map<std::string, Shader> Shader::Shaders;

class XML_ShaderParser: public XML_ElementParser {

	FileSpecifier _vert, _frag;
	std::string _name;
	int16 _passes;
public:

	virtual bool HandleAttribute(const char *Tag, const char *Value);
	virtual bool AttributesDone();
	virtual bool ResetValues();
	XML_ShaderParser(): XML_ElementParser("shader"), _passes(-1) {}
};

bool XML_ShaderParser::HandleAttribute(const char *Tag, const char *Value) {

	if(StringsEqual(Tag,"name")) {
		_name = Value;
		return true;
	} else if(StringsEqual(Tag,"vert")) {
		_vert.SetNameWithPath(Value);
		return true;
	} else if(StringsEqual(Tag,"frag")) {
		_frag.SetNameWithPath(Value);
		return true;
	} else if(StringsEqual(Tag,"passes")) {
		return ReadInt16Value(Value,_passes);
	}
	UnrecognizedTag();
	return true;
};

bool XML_ShaderParser::AttributesDone() {
    initDefaultPrograms();
	Shader::Shaders[_name] = Shader(_name, _vert, _frag, _passes);
	return true;
}

bool XML_ShaderParser::ResetValues() {
	Shader::Shaders.clear();
	return true;
}

static XML_ShaderParser ShaderParser;
XML_ElementParser *Shader_GetParser() {return &ShaderParser;}

GLcharARB* parseFile(FileSpecifier& fileSpec) {

	if (fileSpec == FileSpecifier() || !fileSpec.Exists()) {
		return NULL;
	}

	OpenedFile file;
	if (!fileSpec.Open(file))
	{
		fprintf(stderr, "%s not found\n", fileSpec.GetPath());
		return NULL;
	}

	int32 length;
	file.GetLength(length);
	
	GLcharARB* str = new GLcharARB[length + 1];
	file.Read(length, str);
	str[length] = 0;

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

Shader* Shader::get(const std::string& name) {
	
    if (Shaders.count(name) > 0) {
        return &Shaders[name];			
    }
    initDefaultPrograms();
    if ((defaultVertexPrograms.count(name) > 0) &&
        (defaultFragmentPrograms.count(name) > 0)) {
        Shaders[name] = Shader(name);
        return &Shaders[name];
    }
    fprintf(stderr, "No shader for %s\n", name.c_str());
    return NULL;
}

void Shader::unloadAll() {
	for (std::map<std::string, Shader>::iterator it = Shaders.begin();
		 it != Shaders.end();
		 ++it)
		it->second.unload();
}

Shader::Shader(const std::string& name) : _programObj(NULL), _passes(-1), _loaded(false), _vert(NULL), _frag(NULL) {
    initDefaultPrograms();
    if (defaultVertexPrograms.count(name) > 0) {
        _vert = new GLcharARB[defaultVertexPrograms[name].size() + 1];
        strcpy(_vert, defaultVertexPrograms[name].c_str());
    }
    if (defaultFragmentPrograms.count(name) > 0) {
        _frag = new GLcharARB[defaultFragmentPrograms[name].size() + 1];
        strcpy(_frag, defaultFragmentPrograms[name].c_str());
    }
}    

Shader::Shader(const std::string& name, FileSpecifier& vert, FileSpecifier& frag, int16& passes) : _programObj(NULL), _passes(passes), _loaded(false) {
    initDefaultPrograms();
	
	_vert = parseFile(vert);
	if (!_vert && defaultVertexPrograms.count(name) > 0) {
        _vert = new GLcharARB[defaultVertexPrograms[name].size() + 1];
        strcpy(_vert, defaultVertexPrograms[name].c_str());
    }

	_frag = parseFile(frag);
	if (!_frag && defaultFragmentPrograms.count(name) > 0) {
        _frag = new GLcharARB[defaultFragmentPrograms[name].size() + 1];
        strcpy(_frag, defaultFragmentPrograms[name].c_str());
    }
}

void Shader::init() {

	_loaded = true;

	_programObj = glCreateProgramObjectARB();

	assert(_vert);
	GLhandleARB vertexShader = parseShader(_vert, GL_VERTEX_SHADER_ARB);
	assert(vertexShader);
	glAttachObjectARB(_programObj, vertexShader);
	glDeleteObjectARB(vertexShader);

	assert(_frag);
	GLhandleARB fragmentShader = parseShader(_frag, GL_FRAGMENT_SHADER_ARB);
	assert(fragmentShader);
	glAttachObjectARB(_programObj, fragmentShader);
	glDeleteObjectARB(fragmentShader);
	
	glLinkProgramARB(_programObj);

	assert(_programObj);

	glUseProgramObjectARB(_programObj);

	glUniform1iARB(glGetUniformLocationARB(_programObj, "texture0"), 0);
	glUniform1iARB(glGetUniformLocationARB(_programObj, "texture1"), 1);
	glUniform1iARB(glGetUniformLocationARB(_programObj, "texture2"), 2);
	glUniform1iARB(glGetUniformLocationARB(_programObj, "texture3"), 3);	
	glUniform1fARB(glGetUniformLocationARB(_programObj, "time"), 0.0);
	glUniform1fARB(glGetUniformLocationARB(_programObj, "wobble"), 0.0);
	glUniform1fARB(glGetUniformLocationARB(_programObj, "flare"), 0.0);
	glUniform1fARB(glGetUniformLocationARB(_programObj, "bloomScale"), 0.0);
	glUniform1fARB(glGetUniformLocationARB(_programObj, "bloomShift"), 0.0);
	glUniform1fARB(glGetUniformLocationARB(_programObj, "repeat"), 0.0);

	glUseProgramObjectARB(NULL);

//	assert(glGetError() == GL_NO_ERROR);
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

void Shader::unload() {
	if(_programObj) {
		glDeleteObjectARB(_programObj);
		_programObj = NULL;
		_loaded = false;
	}
}

int16 Shader::passes() {
	return _passes;
}

void initDefaultPrograms() {
    if (defaultVertexPrograms.size() > 0)
        return;
    
    defaultVertexPrograms["blur"] = ""
        "varying vec4 vertexColor;\n"
        "void main(void) {\n"
        "	gl_TexCoord[0] = gl_MultiTexCoord0;\n"
        "	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
        "	vertexColor = gl_Color;\n"
        "}\n";
    defaultFragmentPrograms["blur"] = ""
        "uniform sampler2DRect texture0;\n"
        "uniform float offsetx;\n"
        "uniform float offsety;\n"
        "uniform float pass;\n"
        "varying vec4 vertexColor;\n"
        "void main (void) {\n"
        "	vec2 s = vec2(offsetx, offsety);\n"
        "	// Thanks to Renaud Bedard - http://theinstructionlimit.com/?p=43\n"
        "	vec3 t = 0.1468 * texture2DRect(texture0, gl_TexCoord[0].xy).rgb;\n"
        "	t += 0.2506 * texture2DRect(texture0, gl_TexCoord[0].xy - 1.45*s).rgb;\n"
        "	t += 0.2506 * texture2DRect(texture0, gl_TexCoord[0].xy + 1.45*s).rgb;\n"
        "	t += 0.1332 * texture2DRect(texture0, gl_TexCoord[0].xy - 3.39*s).rgb;\n"
        "	t += 0.1332 * texture2DRect(texture0, gl_TexCoord[0].xy + 3.39*s).rgb;\n"
        "	t += 0.0427 * texture2DRect(texture0, gl_TexCoord[0].xy - 5.33*s).rgb;\n"
        "	t += 0.0427 * texture2DRect(texture0, gl_TexCoord[0].xy + 5.33*s).rgb;\n"
        "	gl_FragColor = vec4(t, 1.0) * vertexColor;\n"
        "}\n";    
    
    defaultVertexPrograms["bloom"] = defaultVertexPrograms["blur"];
    defaultFragmentPrograms["bloom"] = ""
		"uniform sampler2DRect texture0;\n"
		"varying vec4 vertexColor;\n"
		"void main (void) {\n"
		"	vec4 color = texture2DRect(texture0, gl_TexCoord[0].xy);\n"
		"	gl_FragColor = color * vertexColor;\n"
		"}\n";    
    
    defaultVertexPrograms["landscape"] = ""
        "varying vec3 viewDir;\n"
        "varying float texScale;\n"
        "varying float texOffset;\n"
        "varying vec4 vertexColor;\n"
        "void main(void) {\n"
        "	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
        "	gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;\n"
        "	vec4 v = gl_ModelViewMatrixInverse * vec4(0.0, 0.0, 0.0, 1.0);\n"
        "	viewDir = (gl_Vertex - v).xyz;\n"
        "	texScale = gl_TextureMatrix[0][1][1];\n"
        "	texOffset = gl_TextureMatrix[0][3][1];\n"
        "	vertexColor = gl_Color;\n"
        "}\n";
    defaultFragmentPrograms["landscape"] = ""
        "uniform sampler2D texture0;\n"
        "uniform float repeat;\n"
        "uniform float usefog;\n"
        "varying vec3 viewDir;\n"
        "varying float texScale;\n"
        "varying float texOffset;\n"
        "varying vec4 vertexColor;\n"
        "void main(void) {\n"
        "	float pi = 2.0 * asin(1.0);\n"
        "	vec3 viewv = normalize(viewDir);\n"
        "	float x = atan(viewv.x, viewv.y) / (2.0 * pi) * repeat;\n"
        "	float y = 0.5/abs(texScale) - sign(texOffset) + texOffset + asin(viewv.z) * 0.3 * sign(texScale);\n"
        "   vec4 color = texture2D(texture0, vec2(-x + 0.25*repeat, y));\n"
        "   vec3 intensity = color.rgb;\n"
        "   if (usefog > 0.0) {\n"
        "       intensity = gl_Fog.color.rgb;\n"
        "   }\n"
        "	gl_FragColor = vec4(intensity, 1.0);\n"
        "}\n";
    defaultVertexPrograms["landscape_bloom"] = defaultVertexPrograms["landscape"];
    defaultFragmentPrograms["landscape_bloom"] = ""
        "uniform sampler2D texture0;\n"
        "uniform float repeat;\n"
        "uniform float usefog;\n"
        "uniform float bloomScale;\n"
        "uniform float bloomShift;\n"
        "varying vec3 viewDir;\n"
        "varying float texScale;\n"
        "varying float texOffset;\n"
        "varying vec4 vertexColor;\n"
        "void main(void) {\n"
        "	float pi = 2.0 * asin(1.0);\n"
        "	vec3 viewv = normalize(viewDir);\n"
        "	float x = atan(viewv.x, viewv.y) / (2.0 * pi) * repeat;\n"
        "	float y = 0.5/abs(texScale) - sign(texOffset) + texOffset + asin(viewv.z) * 0.3 * sign(texScale);\n"
        "   vec4 color = texture2D(texture0, vec2(-x + 0.25*repeat, y));\n"
        "   vec3 intensity = vec3(1.0, 1.0, 1.0);\n"
        "	intensity = color.rgb * clamp(intensity * bloomScale + bloomShift, 0.0, 1.0);\n"
        "   if (usefog > 0.0) {\n"
        "       intensity = vec3(0.0, 0.0, 0.0);\n"
        "   }\n"
        "	gl_FragColor = vec4(intensity, 1.0);\n"
    "}\n";
	
    defaultVertexPrograms["sprite"] = ""
        "uniform float depth;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying float MLxLOG2E;\n"
        "void main(void) {\n"
        "	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
        "	gl_Position.z = gl_Position.z + depth*gl_Position.z/65536.0;\n"
        "	gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;\n"
        "	vec4 v = gl_ModelViewMatrixInverse * vec4(0.0, 0.0, 0.0, 1.0);\n"
        "	viewDir = (gl_Vertex - v).xyz;\n"
        "	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;\n"
        "	vertexColor = gl_Color;\n"
        "	FDxLOG2E = -gl_Fog.density * gl_Fog.density * 1.442695;\n"
        "	MLxLOG2E = -0.0000003 * 1.442695;\n"
        "}\n";    
    defaultFragmentPrograms["sprite"] = ""
        "uniform sampler2D texture0;\n"
        "uniform float glow;\n"
        "uniform float flare;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying float MLxLOG2E;\n"
        "void main (void) {\n"
        "	float flash = exp2((flare - 1.0) * 2.0);\n"
        "	float mlFactor = exp2(MLxLOG2E * dot(viewDir, viewDir) / flash + 1.0); \n"
        "	mlFactor = clamp(mlFactor, 0.0, flare - 0.5) * 0.5;\n"
        "	vec4 color = texture2D(texture0, gl_TexCoord[0].xy);\n"
        "	vec3 intensity = color.rgb * clamp(vertexColor.rgb + mlFactor, glow, 1.0);\n"
        "	float fogFactor = clamp(exp2(FDxLOG2E * dot(viewDir, viewDir)), 0.0, 1.0);\n"
        "	gl_FragColor = vec4(mix(gl_Fog.color.rgb, intensity, fogFactor), vertexColor.a * color.a);\n"
        "}\n";
    defaultVertexPrograms["sprite_bloom"] = defaultVertexPrograms["sprite"];
    defaultFragmentPrograms["sprite_bloom"] = ""
        "uniform sampler2D texture0;\n"
        "uniform float glow;\n"
        "uniform float bloomScale;\n"
        "uniform float bloomShift;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying float MLxLOG2E;\n"
        "void main (void) {\n"
        "	vec4 color = texture2D(texture0, gl_TexCoord[0].xy);\n"
        "	vec3 intensity = clamp(vertexColor.rgb, glow, 1.0);\n"
        "	intensity = color.rgb * clamp(intensity * bloomScale + bloomShift, 0.0, 1.0);\n"
        "	float fogFactor = clamp(exp2(FDxLOG2E * dot(viewDir, viewDir)), 0.0, 1.0);\n"
        "	gl_FragColor = vec4(mix(vec3(0.0, 0.0, 0.0), intensity, fogFactor), vertexColor.a * color.a);\n"
        "}\n";
    
    defaultVertexPrograms["invincible"] = defaultVertexPrograms["sprite"];
    defaultFragmentPrograms["invincible"] = ""
        "uniform sampler2D texture0;\n"
        "uniform float time;\n"
        "uniform float usestatic;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "void main(void) {\n"
        "	float a = fract(sin(usestatic*(gl_TexCoord[0].x * 133.0 + gl_TexCoord[0].y * 471.0) + time * 7.0) * 43757.0); \n"
        "	float b = fract(sin(usestatic*(gl_TexCoord[0].x * 2331.0 + gl_TexCoord[0].y * 63.0) + time * 3.0) * 32451.0); \n"
        "	float c = fract(sin(usestatic*(gl_TexCoord[0].x * 41.0 + gl_TexCoord[0].y * 12911.0) + time * 31.0) * 34563.0);\n"
        "	vec4 color = texture2D(texture0, gl_TexCoord[0].xy);\n"
        "	vec3 intensity = vec3(a, b, c);\n"
        "	float fogFactor = clamp(exp2(FDxLOG2E * dot(viewDir, viewDir)), 0.0, 1.0);\n"
        "	gl_FragColor = vec4(mix(gl_Fog.color.rgb, intensity, fogFactor), vertexColor.a * color.a);\n"
        "}\n";
    defaultVertexPrograms["invincible_bloom"] = defaultVertexPrograms["invincible"];
    defaultFragmentPrograms["invincible_bloom"] = ""
        "uniform sampler2D texture0;\n"
        "uniform float time;\n"
        "uniform float usestatic;\n"
        "uniform float bloomScale;\n"
        "uniform float bloomShift;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "void main(void) {\n"
        "	vec4 color = texture2D(texture0, gl_TexCoord[0].xy);\n"
        "	vec3 intensity = vec3(0.0, 0.0, 0.0);\n"
        "	float fogFactor = exp2(FDxLOG2E * dot(viewDir, viewDir));\n"
        "	gl_FragColor = vec4(mix(vec3(0.0, 0.0, 0.0), intensity, fogFactor), vertexColor.a * color.a);\n"
        "}\n";
    
    defaultVertexPrograms["invisible"] = defaultVertexPrograms["sprite"];
    defaultFragmentPrograms["invisible"] = ""
        "uniform sampler2D texture0;\n"
        "uniform float visibility;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "void main(void) {\n"
        "	vec4 color = texture2D(texture0, gl_TexCoord[0].xy);\n"
        "   vec3 intensity = vec3(0.0, 0.0, 0.0);\n"
        "	float fogFactor = clamp(exp2(FDxLOG2E * dot(viewDir, viewDir)), 0.0, 1.0);\n"
        "	gl_FragColor = vec4(mix(gl_Fog.color.rgb, intensity, fogFactor), vertexColor.a * color.a * visibility);\n"
        "}\n";
    defaultVertexPrograms["invisible_bloom"] = defaultVertexPrograms["invisible"];
    defaultFragmentPrograms["invisible_bloom"] = ""
        "uniform sampler2D texture0;\n"
        "uniform float visibility;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "void main(void) {\n"
        "	vec4 color = texture2D(texture0, gl_TexCoord[0].xy);\n"
        "   vec3 intensity = vec3(0.0, 0.0, 0.0);\n"
        "	float fogFactor = clamp(exp2(FDxLOG2E * dot(viewDir, viewDir)), 0.0, 1.0);\n"
        "	gl_FragColor = vec4(mix(vec3(0.0, 0.0, 0.0), intensity, fogFactor), vertexColor.a * color.a * visibility);\n"
        "}\n";
	
    defaultVertexPrograms["wall"] = ""
        "uniform float depth;\n"
        "varying vec3 viewXY;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying float MLxLOG2E;\n"
        "void main(void) {\n"
        "	gl_Position  = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
        "	gl_Position.z = gl_Position.z + depth*gl_Position.z/65536.0;\n"
        "	gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;\n"
        "	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;\n"
        "	/* SETUP TBN MATRIX in normal matrix coords, gl_MultiTexCoord1 = tangent vector */\n"
        "	vec3 n = normalize(gl_NormalMatrix * gl_Normal);\n"
        "	vec3 t = normalize(gl_NormalMatrix * gl_MultiTexCoord1.xyz);\n"
        "	vec3 b = normalize(cross(n, t) * gl_MultiTexCoord1.w);\n"
        "	/* (column wise) */\n"
        "	mat3 tbnMatrix = mat3(t.x, b.x, n.x, t.y, b.y, n.y, t.z, b.z, n.z);\n"
        "	\n"
        "	/* SETUP VIEW DIRECTION in unprojected local coords */\n"
        "	viewDir = tbnMatrix * (gl_ModelViewMatrix * gl_Vertex).xyz;\n"
        "	viewXY = -(gl_TextureMatrix[0] * vec4(viewDir.xyz, 1.0)).xyz;\n"
        "	viewDir = -viewDir;\n"
        "	vertexColor = gl_Color;\n"
        "	FDxLOG2E = -gl_Fog.density * gl_Fog.density * 1.442695;\n"
        "	MLxLOG2E = -0.0000003 * 1.442695;\n"
        "}\n";
    defaultFragmentPrograms["wall"] = ""
        "uniform sampler2D texture0;\n"
        "uniform float wobble;\n"
        "uniform float glow;\n"
        "uniform float flare;\n"
        "varying vec3 viewXY;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying float MLxLOG2E;\n"
        "void main (void) {\n"
        "	vec3 texCoords = vec3(gl_TexCoord[0].xy, 0.0);\n"
        "	texCoords += vec3(normalize(viewXY).yx * wobble, 0.0);\n"
        "	float flash = exp2((flare - 1.0) * 2.0);\n"
        "	float mlFactor = exp2(MLxLOG2E * dot(viewDir, viewDir) / flash + 1.0); \n"
        "	mlFactor = clamp(mlFactor, 0.0, flare - 0.5) * 0.5;\n"
        "	vec3 viewv = normalize(viewDir);\n"
        "	vec3 norm = vec3(0.0, 0.0, 1.0);\n"
        "	float diffuse = 0.5 + abs(dot(norm, viewv))*0.5;\n"
        "   if (glow > 0.001) {\n"
        "       diffuse = 1.0;\n"
        "   }\n"
        "	vec4 color = texture2D(texture0, texCoords.xy);\n"
        "	vec3 intensity = color.rgb * clamp((vertexColor.rgb + mlFactor) * diffuse, glow, 1.0);\n"
        "	float fogFactor = clamp(exp2(FDxLOG2E * dot(viewDir, viewDir)), 0.0, 1.0);\n"
        "	gl_FragColor = vec4(mix(gl_Fog.color.rgb, intensity, fogFactor), vertexColor.a * color.a);\n"
        "}\n";
    defaultVertexPrograms["wall_bloom"] = defaultVertexPrograms["wall"];
    defaultFragmentPrograms["wall_bloom"] = ""
        "uniform sampler2D texture0;\n"
        "uniform float wobble;\n"
        "uniform float glow;\n"
        "uniform float flare;\n"
        "uniform float bloomScale;\n"
        "uniform float bloomShift;\n"
        "varying vec3 viewXY;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying float MLxLOG2E;\n"
        "void main (void) {\n"
        "	vec3 texCoords = vec3(gl_TexCoord[0].xy, 0.0);\n"
        "	texCoords += vec3(normalize(viewXY).yx * wobble, 0.0);\n"
        "	float flash = exp2((flare - 1.0) * 2.0);\n"
        "	float mlFactor = exp2(MLxLOG2E * dot(viewDir, viewDir) / flash + 1.0); \n"
        "	mlFactor = clamp(mlFactor, 0.0, flare - 0.5) * 0.5;\n"
        "	vec3 viewv = normalize(viewDir);\n"
        "	vec3 norm = vec3(0.0, 0.0, 1.0);\n"
        "	float diffuse = 0.5 + abs(dot(norm, viewv))*0.5;\n"
        "   if (glow > 0.001) {\n"
        "       diffuse = 1.0;\n"
        "   }\n"
        "	vec4 color = texture2D(texture0, texCoords.xy);\n"
        "	vec3 intensity = clamp(vertexColor.rgb, glow, 1.0);\n"
        "	vec3 corr = clamp((vertexColor.rgb + mlFactor) * (0.5*diffuse + 0.5), glow, 1.0);\n"
        "	intensity = intensity - 0.33*(corr - intensity);\n"
        "	intensity = color.rgb * clamp(intensity * bloomScale + bloomShift, 0.0, 1.0);\n"
        "	float fogFactor = clamp(exp2(FDxLOG2E * dot(viewDir, viewDir)), 0.0, 1.0);\n"
        "	gl_FragColor = vec4(mix(vec3(0.0, 0.0, 0.0), intensity, fogFactor), vertexColor.a * color.a);\n"
        "}\n";
    
    defaultVertexPrograms["bump"] = defaultVertexPrograms["wall"];
    defaultFragmentPrograms["bump"] = ""
        "uniform sampler2D texture0;\n"
        "uniform sampler2D texture1;\n"
        "uniform float wobble;\n"
        "uniform float glow;\n"
        "uniform float flare;\n"
        "varying vec3 viewXY;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying float MLxLOG2E;\n"
        "void main (void) {\n"
        "	vec3 texCoords = vec3(gl_TexCoord[0].xy, 0.0);\n"
        "	texCoords += vec3(normalize(viewXY).yx * wobble, 0.0);\n"
        "	float flash = exp2((flare - 1.0) * 2.0);\n"
        "	float mlFactor = exp2(MLxLOG2E * dot(viewDir, viewDir) / flash + 1.0); \n"
        "	mlFactor = clamp(mlFactor, 0.0, flare - 0.5) * 0.5;\n"
        "	vec3 viewv = normalize(viewDir);\n"
        "	// iterative parallax mapping\n"
        "	float scale = 0.010;\n"
        "	float bias = -0.005;\n"
        "	for(int i = 0; i < 4; ++i) {\n"
        "		vec4 normal = texture2D(texture1, texCoords.xy);\n"
        "		float h = normal.a * scale + bias;\n"
        "		texCoords.x += h * viewv.x;\n"
        "		texCoords.y -= h * viewv.y;\n"
        "	}\n"
        "	vec3 norm = (texture2D(texture1, texCoords.xy).rgb - 0.5) * 2.0;\n"
        "	float diffuse = 0.5 + abs(dot(norm, viewv))*0.5;\n"
        "   if (glow > 0.001) {\n"
        "       diffuse = 1.0;\n"
        "   }\n"
        "	vec4 color = texture2D(texture0, texCoords.xy);\n"
        "	vec3 intensity = color.rgb * clamp((vertexColor.rgb + mlFactor) * diffuse, glow, 1.0);\n"
        "	float fogFactor = clamp(exp2(FDxLOG2E * dot(viewDir, viewDir)), 0.0, 1.0);\n"
        "	gl_FragColor = vec4(mix(gl_Fog.color.rgb, intensity, fogFactor), vertexColor.a * color.a);\n"
        "}\n";
    defaultVertexPrograms["bump_bloom"] = defaultVertexPrograms["bump"];
    defaultFragmentPrograms["bump_bloom"] = ""
        "uniform sampler2D texture0;\n"
        "uniform sampler2D texture1;\n"
        "uniform float wobble;\n"
        "uniform float glow;\n"
        "uniform float flare;\n"
        "uniform float bloomScale;\n"
        "uniform float bloomShift;\n"
        "varying vec3 viewXY;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying float MLxLOG2E;\n"
        "void main (void) {\n"
        "	vec3 texCoords = vec3(gl_TexCoord[0].xy, 0.0);\n"
        "	texCoords += vec3(normalize(viewXY).yx * wobble, 0.0);\n"
        "	float flash = exp2((flare - 1.0) * 2.0);\n"
        "	float mlFactor = exp2(MLxLOG2E * dot(viewDir, viewDir) / flash + 1.0); \n"
        "	mlFactor = clamp(mlFactor, 0.0, flare - 0.5) * 0.5;\n"
        "	vec3 viewv = normalize(viewDir);\n"
        "	// iterative parallax mapping\n"
        "	float scale = 0.010;\n"
        "	float bias = -0.005;\n"
        "	for(int i = 0; i < 4; ++i) {\n"
        "		vec4 normal = texture2D(texture1, texCoords.xy);\n"
        "		float h = normal.a * scale + bias;\n"
        "		texCoords.x += h * viewv.x;\n"
        "		texCoords.y -= h * viewv.y;\n"
        "	}\n"
        "	vec3 norm = (texture2D(texture1, texCoords.xy).rgb - 0.5) * 2.0;\n"
        "	float diffuse = 0.5 + abs(dot(norm, viewv))*0.5;\n"
        "   if (glow > 0.001) {\n"
        "       diffuse = 1.0;\n"
        "   }\n"
        "	vec4 color = texture2D(texture0, texCoords.xy);\n"
        "	vec3 intensity = clamp(vertexColor.rgb, glow, 1.0);\n"
        "	vec3 corr = clamp((vertexColor.rgb + mlFactor) * (0.5*diffuse + 0.5), glow, 1.0);\n"
        "	intensity = intensity - 0.33*(corr - intensity);\n"
        "	intensity = color.rgb * clamp(intensity * bloomScale + bloomShift, 0.0, 1.0);\n"
        "	float fogFactor = clamp(exp2(FDxLOG2E * dot(viewDir, viewDir)), 0.0, 1.0);\n"
        "	gl_FragColor = vec4(mix(vec3(0.0, 0.0, 0.0), intensity, fogFactor), vertexColor.a * color.a);\n"
        "}\n";
		
	if (DisableClipVertex()) {
		std::string query = "gl_ClipVertex";
		std::string replace = "// gl_ClipVertex";
		std::map<std::string, std::string>::iterator it;
		for (it = defaultVertexPrograms.begin(); it != defaultVertexPrograms.end(); it++) {
			size_t found = 0;
			do {
				found = it->second.find(query, found);
				if (found != std::string::npos) {
					it->second.replace(found, query.length(), replace);
					found += replace.length();
				}
			} while (found != std::string::npos);
		}
	}
}
    
