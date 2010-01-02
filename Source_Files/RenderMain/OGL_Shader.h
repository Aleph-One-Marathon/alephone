#ifndef _OGL_SHADER_
#define _OGL_SHADER_
/*
 OGL_SHADER.H
 
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

#include <string>
#include <map>
#include "OGL_Headers.h"
#include "FileHandler.h"

class Shader {

friend class XML_ShaderParser;
private:

	GLhandleARB _programObj;
	GLcharARB *_vert;
	GLcharARB *_frag;
	int16 _passes;
	bool _loaded;

	static std::map<std::string, Shader> Shaders;

public:

	static Shader* get(const std::string& name);
	static void unloadAll();
	
	Shader() : _programObj(NULL), _passes(-1), _loaded(false) {}
	Shader(const std::string& name);
	Shader(const std::string& name, FileSpecifier& vert, FileSpecifier& frag, int16& passes);
	~Shader();

	void load();
	void init();
	void enable();
	void unload();
	void setFloat(const char* name, float);
	int16 passes();

	static void disable();
};

#include "cseries.h"
#include "XML_ElementParser.h"

XML_ElementParser *Shader_GetParser();

#endif
