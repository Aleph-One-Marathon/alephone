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
#include "OGL_FBO.h"

#ifdef HAVE_OPENGL

#include "OGL_Setup.h"
#include "OGL_Render.h"

std::vector<FBO *> FBO::active_chain;

FBO::FBO(GLuint w, GLuint h, bool srgb) : _h(h), _w(w), _srgb(srgb) {
	glGenFramebuffersEXT(1, &_fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fbo);
	
	glGenRenderbuffersEXT(1, &_depthBuffer);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, _depthBuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, _w, _h);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, _depthBuffer);
	
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texID);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, srgb ? GL_SRGB : GL_RGB8, _w, _h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, texID, 0);
	assert(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void FBO::activate(bool clear, GLuint fboTarget) {
	if (!active_chain.size() || active_chain.back() != this) {
		active_chain.push_back(this);
		_fboTarget = fboTarget;
		glBindFramebufferEXT(fboTarget, _fbo);
		glPushAttrib(GL_VIEWPORT_BIT);
		glViewport(0, 0, _w, _h);
		if (_srgb)
			glEnable(GL_FRAMEBUFFER_SRGB_EXT);
		else
			glDisable(GL_FRAMEBUFFER_SRGB_EXT);
		if (clear)
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

void FBO::deactivate() {
	if (active_chain.size() && active_chain.back() == this) {
		active_chain.pop_back();
		glPopAttrib();
		
		GLuint prev_fbo = 0;
		bool prev_srgb = Using_sRGB;
		if (active_chain.size()) {
			prev_fbo = active_chain.back()->_fbo;
			prev_srgb = active_chain.back()->_srgb;
		}
		glBindFramebufferEXT(_fboTarget, prev_fbo);
		if (prev_srgb)
			glEnable(GL_FRAMEBUFFER_SRGB_EXT);
		else
			glDisable(GL_FRAMEBUFFER_SRGB_EXT);
	}
}

void FBO::draw() {
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texID);
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
	OGL_RenderTexturedRect(0, 0, _w, _h, 0, _h, _w, 0);
	glDisable(GL_TEXTURE_RECTANGLE_ARB);
}

void FBO::prepare_drawing_mode(bool blend) {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glDisable(GL_DEPTH_TEST);
	if (!blend)
		glDisable(GL_BLEND);
	
	glOrtho(0, _w, _h, 0, -1, 1);
	glColor4f(1.0, 1.0, 1.0, 1.0);
}

void FBO::reset_drawing_mode() {
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void FBO::draw_full(bool blend) {
	prepare_drawing_mode(blend);
	draw();
	reset_drawing_mode();
}

FBO::~FBO() {
	glDeleteFramebuffersEXT(1, &_fbo);
	glDeleteRenderbuffersEXT(1, &_depthBuffer);
}


void FBOSwapper::activate() {
	if (active)
		return;
	if (draw_to_first)
		first.activate(clear_on_activate);
	else
		second.activate(clear_on_activate);
	active = true;
	clear_on_activate = false;
}

void FBOSwapper::deactivate() {
	if (!active)
		return;
	if (draw_to_first)
		first.deactivate();
	else
		second.deactivate();
	active = false;
}

void FBOSwapper::swap() {
	deactivate();
	draw_to_first = !draw_to_first;
	clear_on_activate = true;
}

void FBOSwapper::draw(bool blend) {
	current_contents().draw_full(blend);
}

void FBOSwapper::filter(bool blend) {
	activate();
	draw(blend);
	swap();
}

void FBOSwapper::copy(FBO& other, bool srgb) {
	clear_on_activate = true;
	activate();
	other.draw_full(false);
	swap();
}

void FBOSwapper::blend(FBO& other, bool srgb) {
	activate();
	if (!srgb)
		glDisable(GL_FRAMEBUFFER_SRGB_EXT);
	else
		glEnable(GL_FRAMEBUFFER_SRGB_EXT);
	other.draw_full(true);
	deactivate();
}

void FBOSwapper::blend_multisample(FBO& other) {
	swap();
	activate();
	
	// set up FBO passed in as texture #1
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, other.texID);
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	GLint multi_coordinates[8] = { 0, GLint(other._h), GLint(other._w), GLint(other._h), GLint(other._w), 0, 0, 0 };
	glTexCoordPointer(2, GL_INT, 0, multi_coordinates);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	
	draw(true);
	
	// tear down multitexture stuff
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glDisable(GL_TEXTURE_RECTANGLE_ARB);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	
	deactivate();
}

#endif
