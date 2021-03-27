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
#include "screen.h"

#include "map.h"
#include "projectiles.h"


    //Caches for texture attributes as set by the texture manager.
    //These get cleared once drawn or fed into a buffer.
GLfloat scaleX, offsetX, scaleY, offsetY, bloomScale, bloomShift, flare, selfLuminosity, pulsate, wobble, depth, glow;


DrawBuffer drawBuffers[NUM_DRAW_BUFFERS];
DrawBuffer immediateBuffer; //Used to briefly hold attributes for a single geometry and draw call.

int numLightsInScene;
GLfloat lightPositions[LIGHTS_MAX * 4]; //Format: x, y, z (location), w (size in world units)
GLfloat lightColors[LIGHTS_MAX * 4]; //Format: r, g, b, intensity?
    
    //The actual data fed into the shader.
    //Same format as above, but position list must be terminated with a {0,0,0,0} light.
GLfloat activeLightPositions[(ACTIVE_LIGHTS_MAX+1) * 4];
GLfloat activeLightColors[ACTIVE_LIGHTS_MAX * 4];

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

void DrawCache::startGatheringLights() {
    numLightsInScene = 0;
    gatheringLights = 1;
}

void DrawCache::addLight(GLfloat x, GLfloat y, GLfloat z, GLfloat size, GLfloat red, GLfloat green, GLfloat blue, GLfloat intensity ) {
    if(!gatheringLights) return;
    
    if(numLightsInScene < LIGHTS_MAX) {
        lightPositions[numLightsInScene*4 + 0] = x;
        lightPositions[numLightsInScene*4 + 1] = y;
        lightPositions[numLightsInScene*4 + 2] = z;
        lightPositions[numLightsInScene*4 + 3] = size; //Size in world units. 0 means no light. 1000ish would be typical
        
        lightColors[numLightsInScene*4 + 0] = red; //Red
        lightColors[numLightsInScene*4 + 1] = green; //Green
        lightColors[numLightsInScene*4 + 2] = blue; //Blue

        lightColors[numLightsInScene*4 + 3] = intensity; //Intensity
        numLightsInScene++;
    }
}

void DrawCache::finishGatheringLights() {
    gatheringLights = 0;
}

//DEPRECATED
void DrawCache::gatherLights() {
    numLightsInScene = 0;
    
    //Iterate through object list to find anything that is glowing.
    for( projectile_data aProjectile : ProjectileList ) {
  
        world_point3d location = ObjectList[aProjectile.object_index].location;
        
        shape_descriptor shape = ObjectList[aProjectile.object_index].shape;
        
        //short TransferMode = 0;
        //short CollColor = GET_DESCRIPTOR_COLLECTION(shape);
        //short Collection = GET_COLLECTION(CollColor);

        //if ()
        {
            
            /*short CTable = GET_COLLECTION_CLUT(CollColor);
            TxtrStatePtr = &CBTS.CTStates[CTable];
            TextureState &CTState = *TxtrStatePtr;

            if(CTState.IsGlowing == TRUE) {
                
            }*/
        
            if(numLightsInScene < LIGHTS_MAX) {
                lightPositions[numLightsInScene*4 + 0] = location.x;
                lightPositions[numLightsInScene*4 + 1] = location.y;
                lightPositions[numLightsInScene*4 + 2] = location.z;
                lightPositions[numLightsInScene*4 + 3] = 2000; //Size in world units. 0 means no light.
                
                //if(GET_OBJECT_STATUS(&anObject) == _object_is_effect) {
                    lightColors[numLightsInScene*4 + 0] = 1.0; //Red
                    lightColors[numLightsInScene*4 + 1] = .7; //Green
                    lightColors[numLightsInScene*4 + 2] = 0; //Blue
               // } else {
                //    lightColors[numLightsInScene*4 + 0] = 0.0; //Red
                //    lightColors[numLightsInScene*4 + 1] = 1.0; //Green
                 //   lightColors[numLightsInScene*4 + 2] = 0.0; //Blue

                //}
                lightColors[numLightsInScene*4 + 3] = 1.0; //Intensity
                numLightsInScene++;
            }
        }
    }
    
}

bool DrawCache::isPolygonOnScreen(int vertex_count, GLfloat *vertex_array) {
    if(vertex_count < 1) {return 0;}
    
    GLfloat vertexOnScreen[3] = {vertex_array[0], vertex_array[1], vertex_array[2]};
    MatrixStack::Instance()->transformVertex(vertexOnScreen[0], vertexOnScreen[1], vertexOnScreen[2]);
    
    float xOnScreen = vertexOnScreen[0];
    float yOnScreen = vertexOnScreen[1];

    float left_x = xOnScreen;
    float right_x = xOnScreen;
    float top_y = yOnScreen;
    float bottom_y = yOnScreen;
    
        //Build out a bounding box in screen coordinates that contains all of the vertices.
    for (int i = 1; i < vertex_count; ++i) {
        vertexOnScreen[0] = vertex_array[i*3 + 0];
        vertexOnScreen[1] = vertex_array[i*3 + 1];
        vertexOnScreen[2] = vertex_array[i*3 + 2];
        MatrixStack::Instance()->transformVertex(vertexOnScreen[0], vertexOnScreen[1], vertexOnScreen[2]);
        
        xOnScreen = vertexOnScreen[0];
        yOnScreen = vertexOnScreen[1];
        
        if( xOnScreen < left_x) { left_x = xOnScreen; }
        if( xOnScreen > right_x) { right_x = xOnScreen; }
        if( yOnScreen < bottom_y) { bottom_y = yOnScreen; }
        if( yOnScreen > top_y) { top_y = yOnScreen; }
    }
    
    //Convert to normalized device coordinates
    right_x /= MainScreenPixelWidth()/2;
    left_x /= MainScreenPixelWidth()/2;
    top_y /= MainScreenPixelHeight()/2;
    bottom_y /= MainScreenPixelHeight()/2;
    
    //TODO: This function won't work until we figure out what NDC looks like for non-3d perspective.
    
    
        //Is this centered on screen?
    /*if(left_x < 0 && right_x > 0 && bottom_y < 0 && top_y > 0) {
        printf("Centered l %f, r %f, t %f, b %f\n", left_x, right_x, top_y, bottom_y);
    }*/
    

    
}

int DrawCache::getBufferFor(Shader* shader, GLuint texID, GLuint texID1, int vertex_count) {

    int firstEmptyBuffer = -1;
    for(int i = 0; i < NUM_DRAW_BUFFERS; ++i) {
        if(drawBuffers[i].verticesFilled == 0 && firstEmptyBuffer < 0 ) {firstEmptyBuffer=i;}
        
        if(drawBuffers[i].shader == shader && drawBuffers[i].textureID == texID && (texID1 == 0 || texID1 == drawBuffers[i].textureID1)) {
            
                //If we convert the fan into triangles, about how many vertices will we need?
            int neededVertices = vertex_count * 3;
            
                //If this buffer is full, draw and reset it, then return the index.
            if (drawBuffers[i].verticesFilled + neededVertices >= DRAW_BUFFER_MAX) {
                drawAndResetBuffer(i);
                //printf ("Reset full buffer\n");
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

//THIS IS DEPRECATED
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
    
    GLint whichUnit, whichTextureID, whichTextureID1;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &whichUnit); //Store active texture so we can reset it later.
    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &whichTextureID);
    glActiveTexture(GL_TEXTURE1);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &whichTextureID1);
    glActiveTexture(whichUnit);
    
    
    //int b = getBufferFor(lastEnabledShader(), lastActiveTexture, vertex_count);
    int b = getBufferFor(lastEnabledShader(), whichTextureID, whichTextureID1, vertex_count);
    
        //Capture volatile state data.
    GLfloat *color = MSI()->color();
    GLfloat clipPlane0[4], clipPlane1[4], clipPlane5[4];
    drawBuffers[b].shader = lastEnabledShader();
    //drawBuffers[b].textureID = lastActiveTexture;
    
    
    drawBuffers[b].textureID = whichTextureID; //There should always be a texture0
    drawBuffers[b].textureID1 = whichTextureID1;
    if(whichTextureID1) {
        drawBuffers[b].hasTexture1 = 1;
    }
    
    drawBuffers[b].landscapeTexture = lastTextureIsLandscape;
    MSI()->getFloatv(MS_TEXTURE, drawBuffers[b].textureMatrix);
    MSI()->getPlanev(0, clipPlane0);
    MSI()->getPlanev(1, clipPlane1);
    MSI()->getPlanev(5, clipPlane5);
    
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
    
        //Prime BB with the first vertex.
    if(drawBuffers[b].verticesFilled == 0) {
        drawBuffers[b].bb_high_x=vertex_array[0];
        drawBuffers[b].bb_low_x=vertex_array[0];
        drawBuffers[b].bb_high_y=vertex_array[1];
        drawBuffers[b].bb_low_y=vertex_array[1];
        drawBuffers[b].bb_high_z=vertex_array[2];
        drawBuffers[b].bb_low_z=vertex_array[2];

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
        
        //Grow bounding box
        if(vertex_array[n] >= drawBuffers[b].bb_high_x) drawBuffers[b].bb_high_x = vertex_array[n];
        if(vertex_array[n] <= drawBuffers[b].bb_low_x) drawBuffers[b].bb_low_x = vertex_array[n];
        if(vertex_array[n+1] >= drawBuffers[b].bb_high_y) drawBuffers[b].bb_high_y = vertex_array[n+1];
        if(vertex_array[n+1] <= drawBuffers[b].bb_low_y) drawBuffers[b].bb_low_y = vertex_array[n+1];
        if(vertex_array[n+2] >= drawBuffers[b].bb_high_z) drawBuffers[b].bb_high_z = vertex_array[n+2];
        if(vertex_array[n+2] <= drawBuffers[b].bb_low_z) drawBuffers[b].bb_low_z = vertex_array[n+2];
        
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

    
    GLint whichUnit;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &whichUnit);
    
    if( drawBuffers[index].hasTexture1 ) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, drawBuffers[index].textureID1);
    } else {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, drawBuffers[index].textureID);
    
    glActiveTexture(whichUnit);
    
    
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
    
    //Attach Lights
    bool usedDynamicLight = 0;
    bool didFirstDraw = 0; //Have we drawn any geometry with lights yet?
    int lightsAttached = 0;
    int lightsChecked = 0;
    GLfloat x,y,z,size, red,green,blue,intensity;
    if (!gatheringLights && !drawBuffers[index].landscapeTexture) {
        for(int i = 0; i < numLightsInScene; i++) {
            x = lightPositions[i*4];
            y = lightPositions[i*4 + 1];
            z = lightPositions[i*4 + 2];
            size = lightPositions[i*4 + 3];
            red = lightColors[i*4];
            green = lightColors[i*4 + 1];
            blue = lightColors[i*4 + 2];
            intensity = lightColors[i*4 + 3];
            
                //Is the light inside the bounding box (plus the light size)?
            if(x >= (drawBuffers[index].bb_low_x-size) &&
               x <= (drawBuffers[index].bb_high_x+size) &&
               y >= (drawBuffers[index].bb_low_y-size) &&
               y <= (drawBuffers[index].bb_high_y+size) &&
               z >= (drawBuffers[index].bb_low_z-size) &&
               z <= (drawBuffers[index].bb_high_z+size) ) {
            
                //The vertex needs to be in eyespace
                MSI()->transformVertexToEyespace(x, y, z);
                
                //We can only attach up to four lights, until we need to do another pass with additive only.
                if(lightsAttached < ACTIVE_LIGHTS_MAX){
                    activeLightPositions[lightsAttached*4] = x;
                    activeLightPositions[lightsAttached*4 +1] = y;
                    activeLightPositions[lightsAttached*4 +2] = z;
                    activeLightPositions[lightsAttached*4 +3] = size;

                    activeLightColors[lightsAttached*4] = red;
                    activeLightColors[lightsAttached*4 +1] = green;
                    activeLightColors[lightsAttached*4 +2] = blue;
                    activeLightColors[lightsAttached*4 +3] = intensity;
                    
                    lightsAttached++;
                }
                
            }
            
        }
        
        //Terminate active light list.
        activeLightPositions[lightsAttached*4] = 0;
        activeLightPositions[lightsAttached*4 +1] = 0;
        activeLightPositions[lightsAttached*4 +2] = 0;
        activeLightPositions[lightsAttached*4 +3] = 0;
    }
    
    
    
    if(lightsAttached > 1) {
        lightsAttached = lightsAttached;
    }
    
    drawBuffers[index].shader->setVec4v(Shader::U_LightPositions, lightsAttached + 1, activeLightPositions);
    drawBuffers[index].shader->setVec4v(Shader::U_LightColors, lightsAttached, activeLightColors);

    glDrawElements(GL_TRIANGLES, drawBuffers[index].numIndices, GL_UNSIGNED_INT, drawBuffers[index].indices);
    
    //Reset lights in the shader so later draws don't see them accidentially.
    lightsAttached = 0;
    for(int n = 0; n < 4; n++) {
        activeLightPositions[n]=0;
        activeLightColors[n]=0;
    }
    drawBuffers[index].shader->setVec4v(Shader::U_LightPositions, 1, activeLightPositions);
    drawBuffers[index].shader->setVec4v(Shader::U_LightColors, 1, activeLightColors);


        //Reset what we care about.
    drawBuffers[index].verticesFilled = 0;
    drawBuffers[index].numIndices = 0;
    drawBuffers[index].shader = NULL;
    drawBuffers[index].textureID = 0;
    drawBuffers[index].hasTexture1 = 0;
    drawBuffers[index].textureID1 = 0;
    
    if(originalShader) {
        originalShader->enable(); //We need to restore whatever shader was active, so we don't pollute outside state.
    }
}

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
