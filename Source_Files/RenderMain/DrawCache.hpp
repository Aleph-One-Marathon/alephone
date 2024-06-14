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
#include "scottish_textures.h"


    //Maximum number of dynamic lights per frame
#define LIGHTS_MAX 8000

	//Maximum number of area light vertices
#define AREA_LIGHT_VERTICES_MAX LIGHTS_MAX*6

//Defines the number of each light type that can affect a single surface from tiwthin the shader. The shader MUST make these same size assumptions.
//You can change these, but the shader must be updated to reflect that also.
//Each list can be terminated with a size of zero, to signal the shader to stop processing the list.
#define POINT_LIGHT_DATA_SIZE 7 //Format of point light data: position, color, size (7 elements per light).
#define MAX_POINT_LIGHT_ELEMENTS (64 * POINT_LIGHT_DATA_SIZE) //Max number of elements allowed inthe shader. Must match the assumed value in the shader!
#define SPOT_LIGHT_DATA_SIZE 12 //Format of spot light data: position, direction, innerAngleRadiansCos, outerAngleRadiansCos, color, size (12 elements per light).
#define MAX_SPOT_LIGHT_ELEMENTS (6 * SPOT_LIGHT_DATA_SIZE) //Max number of elements allowed inthe shader. Must match the assumed value in the shader!
#define MAX_AREA_LIGHT_ELEMENTS (4 * 21) //Format of area light data: color, innerAngleRadiansCos, outerAngleRadiansCos, positionXYZ,size, positionXYZ,size, positionXYZ,0. Only last 'size' is zero, indicating eol. (5 + (4*number of vertices elements) per light. Assume approximately 4 per light, but the actual size of these are variable. 21 might be a good guess).

enum //standard way of referencing bounding box elements
{
	BB_HIGH_X,
	BB_LOW_X,
	BB_HIGH_Y,
	BB_LOW_Y,
	BB_HIGH_Z,
	BB_LOW_Z,
	NUM_BB_ELEMENTS
};

	//Structure containing properties that apply to a single surface, composed of one or more triangles.
	//These are generally going to be uniforms, or some other state that applies to the surface
struct geometryProperties {
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
	
	GLfloat AABB[NUM_BB_ELEMENTS]; //Axis-aligned bounding box that contains all vertices.
};

enum //Different light types.
{
	POINT_LIGHT,
	SPOT_LIGHT,
	AREA_LIGHT,
	DIRECTIONAL_LIGHT,
	NUM_LIGHT_TYPES
};

struct dynamicLight {
		//Light properties. Not all are used for each type of light
	int type; 				//Type of light, based on enumeration
	GLfloat color[3]; 		//RGB
	GLfloat position[3]; 	//World space position of light
	GLfloat size;			//Reach of light, in world units. Essentially the radius of the light.
	GLfloat direction[3]; 	//Vector for light direction
	GLfloat innerAngle;		//Angle inside of which light is at max brightness
	GLfloat outerAngle;		//Angle outside of which light is at zero brightness
	
	GLfloat AABB[6]; 			//Axis-aligned bounding box beyond which the light has no effect, in world coordinates. Used to assign lights per surface drawn.
	int areaLightStartIndex; 	//Index int light structure for start of Area Lights.
	int numVertices;			//For area lights, this is the number of vertices used.
	
	bool negative; 				//Is this a negative light.
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
	void addModel(rectangle_definition& RenderRectangle); //Add model data, to be drawn at a later time.
	
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
    bool addPointLight(GLfloat x, GLfloat y, GLfloat z, GLfloat size, GLfloat red, GLfloat green, GLfloat blue, bool negative);
	bool addSpotLight(GLfloat x, GLfloat y, GLfloat z, GLfloat size,  GLfloat dirX, GLfloat dirY, GLfloat dirZ, GLfloat outerAngle, GLfloat innerAngle,  GLfloat red, GLfloat green, GLfloat blue, bool negative);
	bool addAreaLightFan(GLfloat *vertex_array, int vertex_count, GLfloat size,  GLfloat dirX, GLfloat dirY, GLfloat dirZ, GLfloat outerAngle, GLfloat innerAngle,  GLfloat red, GLfloat green, GLfloat blue, bool negative);  //Format: vertices in triangle fan format, number of vertices, size, vector,  inside, outside angles, RGB color, negative.
	void finishGatheringLights();
	
private:
    DrawCache(){
		current_vertex_list_size = 0;
		current_geometry_list_size = 0;
		current_index_list_size = 0;
		geometryFilled = 0;
		indicesFilled = 0;
		verticesFilled = 0;
		gatheringLights = 0;
		
		clearTextureAttributeCaches();
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
	geometryProperties *geometry;

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
	
	//Sets desired lights to the buffers needed for the specified geometry
	void setAttachedLightsForGeometry(int g);
	void clearAttachedLights(); //Zeros out any light attachment buffers
	
	//Returns whether a specific light will possibly affect a specific geometry, based on AABB intersection.
	bool lightAndGeometryIntersect(dynamicLight l, geometryProperties g);
	
	//Sets the cached geometry state values back to default.
	void clearTextureAttributeCaches();
};

DrawCache* DC(); //Convenience instance access


#endif /* DrawCache_hpp */
