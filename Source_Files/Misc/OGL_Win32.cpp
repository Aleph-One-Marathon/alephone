#define __OGL_Win32_cpp__

#include "OGL_Win32.h"
#include <windows.h>
#include <GL/gl.h>
#include "SDL.h"

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

}

#undef __OGL_Win32_cpp__
