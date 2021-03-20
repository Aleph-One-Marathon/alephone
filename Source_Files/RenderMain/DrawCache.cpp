//
//  DrawCache.cpp
//  AlephOne
//
//  Created by Dustin Wenz on 3/14/21.
//

#include "DrawCache.hpp"

#include "OGL_Headers.h"
#include "OGL_Shader.h"
#include "MatrixStack.hpp"




    //Caches for texture attributes as set by the texture manager.
    //These get cleared once drawn or fed into a buffer.
GLfloat scaleX, offsetX, scaleY, offsetY, bloomScale, bloomShift, flare, selfLuminosity, pulsate, wobble, depth, glow;


DrawBuffer drawBuffers[NUM_DRAW_BUFFERS];
DrawBuffer immediateBuffer; //Used to briefly hold attributes for a single geometry and draw call.

GLuint lastActiveTexture;
bool lastTextureIsLandscape;

DrawCache* DrawCache::m_pInstance = NULL;

DrawCache* DrawCache::Instance()
{
  if (!m_pInstance)
    m_pInstance = new DrawCache;
  
    return m_pInstance;
}

DrawCache* DC() {
    return DrawCache::Instance();
}

void DrawCache::drawAll() {
    //printf ("Drawing all buffers\n");
    for(int i = 0; i < NUM_DRAW_BUFFERS; ++i) {
        drawAndResetBuffer(i);
    }
}

int DrawCache::getBufferFor(Shader* shader, GLuint texID, int vertex_count) {

    int firstEmptyBuffer = -1;
    for(int i = 0; i < NUM_DRAW_BUFFERS; ++i) {
        if(drawBuffers[i].verticesFilled == 0 && firstEmptyBuffer < 0 ) {firstEmptyBuffer=i;}
        
        if(drawBuffers[i].shader == shader && drawBuffers[i].textureID == texID) {
            
                //If we convert the fan into triangles, about how many vertices will we need?
            int neededVertices = vertex_count * 3;
            
                //If this buffer is full, draw and reset it, then return the index.
            if (drawBuffers[i].verticesFilled + neededVertices >= DRAW_BUFFER_MAX) {
                drawAndResetBuffer(i);
                printf ("Reset full buffer\n");
            }
            return i;
            
        }
        
    }
    
        //If there are no matching buffers, return the last empty one found.
    if( firstEmptyBuffer >= 0 ) {
        return firstEmptyBuffer;
    }
    
        //If we get here, all buffers are used and we need to flush and return any index.
    //sprintf ("All buffers full.\n");

    drawAll();
    return 0;
}

//Requires 3 GLFloats in vertex_array per vertex, and 2 GLfloats per texcoord
//tex4 is a 4-dimensional array, which is surface normal vector + sign.
//Normalized is assumed to be GL_FALSE and Stride must be 0.
void DrawCache::drawSurfaceImmediate(int vertex_count, GLfloat *vertex_array, GLfloat *texcoord_array, GLfloat *tex4) {

        //The incoming data is a triangle fan: 0,1,2,3,4,5
        //We need to create indices that convert into triangles: 0,1,2, 0,2,3, 0,3,4, 0,3,5
    immediateBuffer.verticesFilled = 0;
    immediateBuffer.numIndices = 0;
    int numTriangles = vertex_count - 2; //The first 3 vertices make a triangle, and each subsequent vertex adds another.
    for(int i = 0; i < numTriangles; ++i) {
        immediateBuffer.indices[immediateBuffer.numIndices] = immediateBuffer.verticesFilled;
        immediateBuffer.indices[immediateBuffer.numIndices + 1] = immediateBuffer.verticesFilled + i + 1;
        immediateBuffer.indices[immediateBuffer.numIndices + 2] = immediateBuffer.verticesFilled + i + 2;
        immediateBuffer.numIndices += 3;
    }
        
    MatrixStack::Instance()->getFloatv(MS_TEXTURE, immediateBuffer.textureMatrix);
    
    GLfloat *color = MSI()->color();
    GLfloat clipPlane0[4], clipPlane1[4], clipPlane5[4];
    MSI()->getPlanev(0, clipPlane0);
    MSI()->getPlanev(1, clipPlane1);
    MSI()->getPlanev(5, clipPlane5);
        
    //Fill the immediate buffers with 4-element components
    for(int i = 0; i < vertex_count * 4; i += 4) {
        immediateBuffer.color[i] = color[0]; immediateBuffer.color[i+1] = color[1]; immediateBuffer.color[i+2] = color[2]; immediateBuffer.color[i+3] = color[3];
        immediateBuffer.texCoords4[i] = tex4[0]; immediateBuffer.texCoords4[i+1] = tex4[1]; immediateBuffer.texCoords4[i+2] = tex4[2]; immediateBuffer.texCoords4[i+3] = tex4[3];
        immediateBuffer.clipPlane0[i] = clipPlane0[0]; immediateBuffer.clipPlane0[i+1] = clipPlane0[1]; immediateBuffer.clipPlane0[i+2] = clipPlane0[2]; immediateBuffer.clipPlane0[i+3] = clipPlane0[3];
        immediateBuffer.clipPlane1[i] = clipPlane1[0]; immediateBuffer.clipPlane1[i+1] = clipPlane1[1]; immediateBuffer.clipPlane1[i+2] = clipPlane1[2]; immediateBuffer.clipPlane1[i+3] = clipPlane1[3];
        immediateBuffer.clipPlane5[i] = clipPlane5[0]; immediateBuffer.clipPlane5[i+1] = clipPlane5[1]; immediateBuffer.clipPlane5[i+2] = clipPlane5[2]; immediateBuffer.clipPlane5[i+3] = clipPlane5[3];
    
        
        immediateBuffer.vSxOxSyOy[i] = scaleX; immediateBuffer.vSxOxSyOy[i+1] = offsetX; immediateBuffer.vSxOxSyOy[i+2] = scaleY; immediateBuffer.vSxOxSyOy[i+3] = offsetY;
        immediateBuffer.vBsBtFlSl[i] = bloomScale; immediateBuffer.vBsBtFlSl[i+1] = bloomShift; immediateBuffer.vBsBtFlSl[i+2] = flare; immediateBuffer.vBsBtFlSl[i+3] = selfLuminosity;
        immediateBuffer.vPuWoDeGl[i] = pulsate; immediateBuffer.vPuWoDeGl[i+1] = wobble; immediateBuffer.vPuWoDeGl[i+2] = depth; immediateBuffer.vPuWoDeGl[i+3] = glow;
    }
        
    clearTextureAttributeCaches();
    
    immediateBuffer.verticesFilled = vertex_count;
        
        //Normally, we'd cache these three in the buffer, but it's not needed for immediate drawing.
    glVertexAttribPointer(Shader::ATTRIB_TEXCOORDS, 2, GL_FLOAT, 0, 0, texcoord_array);
    glEnableVertexAttribArray(Shader::ATTRIB_TEXCOORDS);
    
    glVertexAttribPointer(Shader::ATTRIB_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, vertex_array);
    glEnableVertexAttribArray(Shader::ATTRIB_VERTEX);
    
    glVertexAttribPointer(Shader::ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, MSI()->normals());
    glEnableVertexAttribArray(Shader::ATTRIB_NORMAL);
    
        
    glVertexAttribPointer(Shader::ATTRIB_COLOR, 4, GL_FLOAT, GL_FALSE, 0, immediateBuffer.color);
    glEnableVertexAttribArray(Shader::ATTRIB_COLOR);
    
    glVertexAttribPointer(Shader::ATTRIB_TEXCOORDS4, 4, GL_FLOAT, GL_FALSE, 0, immediateBuffer.texCoords4);
    glEnableVertexAttribArray(Shader::ATTRIB_TEXCOORDS4);
    
    //I think we only need 0, 1, and 5 for normal walls.
    glVertexAttribPointer(Shader::ATTRIB_CLIPPLANE0, 4, GL_FLOAT, GL_FALSE, 0, immediateBuffer.clipPlane0);
    glEnableVertexAttribArray(Shader::ATTRIB_CLIPPLANE0);
    glVertexAttribPointer(Shader::ATTRIB_CLIPPLANE1, 4, GL_FLOAT, GL_FALSE, 0, immediateBuffer.clipPlane1);
    glEnableVertexAttribArray(Shader::ATTRIB_CLIPPLANE1);
    glVertexAttribPointer(Shader::ATTRIB_CLIPPLANE5, 4, GL_FLOAT, GL_FALSE, 0, immediateBuffer.clipPlane5);
    glEnableVertexAttribArray(Shader::ATTRIB_CLIPPLANE5);
        
    glVertexAttribPointer(Shader::ATTRIB_SxOxSyOy, 4, GL_FLOAT, GL_FALSE, 0, immediateBuffer.vSxOxSyOy);
    glEnableVertexAttribArray(Shader::ATTRIB_SxOxSyOy);
    glVertexAttribPointer(Shader::ATTRIB_BsBtFlSl, 4, GL_FLOAT, GL_FALSE, 0, immediateBuffer.vBsBtFlSl);
    glEnableVertexAttribArray(Shader::ATTRIB_BsBtFlSl);
    glVertexAttribPointer(Shader::ATTRIB_PuWoDeGl, 4, GL_FLOAT, GL_FALSE, 0, immediateBuffer.vPuWoDeGl);
    glEnableVertexAttribArray(Shader::ATTRIB_PuWoDeGl);
    
    
    /*Shader* lastShader = lastEnabledShader();
    if (lastShader) {
      lastShader->setVec4(Shader::U_Color, MSI()->color());
      lastShader->setVec4(Shader::U_TexCoords4, tex4);
    }*/
    
    //glDrawArrays(GL_TRIANGLE_FAN, 0, vertex_count);
    glDrawElements(GL_TRIANGLES, immediateBuffer.numIndices, GL_UNSIGNED_INT, immediateBuffer.indices);
}

void DrawCache::drawSurfaceBuffered(int vertex_count, GLfloat *vertex_array, GLfloat *texcoord_array, GLfloat *tex4) {
    
    int b = getBufferFor(lastEnabledShader(), lastActiveTexture, vertex_count);
    
        //Capture volatile state data.
    GLfloat *color = MSI()->color();
    GLfloat clipPlane0[4], clipPlane1[4], clipPlane5[4];
    drawBuffers[b].shader = lastEnabledShader();
    drawBuffers[b].textureID = lastActiveTexture;
    drawBuffers[b].landscapeTexture = lastTextureIsLandscape;
    MSI()->getFloatv(MS_TEXTURE, drawBuffers[b].textureMatrix);
    MSI()->getPlanev(0, clipPlane0);
    MSI()->getPlanev(1, clipPlane1);
    MSI()->getPlanev(5, clipPlane5);
    lastActiveTexture = -1; //We don't need this anymore.
    
    //Transparent surfaces always require a flush
    if(color[3] < 1) {
        drawAll();
    }
    
        //The incoming data is a triangle fan: 0,1,2,3,4,5
        //We need to create indices that convert into triangles: 0,1,2, 0,2,3, 0,3,4, 0,3,5
    int numTriangles = vertex_count - 2; //The first 3 vertices make a triangle, and each subsequent vertex adds another.
    for(int i = 0; i < numTriangles; ++i) {
        drawBuffers[b].indices[drawBuffers[b].numIndices] = drawBuffers[b].verticesFilled;
        drawBuffers[b].indices[drawBuffers[b].numIndices + 1] = drawBuffers[b].verticesFilled + i + 1;
        drawBuffers[b].indices[drawBuffers[b].numIndices + 2] = drawBuffers[b].verticesFilled + i + 2;
        drawBuffers[b].numIndices += 3;
    }
    
    //Fill 2-element components.
    int n = 0;
    for(int i = drawBuffers[b].verticesFilled*2; i < (drawBuffers[b].verticesFilled*2 + (vertex_count * 2)); i += 2) {
        drawBuffers[b].texcoordArray[i] = texcoord_array[n]; drawBuffers[b].texcoordArray[i+1] = texcoord_array[n+1];
        n+=2;
    }
    
    //Fill the 3-element components.
    n = 0;
    GLfloat *normal_array = MSI()->normals();
    for(int i = drawBuffers[b].verticesFilled*3; i < (drawBuffers[b].verticesFilled*3 + (vertex_count * 3)); i += 3) {
        drawBuffers[b].vertexArray[i] = vertex_array[n]; drawBuffers[b].vertexArray[i+1] = vertex_array[n+1]; drawBuffers[b].vertexArray[i+2] = vertex_array[n+2];
        drawBuffers[b].normalArray[i] = normal_array[n]; drawBuffers[b].normalArray[i+1] = normal_array[n+1]; drawBuffers[b].normalArray[i+2] = normal_array[n+2];
        n+=3;
    }
    
    //Fill the 4-element components
    for(int i = drawBuffers[b].verticesFilled*4; i < (drawBuffers[b].verticesFilled*4 + (vertex_count * 4)); i += 4) {
        drawBuffers[b].color[i] = color[0]; drawBuffers[b].color[i+1] = color[1]; drawBuffers[b].color[i+2] = color[2]; drawBuffers[b].color[i+3] = color[3];
        drawBuffers[b].texCoords4[i] = tex4[0]; drawBuffers[b].texCoords4[i+1] = tex4[1]; drawBuffers[b].texCoords4[i+2] = tex4[2]; drawBuffers[b].texCoords4[i+3] = tex4[3];
        drawBuffers[b].clipPlane0[i] = clipPlane0[0]; drawBuffers[b].clipPlane0[i+1] = clipPlane0[1]; drawBuffers[b].clipPlane0[i+2] = clipPlane0[2]; drawBuffers[b].clipPlane0[i+3] = clipPlane0[3];
        drawBuffers[b].clipPlane1[i] = clipPlane1[0]; drawBuffers[b].clipPlane1[i+1] = clipPlane1[1]; drawBuffers[b].clipPlane1[i+2] = clipPlane1[2]; drawBuffers[b].clipPlane1[i+3] = clipPlane1[3];
        drawBuffers[b].clipPlane5[i] = clipPlane5[0]; drawBuffers[b].clipPlane5[i+1] = clipPlane5[1]; drawBuffers[b].clipPlane5[i+2] = clipPlane5[2]; drawBuffers[b].clipPlane5[i+3] = clipPlane5[3];
    
        
        drawBuffers[b].vSxOxSyOy[i] = scaleX; drawBuffers[b].vSxOxSyOy[i+1] = offsetX; drawBuffers[b].vSxOxSyOy[i+2] = scaleY; drawBuffers[b].vSxOxSyOy[i+3] = offsetY;
        drawBuffers[b].vBsBtFlSl[i] = bloomScale; drawBuffers[b].vBsBtFlSl[i+1] = bloomShift; drawBuffers[b].vBsBtFlSl[i+2] = flare; drawBuffers[b].vBsBtFlSl[i+3] = selfLuminosity;
        drawBuffers[b].vPuWoDeGl[i] = pulsate; drawBuffers[b].vPuWoDeGl[i+1] = wobble; drawBuffers[b].vPuWoDeGl[i+2] = depth; drawBuffers[b].vPuWoDeGl[i+3] = glow;
    }
    //printf("Added vertices %i to %i with texture %i\n", vertex_count, drawBuffers[b].verticesFilled, drawBuffers[b].textureID);
    clearTextureAttributeCaches();
    drawBuffers[b].verticesFilled += vertex_count;
    
    //For debugging, it helps to draw right away. Much slower, though.
    //Normnally this should be commented out.
    //drawAndResetBuffer(b);
}


void DrawCache::drawAndResetBuffer(int index) {

    if (drawBuffers[index].shader == NULL || drawBuffers[index].verticesFilled == 0 || drawBuffers[index].numIndices == 0) {
        return;
    }
    //printf("Drawing buffer of size %i\n", drawBuffers[index].verticesFilled );
    Shader *originalShader = lastEnabledShader();
    
    //drawBuffers[index].shader->enable();
    drawBuffers[index].shader->enableAndSetStandardUniforms();
    drawBuffers[index].shader->setMatrix4(Shader::U_TextureMatrix, drawBuffers[index].textureMatrix);
    glBindTexture(GL_TEXTURE_2D, drawBuffers[index].textureID);
    
    if(drawBuffers[index].landscapeTexture) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //DCW added for landscape. Repeat horizontally
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT); //DCW added for landscape. Mirror vertically.

    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //DCW this is probably better for non-landscapes
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //DCW this is probably better for non-landscapes
    }

    glVertexAttribPointer(Shader::ATTRIB_TEXCOORDS, 2, GL_FLOAT, 0, 0, drawBuffers[index].texcoordArray);
    glEnableVertexAttribArray(Shader::ATTRIB_TEXCOORDS);
    
    glVertexAttribPointer(Shader::ATTRIB_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, drawBuffers[index].vertexArray);
    glEnableVertexAttribArray(Shader::ATTRIB_VERTEX);
    
    glVertexAttribPointer(Shader::ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, drawBuffers[index].normalArray);
    glEnableVertexAttribArray(Shader::ATTRIB_NORMAL);
    
        
    glVertexAttribPointer(Shader::ATTRIB_COLOR, 4, GL_FLOAT, GL_FALSE, 0, drawBuffers[index].color);
    glEnableVertexAttribArray(Shader::ATTRIB_COLOR);
    
    glVertexAttribPointer(Shader::ATTRIB_TEXCOORDS4, 4, GL_FLOAT, GL_FALSE, 0, drawBuffers[index].texCoords4);
    glEnableVertexAttribArray(Shader::ATTRIB_TEXCOORDS4);
    
    //I think we only need 0, 1, and 5 for normal walls.
    glVertexAttribPointer(Shader::ATTRIB_CLIPPLANE0, 4, GL_FLOAT, GL_FALSE, 0, drawBuffers[index].clipPlane0);
    glEnableVertexAttribArray(Shader::ATTRIB_CLIPPLANE0);
    glVertexAttribPointer(Shader::ATTRIB_CLIPPLANE1, 4, GL_FLOAT, GL_FALSE, 0, drawBuffers[index].clipPlane1);
    glEnableVertexAttribArray(Shader::ATTRIB_CLIPPLANE1);
    glVertexAttribPointer(Shader::ATTRIB_CLIPPLANE5, 4, GL_FLOAT, GL_FALSE, 0, drawBuffers[index].clipPlane5);
    glEnableVertexAttribArray(Shader::ATTRIB_CLIPPLANE5);
        
    glVertexAttribPointer(Shader::ATTRIB_SxOxSyOy, 4, GL_FLOAT, GL_FALSE, 0, drawBuffers[index].vSxOxSyOy);
    glEnableVertexAttribArray(Shader::ATTRIB_SxOxSyOy);
    glVertexAttribPointer(Shader::ATTRIB_BsBtFlSl, 4, GL_FLOAT, GL_FALSE, 0, drawBuffers[index].vBsBtFlSl);
    glEnableVertexAttribArray(Shader::ATTRIB_BsBtFlSl);
    glVertexAttribPointer(Shader::ATTRIB_PuWoDeGl, 4, GL_FLOAT, GL_FALSE, 0, drawBuffers[index].vPuWoDeGl);
    glEnableVertexAttribArray(Shader::ATTRIB_PuWoDeGl);
    
    glEnable(GL_BLEND); //We might always want to blend.
    
    glDrawElements(GL_TRIANGLES, drawBuffers[index].numIndices, GL_UNSIGNED_INT, drawBuffers[index].indices);
    
        //Reset what we care about.
    drawBuffers[index].verticesFilled = 0;
    drawBuffers[index].numIndices = 0;
    drawBuffers[index].shader = NULL;
    drawBuffers[index].textureID = 0;
    
    if(originalShader) {
        originalShader->enable(); //We need to restore whatever shader was active, so we don't pollute outside state.
    }
}

void DrawCache::cacheActiveTextureID(GLuint texID) {lastActiveTexture = texID;}
void DrawCache::cacheLandscapeTextureStatus(bool isLand) {lastTextureIsLandscape = isLand;}

void DrawCache::cacheScaleX(GLfloat v) {scaleX = v;}
void DrawCache::cacheOffsetX(GLfloat v) {offsetX = v;}
void DrawCache::cacheScaleY(GLfloat v) {scaleY = v;}
void DrawCache::cacheOffsetY(GLfloat v) {offsetY = v;}
void DrawCache::cacheBloomScale(GLfloat v) {bloomScale = v;}
void DrawCache::cacheBloomShift(GLfloat v) {bloomShift = v;}
void DrawCache::cacheFlare(GLfloat v) {flare = v;}
void DrawCache::cacheSelfLuminosity(GLfloat v) {selfLuminosity = v;}
void DrawCache::cachePulsate(GLfloat v) {pulsate = v;}
void DrawCache::cacheWobble(GLfloat v) {wobble = v;}
void DrawCache::cacheDepth(GLfloat v) {depth = v;}
void DrawCache::cacheGlow(GLfloat v) {glow = v;}

void DrawCache::clearTextureAttributeCaches() {
    scaleX = 0;
    offsetX = 0;
    scaleY = 0;
    offsetY = 0;
    bloomScale = 0;
    bloomShift = 0;
    flare = 0;
    selfLuminosity = 0;
    pulsate = 0;
    wobble = 0;
    depth = 0;
    glow = 0;
}
