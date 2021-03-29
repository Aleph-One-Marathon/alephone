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

//Global pointer to the last shader object enabled. May be NULL.    
Shader* lastEnabledShaderRef;
Shader* lastEnabledShader() {
  return lastEnabledShaderRef;
}
void setLastEnabledShader(Shader* theShader) {
  lastEnabledShaderRef = theShader;
}

static std::map<std::string, std::string> defaultVertexPrograms;
static std::map<std::string, std::string> defaultFragmentPrograms;
void initDefaultPrograms();

std::vector<Shader> Shader::_shaders;

const char* Shader::_uniform_names[NUMBER_OF_UNIFORM_LOCATIONS] = 
{
	"texture0",
    "texture0_size",
	"texture1",
    "texture1_size",
	"texture2",
    "texture2_size",
	"texture3",
    "texture3_size",
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
	"pixelHeight",
    "MS_ModelViewProjectionMatrix",
    "MS_ModelViewMatrix",
    "MS_ModelViewMatrixInverse",
    "MS_TextureMatrix",
    "uColor",
    "uFogColor",
    "uTexCoord4",
    "clipPlane0",
    "clipPlane1",
    "clipPlane2",
    "clipPlane3",
    "clipPlane4",
    "clipPlane5",
    "clipPlane6",
    "lightPositions",
    "lightColors"
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
	"gamma",
    "rect",
    "plain_rect",
    "solid_color"
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
    Shader::loadAll(); //We need to reload these immediately, since the shaders are needed to draw the UI
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


GLuint parseShader(const GLcharARB* str, GLenum shaderType) {

    GLint status;
    GLuint shader = glCreateShader(shaderType);

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

    glShaderSource(shader, source.size(), &source[0], NULL);

    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

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
        
        glDeleteShader(shader);
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
    shaderName = name;

    initDefaultPrograms();
        
    if (defaultVertexPrograms.count(name) > 0) {
        _vert = defaultVertexPrograms[name];
    }
    if (defaultFragmentPrograms.count(name) > 0) {
        _frag = defaultFragmentPrograms[name];
    }
}

Shader::Shader(const std::string& name, FileSpecifier& vert, FileSpecifier& frag, int16& passes) : _programObj(0), _passes(passes), _loaded(false) {
    shaderName = name;
    
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

    _programObj = glCreateProgram();

    assert(!_vert.empty());
    
    logError("Parsing vertex shader: %s\n", shaderName.c_str());
    
    GLuint vertexShader = parseShader(_vert.c_str(), GL_VERTEX_SHADER_ARB);
    
    if(!vertexShader) {
        _vert = defaultVertexPrograms["error"];
        vertexShader = parseShader(_vert.c_str(), GL_VERTEX_SHADER_ARB);
    }
    
    glAttachShader((GLuint)(size_t)_programObj, (GLuint)(size_t)vertexShader);
    glDeleteShader((GLuint)(size_t)vertexShader);

    logError("Parsing fragment shader: %s\n", shaderName.c_str());
    
    assert(!_frag.empty());
    GLuint fragmentShader = parseShader(_frag.c_str(), GL_FRAGMENT_SHADER_ARB);
    glPushGroupMarkerEXT(0, "Draw ES Quad");
    if(!fragmentShader) {
        _frag = defaultFragmentPrograms["error"];
        fragmentShader = parseShader(_frag.c_str(), GL_FRAGMENT_SHADER_ARB);
    }
    
    glAttachShader((GLuint)(size_t)_programObj, (GLuint)(size_t)fragmentShader);
    glDeleteShader((GLuint)(size_t)fragmentShader);
    
    glBindAttribLocation(_programObj, Shader::ATTRIB_VERTEX, "vPosition");
    glBindAttribLocation(_programObj, Shader::ATTRIB_TEXCOORDS, "vTexCoord");
    glBindAttribLocation(_programObj, Shader::ATTRIB_NORMAL, "vNormal");
    glBindAttribLocation(_programObj, Shader::ATTRIB_COLOR, "vColor");
    glBindAttribLocation(_programObj, Shader::ATTRIB_TEXCOORDS4, "vTexCoords4");
    glBindAttribLocation(_programObj, Shader::ATTRIB_CLIPPLANE0, "vClipPlane0");
    glBindAttribLocation(_programObj, Shader::ATTRIB_CLIPPLANE1, "vClipPlane1");
    glBindAttribLocation(_programObj, Shader::ATTRIB_CLIPPLANE5, "vClipPlane5");
    glBindAttribLocation(_programObj, Shader::ATTRIB_SxOxSyOy, "vSxOxSyOy");
    glBindAttribLocation(_programObj, Shader::ATTRIB_BsBtFlSl, "vBsBtFlSl");
    glBindAttribLocation(_programObj, Shader::ATTRIB_PuWoDeGl, "vPuWoDeGl");
    
    glLinkProgram(_programObj);

    GLint linked;
    glGetProgramiv(_programObj, GL_LINK_STATUS, &linked);
    if(!linked)
    {
      GLint infoLen = 0;
      glGetProgramiv(_programObj, GL_INFO_LOG_LENGTH, &infoLen);
      if(infoLen > 1)
      {
        char* infoLog = (char*) malloc(sizeof(char) * infoLen);
        glGetProgramInfoLog(_programObj, infoLen, NULL, infoLog);
        logError("Error linking program:\n%s\n", infoLog);
        free(infoLog);
      }
      glDeleteProgram(_programObj);
    }
    
    assert(_programObj);

    glUseProgram(_programObj);

    glUniform1i(getUniformLocation(U_Texture0), 0);
    glUniform1i(getUniformLocation(U_Texture1), 1);
    glUniform1i(getUniformLocation(U_Texture2), 2);
    glUniform1i(getUniformLocation(U_Texture3), 3);

    glUseProgram(0);

//    assert(glGetError() == GL_NO_ERROR);
}

void Shader::enableAndSetStandardUniforms() {
    
    GLfloat modelMatrix[16], modelProjection[16], modelMatrixInverse[16];
    MSI()->getFloatv(MS_MODELVIEW, modelMatrix);
    MSI()->getFloatvInverse(MS_MODELVIEW, modelMatrixInverse);
    MSI()->getFloatvModelviewProjection(modelProjection);
    
    this->enable();
    this->setMatrix4(Shader::U_ModelViewMatrix, modelMatrix);
    this->setMatrix4(Shader::U_ModelViewProjectionMatrix, modelProjection);
    this->setMatrix4(Shader::U_ModelViewMatrixInverse, modelMatrixInverse);
    this->setVec4(Shader::U_FogColor, MatrixStack::Instance()->fog());
}

void Shader::setFloat(UniformName name, float f) {

    if (_cached_floats[name] != f) {
        _cached_floats[name] = f;
        glUniform1f(getUniformLocation(name), f);
    }
}

void Shader::setMatrix4(UniformName name, float *f) {

    glUniformMatrix4fv(getUniformLocation(name), 1, false, f);
}

void Shader::setVec4(UniformName name, float *f) {
    glUniform4f(getUniformLocation(name), f[0], f[1], f[2], f[3]);
}

void Shader::setVec4v(UniformName name, int count, float *f) {

    glUniform4fv(getUniformLocation(name), count, f);
}

void Shader::setVec2(UniformName name, float *f) {
  glUniform2f(getUniformLocation(name), f[0], f[1]);
}

Shader::~Shader() {
	unload();
}

void Shader::enable() {
    if(!_loaded) { init(); }
    
    glUseProgram(_programObj);
    setLastEnabledShader(this);
}

void Shader::disable() {
    setLastEnabledShader(NULL);
    glUseProgram(0);
}

void Shader::unload() {
    if(_programObj) {
        if(lastEnabledShader() == this) {
            setLastEnabledShader(NULL);
        }
        glDeleteProgram(_programObj);
        glUseProgram(0);
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
    "uniform mat4 MS_ModelViewProjectionMatrix;"
    "attribute vec4 vPosition;\n"
    "varying vec4 vertexColor;\n"
    "void main(void) {\n"
    "    gl_Position = MS_ModelViewProjectionMatrix * vPosition;\n"
    "    vertexColor = vec4(1.0, 1.0, 0.0, 1.0);\n"
    "}\n";
    defaultFragmentPrograms["error"] = ""
    "precision highp float;\n"
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
    "       gl_FragColor.a = 0.05;\n"
    "    }\n"
    "}\n";
    
	defaultVertexPrograms["gamma"] = ""
      "uniform mat4 MS_ModelViewProjectionMatrix;\n"
      "uniform mat4 MS_ModelViewMatrix;\n"
      "attribute vec4 vPosition;\n"
      "uniform vec4 uColor;\n"
      "uniform vec4 uFogColor;\n"
      "attribute vec2 vTexCoord;   \n"
      "varying vec2 textureUV;   \n"
      "varying vec4 fogColor;\n"
        "varying vec4 vertexColor;\n"
        "void main(void) {\n"
        "    textureUV = vTexCoord;\n"
        "    gl_Position = MS_ModelViewProjectionMatrix * vPosition;\n"
        "    vertexColor = uColor;\n"
        "}\n";
        defaultFragmentPrograms["gamma"] = ""
      "precision highp float;\n"
      "varying highp vec2 textureUV; \n"
        "uniform sampler2D texture0;\n"
        "uniform float gammaAdjust;\n"
        "uniform vec2 texture0_size;\n"
           "void main (void) {\n"
        "    vec2 normalizedUV = vec2(textureUV.x/texture0_size.x , textureUV.y/texture0_size.y);\n"
        "    vec4 color0 = texture2D(texture0, normalizedUV);\n"
        "    gl_FragColor = vec4(pow(color0.r, gammaAdjust), pow(color0.g, gammaAdjust), pow(color0.b, gammaAdjust), 1.0);\n"
        "}\n";

    defaultVertexPrograms["plain_rect"] = ""
      "attribute vec4 vPosition;   \n"
      "attribute vec2 vTexCoord;   \n"
      "varying vec2 textureUV;   \n"
      "void main()                 \n"
      "{                           \n"
      "  textureUV = vTexCoord;\n"
      "  gl_Position = vPosition;  \n"
      "} \n";
    
      defaultFragmentPrograms["plain_rect"] = ""
      "precision highp float;\n"
      "varying highp vec2 textureUV; \n"
      "uniform highp sampler2D texture0;\n"
      "uniform vec4 uColor;\n"
      "void main()                                \n"
      "{                                          \n"
      " gl_FragColor = texture2D(texture0, textureUV) * uColor;\n"
      "} \n";
    
    defaultVertexPrograms["rect"] = ""
      "uniform mat4 MS_ModelViewProjectionMatrix;\n"
      "uniform mat4 MS_TextureMatrix;\n"
      "uniform vec4 uColor;\n"
      "attribute vec4 vPosition;   \n"
      "attribute vec2 vTexCoord;   \n"
      "varying vec2 textureUV;   \n"
      "varying vec4 vertexColor;\n"
      "void main()                 \n"
      "{                           \n"
      "  vec4 UV4 = vec4(vTexCoord.x, vTexCoord.y, 0.0, 1.0);\n"
      "  textureUV = (MS_TextureMatrix * UV4).xy;\n"
      "  vertexColor = uColor;\n"
      "  gl_Position = MS_ModelViewProjectionMatrix * vPosition;  \n"
      "} \n";
    
      defaultFragmentPrograms["rect"] = ""
      "precision highp float;\n"
      "varying highp vec2 textureUV; \n"
      "varying vec4 vertexColor;\n"
      "uniform highp sampler2D texture0;\n"
      "void main()                                \n"
      "{                                          \n"
      "gl_FragColor = texture2D(texture0, textureUV.xy) * vertexColor;\n"
      "} \n";

    defaultVertexPrograms["solid_color"] = ""
      "uniform mat4 MS_ModelViewProjectionMatrix;\n"
      "uniform vec4 uColor;\n"
      "attribute vec4 vPosition;   \n"
      "varying vec4 vertexColor;\n"
      "void main()                 \n"
      "{                           \n"
      "  vertexColor = uColor;\n"
      "  gl_Position = MS_ModelViewProjectionMatrix * vPosition;  \n"
      "} \n";
    
      defaultFragmentPrograms["solid_color"] = ""
      "precision highp float;\n"
      "varying vec4 vertexColor;\n"
      "void main()                                \n"
      "{                                          \n"
      "     gl_FragColor = vertexColor;\n"
      "} \n";
    
    defaultVertexPrograms["blur"] = ""
        "uniform mat4 MS_ModelViewProjectionMatrix;\n"
        "uniform mat4 MS_ModelViewMatrix;\n"
        "uniform vec4 uColor;\n"
        "uniform vec4 uFogColor;\n"
        "attribute vec2 vTexCoord;   \n"
        "attribute vec4 vPosition;\n"
        "varying vec2 textureUV;   \n"
        "varying vec4 vertexColor;\n"
        "void main(void) {\n"
        "    textureUV = vTexCoord;\n"
        "    gl_Position = MS_ModelViewProjectionMatrix * vPosition;\n"
        "    vertexColor = uColor;\n"
        "}\n";
    defaultFragmentPrograms["blur"] = ""
        "precision highp float;\n"
        "uniform highp sampler2D texture0;\n"
        "uniform vec2 texture0_size;\n"
        "uniform float offsetx;\n"
        "uniform float offsety;\n"
        "uniform float pass;\n"
        "varying highp vec2 textureUV; \n"
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
        " vec2 normalizeRectUV(vec2 rectUV) { \n "
        " return vec2((rectUV.x)/texture0_size.x, (rectUV.y)/texture0_size.y); }\n"
        "void main (void) {\n"
        "    vec2 s = vec2(offsetx, offsety);\n"
        "    // Thanks to Renaud Bedard - http://theinstructionlimit.com/?p=43\n"
        "    vec3 c = s2l(texture2D(texture0, normalizeRectUV(textureUV.xy)).rgb);\n"
        "    vec3 t = f0 * c;\n"
        "    t += f1 * s2l(texture2D(texture0, normalizeRectUV(textureUV.xy - o1*s) ).rgb);\n"
        "    t += f1 * s2l(texture2D(texture0, normalizeRectUV(textureUV.xy + o1*s) ).rgb);\n"
        "    t += f2 * s2l(texture2D(texture0, normalizeRectUV(textureUV.xy - o2*s) ).rgb);\n"
        "    t += f2 * s2l(texture2D(texture0, normalizeRectUV(textureUV.xy + o2*s) ).rgb);\n"
        "    t += f3 * s2l(texture2D(texture0, normalizeRectUV(textureUV.xy - o3*s) ).rgb);\n"
        "    t += f3 * s2l(texture2D(texture0, normalizeRectUV(textureUV.xy + o3*s) ).rgb);\n"
        "    gl_FragColor = vec4(l2s(t), 1.0) * vertexColor;\n"
         "}\n";

    defaultVertexPrograms["bloom"] = ""
        "uniform mat4 MS_ModelViewProjectionMatrix;\n"
        "uniform mat4 MS_ModelViewMatrix;\n"
        "uniform vec4 uColor;\n"
        "uniform vec4 uFogColor;\n"
        "uniform vec4 uTexCoord4;   \n"
        "uniform vec2 texture0_size;\n"
        "uniform vec2 texture1_size;\n"
        "attribute vec2 vTexCoord;   \n"
        "attribute vec4 vPosition;\n"
        "varying vec2 textureUV;   \n"
        "varying vec2 textureUV2;   \n"
        "varying vec4 fogColor;\n"
        "varying vec4 vertexColor;\n"
        "void main(void) {\n"
        "    textureUV = vTexCoord;\n"
        "    textureUV2 = vec2((vTexCoord.x/texture0_size.x) * texture1_size.x, (vTexCoord.y/texture0_size.y) * texture1_size.y);\n"
        "    gl_Position = MS_ModelViewProjectionMatrix * vPosition;\n"
        "    vertexColor = uColor;\n"
        "}\n";
    defaultFragmentPrograms["bloom"] = ""
    "precision highp float;\n"
    "uniform highp sampler2D texture0;\n"
    "uniform vec2 texture0_size;\n"
    "uniform highp sampler2D texture1;\n"
    "uniform vec2 texture1_size;\n"
    "uniform float pass;\n"
    //"varying vec4 vertexColor;\n"
    "varying highp vec2 textureUV; \n"
    "varying highp vec2 textureUV2; \n"
    "vec3 s2l(vec3 srgb) { return srgb * srgb; }\n"
    "vec3 l2s(vec3 linear) { return sqrt(linear); }\n"
    "#ifndef BLOOM_SRGB_FRAMEBUFFER\n"
    "vec3 b2l(vec3 bloom) { return bloom * bloom; }\n"
    "#else\n"
    "vec3 b2l(vec3 bloom) { return bloom; }\n"
    "#endif\n"
    " vec2 normalizeRectUV(vec2 rectUV, vec2 textureSize) { \n "
    " return vec2((rectUV.x)/textureSize.x, (rectUV.y)/textureSize.y); }\n"
    "void main (void) {\n"
    "    vec2 normalizedUV = normalizeRectUV(textureUV.xy, texture0_size);\n"
    "    vec2 normalizedUV2 = normalizeRectUV(textureUV2.xy, texture1_size);\n"
    "    vec4 color0 = texture2D(texture0, normalizedUV);\n"
    "    vec4 color1 = texture2D(texture1, normalizedUV2);\n"
    "    vec3 color = l2s(s2l(color0.rgb) + b2l(color1.rgb));\n"
    "    gl_FragColor = vec4(color, 1.0);\n"
    "}\n";

    defaultVertexPrograms["landscape"] = ""
        "uniform mat4 MS_ModelViewProjectionMatrix;\n"
        "uniform mat4 MS_ModelViewMatrix;\n"
        "uniform mat4 landscapeInverseMatrix;\n" //What is this for?
        "uniform vec4 uFogColor;\n"
        //"uniform vec4 uColor;\n"
    
        "attribute vec4 vPosition;\n"

        "attribute vec4 vColor;\n"
        "attribute vec4 vClipPlane0;   \n"
        "attribute vec4 vClipPlane1;   \n"
        "attribute vec4 vClipPlane5;   \n"
        "attribute vec4 vSxOxSyOy; \n"
        "attribute vec4 vBsBtFlSl; \n"
        "attribute vec4 vPuWoDeGl; \n"
    
        "varying vec4 fSxOxSyOy; \n"
        "varying vec4 fBsBtFlSl; \n"
        "varying vec4 fPuWoDeGl; \n"
        "varying vec4 fClipPlane0;   \n"
        "varying vec4 fClipPlane1;   \n"
        "varying vec4 fClipPlane5;   \n"
    
        "varying vec4 fogColor;\n"
        "varying vec3 relDir;\n"
        "varying vec4 vertexColor;\n"
        "varying vec4 vPosition_eyespace;\n"

        "void main(void) {\n"
        "    gl_Position = MS_ModelViewProjectionMatrix * vPosition;\n"
        "    vPosition_eyespace = MS_ModelViewMatrix * vPosition;\n"
        "    relDir = (MS_ModelViewMatrix * vPosition).xyz;\n"
        "    vertexColor = vColor;\n"
        "    fogColor = uFogColor;\n"
    
        "    fSxOxSyOy = vSxOxSyOy;\n"
        "    fBsBtFlSl = vBsBtFlSl;\n"
        "    fPuWoDeGl = vPuWoDeGl;\n"
        "    fClipPlane0 = vClipPlane0;\n"
        "    fClipPlane1 = vClipPlane1;\n"
        "    fClipPlane5 = vClipPlane5;\n"

        "}\n";
    defaultFragmentPrograms["landscape"] = ""
              "precision highp float;\n"
              "varying highp vec4 fogColor; \n"
              "uniform sampler2D texture0;\n"
              "uniform float usefog;\n"
    
              //"uniform float scalex;\n"
              //"uniform float scaley;\n"
              //"uniform float offsetx;\n"
              //"uniform float offsety;\n"
            
            "varying vec4 fSxOxSyOy; \n"
            "varying vec4 fBsBtFlSl; \n"
            "varying vec4 fPuWoDeGl; \n"
            "varying vec4 fClipPlane0;   \n"
            "varying vec4 fClipPlane1;   \n"
            "varying vec4 fClipPlane5;   \n"

    
              "uniform float yaw;\n"
              "uniform float pitch;\n"
              "varying vec3 relDir;\n"
              "varying vec4 vertexColor;\n"
              "const float zoom = 1.2;\n"
              "const float pitch_adjust = 0.96;\n"
              "varying vec4 vPosition_eyespace;\n"

              "void main(void) {\n"
                "   if( dot( vPosition_eyespace, fClipPlane0) < 0.0 ) {discard;}\n"
                "   if( dot( vPosition_eyespace, fClipPlane1) < 0.0 ) {discard;}\n"
                "   if( dot( vPosition_eyespace, fClipPlane5) < 0.0 ) {discard;}\n"
    
              "   float scalex = fSxOxSyOy.x;\n"
              "   float scaley = fSxOxSyOy.z;\n"
              "   float offsetx = fSxOxSyOy.y;\n"
              "   float offsety = fSxOxSyOy.w;\n"
            
              "    vec3 facev = vec3(cos(yaw), sin(yaw), sin(pitch));\n"
              "    vec3 relv  = (relDir);\n"
              "    float x = relv.x / (relv.z * zoom) + atan(facev.x, facev.y);\n"
              "    float y = relv.y / (relv.z * zoom) - (facev.z * pitch_adjust);\n"
              "    vec4 color = texture2D(texture0, vec2(offsetx - x * scalex, offsety - y * scaley));\n"
              "    vec3 intensity = color.rgb;\n"
              "    if (usefog > 0.0) {\n"
              "        intensity = fogColor.rgb;\n"
              "    }\n"
              "    gl_FragColor = vec4(intensity, 1.0);\n"
              "}\n";
    defaultVertexPrograms["landscape_bloom"] = defaultVertexPrograms["landscape"];
    defaultFragmentPrograms["landscape_bloom"] = ""
        "precision highp float;\n"
        "uniform mat4 MS_ModelViewProjectionMatrix;\n"
        "uniform mat4 MS_ModelViewMatrix;\n"
        "uniform sampler2D texture0;\n"
        "uniform float usefog;\n"
        //"uniform float scalex;\n"
        //"uniform float scaley;\n"
        //"uniform float offsetx;\n"
        //"uniform float offsety;\n"
    
        "varying vec4 fSxOxSyOy; \n"
        "varying vec4 fBsBtFlSl; \n"
        "varying vec4 fPuWoDeGl; \n"
        "varying vec4 fClipPlane0;   \n"
        "varying vec4 fClipPlane1;   \n"
        "varying vec4 fClipPlane5;   \n"

        "uniform float yaw;\n"
        "uniform float pitch;\n"
        "uniform float bloomScale;\n"
        "varying highp vec4 fogColor; \n"
        "varying vec3 relDir;\n"
        "varying vec4 vertexColor;\n"
        "const float zoom = 1.205;\n"
        "const float pitch_adjust = 0.955;\n"
        "varying vec4 vPosition_eyespace;\n"
        "void main(void) {\n"
        "   if( dot( vPosition_eyespace, fClipPlane0) < 0.0 ) {discard;}\n"
        "   if( dot( vPosition_eyespace, fClipPlane1) < 0.0 ) {discard;}\n"
        "   if( dot( vPosition_eyespace, fClipPlane5) < 0.0 ) {discard;}\n"

      "   float scalex = fSxOxSyOy.x;\n"
      "   float scaley = fSxOxSyOy.z;\n"
      "   float offsetx = fSxOxSyOy.y;\n"
      "   float offsety = fSxOxSyOy.w;\n"
        
        "    vec3 facev = vec3(cos(yaw), sin(yaw), sin(pitch));\n"
        "    vec3 relv  = normalize(relDir);\n"
        "    float x = relv.x / (relv.z * zoom) + atan(facev.x, facev.y);\n"
        "    float y = relv.y / (relv.z * zoom) - (facev.z * pitch_adjust);\n"
        "    vec4 color = texture2D(texture0, vec2(offsetx - x * scalex, offsety - y * scaley));\n"
        "    float intensity = clamp(bloomScale, 0.0, 1.0);\n"
        "    if (usefog > 0.0) {\n"
        "        intensity = 0.0;\n"
        "    }\n"
        "#ifdef GAMMA_CORRECTED_BLENDING\n"
        "    //intensity = intensity * intensity;\n"
        "    color.rgb = (color.rgb - 0.01) * 1.01;\n"
        "#else\n"
        "    color.rgb = (color.rgb - 0.1) * 1.11;\n"
        "#endif\n"
        "    gl_FragColor = vec4(color.rgb * intensity, 1.0);\n"
        "}\n";

    defaultVertexPrograms["landscape_infravision"] = defaultVertexPrograms["landscape"];
	defaultFragmentPrograms["landscape_infravision"] =
#include "Shaders/landscape_infravision.frag"
		;
	
    defaultVertexPrograms["sprite"] = ""
        "uniform mat4 MS_ModelViewProjectionMatrix;\n"
        "uniform mat4 MS_ModelViewMatrix;\n"
        "uniform mat4 MS_ModelViewMatrixInverse;\n"
        "uniform mat4 MS_TextureMatrix;\n"
        "uniform vec4 uColor;\n"
        "uniform vec4 uFogColor;\n"
        "uniform float depth;\n"
        "uniform float strictDepthMode;\n"
        "attribute vec4 vPosition;\n"
        "attribute vec2 vTexCoord;   \n"
        "varying vec2 textureUV;   \n"
        "varying vec4 fogColor;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying float classicDepth;\n"
        "varying vec4 vPosition_eyespace;\n"
        "void main(void) {\n"
        "    vPosition_eyespace = MS_ModelViewMatrix * vPosition;\n"
        "    gl_Position = MS_ModelViewProjectionMatrix * vPosition;\n"
        "    classicDepth = gl_Position.z / 8192.0;\n"
        "    vec4 v = MS_ModelViewMatrixInverse * vec4(0.0, 0.0, 0.0, 1.0);\n"
        "    viewDir = (vPosition - v).xyz;\n"
        "    vec4 UV4 = vec4(vTexCoord.x, vTexCoord.y, 0.0, 1.0);\n"           //DCW shitty attempt to stuff texUV into a vec4
        "    textureUV = (MS_TextureMatrix * UV4).xy;\n"
        "    vertexColor = uColor;\n"
        "    FDxLOG2E = -uFogColor.a * 1.442695;\n"
        "    fogColor = uFogColor;\n"
        "}\n";
    defaultFragmentPrograms["sprite"] = ""
        "precision highp float;\n"
        "uniform sampler2D texture0;\n"
        "uniform float glow;\n"
        "uniform float flare;\n"
        "uniform float selfLuminosity;\n"
        "uniform vec4 clipPlane0;\n"
        "uniform vec4 clipPlane1;\n"
        "uniform vec4 clipPlane5;\n"
        "uniform float logicalWidth;\n"
        "uniform float logicalHeight;\n"
        "uniform float pixelWidth;\n"
        "uniform float pixelHeight;\n"
        "varying highp vec4 fogColor; \n"
        "varying highp vec2 textureUV; \n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying float classicDepth;\n"
        "varying vec4 vPosition_eyespace;\n"

        "void main (void) {\n"
        "  if( dot( vPosition_eyespace, clipPlane0) < 0.0 ) {discard;}\n"
        "  if( dot( vPosition_eyespace, clipPlane1) < 0.0 ) {discard;}\n"
        "  if( dot( vPosition_eyespace, clipPlane5) < 0.0 ) {discard;}\n"
        "    float mlFactor = clamp(selfLuminosity + flare - classicDepth, 0.0, 1.0);\n"
        "    // more realistic: replace classicDepth with (length(viewDir)/8192.0)\n"
        "    vec3 intensity;\n"
        "    if (vertexColor.r > mlFactor) {\n"
        "        intensity = vertexColor.rgb + (mlFactor * 0.5); }\n"
        "    else {\n"
        "        intensity = (vertexColor.rgb * 0.5) + mlFactor; }\n"
        "    intensity = clamp(intensity, glow, 1.0);\n"
        "#ifdef GAMMA_CORRECTED_BLENDING\n"
        "    intensity = intensity * intensity; // approximation of pow(intensity, 2.2)\n"
        "#endif\n"
        "    vec4 color = texture2D(texture0, textureUV.xy);\n"
        "    float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);\n"
        "    gl_FragColor = vec4(mix(fogColor.rgb, color.rgb * intensity, fogFactor), vertexColor.a * color.a);\n"
//        "    if ( gl_FragColor.a == 0.0 ) {discard;} //discard transparent fragments so they don't write on the depth buffer \n "
        "}\n";
    defaultVertexPrograms["sprite_bloom"] = defaultVertexPrograms["sprite"];
    defaultFragmentPrograms["sprite_bloom"] = ""
        "precision highp float;\n"
        "uniform sampler2D texture0;\n"
        "uniform float glow;\n"
        "uniform float bloomScale;\n"
        "uniform float bloomShift;\n"
        "uniform vec4 clipPlane0;\n"
        "uniform vec4 clipPlane1;\n"
        "uniform vec4 clipPlane5;\n"
        "varying highp vec4 fogColor; \n"
        "varying highp vec2 textureUV; \n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying float classicDepth;\n"
        "varying vec4 vPosition_eyespace;\n"
        "void main (void) {\n"
        "  if( dot( vPosition_eyespace, clipPlane0) < 0.0 ) {discard;}\n"
        "  if( dot( vPosition_eyespace, clipPlane1) < 0.0 ) {discard;}\n"
        "  if( dot( vPosition_eyespace, clipPlane5) < 0.0 ) {discard;}\n"
        "    vec4 color = texture2D(texture0, textureUV.xy);\n"
        "    vec3 intensity = clamp(vertexColor.rgb, glow, 1.0);\n"
        "    //intensity = intensity * clamp(2.0 - length(viewDir)/8192.0, 0.0, 1.0);\n"
        "    intensity = clamp(intensity * bloomScale + bloomShift, 0.0, 1.0);\n"
        "#ifdef GAMMA_CORRECTED_BLENDING\n"
        "    intensity = intensity * intensity;  // approximation of pow(intensity, 2.2)\n"
        "    color.rgb = (color.rgb - 0.06) * 1.02;\n"
        "#else\n"
        "  color.rgb = (color.rgb - 0.2) * 1.25;\n"
        "#endif\n"
        "    float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);\n"
        "    gl_FragColor = vec4(mix(vec3(0.0, 0.0, 0.0), color.rgb * intensity, fogFactor), vertexColor.a * color.a);\n"
        "}\n";

	defaultVertexPrograms["sprite_infravision"] = defaultVertexPrograms["sprite"];
	defaultFragmentPrograms["sprite_infravision"] =
#include "Shaders/sprite_infravision.frag"
		;
	
    defaultVertexPrograms["invincible"] = defaultVertexPrograms["sprite"];
    defaultFragmentPrograms["invincible"] = ""
        "precision highp float;\n"
        "uniform sampler2D texture0;\n"
        "uniform float time;\n"
        "uniform float logicalWidth;\n"
        "uniform float logicalHeight;\n"
        "uniform float pixelWidth;\n"
        "uniform float pixelHeight;\n"
        "uniform vec4 clipPlane0;\n"
        "uniform vec4 clipPlane1;\n"
        "uniform vec4 clipPlane5;\n"
        "varying highp vec4 fogColor; \n"
        "varying highp vec2 textureUV; \n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying vec4 vPosition_eyespace;\n"
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
        "void main (void) {\n"
        "  if( dot( vPosition_eyespace, clipPlane0) < 0.0 ) {discard;}\n"
        "  if( dot( vPosition_eyespace, clipPlane1) < 0.0 ) {discard;}\n"
        "  if( dot( vPosition_eyespace, clipPlane5) < 0.0 ) {discard;}\n"
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
        "   vec4 color = texture2D(texture0, textureUV.xy);\n"
        "#ifdef GAMMA_CORRECTED_BLENDING\n"
        "   intensity = intensity * intensity;  // approximation of pow(intensity, 2.2)\n"
        "#endif\n"
        "   float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);\n"
        "   gl_FragColor = vec4(mix(fogColor.rgb, intensity, fogFactor), vertexColor.a * color.a);\n"
//        "    if ( gl_FragColor.a == 0.0 ) {discard;} //discard transparent fragments so they don't write on the depth buffer \n "
        "}\n";
    defaultVertexPrograms["invincible_bloom"] = defaultVertexPrograms["invincible"];
    defaultFragmentPrograms["invincible_bloom"] = ""
        "precision highp float;\n"
        "uniform sampler2D texture0;\n"
        "uniform float time;\n"
        "uniform vec4 clipPlane0;\n"
        "uniform vec4 clipPlane1;\n"
        "uniform vec4 clipPlane5;\n"
        "varying highp vec4 fogColor; \n"
        "varying highp vec2 textureUV; \n"
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
        "varying vec4 vPosition_eyespace;\n"
        "void main (void) {\n"
        "  if( dot( vPosition_eyespace, clipPlane0) < 0.0 ) {discard;}\n"
        "  if( dot( vPosition_eyespace, clipPlane1) < 0.0 ) {discard;}\n"
        "  if( dot( vPosition_eyespace, clipPlane5) < 0.0 ) {discard;}\n"
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
        "   vec4 color = texture2D(texture0, textureUV.xy);\n"
        "#ifdef GAMMA_CORRECTED_BLENDING\n"
        "   intensity = intensity * intensity;  // approximation of pow(intensity, 2.2)\n"
        "#endif\n"
        "   float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);\n"
        "   gl_FragColor = vec4(mix(fogColor.rgb, intensity, fogFactor), vertexColor.a * color.a);\n"
        "}\n";

    defaultVertexPrograms["invisible"] = defaultVertexPrograms["sprite"];
    defaultFragmentPrograms["invisible"] = ""
        "uniform sampler2D texture0;\n"
        "uniform float visibility;\n"
        "uniform vec4 clipPlane0;\n"
        "uniform vec4 clipPlane1;\n"
        "uniform vec4 clipPlane5;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying vec4 vPosition_eyespace;\n"
        "void main (void) {\n"
        "  if( dot( vPosition_eyespace, clipPlane0) < 0.0 ) {discard;}\n"
        "  if( dot( vPosition_eyespace, clipPlane1) < 0.0 ) {discard;}\n"
        "  if( dot( vPosition_eyespace, clipPlane5) < 0.0 ) {discard;}\n"
        "    vec4 color = texture2D(texture0, gl_TexCoord[0].xy);\n"
        "   vec3 intensity = vec3(0.0, 0.0, 0.0);\n"
        "    float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);\n"
        "    gl_FragColor = vec4(mix(gl_Fog.color.rgb, intensity, fogFactor), vertexColor.a * color.a * visibility);\n"
        "}\n";
    defaultVertexPrograms["invisible_bloom"] = defaultVertexPrograms["invisible"];
    defaultFragmentPrograms["invisible_bloom"] = ""
        "uniform sampler2D texture0;\n"
        "uniform float visibility;\n"
        "uniform vec4 clipPlane0;\n"
        "uniform vec4 clipPlane1;\n"
        "uniform vec4 clipPlane5;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying vec4 vPosition_eyespace;\n"
        "void main (void) {\n"
        "  if( dot( vPosition_eyespace, clipPlane0) < 0.0 ) {discard;}\n"
        "  if( dot( vPosition_eyespace, clipPlane1) < 0.0 ) {discard;}\n"
        "  if( dot( vPosition_eyespace, clipPlane5) < 0.0 ) {discard;}\n"
        "    vec4 color = texture2D(texture0, gl_TexCoord[0].xy);\n"
        "   vec3 intensity = vec3(0.0, 0.0, 0.0);\n"
        "    float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);\n"
        "    gl_FragColor = vec4(mix(vec3(0.0, 0.0, 0.0), intensity, fogFactor), vertexColor.a * color.a * visibility);\n"
        "}\n";
	
    defaultVertexPrograms["wall"] = ""
        "uniform mat4 MS_ModelViewProjectionMatrix;\n"
        "uniform mat4 MS_ModelViewMatrix;\n"
        "uniform mat4 MS_ModelViewMatrixInverse;\n"
        "uniform mat4 MS_TextureMatrix;\n"
        
        "uniform vec4 uFogColor;\n"
        //"uniform float depth;\n"
        "attribute vec2 vTexCoord;   \n"
        "attribute vec3 vNormal;   \n"
        "attribute vec4 vPosition;\n"
        "attribute vec4 vColor;\n"
        "attribute vec4 vTexCoord4;   \n"
        "attribute vec4 vClipPlane0;   \n"
        "attribute vec4 vClipPlane1;   \n"
        "attribute vec4 vClipPlane5;   \n"
        "attribute vec4 vSxOxSyOy; \n"
        "attribute vec4 vBsBtFlSl; \n"
        "attribute vec4 vPuWoDeGl; \n"
    
        "varying vec4 fSxOxSyOy; \n"
        "varying vec4 fBsBtFlSl; \n"
        "varying vec4 fPuWoDeGl; \n"
        "varying vec4 fClipPlane0;   \n"
        "varying vec4 fClipPlane1;   \n"
        "varying vec4 fClipPlane5;   \n"

        "varying vec2 textureUV2;   \n"
        "varying vec2 textureUV;   \n"
        "varying vec4 fogColor;\n"
        "varying vec3 viewXY;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying float classicDepth;\n"
        "varying mat3 tbnMatrix;"
        "varying vec4 vPosition_eyespace;\n"
        "varying vec3 eyespaceNormal;\n"
        "highp mat4 transpose(in highp mat4 inMatrix) {\n"  //I have not tested this.
        "    highp vec4 i0 = inMatrix[0];\n"
         "   highp vec4 i1 = inMatrix[1];\n"
        "    highp vec4 i2 = inMatrix[2];\n"
        "    highp vec4 i3 = inMatrix[3];\n"

        "    highp mat4 outMatrix = mat4(\n"
        "                 vec4(i0.x, i1.x, i2.x, i3.x),\n"
        "                 vec4(i0.y, i1.y, i2.y, i3.y),\n"
        "                 vec4(i0.z, i1.z, i2.z, i3.z),\n"
        "                 vec4(i0.w, i1.w, i2.w, i3.w)\n"
        "                 );\n"

        "    return outMatrix;\n"
        "}\n"
        "void main(void) {\n"
        "    vPosition_eyespace = MS_ModelViewMatrix * vPosition;\n"
        "    gl_Position  = MS_ModelViewProjectionMatrix * vPosition;\n"
        "    float depth = vPuWoDeGl.z;\n"
        "    gl_Position.z = gl_Position.z + depth*gl_Position.z/65536.0;\n"
        "    classicDepth = gl_Position.z / 8192.0;\n"
        "#ifndef DISABLE_CLIP_VERTEX\n"
//        "    gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;\n"
        "#endif\n"
        "    vec4 UV4 = vec4(vTexCoord.x, vTexCoord.y, 0.0, 1.0);\n"           //DCW shitty attempt to stuff texUV into a vec4
        "    mat3 normalMatrix = mat3(transpose(MS_ModelViewMatrixInverse));\n"           //DCW shitty replacement for gl_NormalMatrix
        "    textureUV = (MS_TextureMatrix * UV4).xy;\n"
        "    /* SETUP TBN MATRIX in normal matrix coords, gl_MultiTexCoord1 = tangent vector */\n"
        "    vec3 n = normalize(normalMatrix * vNormal);\n"
        "    vec3 t = normalize(normalMatrix * vTexCoord4.xyz);\n"
        "    vec3 b = normalize(cross(n, t) * vTexCoord4.w);\n"
        "    /* (column wise) */\n"
        "    tbnMatrix = mat3(t.x, b.x, n.x, t.y, b.y, n.y, t.z, b.z, n.z);\n"
        "    \n"
        "    /* SETUP VIEW DIRECTION in unprojected local coords */\n"
        "    viewDir = tbnMatrix * (MS_ModelViewMatrix * vPosition).xyz;\n"
        "    viewXY = -(MS_TextureMatrix * vec4(viewDir.xyz, 1.0)).xyz;\n"
        "    viewDir = -viewDir;\n"
        "    vertexColor = vColor;\n"
        "    FDxLOG2E = -uFogColor.a * 1.442695;\n"
        "    fogColor = uFogColor;"
        "    fSxOxSyOy = vSxOxSyOy;\n"
        "    fBsBtFlSl = vBsBtFlSl;\n"
        "    fPuWoDeGl = vPuWoDeGl;\n"
        "    fClipPlane0 = vClipPlane0;\n"
        "    fClipPlane1 = vClipPlane1;\n"
        "    fClipPlane5 = vClipPlane5;\n"
        "    eyespaceNormal = vec3(MS_ModelViewMatrix * vec4(vNormal, 0.0));\n"
        "}\n";
    defaultFragmentPrograms["wall"] = ""
        "precision highp float;\n"
        //"uniform vec4 clipPlane0;\n"
        //"uniform vec4 clipPlane1;\n"
        //"uniform vec4 clipPlane5;\n"
        "varying vec4 fogColor; \n"
        "varying vec2 textureUV; \n"
        "uniform sampler2D texture0;\n"
        "uniform vec4 lightPositions[32];\n"
        "uniform vec4 lightColors[32];\n"

        //"uniform mat4 MS_ModelViewMatrix;\n"
        //"uniform float pulsate;\n"
        //"uniform float wobble;\n"
        //"uniform float glow;\n"
        //"uniform float flare;\n"
        //"uniform float selfLuminosity;\n"
    
        "varying vec4 fSxOxSyOy; \n"
        "varying vec4 fBsBtFlSl; \n"
        "varying vec4 fPuWoDeGl; \n"
        "varying vec4 fClipPlane0;   \n"
        "varying vec4 fClipPlane1;   \n"
        "varying vec4 fClipPlane5;   \n"
    
        "varying vec3 viewXY;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying float classicDepth;\n"
        "varying vec4 vPosition_eyespace;\n"
        "varying vec3 eyespaceNormal;\n"
        "void main (void) {\n"
        "   if( dot( vPosition_eyespace, fClipPlane0) < 0.0 ) {discard;}\n"
        "   if( dot( vPosition_eyespace, fClipPlane1) < 0.0 ) {discard;}\n"
        "   if( dot( vPosition_eyespace, fClipPlane5) < 0.0 ) {discard;}\n"
        "    float pulsate = fPuWoDeGl.x;\n"
        "    float wobble = fPuWoDeGl.y;\n"
        "    float glow = fPuWoDeGl.w;\n"
        "    float flare = fBsBtFlSl.z;\n"
        "    float selfLuminosity = fBsBtFlSl.w;\n"
        "    vec3 texCoords = vec3(textureUV.xy, 0.0);\n"
        "    vec3 normXY = normalize(viewXY);\n"
        "    texCoords += vec3(normXY.y * -pulsate, normXY.x * pulsate, 0.0);\n"
        "    texCoords += vec3(normXY.y * -wobble * texCoords.y, wobble * texCoords.y, 0.0);\n"
        "    float mlFactor = clamp(selfLuminosity + flare - classicDepth, 0.0, 1.0);\n"
        "    // more realistic: replace classicDepth with (length(viewDir)/8192.0)\n"
        "    vec3 intensity;\n"
        "    if (vertexColor.r > mlFactor) {\n"
        "        intensity = vertexColor.rgb + (mlFactor * 0.5); }\n"
        "    else {\n"
        "        intensity = (vertexColor.rgb * 0.5) + mlFactor; }\n"
        "    intensity = clamp(intensity, glow, 1.0);\n"
        "#ifdef GAMMA_CORRECTED_BLENDING\n"
        "    intensity = intensity * intensity; // approximation of pow(intensity, 2.2)\n"
        "#endif\n"
        "    vec4 color = texture2D(texture0, texCoords.xy);\n"
        "    float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);\n"
        "    fogFactor=clamp( length(viewDir), 0.0, 1.0);\n"
        "    gl_FragColor = vec4(mix(fogColor.rgb, color.rgb * intensity, fogFactor), vertexColor.a * color.a);\n"
    
        //Add in light diffuse
        "    for(int i = 0; i < 32; ++i) {\n"
        "       float size = lightPositions[i].w;\n"
        "       if( size < .1) { break; }\n" //End of light list
        "       vec3 lightPosition = vec3(lightPositions[i].xyz);\n"
        "       vec4 lightColor = vec4(lightColors[i].rgb, 1.0);\n"
        "       float distance = length(lightPosition - vPosition_eyespace.xyz);\n"
        "       vec3 lightVector = normalize(lightPosition - vPosition_eyespace.xyz);\n"
        "       float diffuse = max(dot(eyespaceNormal, lightVector), 0.0);\n"
        "       diffuse = diffuse * max((size*size - distance*distance)/(size*size), 0.0 );\n" //Attenuation
        "       gl_FragColor = gl_FragColor + color * diffuse * lightColor;\n"
        "    }\n"

        "}\n";
    defaultVertexPrograms["wall_bloom"] = defaultVertexPrograms["wall"];
    defaultFragmentPrograms["wall_bloom"] = ""
        "precision highp float;\n"
        "uniform sampler2D texture0;\n"

        /*"uniform float pulsate;\n"
        "uniform float wobble;\n"
        "uniform float glow;\n"
        "uniform float flare;\n"
        "uniform float bloomScale;\n"
        "uniform float bloomShift;\n"*/
        //"uniform vec4 clipPlane0;\n"
        //"uniform vec4 clipPlane1;\n"
        //"uniform vec4 clipPlane5;\n"

        "varying vec4 fSxOxSyOy; \n"
        "varying vec4 fBsBtFlSl; \n"
        "varying vec4 fPuWoDeGl; \n"
        "varying vec4 fClipPlane0;   \n"
        "varying vec4 fClipPlane1;   \n"
        "varying vec4 fClipPlane5;   \n"
        "varying vec4 fogColor; \n"
        "varying vec2 textureUV; \n"
        "varying vec3 viewXY;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying vec4 vPosition_eyespace;\n"
        "varying vec3 eyespaceNormal;\n"
        "void main (void) {\n"
        "   if( dot( vPosition_eyespace, fClipPlane0) < 0.0 ) {discard;}\n"
        "   if( dot( vPosition_eyespace, fClipPlane1) < 0.0 ) {discard;}\n"
        "   if( dot( vPosition_eyespace, fClipPlane5) < 0.0 ) {discard;}\n"
        "    float pulsate = fPuWoDeGl.x;\n"
        "    float wobble = fPuWoDeGl.y;\n"
        "    float glow = fPuWoDeGl.w;\n"
        "    float flare = fBsBtFlSl.z;\n"
        "    float bloomScale = fBsBtFlSl.x;\n"
        "    float bloomShift = fBsBtFlSl.y;\n"
        "    vec3 texCoords = vec3(textureUV.xy, 0.0);\n"
        "    vec3 normXY = normalize(viewXY);\n"
        "    texCoords += vec3(normXY.y * -pulsate, normXY.x * pulsate, 0.0);\n"
        "    texCoords += vec3(normXY.y * -wobble * texCoords.y, wobble * texCoords.y, 0.0);\n"
        "    vec4 color = texture2D(texture0, texCoords.xy);\n"
        "    vec3 intensity = clamp(vertexColor.rgb, glow, 1.0);\n"
        "    float diffuse = abs(dot(vec3(0.0, 0.0, 1.0), normalize(viewDir)));\n"
        "    intensity = clamp(intensity * bloomScale + bloomShift, 0.0, 1.0);\n"
        "#ifdef GAMMA_CORRECTED_BLENDING\n"
        "    intensity = intensity * intensity; // approximation of pow(intensity, 2.2)\n"
        "#endif\n"
        "    float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);\n"
        "    fogFactor = 1.0;" //dcw this is not working yet. Just make it 1.0 for now.
        "    gl_FragColor = vec4(mix(vec3(0.0, 0.0, 0.0), color.rgb * intensity, fogFactor), vertexColor.a * color.a);\n"
        "}\n";
	defaultVertexPrograms["wall_infravision"] = defaultVertexPrograms["wall"];
	defaultFragmentPrograms["wall_infravision"] =
#include "Shaders/wall_infravision.frag"
		;
    
    defaultVertexPrograms["bump"] = defaultVertexPrograms["wall"];
    defaultFragmentPrograms["bump"] = ""
        "precision highp float;\n"
        "uniform vec4 clipPlane0;\n"
        "uniform vec4 clipPlane1;\n"
        "uniform vec4 clipPlane5;\n"
        "uniform sampler2D texture0;\n"
        "uniform sampler2D texture1;\n"
        "uniform float pulsate;\n"
        "uniform float wobble;\n"
        "uniform float glow;\n"
        "uniform float flare;\n"
        "uniform float selfLuminosity;\n"
        "uniform vec4 lightPositions[32];\n"
        "uniform vec4 lightColors[32];\n"
        "varying vec4 fClipPlane0;   \n"
        "varying vec4 fClipPlane1;   \n"
        "varying vec4 fClipPlane5;   \n"
        "varying vec2 textureUV; \n"
        "varying vec4 fogColor;\n"
        "varying vec3 c;\n"
        "varying vec3 viewXY;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying mat3 tbnMatrix;"
        "varying vec4 vPosition_eyespace;\n"
        "varying vec3 eyespaceNormal;\n"
        "void main (void) {\n"
        "   if( dot( vPosition_eyespace, fClipPlane0) < 0.0 ) {discard;}\n"
        "   if( dot( vPosition_eyespace, fClipPlane1) < 0.0 ) {discard;}\n"
        "   if( dot( vPosition_eyespace, fClipPlane5) < 0.0 ) {discard;}\n"
        "   vec3 texCoords = vec3(textureUV.xy, 0.0);\n"
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
        "    float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);\n"
        "    fogFactor=clamp( length(viewDir), 0.0, 1.0);\n"

        "	gl_FragColor = vec4(mix(fogColor.rgb, color.rgb * intensity, fogFactor), vertexColor.a * color.a);\n"

        //Add in light diffuse
        "       vec3 t = vec3(tbnMatrix[0][0], tbnMatrix[1][0], tbnMatrix[2][0]);\n"
        "       vec3 b = vec3(tbnMatrix[0][1], tbnMatrix[1][1], tbnMatrix[2][1]);\n"
        "       vec3 n = vec3(tbnMatrix[0][2], tbnMatrix[1][2], tbnMatrix[2][2]);\n"

        "    for(int i = 0; i < 32; ++i) {\n"
        "       float size = lightPositions[i].w;\n"
        "       if( size < .1) { break; }\n" //End of light list
        "       vec3 lightPosition = vec3(lightPositions[i].xyz);\n"
        "       vec4 lightColor = vec4(lightColors[i].rgb, 1.0);\n"
        "       float distance = length(lightPosition - vPosition_eyespace.xyz);\n"
        "       vec3 lightVector = normalize(lightPosition - vPosition_eyespace.xyz);\n"

        "       vec3 lightVecTangent;\n"
        "       lightVecTangent.x = dot(lightVector, t);\n"
        "       lightVecTangent.y = dot(lightVector, b);\n"
        "       lightVecTangent.z = dot(lightVector, n);\n"
        "       lightVecTangent = normalize(lightVecTangent);\n"
        "       lightVector = tbnMatrix * lightVector;"
        "       float diffuse = max(dot(lightVecTangent, norm), 0.0);\n"

        "       diffuse = diffuse * max((size*size - distance*distance)/(size*size), 0.0 );\n" //Attenuation
        "       gl_FragColor = gl_FragColor + color * diffuse * lightColor;\n"
        "    }\n"

    
        //Add in light diffuse
     /*   "    for(int i = 0; i < 32; ++i) {\n"
        "       float size = lightPositions[i].w;\n"
        "       if( size < .1) { break; }\n" //End of light list
        "       vec3 lightPosition = vec3(lightPositions[i].xyz);\n"
        "       vec4 lightColor = vec4(lightColors[i].rgb, 1.0);\n"
        "       float distance = length(lightPosition - vPosition_eyespace.xyz);\n"
        "       vec3 lightVector = normalize(lightPosition - vPosition_eyespace.xyz);\n"
        "       lightVector = lightVector * tbnMatrix;"
       // "       float diffuse = max(dot(lightVector, norm), 0.0);\n"

            "       float diffuse = max(dot(eyespaceNormal, lightVector), 0.0);\n"
        "       diffuse = diffuse * max((size*size - distance*distance)/(size*size), 0.0 );\n" //Attenuation
        "       gl_FragColor = gl_FragColor + color * diffuse * lightColor;\n"
        "    }\n"*/


        "}\n";
    defaultVertexPrograms["bump_bloom"] = defaultVertexPrograms["bump"];
    defaultFragmentPrograms["bump_bloom"] = ""
        "precision highp float;\n"
        "uniform vec4 clipPlane0;\n"
        "uniform vec4 clipPlane1;\n"
        "uniform vec4 clipPlane5;\n"
        "uniform sampler2D texture0;\n"
        "uniform sampler2D texture1;\n"
        "uniform float pulsate;\n"
        "uniform float wobble;\n"
        "uniform float glow;\n"
        "uniform float flare;\n"
        "uniform float bloomScale;\n"
        "uniform float bloomShift;\n"
        "varying vec4 fClipPlane0;   \n"
        "varying vec4 fClipPlane1;   \n"
        "varying vec4 fClipPlane5;   \n"
        "varying vec2 textureUV; \n"
        "varying vec3 viewXY;\n"
        "varying vec3 viewDir;\n"
        "varying vec4 vertexColor;\n"
        "varying float FDxLOG2E;\n"
        "varying vec4 vPosition_eyespace;\n"
        "void main (void) {\n"
        "   if( dot( vPosition_eyespace, fClipPlane0) < 0.0 ) {discard;}\n"
        "   if( dot( vPosition_eyespace, fClipPlane1) < 0.0 ) {discard;}\n"
        "   if( dot( vPosition_eyespace, fClipPlane5) < 0.0 ) {discard;}\n"
        "   vec3 texCoords = vec3(textureUV.xy, 0.0);\n"
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
        "   fogFactor=clamp( length(viewDir), 0.0, 1.0);\n"
        "	gl_FragColor = vec4(mix(vec3(0.0, 0.0, 0.0), color.rgb * intensity, fogFactor), vertexColor.a * color.a);\n"
        "}\n";
//		defaultFragmentPrograms["sprite"];
}

