/*
 
 MatrixStack.cpp - Singleton for emulating legacy OpenGL state and matrix math. Not intended for long term use; only to ease transition to fully programmable pipeline.
 
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

#include "MatrixStack.hpp"
#include "Logging.h"
#include <stddef.h>

MatrixStack* MatrixStack::m_pInstance = NULL;

MatrixStack* MatrixStack::Instance()
{
  if (!m_pInstance)
    m_pInstance = new MatrixStack;
  
    return m_pInstance;
}

void MatrixStack::matrixMode(int newMode){
  switch (newMode) {
    case(MS_PROJECTION):
    case(MS_MODELVIEW) :
    case(MS_TEXTURE) :
      activeMode=newMode;
      break;
    default:
      logWarning("Attempted to set undefined matrix mode!");
      break;
  }

}

int MatrixStack::currentActiveMode(){
  return activeMode;
}

glm::mat4 MatrixStack::activeMatrix() {
  return activeStack()[activeStackIndex()];
}

glm::mat4 (&MatrixStack::activeStack())[STACK_MAX] {
  switch (activeMode) {
    case(MS_PROJECTION):
      return projectionStack;
      break;
    case(MS_MODELVIEW) :
      return modelviewStack;
      break;
    case(MS_TEXTURE) :
      return textureStack;
      break;
    default:
      logWarning("No valid matrix is active!");
      break;
  }
  return modelviewStack; //Default stack, if none selected.
}

int MatrixStack::activeStackIndex() {
  switch (activeMode) {
    case(MS_PROJECTION):
      return projectionIndex;
      break;
    case(MS_MODELVIEW) :
      return modelviewIndex;
      break;
    case(MS_TEXTURE) :
      return textureIndex;
      break;
    default:
      logWarning("No valid matrix is active for index selection!");
      break;
  }
  return modelviewIndex; //Default stack, if none selected.
}

void MatrixStack::setActiveStackIndex(int index) {
  if (index < 0 || index >= STACK_MAX) {
    logWarning("Specified matrix index out of range!");
    return;
  }
  
  switch (activeMode) {
    case(MS_PROJECTION):
      projectionIndex=index;
      break;
    case(MS_MODELVIEW) :
      modelviewIndex=index;
      break;
    case(MS_TEXTURE) :
      textureIndex=index;
      break;
    default:
      logWarning("No valid matrix is active to set index for!");
      break;
  }
  return;
}

void MatrixStack::pushMatrix()
{
    //As in OpenGL, pushing a matrix causes the new top of stack to be a duplicate of the matrix just pushed.
  activeStack()[activeStackIndex()+1] = activeMatrix();
  setActiveStackIndex(activeStackIndex() + 1);
}

void MatrixStack::popMatrix()
{
  setActiveStackIndex(activeStackIndex() - 1);
}

void MatrixStack::getFloatv(GLenum pname, GLfloat* params){
  GLfloat* m = glm::value_ptr(modelviewStack[modelviewIndex]); //Default
  
  switch (pname) {
      case(MS_MODELVIEW):
        //Default (See above)
      break;
      case(MS_PROJECTION):
        m = glm::value_ptr(projectionStack[projectionIndex]);
      break;
      case(MS_TEXTURE) :
        m = glm::value_ptr(textureStack[textureIndex]);
      break;
    
    default:
      logWarning("No valid matrix is active!");
      break;
  }
  
  params[0]  = m[0];  params[1]  = m[1];  params[2]  = m[2];  params[3]  = m[3];
  params[4]  = m[4];  params[5]  = m[5];  params[6]  = m[6];  params[7]  = m[7];
  params[8]  = m[8];  params[9]  = m[9];  params[10] = m[10]; params[11] = m[11];
  params[12] = m[12]; params[13] = m[13]; params[14] = m[14]; params[15] = m[15];
  return;
}

void MatrixStack::getFloatvInverse(GLenum pname, GLfloat* params){
 
  glm::mat4 inverse = glm::affineInverse(modelviewStack[modelviewIndex]); //Default
  
  switch (pname) {
    case(MS_MODELVIEW):
      //Default (See above)
    break;
    case(MS_PROJECTION):
      inverse = glm::affineInverse(projectionStack[projectionIndex]);
      break;
    case(MS_TEXTURE) :
      inverse = glm::affineInverse(textureStack[textureIndex]);
      break;
    default:
      logWarning("No valid matrix is active!");
      break;
  }
  
  GLfloat* m = glm::value_ptr(inverse);

  params[0]  = m[0];  params[1]  = m[1];  params[2]  = m[2];  params[3]  = m[3];
  params[4]  = m[4];  params[5]  = m[5];  params[6]  = m[6];  params[7]  = m[7];
  params[8]  = m[8];  params[9]  = m[9];  params[10] = m[10]; params[11] = m[11];
  params[12] = m[12]; params[13] = m[13]; params[14] = m[14]; params[15] = m[15];
  return;
}

void MatrixStack::getFloatvModelviewProjection(GLfloat* params){
  
  glm::mat4 product = projectionStack[projectionIndex] * modelviewStack[modelviewIndex];
  GLfloat* m = glm::value_ptr(product);
  
  params[0]  = m[0];  params[1]  = m[1];  params[2]  = m[2];  params[3]  = m[3];
  params[4]  = m[4];  params[5]  = m[5];  params[6]  = m[6];  params[7]  = m[7];
  params[8]  = m[8];  params[9]  = m[9];  params[10] = m[10]; params[11] = m[11];
  params[12] = m[12]; params[13] = m[13]; params[14] = m[14]; params[15] = m[15];
  return;
}
void MatrixStack::getFloatvModelview(GLfloat* params){
  GLfloat* m = glm::value_ptr(modelviewStack[modelviewIndex]);
  
  params[0]  = m[0];  params[1]  = m[1];  params[2]  = m[2];  params[3]  = m[3];
  params[4]  = m[4];  params[5]  = m[5];  params[6]  = m[6];  params[7]  = m[7];
  params[8]  = m[8];  params[9]  = m[9];  params[10] = m[10]; params[11] = m[11];
  params[12] = m[12]; params[13] = m[13]; params[14] = m[14]; params[15] = m[15];
  return;
}

void MatrixStack::loadIdentity(){
  activeStack()[activeStackIndex()] = glm::mat4(1.0);
}
void MatrixStack::loadZero(){
  GLfloat* target = glm::value_ptr(activeStack()[activeStackIndex()]);
  target[0]  = 0;  target[1]  = 0;  target[2]  = 0;  target[3]  = 0;
  target[4]  = 0;  target[5]  = 0;  target[6]  = 0;  target[7]  = 0;
  target[8]  = 0;  target[9]  = 0;  target[10] = 0;  target[11] = 0;
  target[12] = 0;  target[13] = 0;  target[14] = 0;  target[15] = 0;
}
void MatrixStack::loadMatrixf(const GLfloat *m){
  //activeStack()[activeStackIndex()] = glm::mat4(*m);
  
  GLfloat* target = glm::value_ptr(activeStack()[activeStackIndex()]);
  target[0]  = m[0];  target[1]  = m[1];  target[2]  = m[2];  target[3]  = m[3];
  target[4]  = m[4];  target[5]  = m[5];  target[6]  = m[6];  target[7]  = m[7];
  target[8]  = m[8];  target[9]  = m[9];  target[10] = m[10]; target[11] = m[11];
  target[12] = m[12]; target[13] = m[13]; target[14] = m[14]; target[15] = m[15];
}
void MatrixStack::translatef(GLfloat x, GLfloat y, GLfloat z){
  glm::vec3 v(x,y,z);
  activeStack()[activeStackIndex()] = glm::translate(activeMatrix(), v);
}
void MatrixStack::scalef (GLfloat x, GLfloat y, GLfloat z){
  glm::vec3 v(x,y,z);
  activeStack()[activeStackIndex()] = glm::scale(activeMatrix(), v);
}
void MatrixStack::rotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z){
  
    //Unlike OpenGL, GLM wants the angle in radians.
    //I hope it at least normalizes the rotation vector...
  GLfloat angleR = angle * 3.14159265 / 180.0;

  glm::vec3 v(x,y,z);
  glm::mat4 m = activeStack()[activeStackIndex()];
  activeStack()[activeStackIndex()] = glm::rotate(m, angleR, v);
}
void MatrixStack::multMatrixf (const GLfloat *m){
  glm::mat4 multiplier;
  GLfloat* target = glm::value_ptr(multiplier);
  
  target[0]  = m[0];  target[1]  = m[1];  target[2]  = m[2];  target[3]  = m[3];
  target[4]  = m[4];  target[5]  = m[5];  target[6]  = m[6];  target[7]  = m[7];
  target[8]  = m[8];  target[9]  = m[9];  target[10] = m[10]; target[11] = m[11];
  target[12] = m[12]; target[13] = m[13]; target[14] = m[14]; target[15] = m[15];
  
  activeStack()[activeStackIndex()] = activeMatrix() * multiplier;
}
void MatrixStack::transformVertex (GLfloat &x, GLfloat &y, GLfloat &z){
  glm::vec4 v4(x,y,z, 1.0);
  glm::vec4 Transformed = activeMatrix() *v4;
  
  GLfloat *m = glm::value_ptr(Transformed);
  x = m[0]/m[3];
  y = m[1]/m[3];
  z = m[2]/m[3];
}

void MatrixStack::orthof (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar){
  activeStack()[activeStackIndex()] = glm::ortho(left, right, bottom, top, zNear, zFar);
}
void MatrixStack::frustumf (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar){
  activeStack()[activeStackIndex()] = glm::frustum(left, right, bottom, top, zNear, zFar);
}

void MatrixStack::clipPlanef (int index, const GLfloat *equation){
  glm::vec4 plane(equation[0], equation[1], equation[2], equation[3]);
  glm::mat4 modelviewInverse = glm::affineInverse(modelviewStack[modelviewIndex]);
  glm::vec4 newPlane = plane * modelviewInverse;
  clippingPlanes[index] = newPlane;
}
void MatrixStack::enablePlane (int index){
  planeActivated[index] = 1;
}
void MatrixStack::disablePlane (int index){
  planeActivated[index] = 0;
}
void MatrixStack::getPlanev (int index, GLfloat* params){
  if ( !planeActivated[index] ){
    params[0] = 0; params[1] = 0; params[2] = 0; params[3] = 0;
  } else {
    GLfloat* values = glm::value_ptr(clippingPlanes[index]);
    params[0] = values[0]; params[1] = values[1]; params[2] = values[2]; params[3] = values[3];
  }
}

void MatrixStack::color3f (GLfloat red, GLfloat green, GLfloat blue) {
    color4f(red, green, blue, 1.0);
}

void MatrixStack::color4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
  vertexColor[0] = red;
  vertexColor[1] = green;
  vertexColor[2] = blue;
  vertexColor[3] = alpha;
}

void MatrixStack::fogColor3f (GLfloat red, GLfloat green, GLfloat blue) {
  fogColor[0] = red;
  fogColor[1] = green;
  fogColor[2] = blue;
}

void MatrixStack::fogDensity (GLfloat density) {
  fogColor[3] = density;
}

void MatrixStack::normal3f (GLfloat nx, GLfloat ny, GLfloat nz) {
  //Just fill the entire array with the same data, so it can be used as an attribute in the shader.
  //Shitty mashup of the old of glNormal and glNormalPointer behaviors
  for(int i = 0; i < MAX_NORMAL_ELEMENTS; i+=3) {
    normalArray[i]=nx;
    normalArray[i+1]=ny;
    normalArray[i+2]=nz;
  }
}

GLfloat* MatrixStack::color() {
  return vertexColor;
}

GLfloat* MatrixStack::fog() {
  return fogColor;
}

GLfloat* MatrixStack::normals() {
  return normalArray;
}

MatrixStack* MSI() {
    return MatrixStack::Instance();
}
