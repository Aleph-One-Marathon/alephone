/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
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

*/

#define __OGL_Win32_cpp__

#include "OGL_Win32.h"
#include <windows.h>
#include <GL/gl.h>
#include <SDL.h>

void setup_gl_extensions() {

	glActiveTextureARB_ptr = (GL_ACTIVETEXTUREARB_FUNC)
		SDL_GL_GetProcAddress("glActiveTextureARB");

	glClientActiveTextureARB_ptr = (GL_CLIENTACTIVETEXTUREARB_FUNC)
		SDL_GL_GetProcAddress("glClientActiveTextureARB");
        
	if (glActiveTextureARB_ptr == 0 || glClientActiveTextureARB_ptr == 0) {
		has_multitex = 0;
	} else {
		has_multitex = 1;
	}

	glCompressedTexImage2DARB_ptr = (PFNGLCOMPRESSEDTEXIMAGE2DARBPROC) 
		SDL_GL_GetProcAddress("glCompressedTexImage2DARB");
	if (!glCompressedTexImage2DARB_ptr) fprintf(stderr, "uh oh!\n");
}

#undef __OGL_Win32_cpp__
