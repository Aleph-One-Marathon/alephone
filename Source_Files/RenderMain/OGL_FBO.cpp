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
#include "Logging.h"

#ifdef HAVE_OPENGL

#include "OGL_Setup.h"
#include "OGL_Render.h"
#include "OGL_Shader.h"

std::vector<FBO *> FBO::active_chain;

FBO::FBO(GLuint w, GLuint h, bool srgb) : _h(h), _w(w), _srgb(srgb) {
    setup(w, h, srgb);

    /*
	glGenFramebuffers(1, &_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	
	glGenRenderbuffers(1, &_depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _w, _h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
	
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_RECTANGLE, texID);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, srgb ? GL_SRGB : GL_RGB8, _w, _h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, texID, 0);
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);*/
}

void FBO::setup(GLuint w, GLuint h, bool srgb) {
    GLenum err;
  _h = h; _w = w; _srgb = srgb;
  
  printf("Setup new FBO h:%d, w:%d\n", h, w);
    
  //Do nothing if not valid size. Call again later to initialize.
  if( w == 0 && h == 0) {
    return;
  }
  
  while (glGetError()) {} //Clear earlier errors
    
  glGenFramebuffers(1, &_fbo);
    while ((err = glGetError())) { printf("FBO glGenFramebuffers: OpenGL Error %d\n",err);}
  glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    while ((err = glGetError())) { printf("FBO glBindFramebuffer: OpenGL Error %d\n",err);}
    
  //Create texture and attach it to framebuffer's color attachment point
  glGenTextures(1, &texID);
    while ((err = glGetError())) { printf("FBO glGenTextures: OpenGL Error %d\n",err);}
  glBindTexture(GL_TEXTURE_2D, texID); //DCW was GL_TEXTURE_RECTANGE
    while ((err = glGetError())) { printf("FBO glBindTexture: OpenGL Error %d\n",err);}
  glTexImage2D(GL_TEXTURE_2D, 0, srgb ? GL_SRGB : GL_RGBA, _w, _h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);//DCW was GL_TEXTURE_RECTANGLE, changed GL_RGB to GL_RGBA
    while ((err = glGetError())) { printf("FBO glTexImage2D: OpenGL Error %d\n",err);}
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    while ((err = glGetError())) { printf("FBO glTexParameteri: OpenGL Error %d\n",err);}
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texID, 0);
    while ((err = glGetError())) { printf("FBO glFramebufferTexture2D: OpenGL Error %d\n",err);}

  
  //Generate depth buffer
  glGenRenderbuffers(1, &_depthBuffer);
    while ((err = glGetError())) { printf("FBO glGenRenderbuffers: OpenGL Error %d\n",err);}
  glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer);
    while ((err = glGetError())) { printf("FBO glBindRenderbuffer: OpenGL Error %d\n",err);}
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _w, _h);
    while ((err = glGetError())) { printf("FBO glRenderbufferStorage: OpenGL Error %d\n",err);}
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
    while ((err = glGetError())) { printf("FBO glFramebufferRenderbuffer: OpenGL Error %d\n",err);}
  
  if( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE ) { printf("FBO framebuffer not complete\n"); }
    
  glBindFramebuffer(GL_FRAMEBUFFER, 1);
  glBindRenderbuffer(GL_RENDERBUFFER, 1);
  
  glPopGroupMarkerEXT();
}

void FBO::activate(bool clear, GLuint fboTarget) {
	if (!active_chain.size() || active_chain.back() != this) {
		active_chain.push_back(this);
        _fboTarget = fboTarget;
        glBindFramebuffer(fboTarget, _fbo);
		
        //glPushAttrib(GL_VIEWPORT_BIT);
        glGetIntegerv(GL_VIEWPORT, viewportCache);
        glDisable(GL_SCISSOR_TEST); //DCW Disable scissor, otherwise it cuts into full screen sometimes.
        glViewport(0, 0, _w, _h);
        if (_srgb) {}
			//////glEnable(GL_FRAMEBUFFER_SRGB);
        else {
			//glDisable(GL_FRAMEBUFFER_SRGB); //NOT SUPPORTED ANGLE ENUM
        }
		if (clear)
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

void FBO::deactivate() {
	if (active_chain.size() && active_chain.back() == this) {
		active_chain.pop_back();
		
        //glPopAttrib();
		glViewport(viewportCache[0], viewportCache[1], viewportCache[2], viewportCache[3]);
		GLuint prev_fbo = 0;
		bool prev_srgb = Using_sRGB;
		if (active_chain.size()) {
			prev_fbo = active_chain.back()->_fbo;
			prev_srgb = active_chain.back()->_srgb;
		}
		glBindFramebuffer(_fboTarget, prev_fbo);
        
        if (prev_srgb) {}
			//////glEnable(GL_FRAMEBUFFER_SRGB);
        else {
			//glDisable(GL_FRAMEBUFFER_SRGB); //NOT SUPPORTED ANGLE ENUM
        }
	}
}

void FBO::draw() {
	/*glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texID);
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
	OGL_RenderTexturedRect(0, 0, _w, _h, 0, _h, _w, 0);
	glDisable(GL_TEXTURE_RECTANGLE_ARB);*/
    
    
    glBindTexture(GL_TEXTURE_2D, texID);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
      //DCW if there is a shader already active, draw the quad using that. Otherwise, draw with the default shader.
    if (lastEnabledShader()) {
      drawQuadWithActiveShader(0, 0, _w, _h, 0, _h, _w, 0);
    } else {
      OGL_RenderTexturedRect(0, 0, _w, _h, 0, 1.0, 1.0, 0); //DCW; uses normalized texture coordinates
    }
}

void FBO::drawQuadWithActiveShader(float x, float y, float w, float h, float tleft, float ttop, float tright, float tbottom)
{
  GLfloat modelMatrix[16], modelProjection[16], modelMatrixInverse[16], textureMatrix[16];
  
  Shader *theShader = lastEnabledShader();
  
  MSI()->getFloatv(MS_MODELVIEW, modelMatrix);
  MSI()->getFloatvInverse(MS_MODELVIEW, modelMatrixInverse);
  MSI()->getFloatv(MS_TEXTURE, textureMatrix);
  MatrixStack::Instance()->getFloatvModelviewProjection(modelProjection);
  
  theShader->setMatrix4(Shader::U_ModelViewMatrix, modelMatrix);
  theShader->setMatrix4(Shader::U_ModelViewProjectionMatrix, modelProjection);
  theShader->setMatrix4(Shader::U_ModelViewMatrixInverse, modelMatrixInverse);
  theShader->setMatrix4(Shader::U_TextureMatrix, textureMatrix);
  theShader->setVec4(Shader::U_Color, MatrixStack::Instance()->color());
  
  GLfloat vVertices[12] = { x, y, 0,
    x + w, y, 0,
    x + w, y + h, 0,
    x, y + h, 0};
  
  GLfloat texCoords[8] = { tleft, ttop, tright, ttop, tright, tbottom, tleft, tbottom };
  
  GLubyte indices[] =   {0,1,2,
    0,2,3};
    
  
  glVertexAttribPointer(Shader::ATTRIB_TEXCOORDS, 2, GL_FLOAT, 0, 0, texCoords);
  glEnableVertexAttribArray(Shader::ATTRIB_TEXCOORDS);
  
  glVertexAttribPointer(Shader::ATTRIB_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
  glEnableVertexAttribArray(Shader::ATTRIB_VERTEX);
  
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
    
  glDisableVertexAttribArray(0);
}

void FBO::prepare_drawing_mode(bool blend) {
	MSI()->matrixMode(MS_PROJECTION);
	MSI()->pushMatrix();
	MSI()->loadIdentity();
	MSI()->matrixMode(MS_MODELVIEW);
	MSI()->pushMatrix();
	MSI()->loadIdentity();
	
	glDisable(GL_DEPTH_TEST);
	if (!blend)
		glDisable(GL_BLEND);
    
	MSI()->orthof(0, _w, _h, 0, -1, 1);
	MSI()->color4f(1.0, 1.0, 1.0, 1.0);
}

void FBO::reset_drawing_mode() {
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
    MSI()->matrixMode(MS_PROJECTION);
    MSI()->popMatrix();
	MSI()->matrixMode(MS_MODELVIEW);
	MSI()->popMatrix();
}

void FBO::draw_full(bool blend) {
	prepare_drawing_mode(blend);
	draw();
	reset_drawing_mode();
}

FBO::~FBO() {
	glDeleteFramebuffers(1, &_fbo);
	glDeleteRenderbuffers(1, &_depthBuffer);
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
    if (!srgb){}
		//////glDisable(GL_FRAMEBUFFER_SRGB_EXT);
	else
		//////glEnable(GL_FRAMEBUFFER_SRGB_EXT);
	other.draw_full(true);
	deactivate();
}

void FBOSwapper::blend_multisample(FBO& other) {
	swap();
	activate();
	
	// set up FBO passed in as texture #1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, other.texID);
    /*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);*/
    
    
	//////glEnable(GL_TEXTURE_RECTANGLE_ARB);
	glActiveTexture(GL_TEXTURE0);
    
	//////glClientActiveTexture(GL_TEXTURE1);
	//glEnableClientState(GL_TEXTURE_COORD_ARRAY); //NOT SUPPORTED ANGLE FUNCTION
	//GLint multi_coordinates[8] = { 0, GLint(other._h), GLint(other._w), GLint(other._h), GLint(other._w), 0, 0, 0 };
	//glTexCoordPointer(2, GL_INT, 0, multi_coordinates); //NOT SUPPORTED ANGLE FUNCTION
	//////glClientActiveTexture(GL_TEXTURE0);
	
	draw(true);
	
	// tear down multitexture stuff
	glActiveTexture(GL_TEXTURE1);
	//////glDisable(GL_TEXTURE_RECTANGLE_ARB);
	glActiveTexture(GL_TEXTURE0);
	
	//////glClientActiveTexture(GL_TEXTURE1);
	//glDisableClientState(GL_TEXTURE_COORD_ARRAY); //NOT SUPPORTED ANGLE FUNCTION
	//////glClientActiveTexture(GL_TEXTURE0);
	
	deactivate();
}

#endif
