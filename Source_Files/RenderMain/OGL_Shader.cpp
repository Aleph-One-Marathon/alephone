/*
 OGL_SHADER.CPP
 
 Copyright (C) 2009 by Clemens Unterkofler and the Aleph One developers
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
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
#include "InfoTree.h"
#include "Logging.h"

// gl_clipvertex puts Radeons into software mode on Mac
#if (defined(__APPLE__) && defined(__MACH__))
static bool DisableClipVertex()
{
    const GLubyte* renderer = glGetString(GL_RENDERER);
    return (renderer && strncmp(reinterpret_cast<const char*>(renderer), "AMD", 3) == 0);
}
#else
static bool DisableClipVertex()
{
    return false;
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
	"pulsate",
	"wobble",
	"flare",
	"bloomScale",
	"bloomShift",
	"repeat",
	"offsetx",
	"offsety",
	"pass",
	"usefog",
	"visibility",
	"depth",
	"strictDepthMode",
	"glow",
	"landscapeInverseMatrix",
	"scalex",
	"scaley",
	"yaw",
	"pitch",
	"selfLuminosity",
	"gammaAdjust",
	"logicalWidth",
	"logicalHeight",
	"pixelWidth",
	"pixelHeight"
};

const char* Shader::_shader_names[NUMBER_OF_SHADER_TYPES] = 
{
	"error",
    "blur",
	"bloom",
	"landscape",
	"landscape_bloom",
	"landscape_infravision",
	"sprite",
	"sprite_bloom",
	"sprite_infravision",
	"invincible",
	"invincible_bloom",
	"invisible",
	"invisible_bloom",
	"wall",
	"wall_bloom",
	"wall_infravision",
	"bump",
	"bump_bloom",
	"gamma"
};


class Shader_MML_Parser {
public:
	static void reset();
	static void parse(const InfoTree& root);
};

void Shader_MML_Parser::reset()
{
	Shader::_shaders.clear();
}

void Shader_MML_Parser::parse(const InfoTree& root)
{
	std::string name;
	if (!root.read_attr("name", name))
		return;
	
	for (int i = 0; i < Shader::NUMBER_OF_SHADER_TYPES; ++i) {
		if (name == Shader::_shader_names[i]) {
			initDefaultPrograms();
			Shader::loadAll();
			
			FileSpecifier vert, frag;
			root.read_path("vert", vert);
			root.read_path("frag", frag);
			int16 passes;
			root.read_attr("passes", passes);
			
			Shader::_shaders[i] = Shader(name, vert, frag, passes);
			break;
		}
	}
}

void reset_mml_opengl_shader()
{
	Shader_MML_Parser::reset();
}

void parse_mml_opengl_shader(const InfoTree& root)
{
	Shader_MML_Parser::parse(root);
}

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

        if (DisableClipVertex()) {
            source.push_back("#define DISABLE_CLIP_VERTEX\n");
        }
	if (Wanting_sRGB)
	{
		source.push_back("#define GAMMA_CORRECTED_BLENDING\n");
	}
	if (Bloom_sRGB)
	{
		source.push_back("#define BLOOM_SRGB_FRAMEBUFFER\n");
	}
	source.push_back(str);

	glShaderSourceARB(shader, source.size(), &source[0], NULL);

	glCompileShaderARB(shader);
	glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &status);

	if(status) {
		return shader;
	} else {
        GLint infoLen = 0;
        glGetShaderiv((GLuint)(size_t)shader, GL_INFO_LOG_LENGTH, &infoLen);
        
        if(infoLen > 1)
        {
            char* infoLog = (char*) malloc(sizeof(char) * infoLen);
            glGetShaderInfoLog((GLuint)(size_t)shader, infoLen, NULL, infoLog);
            logError("Error compiling shader:\n%s\n", infoLog);
            free(infoLog);
        }
        
		glDeleteObjectARB(shader);
		return 0;
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

Shader::Shader(const std::string& name) : _programObj(0), _passes(-1), _loaded(false) {
    initDefaultPrograms();
    if (defaultVertexPrograms.count(name) > 0) {
	    _vert = defaultVertexPrograms[name];
    }
    if (defaultFragmentPrograms.count(name) > 0) {
	    _frag = defaultFragmentPrograms[name];
    }
}    

Shader::Shader(const std::string& name, FileSpecifier& vert, FileSpecifier& frag, int16& passes) : _programObj(0), _passes(passes), _loaded(false) {
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
    if(!vertexShader) {
        _vert = defaultVertexPrograms["error"];
        vertexShader = parseShader(_vert.c_str(), GL_VERTEX_SHADER_ARB);
    }
	
	glAttachObjectARB(_programObj, vertexShader);
	glDeleteObjectARB(vertexShader);

	assert(!_frag.empty());
	GLhandleARB fragmentShader = parseShader(_frag.c_str(), GL_FRAGMENT_SHADER_ARB);
	if(!fragmentShader) {
        _frag = defaultFragmentPrograms["error"];
        fragmentShader = parseShader(_frag.c_str(), GL_FRAGMENT_SHADER_ARB);
    }
    
	glAttachObjectARB(_programObj, fragmentShader);
	glDeleteObjectARB(fragmentShader);
	
	glLinkProgramARB(_programObj);
    
    GLint linked;
    glGetProgramiv((GLuint)(size_t)_programObj, GL_LINK_STATUS, &linked);
    if(!linked)
    {
      GLint infoLen = 0;
      glGetProgramiv((GLuint)(size_t)_programObj, GL_INFO_LOG_LENGTH, &infoLen);
      if(infoLen > 1)
      {
        char* infoLog = (char*) malloc(sizeof(char) * infoLen);
        glGetProgramInfoLog((GLuint)(size_t)_programObj, infoLen, NULL, infoLog);
        logError("Error linking program:\n%s\n", infoLog);
        free(infoLog);
      }
      glDeleteProgram((GLuint)(size_t)_programObj);
    }

	assert(_programObj);

	glUseProgramObjectARB(_programObj);

	glUniform1iARB(getUniformLocation(U_Texture0), 0);
	glUniform1iARB(getUniformLocation(U_Texture1), 1);
	glUniform1iARB(getUniformLocation(U_Texture2), 2);
	glUniform1iARB(getUniformLocation(U_Texture3), 3);	

	glUseProgramObjectARB(0);

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
	glUseProgramObjectARB(0);
}

void Shader::unload() {
	if(_programObj) {
		glDeleteObjectARB(_programObj);
		_programObj = 0;
		_loaded = false;
	}
}

int16 Shader::passes() {
	return _passes;
}

void initDefaultPrograms() {
    if (defaultVertexPrograms.size() > 0)
        return;
    
    
    defaultVertexPrograms["error"] = ""
    "varying vec4 vertexColor;\n"
    "void main(void) {\n"
    "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
    "    vertexColor = vec4(1.0, 1.0, 0.0, 1.0);\n"
    "}\n";
    defaultFragmentPrograms["error"] = ""
    "float round(float n){ \n"
    "   float nSign = 1.0; \n"
    "   if ( n < 0.0 ) { nSign = -1.0; }; \n"
    "   return nSign * floor(abs(n)+0.5); \n"
    "} \n"
    "void main (void) {\n"
    "    gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);\n"
    "    float checkerSize = 8.0;\n"
    "    float phase = 0.0;\n"
    "    if( mod(round(gl_FragCoord.y / checkerSize), 2.0) == 0.0) {\n"
    "       phase = checkerSize;\n"
    "    }\n"
    "    if (mod(round((gl_FragCoord.x + phase) / checkerSize), 2.0)==0.0) {\n"
    "       gl_FragColor.a = 0.5;\n"
    "    }\n"
    "}\n";
    
	defaultVertexPrograms["gamma"] = ""
	"varying vec4 vertexColor;\n"
	"void main(void) {\n"
	"	gl_TexCoord[0] = gl_MultiTexCoord0;\n"
	"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"	vertexColor = gl_Color;\n"
	"}\n";
	defaultFragmentPrograms["gamma"] = ""
	"uniform sampler2DRect texture0;\n"
	"uniform float gammaAdjust;\n"
	"void main (void) {\n"
	"	vec4 color0 = texture2DRect(texture0, gl_TexCoord[0].xy);\n"
	"	gl_FragColor = vec4(pow(color0.r, gammaAdjust), pow(color0.g, gammaAdjust), pow(color0.b, gammaAdjust), 1.0);\n"
	"}\n";
	
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
        "const float f0 = 0.14012035;\n"
        "const float f1 = 0.24122258;\n"
        "const float o1 = 1.45387071;\n"
        "const float f2 = 0.13265595;\n"
        "const float o2 = 3.39370426;\n"
        "const float f3 = 0.04518872;\n"
        "const float o3 = 5.33659787;\n"
        "#ifdef BLOOM_SRGB_FRAMEBUFFER\n"
        "vec3 s2l(vec3 srgb) { return srgb; }\n"
        "vec3 l2s(vec3 linear) { return linear; }\n"
        "#else\n"
        "vec3 s2l(vec3 srgb) { return srgb * srgb; }\n"
        "vec3 l2s(vec3 linear) { return sqrt(linear); }\n"
        "#endif\n"
        "void main (void) {\n"
        "	vec2 s = vec2(offsetx, offsety);\n"
        "	// Thanks to Renaud Bedard - http://theinstructionlimit.com/?p=43\n"
        "	vec3 c = s2l(texture2DRect(texture0, gl_TexCoord[0].xy).rgb);\n"
        "	vec3 t = f0 * c;\n"
        "	t += f1 * s2l(texture2DRect(texture0, gl_TexCoord[0].xy - o1*s).rgb);\n"
        "	t += f1 * s2l(texture2DRect(texture0, gl_TexCoord[0].xy + o1*s).rgb);\n"
        "	t += f2 * s2l(texture2DRect(texture0, gl_TexCoord[0].xy - o2*s).rgb);\n"
        "	t += f2 * s2l(texture2DRect(texture0, gl_TexCoord[0].xy + o2*s).rgb);\n"
        "	t += f3 * s2l(texture2DRect(texture0, gl_TexCoord[0].xy - o3*s).rgb);\n"
        "	t += f3 * s2l(texture2DRect(texture0, gl_TexCoord[0].xy + o3*s).rgb);\n"
        "	gl_FragColor = vec4(l2s(t), 1.0) * vertexColor;\n"
        "}\n";    
    
    defaultVertexPrograms["bloom"] = ""
        "varying vec4 vertexColor;\n"
        "void main(void) {\n"
        "	gl_TexCoord[0] = gl_MultiTexCoord0;\n"
        "	gl_TexCoord[1] = gl_MultiTexCoord1;\n"
        "	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
        "	vertexColor = gl_Color;\n"
        "}\n";
    defaultFragmentPrograms["bloom"] = ""
        "uniform sampler2DRect texture0;\n"
        "uniform sampler2DRect texture1;\n"
        "uniform float pass;\n"
        "varying vec4 vertexColor;\n"
        "vec3 s2l(vec3 srgb) { return srgb * srgb; }\n"
        "vec3 l2s(vec3 linear) { return sqrt(linear); }\n"
		"#ifndef BLOOM_SRGB_FRAMEBUFFER\n"
	    "vec3 b2l(vec3 bloom) { return bloom * bloom; }\n"
		"#else\n"
		"vec3 b2l(vec3 bloom) { return bloom; }\n"
        "#endif\n"
        "void main (void) {\n"
        "	vec4 color0 = texture2DRect(texture0, gl_TexCoord[0].xy);\n"
        "	vec4 color1 = texture2DRect(texture1, gl_TexCoord[1].xy);\n"
        "	vec3 color = l2s(s2l(color0.rgb) + b2l(color1.rgb));\n"
        "	gl_FragColor = vec4(color, 1.0);\n"
        "}\n";
    
    defaultVertexPrograms["landscape"] = ""
        "uniform mat4 landscapeInverseMatrix;\n"
        "varying vec3 relDir;\n"
        "varying vec4 vertexColor;\n"
        "void main(void) {\n"
        "	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
        "#ifndef DISABLE_CLIP_VERTEX\n"
        "	gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;\n"
        "#endif\n"
        "	relDir = (gl_ModelViewMatrix * gl_Vertex).xyz;\n"
        "	vertexColor = gl_Color;\n"
        "}\n";
    defaultFragmentPrograms["landscape"] = ""
        "uniform sampler2D texture0;\n"
        "uniform float usefog;\n"
        "uniform float scalex;\n"
        "uniform float scaley;\n"
        "uniform float offsetx;\n"
        "uniform float offsety;\n"
        "uniform float yaw;\n"
        "uniform float pitch;\n"
        "varying vec3 relDir;\n"
        "varying vec4 vertexColor;\n"
        "const float zoom = 1.2;\n"
        "const float pitch_adjust = 0.96;\n"
        "void main(void) {\n"
        "	vec3 facev = vec3(cos(yaw), sin(yaw), sin(pitch));\n"
        "	vec3 relv  = (relDir);\n"
        "	float x = relv.x / (relv.z * zoom) + atan(facev.x, facev.y);\n"
        "	float y = relv.y / (relv.z * zoom) - (facev.z * pitch_adjust);\n"
        "	vec4 color = texture2D(texture0, vec2(offsetx - x * scalex, offsety - y * scaley));\n"
        "	vec3 intensity = color.rgb;\n"
        "	if (usefog > 0.0) {\n"
        "		intensity = gl_Fog.color.rgb;\n"
        "	}\n"
        "	gl_FragColor = vec4(intensity, 1.0);\n"
        "}\n";
    defaultVertexPrograms["landscape_bloom"] = defaultVertexPrograms["landscape"];
    defaultFragmentPrograms["landscape_bloom"] = ""
        "uniform sampler2D texture0;\n"
        "uniform float usefog;\n"
        "uniform float scalex;\n"
        "uniform float scaley;\n"
        "uniform float offsetx;\n"
        "uniform float offsety;\n"	
        "uniform float yaw;\n"
        "uniform float pitch;\n"
        "uniform float bloomScale;\n"
        "varying vec3 relDir;\n"
        "varying vec4 vertexColor;\n"
        "const float zoom = 1.205;\n"
        "const float pitch_adjust = 0.955;\n"
        "void main(void) {\n"
        "	vec3 facev = vec3(cos(yaw), sin(yaw), sin(pitch));\n"
        "	vec3 relv  = normalize(relDir);\n"
        "	float x = relv.x / (relv.z * zoom) + atan(facev.x, facev.y);\n"
        "	float y = relv.y / (relv.z * zoom) - (facev.z * pitch_adjust);\n"
        "	vec4 color = texture2D(texture0, vec2(offsetx - x * scalex, offsety - y * scaley));\n"
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
	defaultVertexPrograms["landscape_infravision"] = defaultVertexPrograms["landscape"];
	defaultFragmentPrograms["landscape_infravision"] =
#include "Shaders/landscape_infravision.frag"
		;
	
    defaultVertexPrograms["sprite"] = ""
        "uniform float depth;\n"
        "uniform float strictDepthMode;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying float classicDepth;\n"
        "void main(void) {\n"

        "	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
        "	classicDepth = gl_Position.z / 8192.0;\n"
        "#ifndef DISABLE_CLIP_VERTEX\n"
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
        "uniform float selfLuminosity;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying float classicDepth;\n"
        "void main (void) {\n"
        "	float mlFactor = clamp(selfLuminosity + flare - classicDepth, 0.0, 1.0);\n"
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

	defaultVertexPrograms["sprite_infravision"] = defaultVertexPrograms["sprite"];
	defaultFragmentPrograms["sprite_infravision"] =
#include "Shaders/sprite_infravision.frag"
		;
	
    defaultVertexPrograms["invincible"] = defaultVertexPrograms["sprite"];
    defaultFragmentPrograms["invincible"] = ""
        "uniform sampler2D texture0;\n"
        "uniform float time;\n"
        "uniform float logicalWidth;\n"
        "uniform float logicalHeight;\n"
        "uniform float pixelWidth;\n"
        "uniform float pixelHeight;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "float rand(vec2 co){ \n"
        "   float a = 12.9898; \n"
        "   float b = 78.233; \n"
        "   float c = 43758.5453; \n"
        "   float dt= dot(co.xy ,vec2(a,b)); \n"
        "   float sn= mod(dt,3.14); \n"
        "   return fract(sin(sn) * c); \n"
        "} \n"
        "float round(float n){ \n"
        "   float nSign = 1.0; \n"
        "   if ( n < 0.0 ) { nSign = -1.0; }; \n"
        "   return nSign * floor(abs(n)+0.5); \n"
        "} \n"
        "void main(void) {\n"
        "   float blockSize = round((logicalHeight/320.0) * (pixelHeight/logicalHeight));\n"
        "   blockSize = max(blockSize, 1.0);\n"
        "   float moment=fract(time/10000.0);\n"
        "   float eX=moment*round(gl_FragCoord.x / blockSize);\n"
        "   float eY=moment*round(gl_FragCoord.y / blockSize);\n"
        "   vec2 entropy = vec2 (eX,eY); \n"
        "   float sr = rand(entropy); \n"
        "   float sg = rand(entropy*sr); \n"
        "   float sb = rand(entropy*sg); \n"
        "   vec3 intensity = vec3(sr, sg, sb); \n"
        "   vec4 color = texture2D(texture0, gl_TexCoord[0].xy);\n"
        "#ifdef GAMMA_CORRECTED_BLENDING\n"
        "   intensity = intensity * intensity;  // approximation of pow(intensity, 2.2)\n"
        "#endif\n"
        "   float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);\n"
        "   gl_FragColor = vec4(mix(gl_Fog.color.rgb, intensity, fogFactor), vertexColor.a * color.a);\n"
        "}\n";
    defaultVertexPrograms["invincible_bloom"] = defaultVertexPrograms["invincible"];
    defaultFragmentPrograms["invincible_bloom"] = ""
        "uniform sampler2D texture0;\n"
        "uniform float time;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "float rand(vec2 co){ \n"
        "   float a = 12.9898; \n"
        "   float b = 78.233; \n"
        "   float c = 43758.5453; \n"
        "   float dt= dot(co.xy ,vec2(a,b)); \n"
        "   float sn= mod(dt,3.14); \n"
        "   return fract(sin(sn) * c); \n"
        "} \n"
        "float round(float n){ \n"
        "   float nSign = 1.0; \n"
        "   if ( n < 0.0 ) { nSign = -1.0; }; \n"
        "   return nSign * floor(abs(n)+0.5); \n"
        "} \n"
        "void main(void) {\n"
        "   float blockHeight=2.0;\n"
        "   float blockWidth=2.0;\n"
        "   float darkBlockProbability=0.8;\n"
        "   float moment=fract(time/10000.0);\n"
        "   float eX=moment*round((gl_FragCoord.x + mod(time, 60.0)*7.0) / blockWidth);\n"
        "   float eY=moment*round((gl_FragCoord.y + mod(time, 60.0)*11.0) / blockHeight);\n"
        "   vec2 entropy = vec2 (eX, eY); \n"
        "   float sr = rand(entropy); \n"
        "   float sg = rand(entropy*sr); \n"
        "   float sb = rand(entropy*sg); \n"
        "   vec3 intensity = vec3(0.0,0.0,0.0);\n"
        "   if (rand(entropy*sr*sg*sb) > darkBlockProbability) {\n"
        "      intensity = vec3(sr*sr, sg*sg, sb); }\n"
        "   vec4 color = texture2D(texture0, gl_TexCoord[0].xy);\n"
        "#ifdef GAMMA_CORRECTED_BLENDING\n"
        "   intensity = intensity * intensity;  // approximation of pow(intensity, 2.2)\n"
        "#endif\n"
        "   float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);\n"
        "   gl_FragColor = vec4(mix(gl_Fog.color.rgb, intensity, fogFactor), vertexColor.a * color.a);\n"
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
        "uniform float pulsate;\n"
        "uniform float wobble;\n"
        "uniform float glow;\n"
        "uniform float flare;\n"
        "uniform float selfLuminosity;\n"
        "varying vec3 viewXY;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying float classicDepth;\n"
        "void main (void) {\n"
        "	vec3 texCoords = vec3(gl_TexCoord[0].xy, 0.0);\n"
        "	vec3 normXY = normalize(viewXY);\n"
        "	texCoords += vec3(normXY.y * -pulsate, normXY.x * pulsate, 0.0);\n"
        "	texCoords += vec3(normXY.y * -wobble * texCoords.y, wobble * texCoords.y, 0.0);\n"
        "	float mlFactor = clamp(selfLuminosity + flare - classicDepth, 0.0, 1.0);\n"
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
        "uniform float pulsate;\n"
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
        "	vec3 normXY = normalize(viewXY);\n"
        "	texCoords += vec3(normXY.y * -pulsate, normXY.x * pulsate, 0.0);\n"
        "	texCoords += vec3(normXY.y * -wobble * texCoords.y, wobble * texCoords.y, 0.0);\n"
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
	defaultVertexPrograms["wall_infravision"] = defaultVertexPrograms["wall"];
	defaultFragmentPrograms["wall_infravision"] =
#include "Shaders/wall_infravision.frag"
		;
    
    defaultVertexPrograms["bump"] = defaultVertexPrograms["wall"];
    defaultFragmentPrograms["bump"] = ""
        "uniform sampler2D texture0;\n"
        "uniform sampler2D texture1;\n"
        "uniform float pulsate;\n"
        "uniform float wobble;\n"
        "uniform float glow;\n"
        "uniform float flare;\n"
        "uniform float selfLuminosity;\n"
        "varying vec3 viewXY;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "void main (void) {\n"
        "	vec3 texCoords = vec3(gl_TexCoord[0].xy, 0.0);\n"
        "	vec3 normXY = normalize(viewXY);\n"
        "	texCoords += vec3(normXY.y * -pulsate, normXY.x * pulsate, 0.0);\n"
        "	texCoords += vec3(normXY.y * -wobble * texCoords.y, wobble * texCoords.y, 0.0);\n"
        "	float mlFactor = clamp(selfLuminosity + flare - (length(viewDir)/8192.0), 0.0, 1.0);\n"
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
        "	vec3 norm = (texture2D(texture1, texCoords.xy).rgb - 0.5) * 2.0;\n"
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
        "uniform float pulsate;\n"
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
        "	vec3 normXY = normalize(viewXY);\n"
        "	texCoords += vec3(normXY.y * -pulsate, normXY.x * pulsate, 0.0);\n"
        "	texCoords += vec3(normXY.y * -wobble * texCoords.y, wobble * texCoords.y, 0.0);\n"
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
        "	intensity = clamp(intensity * bloomScale + bloomShift, 0.0, 1.0);\n"
        "#ifdef GAMMA_CORRECTED_BLENDING\n"
        "	intensity = intensity * intensity; // approximation of pow(intensity, 2.2)\n"
        "#endif\n"
        "	float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);\n"
        "	gl_FragColor = vec4(mix(vec3(0.0, 0.0, 0.0), color.rgb * intensity, fogFactor), vertexColor.a * color.a);\n"
        "}\n";
//		defaultFragmentPrograms["sprite"];
}

