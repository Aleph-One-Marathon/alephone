#ifndef _OGL_FBO_
#define _OGL_FBO_
/*

	Copyright (C) 2015 and beyond by Jeremiah Morris
	and the "Aleph One" developers.
 
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
	
	Framebuffer Object utilities
*/

#include "cseries.h"

#ifdef HAVE_OPENGL

#include "OGL_Headers.h"
#include <vector>

class FBO {
	
private:
	GLuint _fbo;
	GLuint _depthBuffer;
	GLuint _fboTarget;
	static std::vector<FBO *> active_chain;
	
public:
	GLuint _w;
	GLuint _h;
	bool _srgb;
	GLuint texID;
	
	FBO(GLuint w, GLuint h, bool srgb = false);
	~FBO();
	
	void activate(bool clear = false, GLuint fboTarget = GL_FRAMEBUFFER_EXT);
	void deactivate();
	
	void draw();
	void prepare_drawing_mode(bool blend = false);
	void reset_drawing_mode();
	void draw_full(bool blend = false);
	
	static FBO *active_fbo();
};


class FBOSwapper {
private:
	FBO first;
	FBO second;
	bool draw_to_first;
	bool active;
	bool clear_on_activate;
	
public:
	FBOSwapper(GLuint w, GLuint h, bool srgb = false) : first(w, h, srgb), second(w, h, srgb), draw_to_first(true), active(false), clear_on_activate(true) {}
	
	void activate();
	void deactivate();
	void swap();
	
	void draw(bool blend = false);
	void filter(bool blend = false);
	
	void copy(FBO& other, bool srgb);
	void copy(FBO& other) { copy(other, first._srgb); }
	
	void blend(FBO& other, bool srgb);
	void blend(FBO& other) { blend(other, first._srgb); }
	void blend_multisample(FBO& other);
	
	FBO& current_contents() { return draw_to_first ? second : first; }
};


#endif // def HAVE_OPENGL

#endif
