//
//  DrawCache.hpp
//  AlephOne
//
//  Created by Dustin Wenz on 3/14/21.
//

#ifndef DrawCache_hpp
#define DrawCache_hpp


#include "OGL_Headers.h"
#include "OGL_Shader.h"

    //Set max number of vertices that fit into the buffers.
#define DRAW_BUFFER_MAX 1000

    //Set number of allowable draw buffers.
    //Each buffer has a unique textureID * shader.
    //Lots of buffers are nice, but at increasing cost. 20 seems like a good number.
#define NUM_DRAW_BUFFERS 20


struct DrawBuffer
{
    GLint textureID;
    bool hasTexture1;
    GLint textureID1; //If texture unit 1 is active, set the ID here.
    Shader *shader;
    int verticesFilled; //How many of these vertices have been filled?
    
        //Enough indices to convert triangle fans into triangles by index, in the worst possible case.
    GLuint indices[DRAW_BUFFER_MAX*3];
    GLuint numIndices; //The number of indices we have. Should be a multiple of three, since we are drawing triangles.
    
    bool landscapeTexture; //Is this a landscape texture?
    
        //This is assumed to be the same for all geometry with the tectureID above.
    GLfloat textureMatrix[16];
    
    //Core attributes
    GLfloat texcoordArray[DRAW_BUFFER_MAX * 2];
    GLfloat vertexArray[DRAW_BUFFER_MAX * 3];
    GLfloat normalArray[DRAW_BUFFER_MAX * 3];
    
        //4-element attributes (formerly uniforms).
        //We need these because OpenGL ES 2.0 does not have have uniform buffers. Someday, we can maybe switch them.
    GLfloat color[DRAW_BUFFER_MAX * 4];
    GLfloat texCoords4[DRAW_BUFFER_MAX * 4];
    GLfloat clipPlane0[DRAW_BUFFER_MAX * 4];
    GLfloat clipPlane1[DRAW_BUFFER_MAX * 4];
    GLfloat clipPlane2[DRAW_BUFFER_MAX * 4];
    GLfloat clipPlane3[DRAW_BUFFER_MAX * 4];
    GLfloat clipPlane4[DRAW_BUFFER_MAX * 4];
    GLfloat clipPlane5[DRAW_BUFFER_MAX * 4];
    GLfloat clipPlane6[DRAW_BUFFER_MAX * 4];
    GLfloat vSxOxSyOy[DRAW_BUFFER_MAX * 4];
    GLfloat vBsBtFlSl[DRAW_BUFFER_MAX * 4];
    GLfloat vPuWoDeGl[DRAW_BUFFER_MAX * 4];
};


class DrawCache{
public:
  static DrawCache* Instance();
        //Private instance variables and methods
    
    void drawSurfaceImmediate(int vertex_count, GLfloat *vertex_array, GLfloat *texcoord_array, GLfloat *tex4); //Draws the surface immediately.
    void drawSurfaceBuffered(int vertex_count, GLfloat *vertex_array, GLfloat *texcoord_array, GLfloat *tex4); //Draws the surface sometime in the future.
    
    void drawAll(); //Draws what's in every buffer, and resets them. Call this before drawing anything that doesn't write to the depth buffer, or when finished drawing the whole scene.
    
        //Given some vertices, returns whether they will be within the screen space rect.
        //The model/view/projection matrices MUST be final for the scene in the MatrixStack for this call to work.
    bool isPolygonOnScreen(int vertex_count, GLfloat *vertex_array);
    
        //Call this with true/false whenever you bind a landscape texture.
        //Landscapes have different wrap modes, and we need to set those when drawing later.
    void cacheLandscapeTextureStatus(bool isLand);
    
        //Cache texture surface attributes for the next draw operation.
    void cacheScaleX(GLfloat v);
    void cacheOffsetX(GLfloat v);
    void cacheScaleY(GLfloat v);
    void cacheOffsetY(GLfloat v);
    void cacheBloomScale(GLfloat v);
    void cacheBloomShift(GLfloat v);
    void cacheFlare(GLfloat v);
    void cacheSelfLuminosity(GLfloat v);
    void cachePulsate(GLfloat v);
    void cacheWobble(GLfloat v);
    void cacheDepth(GLfloat v);
    void cacheGlow(GLfloat v);

    
private:
    DrawCache(){
        //Initialization
    
    };
  
    DrawCache(DrawCache const&){};
    DrawCache& operator=(DrawCache const&){};
    static DrawCache* m_pInstance;
  
    //Private instance variables and methods
    void clearTextureAttributeCaches();
    
        //Guaranteed to return a buffer index for the shader/texID combo big enough to hold vertex_count.
        //This might trigger a draw operation in order to free up a buffer if they are exhausted or full.
        //If texID1 is zero, it is ignored.
    int getBufferFor(Shader* shader, GLuint texID, GLuint texID1, int vertex_count);
    
    void drawAndResetBuffer(int index);
};

DrawCache* DC(); //Convenience instance access


#endif /* DrawCache_hpp */
