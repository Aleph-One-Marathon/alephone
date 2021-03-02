/*
 
 
 MatrixStack.hpp - Singleton for emulating legacy OpenGL state and matrix math. Not intended for long term use; only to ease transition to fully programmable pipeline.
 
 Created by Dustin Wenz on 2/7/18.
 
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
 
 */

#ifndef MatrixStack_hpp
#define MatrixStack_hpp

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <stdio.h>

#include "map.h"

#ifdef HAVE_OPENGL
    #include "OGL_Headers.h"
#else
    typedef unsigned int    GLenum;
    typedef float           GLfloat;
#endif

#define STACK_MAX 20
#define CLIPPING_PLANES 10

//Cribbed from gl.h.
#define MS_MODELVIEW                      0x1700
#define MS_PROJECTION                     0x1701
#define MS_TEXTURE                        0x1702

#define MS_MODELVIEW_MATRIX               MS_MODELVIEW
#define MS_PROJECTION_MATRIX              MS_PROJECTION
#define MS_TEXTURE_MATRIX                 MS_TEXTURE

  //Our normal array must big enough to hold normals up to MAXIMUM_VERTICES_PER_WORLD_POLYGON and MAXIMUM_VERTICES_PER_POLYGON. Must be multiple of 3.
#define MAX_NORMAL_ELEMENTS (MAXIMUM_VERTICES_PER_POLYGON+4) * 3

class MatrixStack{
public:
  static MatrixStack* Instance();
  
  glm::mat4 (&activeStack())[STACK_MAX]; //Returns a reference to the active matrix stack.
  int activeStackIndex(); //Returns reference to top index of active stack.
  void setActiveStackIndex(int index); //Sets index for the top of active stack.

  void matrixMode(int newMode);
  int currentActiveMode();
  glm::mat4 activeMatrix();
  void pushMatrix();
  void popMatrix();
  void getFloatv (GLenum pname, GLfloat* params);
  void getFloatvInverse (GLenum pname, GLfloat* params);
  void getFloatvModelviewProjection(GLfloat* params); //populates params with the product of modelview and projection
  void getFloatvModelview(GLfloat* params); //populates params with unmodified modelview

  void loadIdentity();
  void loadZero();
  void loadMatrixf(const GLfloat *m);
  void translatef(GLfloat x, GLfloat y, GLfloat z);
  void scalef (GLfloat x, GLfloat y, GLfloat z);
  void rotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
  void multMatrixf (const GLfloat *m);

  void transformVertex (GLfloat &x, GLfloat &y, GLfloat &z);
  
  void orthof (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
  void frustumf (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
  
  void clipPlanef (int index, const GLfloat *equation);
  void enablePlane (int index);
  void disablePlane (int index);
  void getPlanev (int index, GLfloat* params);
  
  void color3f (GLfloat red, GLfloat green, GLfloat blue);
  void color4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
  GLfloat* color();
  
    //Substitutes for glFogfv and glFogf. RGB channels go into the first three elements, and density is in the alpha channel.
  void fogColor3f (GLfloat red, GLfloat green, GLfloat blue);
  void fogDensity (GLfloat density);
  GLfloat* fog();
  
  void normal3f (GLfloat nx, GLfloat ny, GLfloat nz);
  GLfloat* normals();
  
private:
  MatrixStack(){
    activeMode=MS_MODELVIEW; modelviewIndex=projectionIndex=textureIndex=0;
    
    for( int i = 0; i < CLIPPING_PLANES; i++) {
      planeActivated[i]=0;
      glm::vec4 v(0,0,0,0);
      clippingPlanes[i] = v;
    }
    glm::vec4 v(0,0,0,0);
    nullPlane = v;
    
  };
  
  MatrixStack(MatrixStack const&){};
  MatrixStack& operator=(MatrixStack const&){};
  static MatrixStack* m_pInstance;
  
  int activeMode;
  int modelviewIndex;
  int projectionIndex;
  int textureIndex;

  glm::mat4 modelviewStack[STACK_MAX];
  glm::mat4 projectionStack[STACK_MAX];
  glm::mat4 textureStack[STACK_MAX];
  
  bool planeActivated[CLIPPING_PLANES];
  glm::vec4 clippingPlanes[CLIPPING_PLANES];
  glm::vec4 nullPlane;
  
  GLfloat vertexColor[4];
  GLfloat fogColor[4];
  GLfloat normalArray[MAX_NORMAL_ELEMENTS];
};

MatrixStack* MSI(); //Convenience instance access

#endif /* MatrixStack_hpp */
