#ifndef __OGL_Win32_h__
#define __OGL_Win32_h__

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

typedef void (*GL_ACTIVETEXTUREARB_FUNC)(unsigned int);
typedef void (*GL_CLIENTACTIVETEXTUREARB_FUNC)(int);

#ifdef __OGL_Win32_cpp__
GL_ACTIVETEXTUREARB_FUNC glActiveTextureARB_ptr =  0;
GL_CLIENTACTIVETEXTUREARB_FUNC glClientActiveTextureARB_ptr = 0;
int has_multitex = 0;

#else
extern GL_ACTIVETEXTUREARB_FUNC glActiveTextureARB_ptr;
extern GL_CLIENTACTIVETEXTUREARB_FUNC glClientActiveTextureARB_ptr;
extern void setup_gl_extensions();
extern int has_multitex;
#define glClientActiveTextureARB glClientActiveTextureARB_ptr        
#define glActiveTextureARB glActiveTextureARB_ptr        
#endif

#endif


