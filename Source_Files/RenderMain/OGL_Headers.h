#ifndef _OGL_HEADERS_
#define _OGL_HEADERS_

/*

	Copyright (C) 2009 by Gregory Smith
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

	Uniform header for all Aleph One OpenGL users
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_OPENGL

#ifdef __WIN32__
#include <glad/glad.h>
#else

#ifdef __APPLE__
//Below is cribbed from angle_gl.h, which vcpkg for some reason isn't grabbing as expected. Normally you would just need: #include <angle_gl.h>
//In the future, we will just probqbly use something more like #include <SDL_opengles2.h>
    #include "GLES/gl.h"
    #include "GLES/glext.h"
    #include "GLES2/gl2.h"
    #include "GLES2/gl2ext.h"
    #include "GLES3/gl3.h"
    #include "GLES3/gl31.h"
    #include "GLES3/gl32.h"

    // TODO(http://anglebug.com/3730): Autogenerate these enums from gl.xml
    // HACK: Defines for queries that are not in GLES
    #define GL_CONTEXT_PROFILE_MASK 0x9126
    #define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002
    #define GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#endif

#ifndef __APPLE__
#include <SDL2/SDL_opengles2.h>
#endif

#endif

#endif

#endif
