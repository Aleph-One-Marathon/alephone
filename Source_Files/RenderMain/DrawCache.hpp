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

//Light modes, to be packed into a single float value. Point, spot and directional, where each can be positive or negative.
//Boundaries: 0-.16 (point light), .16-.33 (negative point light), .33-.50 (spot light), .50-.66 (negative spot light), .66-.83 (directional light), .83-1.0 (negative directional light)
//Because these are floating point values, we need to be a little bit fuzzy. This list is used in the shader.
#define POINT_LIGHT_PACK_MIN 		0.00
#define POINT_LIGHT 				0.08
#define POINT_LIGHT_NEGATIVE		0.25
#define POINT_LIGHT_PACK_MAX 		0.32

#define SPOT_LIGHT_PACK_MIN			0.33
#define SPOT_LIGHT					0.42
#define SPOT_LIGHT_NEGATIVE			0.58
#define SPOT_LIGHT_PACK_MAX			0.66

#define DIRECTIONAL_LIGHT_PACK_MIN	0.67
#define DIRECTIONAL_LIGHT			0.75
#define DIRECTIONAL_LIGHT_NEGATIVE	0.92
#define DIRECTIONAL_LIGHT_PACK_MAX	1.00

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
	
		//RGBA representation of the solid color for this geometry, if needed.
	GLfloat primaryColor[4];
	
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
	void growToFit(int triangleCount, int vertexCount); //Guarantees that all lists will fit specified number triangles and vertices, including one more geometry item. Might trigger a draw.
	
	void captureState(int g); //Captire all volatile state this geometry will need later. Active texture, matrices, cached properties, etc.
	void primeBoundingBox(int g, GLfloat x, GLfloat y, GLfloat z); //Initialize BB for geometry[g] to a point xyz.
	void growBoundingBox(int g, GLfloat x, GLfloat y, GLfloat z); //Expand BB for geometry[g] to include the point xyz.
	
    void addTriangleFan(int vertex_count, GLfloat *vertex_array, GLfloat *texcoord_array, GLfloat *tex4); //Ingest geometry in the form of a triangle fan, to be drawn at a later time.
	void addTriangles(int index_count, GLushort *index_array, GLfloat *vertex_array, GLfloat *texcoord_array, GLfloat *normals, GLfloat *tangents); //Ingest geometry in the form of a GLushort indexed triangles, to be drawn at a later time.
    void drawAll(); //Draws what's in the buffers, and reset. Typically call this when finished caching the whole scene or maybe if the buffers need flushing before reallocation.
        
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
    void addPointLight(GLfloat x, GLfloat y, GLfloat z, GLfloat size, GLfloat red, GLfloat green, GLfloat blue, bool negative);
	void addSpotLight(GLfloat x, GLfloat y, GLfloat z, GLfloat size,  GLfloat dirX, GLfloat dirY, GLfloat dirZ, GLfloat outerAngle, GLfloat innerAngle,  GLfloat red, GLfloat green, GLfloat blue, bool negative);
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
};

DrawCache* DC(); //Convenience instance access


#endif /* DrawCache_hpp */
