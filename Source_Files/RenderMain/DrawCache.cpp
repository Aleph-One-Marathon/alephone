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
#include "cseries.h"
#include "screen.h"
#include "OGL_Model_Def.h"

#include "map.h"
#include "projectiles.h"
#include "effects.h"

#include "Logging.h"

bool lastTextureIsLandscape;

	//Main list of dynamic lights
	//Some lights (like spot lights) take two slots, so actual number of lights may be as few as half of this.
int numLightsInScene;
dynamicLight dynamicLights[LIGHTS_MAX];
int	lightAreaIndices[(LIGHTS_MAX+1) * 4]; //Sparse array holding indices to the start of geometry in the lightAreas buffer.
int nextAvailableLightAreaIndex;	//Index of the next available area light slot.

	//The actual data fed into the shader. See format for these in the header.
GLfloat activePointLights[MAX_POINT_LIGHT_ELEMENTS];
GLfloat activeSpotLights[MAX_SPOT_LIGHT_ELEMENTS];
GLfloat activeAreaLights[MAX_AREA_LIGHT_ELEMENTS];

enum //One ENUM for each buffer for easy reference.
{
	TEXCOORDARRAY_VBO,
	VERTEXARRAY_VBO,
	NORMALARRAY_VBO,
	COLORS_VBO,
	TEXCOORDS4_VBO,
	NUM_VBOS
};
GLuint vboIDs[NUM_VBOS]; //One buffer ID for each buffer. Can't use one for indices because we need to make extensive use of offsets, which I'm not sure ES 2.0 can do.

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

void DrawCache::growGeometryList() {
	if(current_geometry_list_size == 0) {
		//There can be visual glitches (that only happen for one frame per game launch) if these ever grow during rendering, so start them high-ish.
		//This is due to the instant draw/flush operation, which changes the active texture (and other state) from what the geometry being cached should have.
		//That issue is probably fixable if we want, by caching before the flush operations.
		current_geometry_list_size = 65536;
	} else {
		logWarning("Growing geometry list from %d to %d. A brief visual glitch is possible.\n", current_geometry_list_size, current_geometry_list_size*2);
		drawAll();
		free(geometry);
		current_geometry_list_size *= 2;
	}
	
	geometry = (geometryProperties*)malloc(sizeof(geometryProperties) * current_geometry_list_size);
	geometryFilled = 0;
}

void DrawCache::growIndexList() {
	if(current_index_list_size == 0) {
		current_index_list_size = 131072;
	} else {
		logWarning("Growing index list from %d to %d. A brief visual glitch is possible.\n", current_index_list_size, current_index_list_size*2);
		drawAll();
		free(indices);
		current_index_list_size *= 2;
	}
	
	indices = (GLuint*)malloc(sizeof(GLuint) * current_index_list_size);
	indicesFilled = 0;
}

void DrawCache::growVertexLists() {
	if(current_vertex_list_size == 0) {
		current_vertex_list_size = 65536;
	} else {
		logWarning("Growing vertex list from %d to %d. A brief visual glitch is possible.\n", current_vertex_list_size, current_vertex_list_size*2);
		drawAll();
		free(vertexArray);
		free(texcoordArray);
		free(normalArray);
		free(colors);
		free(texCoords4);
		current_vertex_list_size *= 2;
	}
	
	vertexArray = (GLfloat*)malloc(sizeof(GLfloat) * current_vertex_list_size * 3);
	texcoordArray = (GLfloat*)malloc(sizeof(GLfloat) * current_vertex_list_size * 2);
	normalArray = (GLfloat*)malloc(sizeof(GLfloat) * current_vertex_list_size * 3);
	colors = (GLfloat*)malloc(sizeof(GLfloat) * current_vertex_list_size * 4);
	texCoords4 = (GLfloat*)malloc(sizeof(GLfloat) * current_vertex_list_size * 4);

	verticesFilled = 0;
}

void DrawCache::startGatheringLights() {
    numLightsInScene = 0;
	nextAvailableLightAreaIndex = 0;
    gatheringLights = 1;
}

void DrawCache::addDefaultLight(GLfloat x, GLfloat y, GLfloat z, short objectType, short permutationType) {
	
    if (objectType == _object_is_projectile)
    {
       switch (permutationType)
        {
                case _projectile_grenade:
                case _projectile_trooper_grenade:
                case _projectile_flamethrower_burst:
                case _projectile_alien_weapon:
                case _projectile_lava_yeti:
                    addPointLight(x, y, z, 1000, 1, .8, 0, 0 );
                    break;

                case _projectile_minor_defender:
                case _projectile_major_defender:
                case _projectile_minor_hummer:
                case _projectile_major_hummer:
                case _projectile_durandal_hummer:
					addPointLight(x, y, z, 1000, 0, 1, .1, 0 );
                    break;

                case _projectile_rocket:
                case _projectile_juggernaut_rocket:
                case _projectile_juggernaut_missile:
					addPointLight(x, y, z, 4000, 1, 1, .7, 0 );
                    break;
                    
                case _projectile_staff:
                case _projectile_staff_bolt:
					addPointLight(x, y, z, 1000, .5, 1, rand() / double(RAND_MAX), 0 );
                    break;
                
                case _projectile_minor_cyborg_ball:
                case _projectile_major_cyborg_ball:
                case _projectile_compiler_bolt_minor:
                case _projectile_compiler_bolt_major:
                case _projectile_hunter:
                case _projectile_armageddon_sphere:
                case _projectile_armageddon_electricity:
                    addPointLight(x, y, z, 2000, 0, 1, 1, 0 );
                    break;
                
                case _projectile_fusion_bolt_minor:
                    addPointLight(x, y, z, 2000, .8, .7, 1, 0 );
                    break;
                
                case _projectile_minor_fusion_dispersal:
                case _projectile_major_fusion_dispersal:
                case _projectile_overloaded_fusion_dispersal:
                case _projectile_fusion_bolt_major:
                    addPointLight(x, y, z, 3000, .8, rand() / double(RAND_MAX), 1, 0 );
                    break;
            
                default:
                    break;
            }
    } else if(objectType == _object_is_effect) {
            switch (permutationType)
            {
                case _effect_rocket_explosion:
                case _effect_grenade_explosion:
                    addPointLight(x, y, z, 2000, 1, .9, 0, 0 );
                    break;
                    
                case _effect_alien_lamp_breaking:
                case _effect_water_lamp_breaking:
                case _effect_lava_lamp_breaking:
                case _effect_sewage_lamp_breaking:
                case _effect_rocket_contrail:
                case _effect_grenade_contrail:
                case _effect_juggernaut_missile_contrail:
                    addPointLight(x, y, z, 1000, .8, .8, .8, 0 );
                    break;

                case _effect_alien_weapon_ricochet:
                case _effect_flamethrower_burst:
                    addPointLight(x, y, z, 1000, .8, .7, 0, 0 );
                    break;

                case _effect_compiler_bolt_minor_detonation:
                case _effect_compiler_bolt_major_detonation:
                case _effect_compiler_bolt_major_contrail:
                    addPointLight(x, y, z, 1000, 0, .7, .7, 0 );
                    break;

                case _effect_hunter_projectile_detonation:
                    addPointLight(x, y, z, 1000, 0, 1, .8, 0 );
                    break;

                case _effect_minor_fusion_detonation:
                case _effect_major_fusion_detonation:
                    addPointLight(x, y, z, 2000, 1, 1, 1, 0 );
                    break;

                case _effect_major_fusion_contrail:
                    addPointLight(x, y, z, 500, .7, .8, 1, 0 );
                    break;

                case _effect_minor_defender_detonation:
                case _effect_major_defender_detonation:
                    addPointLight(x, y, z, 1000, .5, .5, .5, 0 );
                    break;


                case _effect_minor_hummer_projectile_detonation:
                case _effect_major_hummer_projectile_detonation:
                case _effect_durandal_hummer_projectile_detonation:
                    addPointLight(x, y, z, 2000, 0, 1, .1, 0 );
                    break;

                case _effect_cyborg_projectile_detonation:
                    addPointLight(x, y, z, 2000, .1, .8, 1, 0 );
                    break;

                case _effect_minor_fusion_dispersal:
                case _effect_major_fusion_dispersal:
                case _effect_overloaded_fusion_dispersal:
                    addPointLight(x, y, z, 4000, .8, 1, 1, 0 );
                    break;

                case _effect_lava_yeti_projectile_detonation:
                    addPointLight(x, y, z, 2000, 1, 0, 0, 0 );
                    break;

                default:
                    break;
            }
        }
    
}

bool DrawCache::addPointLight(GLfloat x, GLfloat y, GLfloat z, GLfloat size, GLfloat red, GLfloat green, GLfloat blue, bool negative) {
    if(!gatheringLights) return 0;
    	
	if (size < 1) { size = 1; } //A size of zero signals the shader to stop processing, so disallow that input.
	
    if(numLightsInScene < LIGHTS_MAX) {
		
		dynamicLights[numLightsInScene].type = POINT_LIGHT;
		
		dynamicLights[numLightsInScene].position[0] = x;
		dynamicLights[numLightsInScene].position[1] = y;
		dynamicLights[numLightsInScene].position[2] = z;
		
		dynamicLights[numLightsInScene].color[0] = red;
		dynamicLights[numLightsInScene].color[1] = green;
		dynamicLights[numLightsInScene].color[2] = blue;
		
		dynamicLights[numLightsInScene].size = size;
		
		dynamicLights[numLightsInScene].AABB[BB_LOW_X] = x-size;
		dynamicLights[numLightsInScene].AABB[BB_HIGH_X] = x+size;
		dynamicLights[numLightsInScene].AABB[BB_LOW_Y] = y-size;
		dynamicLights[numLightsInScene].AABB[BB_HIGH_Y] = y+size;
		dynamicLights[numLightsInScene].AABB[BB_LOW_Z] = z-size;
		dynamicLights[numLightsInScene].AABB[BB_HIGH_Z] = z+size;
		
		dynamicLights[numLightsInScene].negative = negative;

        numLightsInScene++;
		return 1;
    }
	
	return 0;
}

bool DrawCache::addSpotLight(GLfloat x, GLfloat y, GLfloat z, GLfloat size,  GLfloat dirX, GLfloat dirY, GLfloat dirZ, GLfloat outerAngle, GLfloat innerAngle,  GLfloat red, GLfloat green, GLfloat blue, bool negative) {
	if(!gatheringLights) return 0;
	
	if (size < 1) { size = 1; } //A size of zero signals the shader to stop processing, so disallow that input.
	
	if(numLightsInScene < LIGHTS_MAX) {
		int baseIndex = numLightsInScene*4;
		
		dynamicLights[numLightsInScene].type = SPOT_LIGHT;
		
		dynamicLights[numLightsInScene].position[0] = x;
		dynamicLights[numLightsInScene].position[1] = y;
		dynamicLights[numLightsInScene].position[2] = z;
		
		dynamicLights[numLightsInScene].direction[0] = dirX;
		dynamicLights[numLightsInScene].direction[1] = dirY;
		dynamicLights[numLightsInScene].direction[2] = dirZ;
		
		dynamicLights[numLightsInScene].innerAngle = innerAngle;
		dynamicLights[numLightsInScene].outerAngle = outerAngle;
		
		dynamicLights[numLightsInScene].color[0] = red;
		dynamicLights[numLightsInScene].color[1] = green;
		dynamicLights[numLightsInScene].color[2] = blue;
		
		dynamicLights[numLightsInScene].size = size;
		
		dynamicLights[numLightsInScene].AABB[BB_LOW_X] = x-size;
		dynamicLights[numLightsInScene].AABB[BB_HIGH_X] = x+size;
		dynamicLights[numLightsInScene].AABB[BB_LOW_Y] = y-size;
		dynamicLights[numLightsInScene].AABB[BB_HIGH_Y] = y+size;
		dynamicLights[numLightsInScene].AABB[BB_LOW_Z] = z-size;
		dynamicLights[numLightsInScene].AABB[BB_HIGH_Z] = z+size;

		dynamicLights[numLightsInScene].negative = negative;

		numLightsInScene ++;
		return 1;
	}
	return 0;
}

bool DrawCache::addAreaLightFan(GLfloat *vertex_array, int vertex_count, GLfloat size,  GLfloat dirX, GLfloat dirY, GLfloat dirZ, GLfloat outerAngle, GLfloat innerAngle,  GLfloat red, GLfloat green, GLfloat blue, bool negative) {
	if(!gatheringLights) return 0;
	
		//Allowing a single-vertex area would be silly, but I think we can have a 2 vertex "line" light.
	if (vertex_count < 2)
		return 0;
	
	//Verify that there are enough elements available to hold all of the vertices.
	if(nextAvailableLightAreaIndex + (vertex_count * 4) >= AREA_LIGHT_VERTICES_MAX * 4)
		return 0;
	
	int baseIndex = numLightsInScene*4; //Capture base now, because addSpotlight will increment numLightsInScene.
	
	//An area light starts as a spotlight, except that it also has an index to a vertices. (The location fed in is meaningless)
	/*if( addSpotLight(vertex_array[0], vertex_array[1], vertex_array[2], size,  dirX, dirY, dirZ, outerAngle, innerAngle,  red, green, blue, negative) )
	{
		lightColors[baseIndex + 3] = negative ? AREA_LIGHT_NEGATIVE : AREA_LIGHT; //Set light type/mode
		lightAreaIndices[baseIndex] = nextAvailableLightAreaIndex; //Establish relation between this light and the slots in the area vertex array.
		
		int i, n = 0;
		for(i = nextAvailableLightAreaIndex; i < vertex_count*4; i += 4) {
			lightAreas[i + 0] = vertex_array[n + 0];
			lightAreas[i + 1] = vertex_array[n + 1];
			lightAreas[i + 2] = vertex_array[n + 2];
			n++;
			lightAreas[i + 3] = n == vertex_count ? 0 : 1; //Set whether this is the end of the list or not.
		}
		nextAvailableLightAreaIndex = i;
		
		return 1;
	}*/
	return 0;
}



void DrawCache::finishGatheringLights() {
    gatheringLights = 0;
}

void DrawCache::growToFit(int triangleCount, int vertexCount) {
	//Guarantee that the geometry list is large enough for at least one more item.
	while(geometryFilled >= current_geometry_list_size - 1) {
		growGeometryList();
	}
	
	//Guarantee that the index list is large enough. Assume each triangle needs three indices.
	while( indicesFilled + (triangleCount * 3) >= current_index_list_size - 1) {
		growIndexList();
	}
	
	//Guarantee the vertex lists are large enough.
	while( verticesFilled + vertexCount >= current_vertex_list_size - 1) {
		growVertexLists();
	}
}

//Requires 3 GLFloats in vertex_array per vertex, and 2 GLfloats per texcoord
//tex4 is a 4-dimensional array, which is surface normal vector + sign.
//Normalized is assumed to be GL_FALSE and Stride must be 0.
void DrawCache::addTriangleFan(int vertex_count, GLfloat *vertex_array, GLfloat *texcoord_array, GLfloat *tex4) {
	
	int numTriangles = vertex_count - 2; //The first 3 vertices make a triangle, and each subsequent vertex adds another.
	growToFit(numTriangles, vertex_count); //Make room for the incoming triangle fan. Do this first, because it might change geometryFilled should a draw be triggered.
	
	int g = geometryFilled; //Convenience index.
	captureState(g); //Capture volatile state data for this geometry.
	
        //The incoming data is a triangle fan: 0,1,2,3,4,5
        //We need to create indices that convert the fan into triangles: 0,1,2, 0,2,3, 0,3,4, etc
		//Note that indicesFilled is always a multiple of three (because they describe triangles), but verticesFilled is the total number of input vertices.
	geometry[g].numIndices = numTriangles * 3;
    for(int i = 0; i < numTriangles; ++i) {
        indices[indicesFilled] = verticesFilled;
        indices[indicesFilled + 1] = verticesFilled + i + 1;
        indices[indicesFilled + 2] = verticesFilled + i + 2;
		indicesFilled += 3;
    }

    //Fill 2-element components.
    int n = 0;
    for(int i = verticesFilled*2; i < (verticesFilled*2 + (vertex_count * 2)); i += 2) {
        texcoordArray[i] = texcoord_array[n];
		texcoordArray[i+1] = texcoord_array[n+1];
        n+=2;
    }
    
    //Fill the 3-element components, and make sure the bounding box will enclose all vertices.
    n = 0;
    GLfloat *normal_array = MSI()->normals();
	primeBoundingBox(g, vertex_array[0], vertex_array[1], vertex_array[2]);
	
    for(int i = verticesFilled*3; i < (verticesFilled*3 + (vertex_count*3)); i += 3) {
        vertexArray[i] = vertex_array[n]; vertexArray[i+1] = vertex_array[n+1]; vertexArray[i+2] = vertex_array[n+2];
		normalArray[i] = normal_array[n]; normalArray[i+1] = normal_array[n+1]; normalArray[i+2] = normal_array[n+2];
		
		growBoundingBox(g, vertex_array[n], vertex_array[n+1], vertex_array[n+2]);
        n+=3;
    }
    
	//Fill the 4-element components
	for(int i = verticesFilled*4; i < (verticesFilled*4 + (vertex_count * 4)); i += 4) {
		colors[i] = geometry[g].primaryColor[0]; colors[i+1] = geometry[g].primaryColor[1]; colors[i+2] = geometry[g].primaryColor[2]; colors[i+3] = geometry[g].primaryColor[3];
		
		if(tex4) {
			texCoords4[i] = tex4[0]; texCoords4[i+1] = tex4[1]; texCoords4[i+2] = tex4[2]; texCoords4[i+3] = tex4[3];
		} else {
			texCoords4[i] = 0; texCoords4[i+1] = 0; texCoords4[i+2] = 0; texCoords4[i+3] = 1; //not sure if 0s or 1s are better here, or maybe 0,0,0,1...
		}
	}

	clearTextureAttributeCaches();
	
    verticesFilled += vertex_count;
	geometryFilled ++;
}


void DrawCache::addModel(rectangle_definition& RenderRectangle)
{
	// TODO: support glEnable(GL_CULL_FACE);
	// TODO: support glFrontFace
	
	OGL_ModelData *ModelPtr = RenderRectangle.ModelPtr;
	int index_count = ModelPtr->Model.NumVI();
	GLushort *index_array = ModelPtr->Model.VIBase();
	GLfloat *vertex_array = ModelPtr->Model.PosBase();
	GLfloat *texcoord_array = ModelPtr->Model.TCBase();
	GLfloat *normals = ModelPtr->Model.NormBase();
	GLfloat *tangents = ModelPtr->Model.TangentBase();
	
	//Is there a better way to figure out how big vertex_array is?
	int max_index = 0;
	for (int i = 0; i < index_count; i++) {
		if( index_array[i] > max_index )
			max_index = index_array[i];
	}
	int vertex_count = max_index + 1;
	int numTriangles = index_count/3;
	
	growToFit(numTriangles, vertex_count);
	
	int g = geometryFilled; //Convenience index.
	captureState(g); //Capture volatile state data for this geometry.
	
		//Since these are triangles, the number of triangles must always be index_count/3.
		//Can't memcopy the indices though because we need a short to int conversion. :(
	geometry[g].numIndices = index_count;
	for(int i = 0; i < index_count; ++i) {
		indices[indicesFilled] = index_array[i] + verticesFilled;
		indicesFilled ++;
	}
	
	//TODO: use something like memcopy for the arrays.

	//Fill 2-element components.
	int n = 0;
	for(int i = verticesFilled*2; i < (verticesFilled*2 + (vertex_count * 2)); i += 2) {
		texcoordArray[i] = texcoord_array[n];
		texcoordArray[i+1] = texcoord_array[n+1];
		n+=2;
	}
	
	//Fill the 3-element components, and make sure the bounding box will enclose all vertices.
	n = 0;
	for(int i = verticesFilled*3; i < (verticesFilled*3 + (vertex_count*3)); i += 3) {
		vertexArray[i] = vertex_array[n]; vertexArray[i+1] = vertex_array[n+1]; vertexArray[i+2] = vertex_array[n+2];
		normalArray[i] =  0-normals[n]; normalArray[i+1] =  0-normals[n+1]; normalArray[i+2] =  0-normals[n+2];
		n+=3;
	}
	
	//Fill the 4-element components
	for(int i = verticesFilled*4; i < (verticesFilled*4 + (vertex_count * 4)); i += 4) {
		colors[i] = geometry[g].primaryColor[0]; colors[i+1] = geometry[g].primaryColor[1]; colors[i+2] = geometry[g].primaryColor[2]; colors[i+3] = geometry[g].primaryColor[3];
		
		if(tangents) {
			texCoords4[i] = tangents[0]; texCoords4[i+1] = tangents[1]; texCoords4[i+2] = tangents[2]; texCoords4[i+3] = tangents[3];
			//printf("Sign: %f\n", normals[3]);
		} else {
			texCoords4[i] = 0; texCoords4[i+1] = 0; texCoords4[i+2] = 0; texCoords4[i+3] = 1; //not sure if 0s or 1s are better here, or maybe 0,0,0,1...
		}
	}

		//The model should already have a bounding box, but for lighting we must transform the supplied box into a world location and then create a new axis-aligned box which contains it.
	//Create and apply the world space transform.
	const world_point3d& pos = RenderRectangle.Position;
	GLfloat HorizScale = RenderRectangle.Scale*RenderRectangle.HorizScale;
	GLfloat lowerCorner[3], upperCorner[3];
	MSI()->pushMatrix();
		MSI()->loadIdentity();
		MSI()->translatef(pos.x, pos.y, pos.z);
		MSI()->rotatef((360.0/FULL_CIRCLE)*RenderRectangle.Azimuth,0,0,1);
		MSI()->scalef(HorizScale,HorizScale,RenderRectangle.Scale);
		lowerCorner[0] = ModelPtr->Model.BoundingBox[0][0]; lowerCorner[1] = ModelPtr->Model.BoundingBox[0][1]; lowerCorner[2] = ModelPtr->Model.BoundingBox[0][2];
		upperCorner[0] = ModelPtr->Model.BoundingBox[1][0]; upperCorner[1] = ModelPtr->Model.BoundingBox[1][1]; upperCorner[2] = ModelPtr->Model.BoundingBox[1][2];
		MSI()->transformVertex(lowerCorner[0], lowerCorner[1], lowerCorner[2]);
		MSI()->transformVertex(upperCorner[0], upperCorner[1], upperCorner[2]);
	MSI()->popMatrix();
	primeBoundingBox(g, lowerCorner[0], lowerCorner[1], lowerCorner[2]);
	growBoundingBox(g, upperCorner[0], upperCorner[1], upperCorner[2]);
	
	clearTextureAttributeCaches();
	
	verticesFilled += vertex_count;
	geometryFilled++;
}

	//Captures current volatile state information into geometry[g]
void DrawCache::captureState(int g)
{
	GLint initialUnit;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &initialUnit); //Store active texture so we can reset it later.
	glActiveTexture(GL_TEXTURE0);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &(geometry[g].textureID0));
	glActiveTexture(GL_TEXTURE1);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &(geometry[g].textureID1));
	glActiveTexture(initialUnit);
	
	geometry[g].shader = lastEnabledShader();
	geometry[g].landscapeTexture = lastTextureIsLandscape;
	GLfloat *activeColor = MSI()->color();
	MatrixStack::Instance()->getFloatv(MS_MODELVIEW, geometry[g].modelMatrix);
	MatrixStack::Instance()->getFloatv(MS_PROJECTION, geometry[g].projectionMatrix);
	MatrixStack::Instance()->getFloatvInverse(MS_MODELVIEW, geometry[g].modelMatrixInverse);
	MatrixStack::Instance()->getFloatvModelviewProjection(geometry[g].modelProjection);
	MSI()->getFloatv(MS_TEXTURE, geometry[g].textureMatrix);
	MSI()->getPlanev(0, geometry[g].clipPlane0);
	MSI()->getPlanev(1, geometry[g].clipPlane1);
	MSI()->getPlanev(5, geometry[g].clipPlane5);
	glGetIntegerv(GL_DEPTH_FUNC, &(geometry[g].depthFunction));
	glGetIntegerv(GL_DEPTH_TEST, &(geometry[g].depthTest));
	glGetBooleanv(GL_BLEND, &(geometry[g].isBlended));
	geometry[g].scaleX = scaleX;
	geometry[g].offsetX = offsetX;
	geometry[g].scaleY = scaleY;
	geometry[g].offsetY = offsetY;
	geometry[g].bloomScale = bloomScale;
	geometry[g].bloomShift = bloomShift;
	geometry[g].flare = flare;
	geometry[g].selfLuminosity = selfLuminosity;
	geometry[g].pulsate = pulsate;
	geometry[g].wobble = wobble;
	geometry[g].depth = depth;
	geometry[g].glow = glow;
	
	geometry[g].primaryColor[0] = MSI()->color()[0];
	geometry[g].primaryColor[1] = MSI()->color()[1];
	geometry[g].primaryColor[2] = MSI()->color()[2];
	geometry[g].primaryColor[3] = MSI()->color()[3];
}

void DrawCache::primeBoundingBox(int g, GLfloat x, GLfloat y, GLfloat z) {
	geometry[g].AABB[BB_HIGH_X]=x;
	geometry[g].AABB[BB_LOW_X]=x;
	geometry[g].AABB[BB_HIGH_Y]=y;
	geometry[g].AABB[BB_LOW_Y]=y;
	geometry[g].AABB[BB_HIGH_Z]=z;
	geometry[g].AABB[BB_LOW_Z]=z;
}

void DrawCache::growBoundingBox(int g, GLfloat x, GLfloat y, GLfloat z){
	if(x >= geometry[g].AABB[BB_HIGH_X]) geometry[g].AABB[BB_HIGH_X] = x;
	if(x <= geometry[g].AABB[BB_LOW_X]) geometry[g].AABB[BB_LOW_X] = x;
	if(y >= geometry[g].AABB[BB_HIGH_Y]) geometry[g].AABB[BB_HIGH_Y] = y;
	if(y <= geometry[g].AABB[BB_LOW_Y]) geometry[g].AABB[BB_LOW_Y] = y;
	if(z >= geometry[g].AABB[BB_HIGH_Z]) geometry[g].AABB[BB_HIGH_Z] = z;
	if(z <= geometry[g].AABB[BB_LOW_Z]) geometry[g].AABB[BB_LOW_Z] = z;
}


void DrawCache::drawAll() {
	glGenBuffers(NUM_VBOS, vboIDs);
	
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[TEXCOORDARRAY_VBO]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * verticesFilled * 2, texcoordArray, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[VERTEXARRAY_VBO]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * verticesFilled * 3, vertexArray, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[NORMALARRAY_VBO]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * verticesFilled * 3, normalArray, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[COLORS_VBO]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * verticesFilled * 4, colors, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[TEXCOORDS4_VBO]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * verticesFilled * 4, texCoords4, GL_STATIC_DRAW);
	
	int currentIndex = 0;
	
	Shader *originalShader = lastEnabledShader();
	
	//Step through each geometry object and draw what we can.
	for(int g = 0; g < geometryFilled; g++) {
		if (verticesFilled == 0 || indicesFilled == 0) {
			break;
		}
		if(geometry[g].shader){
			geometry[g].shader->enable();
			geometry[g].shader->setMatrix4(Shader::U_ModelViewMatrix, geometry[g].modelMatrix);
			geometry[g].shader->setMatrix4(Shader::U_ModelViewProjectionMatrix, geometry[g].modelProjection);
			geometry[g].shader->setMatrix4(Shader::U_ModelViewMatrixInverse, geometry[g].modelMatrixInverse);
			geometry[g].shader->setMatrix4(Shader::U_TextureMatrix, geometry[g].textureMatrix);
			geometry[g].shader->setVec4(Shader::U_FogColor, MatrixStack::Instance()->fog());
			geometry[g].shader->setVec4(Shader::U_FogStart, MatrixStack::Instance()->fogStart());
			geometry[g].shader->setVec4(Shader::U_FogStart, MatrixStack::Instance()->fogEnd());
			geometry[g].shader->setVec4(Shader::U_ClipPlane0, geometry[g].clipPlane0);
			geometry[g].shader->setVec4(Shader::U_ClipPlane1, geometry[g].clipPlane1);
			geometry[g].shader->setVec4(Shader::U_ClipPlane5, geometry[g].clipPlane5);
			geometry[g].shader->setVec4(Shader::U_FogColor, MatrixStack::Instance()->fog());
			geometry[g].shader->setVec4(Shader::U_FogStart, MatrixStack::Instance()->fogStart());
			geometry[g].shader->setVec4(Shader::U_FogStart, MatrixStack::Instance()->fogEnd());
			
			geometry[g].shader->setFloat(Shader::U_ScaleX, geometry[g].scaleX);
			geometry[g].shader->setFloat(Shader::U_ScaleY, geometry[g].scaleY);
			geometry[g].shader->setFloat(Shader::U_OffsetX, geometry[g].offsetX);
			geometry[g].shader->setFloat(Shader::U_OffsetY, geometry[g].offsetY);
			geometry[g].shader->setFloat(Shader::U_BloomScale, geometry[g].bloomScale);
			geometry[g].shader->setFloat(Shader::U_BloomShift, geometry[g].bloomShift);
			geometry[g].shader->setFloat(Shader::U_Flare, geometry[g].flare);
			geometry[g].shader->setFloat(Shader::U_SelfLuminosity, geometry[g].selfLuminosity);
			geometry[g].shader->setFloat(Shader::U_Pulsate, geometry[g].pulsate);
			geometry[g].shader->setFloat(Shader::U_Wobble, geometry[g].wobble);
			geometry[g].shader->setFloat(Shader::U_Depth, geometry[g].depth);
			geometry[g].shader->setFloat(Shader::U_Glow, geometry[g].glow);
			geometry[g].shader->setFloat(Shader::U_StrictDepthMode, strictDepthMode); //Not sure what this is for, or it it works yet.
			
			//U_TransferFadeOut //TODO: needs to be captured.
			//U_Visibility //TODO: needs to be captured.
			
			glBindBuffer(GL_ARRAY_BUFFER, vboIDs[TEXCOORDARRAY_VBO]);
			glEnableVertexAttribArray(Shader::ATTRIB_TEXCOORDS);
			glVertexAttribPointer(Shader::ATTRIB_TEXCOORDS, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, 0);
			
			glBindBuffer(GL_ARRAY_BUFFER, vboIDs[VERTEXARRAY_VBO]);
			glEnableVertexAttribArray(Shader::ATTRIB_VERTEX);
			glVertexAttribPointer(Shader::ATTRIB_VERTEX, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, 0);
			
			glBindBuffer(GL_ARRAY_BUFFER, vboIDs[NORMALARRAY_VBO]);
			glEnableVertexAttribArray(Shader::ATTRIB_NORMAL);
			glVertexAttribPointer(Shader::ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, 0);
			
			glBindBuffer(GL_ARRAY_BUFFER, vboIDs[COLORS_VBO]);
			glEnableVertexAttribArray(Shader::ATTRIB_COLOR);
			glVertexAttribPointer(Shader::ATTRIB_COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, 0);
			
			glBindBuffer(GL_ARRAY_BUFFER, vboIDs[TEXCOORDS4_VBO]);
			glEnableVertexAttribArray(Shader::ATTRIB_TEXCOORDS4);
			glVertexAttribPointer(Shader::ATTRIB_TEXCOORDS4, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, 0);
			
			/*glEnable(GL_CULL_FACE);
			glFrontFace(GL_CW);*/
			
			if( geometry[g].textureID1 ) {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, geometry[g].textureID1);
			} else {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, geometry[g].textureID0);
						
			if(geometry[g].landscapeTexture) {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //DCW added for landscape. Repeat horizontally
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT); //DCW added for landscape. Mirror vertically.
				
			} else {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //DCW this is probably better for non-landscapes
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //DCW this is probably better for non-landscapes
			}
			
			if(geometry[g].isBlended) {
				glEnable(GL_BLEND);
				glDepthMask(GL_FALSE); //Blended (transparent) surfaces should not update the depth buffer.
			} else {
				glDisable(GL_BLEND);
				glDepthMask(GL_TRUE);
			}
			if(geometry[g].depthTest) {
				glEnable(GL_DEPTH_TEST);
			} else {
				glDisable(GL_DEPTH_TEST);
			}
			glDepthFunc(geometry[g].depthFunction);
			
			//Attach Lights
			setAttachedLightsForGeometry(g);
			geometry[g].shader->setFloatv(Shader::U_PointLights, MAX_POINT_LIGHT_ELEMENTS, activePointLights);
			geometry[g].shader->setFloatv(Shader::U_SpotLights, MAX_SPOT_LIGHT_ELEMENTS, activeSpotLights);
			geometry[g].shader->setFloatv(Shader::U_AreaLights, MAX_AREA_LIGHT_ELEMENTS, activeAreaLights);
			
			glDrawElements(GL_TRIANGLES, geometry[g].numIndices, GL_UNSIGNED_INT, indices + currentIndex);
			
			currentIndex += geometry[g].numIndices;
		}
	}
	
        //Reset what we care about.
    verticesFilled = 0;
	indicesFilled = 0;
	geometryFilled = 0;
	lastTextureIsLandscape = 0;
	
	Shader::disable();
	glDeleteBuffers(NUM_VBOS, vboIDs);
	
    if(originalShader) {
        originalShader->enable(); //We need to restore whatever shader was active, to avoid polluting outside state.
    }
	glDepthMask(GL_TRUE);
}

void DrawCache::setAttachedLightsForGeometry(int g) {

		//Blank out the buffers, so we don't have garbage data in the unfilled portion.
	clearAttachedLights();
	
		//Early exit, if lights do not apply.
	if(gatheringLights || geometry[g].landscapeTexture) {
		return;
	}
	
	int pointLightsAttached = 0;
	int spotLightsAttached = 0;
	int nextAreaLightIndex = 0;
	
	//Iterate through every light in the entire world, and add it to it's respective shader data buffer (if there is room in the buffer, and if the geometry and light bounding boxes intersect).
	for(int l = 0; l < numLightsInScene; l++) {
		//Format of point light data: position, color, size (7 elements per light).
		if(dynamicLights[l].type == POINT_LIGHT && (pointLightsAttached+1) * POINT_LIGHT_DATA_SIZE < MAX_POINT_LIGHT_ELEMENTS && lightAndGeometryIntersect(dynamicLights[l], geometry[g])) {
			int pi = pointLightsAttached * POINT_LIGHT_DATA_SIZE; //Index into active array
			activePointLights[pi + 0] = dynamicLights[l].position[0];
			activePointLights[pi + 1] = dynamicLights[l].position[1];
			activePointLights[pi + 2] = dynamicLights[l].position[2];
			
			//Light positions must now be in eye space
			MSI()->transformVertexToEyespace(activePointLights[pi + 0],activePointLights[pi + 1],activePointLights[pi + 2]);
			
			activePointLights[pi + 3] = dynamicLights[l].color[0];
			activePointLights[pi + 4] = dynamicLights[l].color[1];
			activePointLights[pi + 5] = dynamicLights[l].color[2];
			
			activePointLights[pi + 6] = dynamicLights[l].size;
			
			pointLightsAttached ++;
		}
		
		//Format of spot light data: position, direction, innerAngleRadiansCos, outerAngleRadiansCos, color, size (12 elements per light).
		if(dynamicLights[l].type == SPOT_LIGHT && (spotLightsAttached+1) * SPOT_LIGHT_DATA_SIZE < MAX_SPOT_LIGHT_ELEMENTS && lightAndGeometryIntersect(dynamicLights[l], geometry[g])) {
			int si = spotLightsAttached * SPOT_LIGHT_DATA_SIZE; //Index into active array
			activeSpotLights[si + 0] = dynamicLights[l].position[0];
			activeSpotLights[si + 1] = dynamicLights[l].position[1];
			activeSpotLights[si + 2] = dynamicLights[l].position[2];
			MSI()->transformVertexToEyespace(activeSpotLights[si + 0],activeSpotLights[si + 1],activeSpotLights[si + 2]); //Light positions must now be in eye space
			
			activeSpotLights[si + 3] = dynamicLights[l].direction[0];
			activeSpotLights[si + 4] = dynamicLights[l].direction[1];
			activeSpotLights[si + 5] = dynamicLights[l].direction[2];
			MSI()->transformVectorToEyespace(activeSpotLights[si + 3], activeSpotLights[si + 4], activeSpotLights[si + 5]); //Direction vector, also needs to be in eyespace.
			
			activeSpotLights[si + 6] = cos(dynamicLights[l].innerAngle * 0.0174533); //Convert to cos(radians)
			activeSpotLights[si + 7] = cos(dynamicLights[l].outerAngle * 0.0174533); //Convert to cos(radians)
			
			activeSpotLights[si + 8] = dynamicLights[l].color[0];
			activeSpotLights[si + 9] = dynamicLights[l].color[1];
			activeSpotLights[si + 10] = dynamicLights[l].color[2];
			
			activeSpotLights[si + 11] = dynamicLights[l].size;
			
			spotLightsAttached ++;
		}
	}
		/*
		x = lightPositions[l*4 + 0];
		y = lightPositions[l*4 + 1];
		z = lightPositions[l*4 + 2];
		size = lightPositions[l*4 + 3];
		
		red = lightColors[l*4];
		green = lightColors[l*4 + 1];
		blue = lightColors[l*4 + 2];
		mode = lightColors[l*4 + 3];
		
		//Is this a spot light
		if(mode >= SPOT_LIGHT_PACK_MIN && mode <= SPOT_LIGHT_PACK_MAX) {
			dirX = lightPositions[l*4 + 4];
			dirY = lightPositions[l*4 + 5];
			dirZ = lightPositions[l*4 + 6];
			outerLimitCos = cos(lightColors[l*4 + 4] * 0.0174533); //Convert to cos(radians)
			innerLimitCos = cos(lightColors[l*4 + 5] * 0.0174533); //Convert to cos(radians)
		}
		
		//Is the light inside the bounding box (plus the light size)?
		if( size > 0
		   && x >= (geometry[g].AABB[BB_LOW_X]-size)
		   && x <= (geometry[g].AABB[BB_HIGH_X]+size)
		   && y >= (geometry[g].AABB[BB_LOW_Y]-size)
		   && y <= (geometry[g].AABB[BB_HIGH_Y]+size)
		   && z >= (geometry[g].AABB[BB_LOW_Z]-size)
		   && z <= (geometry[g].AABB[BB_HIGH_Z]+size)
		   ) {
			
			//We can only attach up to ACTIVE_LIGHTS_MAX lights.
			if(lightsAttached < ACTIVE_LIGHTS_MAX){
				
				//Light positions must now be in eye space
				MSI()->transformVertexToEyespace(x,y,z);
				
				activeLightPositions[lightsAttached*4 +0] = x;
				activeLightPositions[lightsAttached*4 +1] = y;
				activeLightPositions[lightsAttached*4 +2] = z;
				activeLightPositions[lightsAttached*4 +3] = size;
				
				activeLightColors[lightsAttached*4 +0] = red;
				activeLightColors[lightsAttached*4 +1] = green;
				activeLightColors[lightsAttached*4 +2] = blue;
				activeLightColors[lightsAttached*4 +3] = mode;
				
				//Spot lights also take up the second slot.
				if(mode >= SPOT_LIGHT_PACK_MIN && mode <= SPOT_LIGHT_PACK_MAX) {
					//CONSIDER THAT 3D models have their own MVP matrix, which is usually different than the active one in MSI.
					MSI()->transformVectorToEyespace(dirX, dirY, dirZ); //Direction vector, if any, also needs to be in eyespace.
					
					activeLightPositions[lightsAttached*4 +4] = dirX;
					activeLightPositions[lightsAttached*4 +5] = dirY;
					activeLightPositions[lightsAttached*4 +6] = dirZ;
					
					activeLightColors[lightsAttached*4 +4] = outerLimitCos;
					activeLightColors[lightsAttached*4 +5] = innerLimitCos;
					lightsAttached++;
					l++; //Skip over processing of this slot on next loop, since it's not a whole light.
				}
				
				lightsAttached++;
			}
			
		}
		
	}
	*/
}

void DrawCache::clearAttachedLights() {
	for(int i = 0; i < MAX_POINT_LIGHT_ELEMENTS; i++) {
		activePointLights[i]=0;
	}
	
	for(int i = 0; i < MAX_SPOT_LIGHT_ELEMENTS; i++) {
		activeSpotLights[i]=0;
	}
	
	for(int i = 0; i < MAX_AREA_LIGHT_ELEMENTS; i++) {
		activeAreaLights[i]=0;
	}
}

bool DrawCache::lightAndGeometryIntersect(dynamicLight l, geometryProperties g) {
	
	float x = l.position[0];
	float y = l.position[1];
	float z = l.position[2];
	
	if( l.size > 0
		   && x >= (g.AABB[BB_LOW_X]	-l.size)
		   && x <= (g.AABB[BB_HIGH_X]	+l.size)
		   && y >= (g.AABB[BB_LOW_Y]	-l.size)
		   && y <= (g.AABB[BB_HIGH_Y]	+l.size)
		   && z >= (g.AABB[BB_LOW_Z]	-l.size)
		   && z <= (g.AABB[BB_HIGH_Z]	+l.size)
	   ) {
		return TRUE;
	}
	
	return FALSE;
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
void DrawCache::cacheStrictDepthMode(GLfloat v) {strictDepthMode = v;}

void DrawCache::clearTextureAttributeCaches() {
    scaleX = 0;
    offsetX = 0;
    scaleY = 0;
    offsetY = 0;
    bloomScale = 0;
    bloomShift = 0;
    flare = 0;
    selfLuminosity = 0.5; //Default is not 0
    pulsate = 0;
    wobble = 0;
    depth = 0;
    glow = 0;
	strictDepthMode = 0;
}
