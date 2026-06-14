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

#if defined(__ANDROID__)

// OpenGL ES 3.x on Android (Meta Quest). No GLEW (ES needs no extension loader)
// and no GLU (it does not exist on ES). Mirrors the headers TBXR/OpenXR uses.
// NOTE: legacy fixed-function symbols (glBegin, the matrix stack, ARB multitexture,
// GL_CLIP_PLANE*, glAlphaFunc, GL_QUADS/GL_POLYGON) do NOT exist here -- the shader
// renderer must be modernized to programmable-pipeline GLES before it will link.
// See docs/VR_PORT_PLAN.md (Phase 1).
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <GLES2/gl2ext.h>

// Map the legacy renderer's ARB/EXT/fixed-function GL usage onto core GLES3.
#include "gl_es_compat.h"

#elif defined(__WIN32__)

#define GLEW_STATIC 1
#include <GL/glew.h>

#else

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif

#include <SDL2/SDL_opengl.h>

#if defined (__APPLE__) && defined(__MACH__)
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#endif

#endif

#endif
