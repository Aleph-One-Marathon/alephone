#ifndef __OGL_Win32_h__
#define __OGL_Win32_h__

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


