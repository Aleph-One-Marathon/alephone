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

#include "logging.h"

bool lastTextureIsLandscape;

	//Main list of dynamic lights
	//Some lights (like spot lights) take two slots, so actual number of lights may be as few as half of this.
int numLightsInScene;

	//Allocate LIGHTS_MAX + 1 because some lights (like spot lights) require an extra slot.
GLfloat lightPositions[(LIGHTS_MAX+1) * 4]; //Format: x, y, z (world space location), w (size in world units).
GLfloat lightColors[(LIGHTS_MAX+1) * 4]; //Format: r, g, b, mode

	//The actual data fed into the shader.
	//Same format as above, but position list must be terminated with a {0,0,0,0} light.
GLfloat activeLightPositions[(ACTIVE_LIGHTS_MAX+1) * 4];
GLfloat activeLightColors[ACTIVE_LIGHTS_MAX * 4];

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
	
	geometry = (GeometryProperties*)malloc(sizeof(GeometryProperties) * current_geometry_list_size);
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

void DrawCache::addPointLight(GLfloat x, GLfloat y, GLfloat z, GLfloat size, GLfloat red, GLfloat green, GLfloat blue, bool negative) {
    if(!gatheringLights) return;
    
	if (size < 1) { size = 1; } //A size of zero signals the shader to stop processing, so disallow that input.
	
    if(numLightsInScene < LIGHTS_MAX) {
        lightPositions[numLightsInScene*4 + 0] = x;
        lightPositions[numLightsInScene*4 + 1] = y;
        lightPositions[numLightsInScene*4 + 2] = z;
        lightPositions[numLightsInScene*4 + 3] = size; //Size in world units. 0 means no light. 1000ish would be typical
        
        lightColors[numLightsInScene*4 + 0] = red; //Red
        lightColors[numLightsInScene*4 + 1] = green; //Green
        lightColors[numLightsInScene*4 + 2] = blue; //Blue
        lightColors[numLightsInScene*4 + 3] = negative ? POINT_LIGHT_NEGATIVE : POINT_LIGHT; //Light mode

        numLightsInScene++;
    }
}

void DrawCache::addSpotLight(GLfloat x, GLfloat y, GLfloat z, GLfloat size,  GLfloat dirX, GLfloat dirY, GLfloat dirZ, GLfloat outerAngle, GLfloat innerAngle,  GLfloat red, GLfloat green, GLfloat blue, bool negative) {
	if(!gatheringLights) return;
	
	if (size < 1) { size = 1; } //A size of zero signals the shader to stop processing, so disallow that input.
	
		//Spot lights take up two lighting slots.
	if(numLightsInScene + 1 < LIGHTS_MAX) {
		int baseIndex = numLightsInScene*4;
		
		lightPositions[baseIndex + 0] = x;
		lightPositions[baseIndex + 1] = y;
		lightPositions[baseIndex + 2] = z;
		lightPositions[baseIndex + 3] = size; //Size in world units. 0 means no light.
		lightPositions[baseIndex + 4] = dirX;
		lightPositions[baseIndex + 5] = dirY;
		lightPositions[baseIndex + 6] = dirZ;
		lightPositions[baseIndex + 7] = 0; //Unused
		
		lightColors[baseIndex + 0] = red; //Red
		lightColors[baseIndex + 1] = green; //Green
		lightColors[baseIndex + 2] = blue; //Blue
		lightColors[baseIndex + 3] = negative ? SPOT_LIGHT_NEGATIVE : SPOT_LIGHT; //Light mode
		lightColors[baseIndex + 4] = outerAngle;
		lightColors[baseIndex + 5] = innerAngle;
		lightColors[baseIndex + 6] = 0; //Unused
		lightColors[baseIndex + 7] = 0; //Unused

		numLightsInScene += 2;
	}
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
	// TODO: support normals. looks like ATTRIB_TEXCOORDS4 were originally tangents?
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
	geometry[g].bb_high_x=x;
	geometry[g].bb_low_x=x;
	geometry[g].bb_high_y=y;
	geometry[g].bb_low_y=y;
	geometry[g].bb_high_z=z;
	geometry[g].bb_low_z=z;
}

void DrawCache::growBoundingBox(int g, GLfloat x, GLfloat y, GLfloat z){
	if(x >= geometry[g].bb_high_x) geometry[g].bb_high_x = x;
	if(x <= geometry[g].bb_low_x) geometry[g].bb_low_x = x;
	if(y >= geometry[g].bb_high_y) geometry[g].bb_high_y = y;
	if(y <= geometry[g].bb_low_y) geometry[g].bb_low_y = y;
	if(z >= geometry[g].bb_high_z) geometry[g].bb_high_z = z;
	if(z <= geometry[g].bb_low_z) geometry[g].bb_low_z = z;
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
			geometry[g].shader->setFloat(Shader::U_SelfLuminosity, selfLuminosity);
			geometry[g].shader->setFloat(Shader::U_Pulsate, geometry[g].pulsate);
			geometry[g].shader->setFloat(Shader::U_Wobble, geometry[g].wobble);
			geometry[g].shader->setFloat(Shader::U_Depth, geometry[g].depth);
			geometry[g].shader->setFloat(Shader::U_Glow, glow);
			geometry[g].shader->setFloat(Shader::U_StrictDepthMode, strictDepthMode);
			
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
			int lightsAttached = 0;
			for(int n = 0; n < ACTIVE_LIGHTS_MAX*4; n++) {
				activeLightPositions[n]=0;
				activeLightColors[n]=0;
			}
			GLfloat x,y,z,size, dirX,dirY,dirZ, outerLimitCos,innerLimitCos, red,green,blue, mode;
			if (!gatheringLights && !geometry[g].landscapeTexture) {
				for(int l = 0; l < numLightsInScene; l++) {
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
					   && x >= (geometry[g].bb_low_x-size)
					   && x <= (geometry[g].bb_high_x+size)
					   && y >= (geometry[g].bb_low_y-size)
					   && y <= (geometry[g].bb_high_y+size)
					   && z >= (geometry[g].bb_low_z-size)
					   && z <= (geometry[g].bb_high_z+size)
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
				
				//Terminate active light list.
				/*activeLightPositions[lightsAttached*4] = 0;
				 activeLightPositions[lightsAttached*4 +1] = 0;
				 activeLightPositions[lightsAttached*4 +2] = 0;
				 activeLightPositions[lightsAttached*4 +3] = 0;*/
			}
			
			geometry[g].shader->setVec4v(Shader::U_LightColors, ACTIVE_LIGHTS_MAX, activeLightColors);
			geometry[g].shader->setVec4v(Shader::U_LightPositions, ACTIVE_LIGHTS_MAX, activeLightPositions);
			
			glDrawElements(GL_TRIANGLES, geometry[g].numIndices, GL_UNSIGNED_INT, indices + currentIndex);
			
			currentIndex += geometry[g].numIndices;
						
			//Reset lights in the shader so later draws don't see them accidentially.
			lightsAttached = 0;
			for(int n = 0; n < ACTIVE_LIGHTS_MAX*4; n++) {
				activeLightPositions[n]=0;
				activeLightColors[n]=0;
			}
			geometry[g].shader->setVec4v(Shader::U_LightPositions, ACTIVE_LIGHTS_MAX, activeLightPositions);
			geometry[g].shader->setVec4v(Shader::U_LightColors, ACTIVE_LIGHTS_MAX, activeLightColors);
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
        originalShader->enable(); //We need to restore whatever shader was active, so we don't pollute outside state.
    }
	glDepthMask(GL_TRUE);
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
