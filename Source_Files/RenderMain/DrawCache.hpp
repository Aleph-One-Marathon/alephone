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


    //Maximum number of dynamic lights per frame
#define LIGHTS_MAX 8000

    //Maximum number of lights allowed in a single draw call
    //Each light requires 8 uniform slots. Don't over do it!
    //This number also is a hard-coded cap in the shader to help with the unroller; if you increase this, increase it in the shaders too.
#define ACTIVE_LIGHTS_MAX 32

	//Structure containing properties that apply to a single surface, composed of one or more triangles.
	//These are generally going to be uniforms, or some other state that applies to the surface
struct GeometryProperties {
	GLuint numIndices; //The number of indices we have in this object. Should be a multiple of three, since we are drawing triangles.
	
	GLint textureID0; //Should be set to a valid texture name (non-zero)
	GLint textureID1; //If not zero, will be a texture name for this unit.
	
	GLfloat modelMatrix[16], projectionMatrix[16], modelProjection[16], modelMatrixInverse[16], textureMatrix[16];
	
	Shader *shader;
	
	bool landscapeTexture; //Is this a landscape texture?
	GLboolean isBlended; //Is this surface blended?
	int depthFunction;
	int depthTest;
	
		//Uniform properties for this surface
	GLfloat clipPlane0[4];
	GLfloat clipPlane1[4];
	GLfloat clipPlane2[4];
	GLfloat clipPlane3[4];
	GLfloat clipPlane4[4];
	GLfloat clipPlane5[4];
	GLfloat clipPlane6[4];
	
	//Feature values
	GLfloat scaleX;
	GLfloat offsetX;
	GLfloat scaleY;
	GLfloat offsetY;
	GLfloat bloomScale;
	GLfloat bloomShift;
	GLfloat flare;
	GLfloat selfLuminosity;
	GLfloat pulsate;
	GLfloat wobble;
	GLfloat depth;
	GLfloat glow;
	
	GLfloat bb_high_x, bb_low_x, bb_high_y, bb_low_y, bb_high_z, bb_low_z; //Axis-aligned bounding box that contains all vertices.
};


class DrawCache{
public:
  static DrawCache* Instance();
        //Private instance variables and methods
	
		//Grows the array lists, but first draws if they contain anything.
	void growGeometryList();
	void growIndexList();
	void growVertexLists();
	
    void addGeometry(int vertex_count, GLfloat *vertex_array, GLfloat *texcoord_array, GLfloat *tex4); //Ingest geometry, to be drawn at a later time.
    void drawAll(); //Draws what's in the buffers, and reset. Typically call this when finished caching the whole scene.
        
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
	void cacheStrictDepthMode(GLfloat v);
    
    void startGatheringLights();
    void addDefaultLight(GLfloat x, GLfloat y, GLfloat z, short objectType, short permutationType); //Only works for effects and projectiles ATM.
    void addLight(GLfloat x, GLfloat y, GLfloat z, GLfloat size, GLfloat red, GLfloat green, GLfloat blue, GLfloat intensity );
    void finishGatheringLights();
    
private:
    DrawCache(){
		current_vertex_list_size = 0;
		current_geometry_list_size = 0;
    };
  
    DrawCache(DrawCache const&) = delete;
    DrawCache& operator=(DrawCache const&) = delete;
    static DrawCache* m_pInstance;
  
    //Private instance variables and methods
    
    bool gatheringLights;
    
		//List sizes (raw capacity; they might not be filled).
	int current_geometry_list_size;
	int current_index_list_size;
	int current_vertex_list_size; //Gets later multiplied by size of vertex element (usually 2, 3, or 4) as needed.
	
	int geometryFilled; //How many surfaces have been added for this scene
	int indicesFilled; //How many indices have been filled for this scene
	int verticesFilled; //How many vertices have been filled for this scene

	//Each unique surface has a single set of geometry properties.
	GeometryProperties *geometry;

	//Indexes that represent triangles that point to somewhere in the buffer arrays. There are typically many more of these than unique vertices.
	GLuint *indices; //Holds three indices per triangle.
	
	//Core buffers
	GLfloat *texcoordArray;
	GLfloat *vertexArray;
	GLfloat *normalArray;
	GLfloat *colors;
	GLfloat *texCoords4;
	
	//Caches for texture attributes as set by the texture manager.
	//These get cleared once drawn or fed into a buffer.
	GLfloat scaleX, offsetX, scaleY, offsetY, bloomScale, bloomShift, flare, selfLuminosity, pulsate, wobble, depth, glow;
	GLfloat strictDepthMode;

    void clearTextureAttributeCaches();
    
        //Guaranteed to return a buffer index for the shader/texID combo big enough to hold vertex_count.
        //This might trigger a draw operation in order to free up a buffer if they are exhausted or full.
        //If texID1 is zero, it is ignored.
    int getBufferFor(Shader* shader, GLuint texID, GLuint texID1, int vertex_count, bool isBlended);
    
    void drawAndResetBuffer(int index);
};

DrawCache* DC(); //Convenience instance access


#endif /* DrawCache_hpp */
