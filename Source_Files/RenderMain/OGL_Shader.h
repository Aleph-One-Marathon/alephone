#ifndef _OGL_SHADER_
#define _OGL_SHADER_
/*
 OGL_SHADER.H
 
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

#include <string>
#include <map>
#include "OGL_Headers.h"
#include "FileHandler.h"

class Shader {

friend class XML_ShaderParser;
friend class Shader_MML_Parser;
public:
	enum UniformName {
		U_Texture0,
		U_Texture1,
		U_Texture2,
		U_Texture3,
		U_Time,
		U_Pulsate,
		U_Wobble,
		U_Flare,
		U_BloomScale,
		U_BloomShift,
		U_Repeat,
		U_OffsetX,
		U_OffsetY,
		U_Pass,
		U_UseFog,
		U_Visibility,
		U_Depth,
		U_StrictDepthMode,
		U_Glow,
		U_LandscapeInverseMatrix,
		U_ScaleX,
		U_ScaleY,
		U_Yaw,
		U_Pitch,
		U_SelfLuminosity,
		U_GammaAdjust,
		U_LogicalWidth,
		U_LogicalHeight,
		U_PixelWidth,
		U_PixelHeight,
		NUMBER_OF_UNIFORM_LOCATIONS
	};

	enum ShaderType {
		S_Error,
        S_Blur,
		S_Bloom,
		S_Landscape,
		S_LandscapeBloom,
		S_LandscapeInfravision,
		S_Sprite,
		S_SpriteBloom,
		S_SpriteInfravision,
		S_Invincible,
		S_InvincibleBloom,
		S_Invisible,
		S_InvisibleBloom,
		S_Wall,
		S_WallBloom,
		S_WallInfravision,
		S_Bump,
		S_BumpBloom,
		S_Gamma,
		NUMBER_OF_SHADER_TYPES
	};
private:

	GLhandleARB _programObj;
	std::string _vert;
	std::string _frag;
	int16 _passes;
	bool _loaded;

	static const char* _shader_names[NUMBER_OF_SHADER_TYPES];
	static std::vector<Shader> _shaders;

	static const char* _uniform_names[NUMBER_OF_UNIFORM_LOCATIONS];
	GLint _uniform_locations[NUMBER_OF_UNIFORM_LOCATIONS];
	float _cached_floats[NUMBER_OF_UNIFORM_LOCATIONS];

	GLint getUniformLocation(UniformName name) { 
		if (_uniform_locations[name] == -1) {
			_uniform_locations[name] = glGetUniformLocationARB(_programObj, _uniform_names[name]);
		}
		return _uniform_locations[name];
	}
	
public:

	static Shader* get(ShaderType type) { return &_shaders[type]; }
	static void loadAll();
	static void unloadAll();
	
	Shader() : _programObj(0), _passes(-1), _loaded(false) {}
	Shader(const std::string& name);
	Shader(const std::string& name, FileSpecifier& vert, FileSpecifier& frag, int16& passes);
	~Shader();

	void load();
	void init();
	void enable();
	void unload();
	void setFloat(UniformName name, float); // shader must be enabled
	void setMatrix4(UniformName name, float *f);

	int16 passes();

	static void disable();
};


class InfoTree;
void parse_mml_opengl_shader(const InfoTree& root);
void reset_mml_opengl_shader();

#endif
