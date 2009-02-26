/*
 *  FBO.h
 *  AlephOne-OSX10.4
 *
 *  Created by Clemens on 2/15/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __FBO_H
#define __FBO_H

#include "OGL_shader.h"

class FBO {
	
	friend class Blur;
private:
	GLuint _fbo;
	GLuint _depthBuffer;
public:
	GLuint _w;
	GLuint _h;
	GLuint texID;
	
	FBO(GLuint w, GLuint h) : _h(h), _w(w) {
		glGenFramebuffersEXT(1, &_fbo);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fbo);		
		
		glGenRenderbuffersEXT(1, &_depthBuffer);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, _depthBuffer);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, _w, _h);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, _depthBuffer);
		
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texID);
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB8, _w, _h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);		
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, texID, 0);
		assert(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);		
	}
	
	void activate() {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fbo);		
		glPushAttrib(GL_VIEWPORT_BIT);
		glViewport(0, 0, _w, _h);
	}
	
	static void deactivate() {
		glPopAttrib();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);		
	}
	
	void draw() {
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texID);
		glEnable(GL_TEXTURE_RECTANGLE_ARB);
		
		glBegin(GL_QUADS);
		glTexCoord2i(0., 0.); glVertex2i(0., 0.);
		glTexCoord2i(0., _h); glVertex2i(0., _h);	
		glTexCoord2i(_w, _h); glVertex2i(_w, _h);
		glTexCoord2i(_w, 0.); glVertex2i(_w, 0.);
		glEnd();
		
		glDisable(GL_TEXTURE_RECTANGLE_ARB);
	}
	
	~FBO() {
		glDeleteFramebuffersEXT(1, &_fbo);
		glDeleteRenderbuffersEXT(1, &_depthBuffer);
	}	
};

class Blur {
	
private:
	FBO _horizontal;
	FBO _vertical;

	Shader *_shader;
	Shader *_shader2;

public:
	
	Blur(GLuint w, GLuint h, Shader* shaderV, Shader* shaderH)
		: _horizontal(w, h), _vertical(w, h), _shader(shaderV), _shader2(shaderH) {}
	
	void resize(GLuint w, GLuint h) {
		_horizontal = FBO(w, h);
		_vertical = FBO(w, h);
	}
	
	void begin() {
		/* draw in first buffer */
		_horizontal.activate();
	}
	
	void end() {
		FBO::deactivate();
	}
	
	void draw() {
		assert(_shader);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();	
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();	

		glDisable(GL_BLEND);
		glOrtho(0, _vertical._w, 0, _vertical._h, 0.0, 1.0);
		glColor4f(1., 1., 1., 1.0);
		_vertical.activate();
		_shader->enable();
		_horizontal.draw();
		_shader->disable();
		FBO::deactivate();
		
		glEnable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);
		glColor4f(1., 1., 1., 1.);
		_shader2->enable();
		_vertical.draw();
		_shader2->disable();
	}
};

#endif
