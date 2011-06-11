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
#include <algorithm>
#include <iostream>

#include "OGL_Shader.h"
#include "FileHandler.h"
#include "OGL_Setup.h"


// gl_ClipVertex workaround
// In Mac OS X 10.4 and Mesa, setting gl_ClipVertex causes a black screen.
// Unfortunately, it's required for proper 5-D space on other
// systems. This workaround comments out its use under 10.4 or Mesa.
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
inline bool DisableClipVertex() { 
	const GLubyte* renderer = glGetString(GL_RENDERER);
	return (renderer && strncmp(reinterpret_cast<const char*>(renderer), "Mesa", 4) == 0);
}
#endif


static std::map<std::string, std::string> defaultVertexPrograms;
static std::map<std::string, std::string> defaultFragmentPrograms;
void initDefaultPrograms();

std::vector<Shader> Shader::_shaders;

const char* Shader::_uniform_names[NUMBER_OF_UNIFORM_LOCATIONS] = 
{
	"texture0",
	"texture1",
	"texture2",
	"texture3",
	"time",
	"wobble",
	"flare",
	"bloomScale",
	"bloomShift",
	"repeat",
	"offsetx",
	"offsety",
	"pass",
	"usestatic",
	"usefog",
	"visibility",
	"depth",
	"strictDepthMode",
	"glow",
	"landscapeInverseMatrix"
};

const char* Shader::_shader_names[NUMBER_OF_SHADER_TYPES] = 
{
	"blur",
	"bloom",
	"landscape",
	"landscape_bloom",
	"sprite",
	"sprite_bloom",
	"invincible",
	"invincible_bloom",
	"invisible",
	"invisible_bloom",
	"wall",
	"wall_bloom",
	"bump",
	"bump_bloom",
	"specular",
	"specular_bloom"
};

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
	Shader::loadAll();
	
	for (int i = 0; i < Shader::NUMBER_OF_SHADER_TYPES; ++i) {
		if (_name == Shader::_shader_names[i]) {
			Shader::_shaders[i] = Shader(_name, _vert, _frag, _passes);
			return true;
		}
	}
	return false;
}

bool XML_ShaderParser::ResetValues() {
	Shader::_shaders.clear();
	return true;
}

static XML_ShaderParser ShaderParser;
XML_ElementParser *Shader_GetParser() {return &ShaderParser;}

void parseFile(FileSpecifier& fileSpec, std::string& s) {

	s.clear();

	if (fileSpec == FileSpecifier() || !fileSpec.Exists()) {
		return;
	}

	OpenedFile file;
	if (!fileSpec.Open(file))
	{
		fprintf(stderr, "%s not found\n", fileSpec.GetPath());
		return;
	}

	int32 length;
	file.GetLength(length);

	s.resize(length);
	file.Read(length, &s[0]);
}


GLhandleARB parseShader(const GLcharARB* str, GLenum shaderType) {

	GLint status;
	GLhandleARB shader = glCreateShaderObjectARB(shaderType);

	std::vector<const GLcharARB*> source;

	if (DisableClipVertex())
	{
		source.push_back("#define DISABLE_CLIP_VERTEX\n");
	}
	if (Wanting_sRGB)
	{
		source.push_back("#define GAMMA_CORRECTED_BLENDING\n");
	}
	source.push_back(str);

	glShaderSourceARB(shader, source.size(), &source[0], NULL);

	glCompileShaderARB(shader);
	glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &status);

	if(status) {
		return shader;
	} else {
		glDeleteObjectARB(shader);
		return NULL;		
	}
}

void Shader::loadAll() {
	initDefaultPrograms();
	if (!_shaders.size()) 
	{
		_shaders.reserve(NUMBER_OF_SHADER_TYPES);
		for (int i = 0; i < NUMBER_OF_SHADER_TYPES; ++i) 
		{
			_shaders.push_back(Shader(_shader_names[i]));
		}
	}
}

void Shader::unloadAll() {
	for (int i = 0; i < _shaders.size(); ++i) 
	{
		_shaders[i].unload();
	}
}

Shader::Shader(const std::string& name) : _programObj(NULL), _passes(-1), _loaded(false) {
    initDefaultPrograms();
    if (defaultVertexPrograms.count(name) > 0) {
	    _vert = defaultVertexPrograms[name];
    }
    if (defaultFragmentPrograms.count(name) > 0) {
	    _frag = defaultFragmentPrograms[name];
    }
}    

Shader::Shader(const std::string& name, FileSpecifier& vert, FileSpecifier& frag, int16& passes) : _programObj(NULL), _passes(passes), _loaded(false) {
	initDefaultPrograms();
	
	parseFile(vert,  _vert);
	if (_vert.empty() && defaultVertexPrograms.count(name) > 0) 
	{
		_vert = defaultVertexPrograms[name];
	}
	
	parseFile(frag, _frag);
	if (_frag.empty() && defaultFragmentPrograms.count(name) > 0) 
	{
		_frag = defaultFragmentPrograms[name];
	}
}

void Shader::init() {

	std::fill_n(_uniform_locations, static_cast<int>(NUMBER_OF_UNIFORM_LOCATIONS), -1);
	std::fill_n(_cached_floats, static_cast<int>(NUMBER_OF_UNIFORM_LOCATIONS), 0.0);

	_loaded = true;

	_programObj = glCreateProgramObjectARB();

	assert(!_vert.empty());
	GLhandleARB vertexShader = parseShader(_vert.c_str(), GL_VERTEX_SHADER_ARB);
	assert(vertexShader);
	glAttachObjectARB(_programObj, vertexShader);
	glDeleteObjectARB(vertexShader);

	assert(!_frag.empty());
	GLhandleARB fragmentShader = parseShader(_frag.c_str(), GL_FRAGMENT_SHADER_ARB);
	assert(fragmentShader);
	glAttachObjectARB(_programObj, fragmentShader);
	glDeleteObjectARB(fragmentShader);
	
	glLinkProgramARB(_programObj);

	assert(_programObj);

	glUseProgramObjectARB(_programObj);

	glUniform1iARB(getUniformLocation(U_Texture0), 0);
	glUniform1iARB(getUniformLocation(U_Texture1), 1);
	glUniform1iARB(getUniformLocation(U_Texture2), 2);
	glUniform1iARB(getUniformLocation(U_Texture3), 3);	

	glUseProgramObjectARB(NULL);

//	assert(glGetError() == GL_NO_ERROR);
}

void Shader::setFloat(UniformName name, float f) {

	if (_cached_floats[name] != f) {
		_cached_floats[name] = f;
		glUniform1fARB(getUniformLocation(name), f);
	}
}

void Shader::setMatrix4(UniformName name, float *f) {

	glUniformMatrix4fvARB(getUniformLocation(name), 1, false, f);
}

Shader::~Shader() {
	unload();
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
        "const float f0 = 0.07692307;\n"
        "const float f1 = 0.15384615;\n"
        "const float o1 = 1.5;\n"
        "const float f2 = 0.15384615;\n"
        "const float o2 = 3.5;\n"
        "const float f3 = 0.15384615;\n"
        "const float o3 = 5.5;\n"
        "void main (void) {\n"
        "	vec2 s = vec2(offsetx, offsety);\n"
        "	// Thanks to Renaud Bedard - http://theinstructionlimit.com/?p=43\n"
        "	vec3 c = texture2DRect(texture0, gl_TexCoord[0].xy).rgb;\n"
        "	vec3 t = f0 * c;\n"
        "	t += f1 * texture2DRect(texture0, gl_TexCoord[0].xy - o1*s).rgb;\n"
        "	t += f1 * texture2DRect(texture0, gl_TexCoord[0].xy + o1*s).rgb;\n"
        "	t += f2 * texture2DRect(texture0, gl_TexCoord[0].xy - o2*s).rgb;\n"
        "	t += f2 * texture2DRect(texture0, gl_TexCoord[0].xy + o2*s).rgb;\n"
        "	t += f3 * texture2DRect(texture0, gl_TexCoord[0].xy - o3*s).rgb;\n"
        "	t += f3 * texture2DRect(texture0, gl_TexCoord[0].xy + o3*s).rgb;\n"
        "	if (pass < 1.5) { t = clamp(t, c, vec3(1.0, 1.0, 1.0)); }\n"
        "	gl_FragColor = vec4(t, 1.0) * vertexColor;\n"
        "}\n";
    
    defaultVertexPrograms["bloom"] = defaultVertexPrograms["blur"];
    defaultFragmentPrograms["bloom"] = ""
        "uniform sampler2DRect texture0;\n"
        "uniform float pass;\n"
        "varying vec4 vertexColor;\n"
        "void main (void) {\n"
        "	vec4 color = texture2DRect(texture0, gl_TexCoord[0].xy);\n"
        "#ifdef GAMMA_CORRECTED_BLENDING\n"
        "	color = color * (1.0 - 0.1 * pass * pass);\n"
        "#else\n"
        "	color = color * (1.0 - 0.05 * pass);\n"
        "#endif\n"
        "	gl_FragColor = color * vertexColor;\n"
        "}\n";
    
    defaultVertexPrograms["landscape"] = ""
        "uniform mat4 landscapeInverseMatrix;\n"
        "varying vec3 viewDir;\n"
        "varying vec3 faceDir;\n"
        "varying vec3 relDir;\n"
        "varying float texScale;\n"
        "varying float texOffset;\n"
        "varying vec4 vertexColor;\n"
        "void main(void) {\n"
        "	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
        "#ifndef DISABLE_CLIP_VERTEX\n"
        "	gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;\n"
        "#endif\n"
        "	vec4 v = landscapeInverseMatrix * vec4(0.0, 0.0, 0.0, 1.0);\n"
        "	viewDir = (gl_Vertex - v).xyz;\n"
        "	faceDir = ((gl_ModelViewMatrixInverse * vec4(0.0, 0.0, -1.0, 1.0)) - v).xyz;\n"
        "	relDir = (gl_ModelViewMatrix * gl_Vertex).xyz;\n"
        "	texScale = gl_TextureMatrix[0][1][1];\n"
        "	texOffset = gl_TextureMatrix[0][3][1];\n"
        "	vertexColor = gl_Color;\n"
        "}\n";
    defaultFragmentPrograms["landscape"] = ""
        "uniform sampler2D texture0;\n"
        "uniform float repeat;\n"
        "uniform float usefog;\n"
        "varying vec3 viewDir;\n"
        "varying vec3 faceDir;\n"
        "varying vec3 relDir;\n"
        "varying float texScale;\n"
        "varying float texOffset;\n"
        "varying vec4 vertexColor;\n"
        "const float pi = 3.141593;\n"
        "const float zoom = 1.2;\n"
        "const float pitch_adjust = 0.96;\n"
        "void main(void) {\n"
        "	vec3 facev = normalize(faceDir);\n"
        "	vec3 relv  = normalize(relDir);\n"
        "	float offset = abs(texScale) > 1.01 ? 0.5 : 0.5 / abs(texScale) - sign(texOffset) + texOffset;\n"
        "	float scale = texScale * repeat / (2.0 * pi);\n"
        "	float x = relv.x / (relv.z * zoom) + atan(facev.x, facev.y);\n"
        "	float y = relv.y / (relv.z * zoom) - (facev.z * pitch_adjust);\n"
        "	vec4 color = texture2D(texture0, vec2(sign(scale) * (0.5 - x * scale), offset - y * scale));\n"
        "	vec3 intensity = color.rgb;\n"
        "	if (usefog > 0.0) {\n"
        "		intensity = gl_Fog.color.rgb;\n"
        "	}\n"
        "	gl_FragColor = vec4(intensity, 1.0);\n"
        "}\n";
    defaultVertexPrograms["landscape_bloom"] = defaultVertexPrograms["landscape"];
    defaultFragmentPrograms["landscape_bloom"] = ""
        "uniform sampler2D texture0;\n"
        "uniform float repeat;\n"
        "uniform float usefog;\n"
        "uniform float bloomScale;\n"
        "varying vec3 viewDir;\n"
        "varying vec3 faceDir;\n"
        "varying vec3 relDir;\n"
        "varying float texScale;\n"
        "varying float texOffset;\n"
        "varying vec4 vertexColor;\n"
        "const float pi = 3.141593;\n"
        "const float zoom = 1.2;\n"
        "const float pitch_adjust = 0.96;\n"
        "void main(void) {\n"
        "	vec3 facev = normalize(faceDir);\n"
        "	vec3 relv  = normalize(relDir);\n"
        "	float offset = abs(texScale) > 1.01 ? 0.5 : 0.5 / abs(texScale) - sign(texOffset) + texOffset;\n"
        "	float scale = texScale * repeat / (2.0 * pi);\n"
        "	float x = relv.x / (relv.z * zoom) + atan(facev.x, facev.y);\n"
        "	float y = relv.y / (relv.z * zoom) - (facev.z * pitch_adjust);\n"
        "	vec4 color = texture2D(texture0, vec2(sign(scale) * (0.5 - x * scale), offset - y * scale));\n"
        "	float intensity = clamp(bloomScale, 0.0, 1.0);\n"
        "	if (usefog > 0.0) {\n"
        "		intensity = 0.0;\n"
        "	}\n"
        "#ifdef GAMMA_CORRECTED_BLENDING\n"
        "	//intensity = intensity * intensity;\n"
        "	color.rgb = (color.rgb - 0.01) * 1.01;\n"
        "#else\n"
        "	color.rgb = (color.rgb - 0.1) * 1.11;\n"
        "#endif\n"
        "	gl_FragColor = vec4(color.rgb * intensity, 1.0);\n"
        "}\n";
	
    defaultVertexPrograms["sprite"] = ""
        "uniform float depth;\n"
        "uniform float strictDepthMode;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying float classicDepth;\n"
        "// to limit clipping, pretend sprites are closer by zforward/1024 WU\n"
        "const float zforward = 256.0;\n"
        "const float znear = 64.0;\n"
        "const float zfar = 131072.0;\n"
        "const float za = 1.00048851978505; // zfar / (zfar - znear)\n"
        "const float zb = -64.0312652662433; // zfar * znear / (znear - zfar)\n"
        "void main(void) {\n"
        "	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
        "	classicDepth = gl_Position.z / 8192.0;\n"
        "	float zadjusted = gl_Position.z - zforward + depth*gl_Position.z/65536.0;\n"
        "	gl_Position.z = ((za + zb/clamp(zadjusted, znear, zfar))*2.0-1.0)*gl_Position.w;\n"
        "#ifndef DISABLE_CLIP_VERTEX\n"
        "	// if (strictDepthMode < 0.5) { gl_Position.z = -gl_Position.w; }\n"
        "	gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;\n"
        "#endif\n"
        "	vec4 v = gl_ModelViewMatrixInverse * vec4(0.0, 0.0, 0.0, 1.0);\n"
        "	viewDir = (gl_Vertex - v).xyz;\n"
        "	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;\n"
        "	vertexColor = gl_Color;\n"
        "	FDxLOG2E = -gl_Fog.density * 1.442695;\n"
        "}\n";    
    defaultFragmentPrograms["sprite"] = ""
        "uniform sampler2D texture0;\n"
        "uniform float glow;\n"
        "uniform float flare;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying float classicDepth;\n"
        "void main (void) {\n"
        "	float mlFactor = clamp(0.5 + flare - classicDepth, 0.0, 1.0);\n"
        "	// more realistic: replace classicDepth with (length(viewDir)/8192.0)\n"
        "	vec3 intensity;\n"
        "	if (vertexColor.r > mlFactor) {\n"
        "		intensity = vertexColor.rgb + (mlFactor * 0.5); }\n"
        "	else {\n"
        "		intensity = (vertexColor.rgb * 0.5) + mlFactor; }\n"
        "	intensity = clamp(intensity, glow, 1.0);\n"
        "#ifdef GAMMA_CORRECTED_BLENDING\n"
        "	intensity = intensity * intensity; // approximation of pow(intensity, 2.2)\n"
        "#endif\n"
        "	vec4 color = texture2D(texture0, gl_TexCoord[0].xy);\n"
        "	float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);\n"
        "	gl_FragColor = vec4(mix(gl_Fog.color.rgb, color.rgb * intensity, fogFactor), vertexColor.a * color.a);\n"
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
        "varying float classicDepth;\n"
        "void main (void) {\n"
        "	vec4 color = texture2D(texture0, gl_TexCoord[0].xy);\n"
        "	vec3 intensity = clamp(vertexColor.rgb, glow, 1.0);\n"
        "	//intensity = intensity * clamp(2.0 - length(viewDir)/8192.0, 0.0, 1.0);\n"
        "	intensity = clamp(intensity * bloomScale + bloomShift, 0.0, 1.0);\n"
        "#ifdef GAMMA_CORRECTED_BLENDING\n"
        "	intensity = intensity * intensity;  // approximation of pow(intensity, 2.2)\n"
        "	color.rgb = (color.rgb - 0.06) * 1.02;\n"
        "#else\n"
        "  color.rgb = (color.rgb - 0.2) * 1.25;\n"
        "#endif\n"
        "	float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);\n"
        "	gl_FragColor = vec4(mix(vec3(0.0, 0.0, 0.0), color.rgb * intensity, fogFactor), vertexColor.a * color.a);\n"
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
        "#ifdef GAMMA_CORRECTED_BLENDING\n"
        "	intensity = intensity * intensity;  // approximation of pow(intensity, 2.2)\n"
        "#endif\n"
        "	float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);\n"
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
        "	float fogFactor = exp2(FDxLOG2E * length(viewDir));\n"
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
        "	float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);\n"
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
        "	float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);\n"
        "	gl_FragColor = vec4(mix(vec3(0.0, 0.0, 0.0), intensity, fogFactor), vertexColor.a * color.a * visibility);\n"
        "}\n";
	
    defaultVertexPrograms["wall"] = ""
        "uniform float depth;\n"
        "varying vec3 viewXY;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying float classicDepth;\n"
        "void main(void) {\n"
        "	gl_Position  = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
        "	gl_Position.z = gl_Position.z + depth*gl_Position.z/65536.0;\n"
        "	classicDepth = gl_Position.z / 8192.0;\n"
        "#ifndef DISABLE_CLIP_VERTEX\n"
        "	gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;\n"
        "#endif\n"
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
        "	FDxLOG2E = -gl_Fog.density * 1.442695;\n"
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
        "varying float classicDepth;\n"
        "void main (void) {\n"
        "	vec3 texCoords = vec3(gl_TexCoord[0].xy, 0.0);\n"
        "	texCoords += vec3(normalize(viewXY).yx * wobble, 0.0);\n"
        "	float mlFactor = clamp(0.5 + flare - classicDepth, 0.0, 1.0);\n"
        "	// more realistic: replace classicDepth with (length(viewDir)/8192.0)\n"
        "	vec3 intensity;\n"
        "	if (vertexColor.r > mlFactor) {\n"
        "		intensity = vertexColor.rgb + (mlFactor * 0.5); }\n"
        "	else {\n"
        "		intensity = (vertexColor.rgb * 0.5) + mlFactor; }\n"
        "	intensity = clamp(intensity, glow, 1.0);\n"
        "#ifdef GAMMA_CORRECTED_BLENDING\n"
        "	intensity = intensity * intensity; // approximation of pow(intensity, 2.2)\n"
        "#endif\n"
        "	vec4 color = texture2D(texture0, texCoords.xy);\n"
        "	float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);\n"
        "	gl_FragColor = vec4(mix(gl_Fog.color.rgb, color.rgb * intensity, fogFactor), vertexColor.a * color.a);\n"
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
        "void main (void) {\n"
        "	vec3 texCoords = vec3(gl_TexCoord[0].xy, 0.0);\n"
        "	texCoords += vec3(normalize(viewXY).yx * wobble, 0.0);\n"
        "	vec4 color = texture2D(texture0, texCoords.xy);\n"
        "	vec3 intensity = clamp(vertexColor.rgb, glow, 1.0);\n"
        "	float diffuse = abs(dot(vec3(0.0, 0.0, 1.0), normalize(viewDir)));\n"
        "	intensity = clamp(intensity * bloomScale + bloomShift, 0.0, 1.0);\n"
        "#ifdef GAMMA_CORRECTED_BLENDING\n"
        "	intensity = intensity * intensity; // approximation of pow(intensity, 2.2)\n"
        "#endif\n"
        "	float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);\n"
        "	gl_FragColor = vec4(mix(vec3(0.0, 0.0, 0.0), color.rgb * intensity, fogFactor), vertexColor.a * color.a);\n"
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
        "void main (void) {\n"
        "	vec3 texCoords = vec3(gl_TexCoord[0].xy, 0.0);\n"
        "	texCoords += vec3(normalize(viewXY).yx * wobble, 0.0);\n"
        "	float mlFactor = clamp(0.5 + flare - (length(viewDir)/8192.0), 0.0, 1.0);\n"
        "	vec3 intensity;\n"
        "	if (vertexColor.r > mlFactor) {\n"
        "		intensity = vertexColor.rgb + (mlFactor * 0.5); }\n"
        "	else {\n"
        "		intensity = (vertexColor.rgb * 0.5) + mlFactor; }\n"
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
        "	vec3 norm = (texture2D(texture1, gl_TexCoord[0].xy).rgb - vec3(0.5, 0.5, 0.0)) * vec3(2.0, 2.0, 1.0);\n"
        "	float diffuse = 0.5 + abs(dot(norm, viewv))*0.5;\n"
        "   if (glow > 0.001) {\n"
        "       diffuse = 1.0;\n"
        "   }\n"
        "	vec4 color = texture2D(texture0, texCoords.xy);\n"
        "	intensity = clamp(intensity * diffuse, glow, 1.0);\n"
        "#ifdef GAMMA_CORRECTED_BLENDING\n"
        "	intensity = intensity * intensity; // approximation of pow(intensity, 2.2)\n"
        "#endif\n"
        "	float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);\n"
        "	gl_FragColor = vec4(mix(gl_Fog.color.rgb, color.rgb * intensity, fogFactor), vertexColor.a * color.a);\n"
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
        "void main (void) {\n"
        "	vec3 texCoords = vec3(gl_TexCoord[0].xy, 0.0);\n"
        "	texCoords += vec3(normalize(viewXY).yx * wobble, 0.0);\n"
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
        "	vec3 norm = (texture2D(texture1, gl_TexCoord[0].xy).rgb - vec3(0.5, 0.5, 0.0)) * vec3(2.0, 2.0, 1.0);\n"
        "	float diffuse = 0.5 + abs(dot(norm, viewv))*0.5;\n"
        "   if (glow > 0.001) {\n"
        "       diffuse = 1.0;\n"
        "   }\n"
        "	vec4 color = texture2D(texture0, texCoords.xy);\n"
        "	vec3 intensity = clamp(vertexColor.rgb, glow, 1.0);\n"
        "	intensity = clamp(intensity * bloomScale + bloomShift, 0.0, 1.0);\n"
        "#ifdef GAMMA_CORRECTED_BLENDING\n"
        "	intensity = intensity * intensity; // approximation of pow(intensity, 2.2)\n"
        "#endif\n"
        "	float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);\n"
        "	gl_FragColor = vec4(mix(vec3(0.0, 0.0, 0.0), color.rgb * intensity, fogFactor), vertexColor.a * color.a);\n"
        "}\n";
    defaultVertexPrograms["specular"] = defaultVertexPrograms["bump"];
    defaultFragmentPrograms["specular"] = ""
        "uniform sampler2D texture0;\n"
        "uniform sampler2D texture1;\n"
        "uniform float glow;\n"
        "uniform float flare;\n"
        "varying vec3 viewXY;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "void main (void) {\n"
        "	float mlFactor = clamp(0.5 + flare - (length(viewDir)/8192.0), 0.0, 1.0);\n"
        "	vec3 intensity;\n"
        "	if (vertexColor.r > mlFactor) {\n"
        "		intensity = vertexColor.rgb + (mlFactor * 0.5); }\n"
        "	else {\n"
        "		intensity = (vertexColor.rgb * 0.5) + mlFactor; }\n"
        "	vec3 viewv = normalize(viewDir);\n"
        "	vec3 norm = (texture2D(texture1, gl_TexCoord[0].xy).rgb - vec3(0.5, 0.5, 0.0)) * vec3(2.0, 2.0, 1.0);\n"
        "	float diffuse = 0.5 + abs(dot(norm, viewv))*0.5;\n"
        "	vec4 color = texture2D(texture0, gl_TexCoord[0].xy);\n"
		"	float diffuse_factor = texture2D(texture1, gl_TexCoord[0].xy).a - 0.5;\n"
		"	diffuse = pow(diffuse, 14.0 * diffuse_factor + 1.0);\n"
        "	float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);\n"
        "	gl_FragColor = vec4(mix(vec3(0.0, 0.0, 0.0), color.rgb * diffuse, fogFactor), vertexColor.a * color.a);\n"
        "}\n";
    defaultVertexPrograms["specular_bloom"] = defaultVertexPrograms["bump"];
    defaultFragmentPrograms["specular_bloom"] = ""
        "uniform sampler2D texture0;\n"
        "uniform sampler2D texture1;\n"
        "uniform float glow;\n"
        "uniform float flare;\n"
        "uniform float bloomScale;\n"
        "uniform float bloomShift;\n"
        "varying vec3 viewXY;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "void main (void) {\n"
        "	vec3 viewv = normalize(viewDir);\n"
        "	vec3 norm = (texture2D(texture1, gl_TexCoord[0].xy).rgb - vec3(0.5, 0.5, 0.0)) * vec3(2.0, 2.0, 1.0);\n"
        "	float diffuse = 0.5 + abs(dot(norm, viewv))*0.5;\n"
        "   if (glow > 0.001) {\n"
        "       diffuse = 1.0;\n"
        "   }\n"
        "	vec4 color = texture2D(texture0, gl_TexCoord[0].xy);\n"
		"	float diffuse_factor = 2.0 * (texture2D(texture1, gl_TexCoord[0].xy).a - 0.5);\n"
		"	diffuse = pow(diffuse, 256.0 * diffuse_factor) * diffuse_factor;\n"
        "	float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);\n"
        "	gl_FragColor = vec4(mix(vec3(0.0, 0.0, 0.0), color.rgb * diffuse, fogFactor), vertexColor.a * color.a);\n"
        "}\n";
}
    
