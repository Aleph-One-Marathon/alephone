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

#include "map.h"
#include "projectiles.h"
#include "effects.h"


bool lastTextureIsLandscape;

	//Main list of dynamic lights
int numLightsInScene;
GLfloat lightPositions[LIGHTS_MAX * 4]; //Format: x, y, z (location), w (size in world units)
GLfloat lightColors[LIGHTS_MAX * 4]; //Format: r, g, b, intensity?

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
		current_geometry_list_size = 8196;
	} else {
		drawAndResetBuffer(0);
		free(geometry);
		current_geometry_list_size *= 2;
	}
	
	geometry = (GeometryProperties*)malloc(sizeof(GeometryProperties) * current_geometry_list_size);
	geometryFilled = 0;
}

void DrawCache::growIndexList() {
	if(current_index_list_size == 0) {
		current_index_list_size = 8196;
	} else {
		drawAndResetBuffer(0);
		free(indices);
		current_index_list_size *= 2;
	}
	
	indices = (GLuint*)malloc(sizeof(GLuint) * current_index_list_size);
	indicesFilled = 0;
}

void DrawCache::growVertexLists() {
	if(current_vertex_list_size == 0) {
		current_vertex_list_size = 8196;
	} else {
		drawAndResetBuffer(0);
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

void DrawCache::drawAll() {
    drawAndResetBuffer(0);
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
                    addLight(x, y, z, 1000, 1, .8, 0, 1 );
                    break;

                case _projectile_minor_defender:
                case _projectile_major_defender:
                case _projectile_minor_hummer:
                case _projectile_major_hummer:
                case _projectile_durandal_hummer:
                    addLight(x, y, z, 1000, 0, 1, .1, 1 );
                    break;

                case _projectile_rocket:
                case _projectile_juggernaut_rocket:
                case _projectile_juggernaut_missile:
                    addLight(x, y, z, 4000, 1, 1, .7, 1 );
                    break;
                    
                case _projectile_staff:
                case _projectile_staff_bolt:
                    addLight(x, y, z, 1000, .5, 1, rand() / double(RAND_MAX), 1 );
                    break;
                
                case _projectile_minor_cyborg_ball:
                case _projectile_major_cyborg_ball:
                case _projectile_compiler_bolt_minor:
                case _projectile_compiler_bolt_major:
                case _projectile_hunter:
                case _projectile_armageddon_sphere:
                case _projectile_armageddon_electricity:
                    addLight(x, y, z, 2000, 0, 1, 1, 1 );
                    break;
                
                case _projectile_fusion_bolt_minor:
                    addLight(x, y, z, 2000, .8, .7, 1, 1 );
                    break;
                
                case _projectile_minor_fusion_dispersal:
                case _projectile_major_fusion_dispersal:
                case _projectile_overloaded_fusion_dispersal:
                case _projectile_fusion_bolt_major:
                    addLight(x, y, z, 3000, .8, rand() / double(RAND_MAX), 1, 1 );
                    break;
            
                default:
                    break;
            }
    } else if(objectType == _object_is_effect) {
            switch (permutationType)
            {
                case _effect_rocket_explosion:
                case _effect_grenade_explosion:
                    addLight(x, y, z, 2000, 1, .9, 0, 1 );
                    break;
                    
                case _effect_alien_lamp_breaking:
                case _effect_water_lamp_breaking:
                case _effect_lava_lamp_breaking:
                case _effect_sewage_lamp_breaking:
                case _effect_rocket_contrail:
                case _effect_grenade_contrail:
                case _effect_juggernaut_missile_contrail:
                    addLight(x, y, z, 1000, .8, .8, .8, 1 );
                    break;

                case _effect_alien_weapon_ricochet:
                case _effect_flamethrower_burst:
                    addLight(x, y, z, 1000, .8, .7, 0, 1 );
                    break;

                case _effect_compiler_bolt_minor_detonation:
                case _effect_compiler_bolt_major_detonation:
                case _effect_compiler_bolt_major_contrail:
                    addLight(x, y, z, 1000, 0, .7, .7, 1 );
                    break;

                case _effect_hunter_projectile_detonation:
                    addLight(x, y, z, 1000, 0, 1, .8, 1 );
                    break;

                case _effect_minor_fusion_detonation:
                case _effect_major_fusion_detonation:
                    addLight(x, y, z, 2000, 1, 1, 1, 1 );
                    break;

                case _effect_major_fusion_contrail:
                    addLight(x, y, z, 500, .7, .8, 1, 1 );
                    break;

                case _effect_minor_defender_detonation:
                case _effect_major_defender_detonation:
                    addLight(x, y, z, 1000, .5, .5, .5, 1 );
                    break;


                case _effect_minor_hummer_projectile_detonation:
                case _effect_major_hummer_projectile_detonation:
                case _effect_durandal_hummer_projectile_detonation:
                    addLight(x, y, z, 2000, 0, 1, .1, 1 );
                    break;

                case _effect_cyborg_projectile_detonation:
                    addLight(x, y, z, 2000, .1, .8, 1, 1 );
                    break;

                case _effect_minor_fusion_dispersal:
                case _effect_major_fusion_dispersal:
                case _effect_overloaded_fusion_dispersal:
                    addLight(x, y, z, 4000, .8, 1, 1, 1 );
                    break;

                case _effect_lava_yeti_projectile_detonation:
                    addLight(x, y, z, 2000, 1, 0, 0, 1 );
                    break;

                default:
                    break;
            }
        }
    
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

//Requires 3 GLFloats in vertex_array per vertex, and 2 GLfloats per texcoord
//tex4 is a 4-dimensional array, which is surface normal vector + sign.
//Normalized is assumed to be GL_FALSE and Stride must be 0.
void DrawCache::addGeometry(int vertex_count, GLfloat *vertex_array, GLfloat *texcoord_array, GLfloat *tex4) {
	
	//Check to see that geometry list is large enough.
	while(geometryFilled >= current_geometry_list_size - 1) {
		growGeometryList();
	}
	
	//Check to see if index list is large enough. We will need enough indices to turn vertex_count into triangles ((vertex_count-2) * 3)
	while( indicesFilled + (vertex_count * 3) >= current_index_list_size - 1) {
		growIndexList();
	}
	
	//Check to see if the vertex lists are large enough.
	while( verticesFilled + vertex_count >= current_vertex_list_size - 1) {
		growVertexLists();
	}
	
	int g = geometryFilled;//Convenience capture of current geometry index.

	GLint whichUnit;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &whichUnit); //Store active texture so we can reset it later.
    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &(geometry[g].textureID0));
    glActiveTexture(GL_TEXTURE1);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &(geometry[g].textureID1));
    glActiveTexture(whichUnit);
        
        //Capture volatile state data for this geometry.
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
	
	
	GLfloat plane0[4], plane1[4], plane5[4], media6[4];
	MatrixStack::Instance()->getPlanev(0, plane0);
	MatrixStack::Instance()->getPlanev(1, plane1);
	MatrixStack::Instance()->getPlanev(5, plane5);
	
	
        //The incoming data is a triangle fan: 0,1,2,3,4,5
        //We need to create indices that convert the fan into triangles: 0,1,2, 0,2,3, 0,3,4, etc
    int numTriangles = vertex_count - 2; //The first 3 vertices make a triangle, and each subsequent vertex adds another.
	//Note that indicesFilled is always a multiple of three (because they describe triangles), but verticesFilled is the total number of input vertices.
	geometry[g].numIndices = numTriangles * 3; //Hopefully this is correct...
    for(int i = 0; i < numTriangles; ++i) {
        indices[indicesFilled] = verticesFilled;
        indices[indicesFilled + 1] = verticesFilled + i + 1;
        indices[indicesFilled + 2] = verticesFilled + i + 2;
		indicesFilled += 3;
    }
    
        //Prime bounding box with the first vertex.
	geometry[g].bb_high_x=vertex_array[0];
	geometry[g].bb_low_x=vertex_array[0];
	geometry[g].bb_high_y=vertex_array[1];
	geometry[g].bb_low_y=vertex_array[1];
	geometry[g].bb_high_z=vertex_array[2];
	geometry[g].bb_low_z=vertex_array[2];
    
    //Fill 2-element components.
    int n = 0;
    for(int i = verticesFilled*2; i < (verticesFilled*2 + (vertex_count * 2)); i += 2) {
        texcoordArray[i] = texcoord_array[n];
		texcoordArray[i+1] = texcoord_array[n+1];
        n+=2;
    }
    
    //Fill the 3-element components.
    n = 0;
    GLfloat *normal_array = MSI()->normals();
    for(int i = verticesFilled*3; i < (verticesFilled*3 + (vertex_count*3)); i += 3) {
        vertexArray[i] = vertex_array[n]; vertexArray[i+1] = vertex_array[n+1]; vertexArray[i+2] = vertex_array[n+2];
		normalArray[i] = normal_array[n]; normalArray[i+1] = normal_array[n+1]; normalArray[i+2] = normal_array[n+2];
        
			//Grow bounding box
        if(vertex_array[n] >= geometry[g].bb_high_x) geometry[g].bb_high_x = vertex_array[n];
        if(vertex_array[n] <= geometry[g].bb_low_x) geometry[g].bb_low_x = vertex_array[n];
        if(vertex_array[n+1] >= geometry[g].bb_high_y) geometry[g].bb_high_y = vertex_array[n+1];
        if(vertex_array[n+1] <= geometry[g].bb_low_y) geometry[g].bb_low_y = vertex_array[n+1];
        if(vertex_array[n+2] >= geometry[g].bb_high_z) geometry[g].bb_high_z = vertex_array[n+2];
        if(vertex_array[n+2] <= geometry[g].bb_low_z) geometry[g].bb_low_z = vertex_array[n+2];
        
        n+=3;
    }
    
	//Fill the 4-element components
	for(int i = verticesFilled*4; i < (verticesFilled*4 + (vertex_count * 4)); i += 4) {
		colors[i] = activeColor[0]; colors[i+1] = activeColor[1]; colors[i+2] = activeColor[2]; colors[i+3] = activeColor[3];
		
		if(tex4) {
			texCoords4[i] = tex4[0]; texCoords4[i+1] = tex4[1]; texCoords4[i+2] = tex4[2]; texCoords4[i+3] = tex4[3];
		} else {
			texCoords4[i] = 0; texCoords4[i+1] = 0; texCoords4[i+2] = 0; texCoords4[i+3] = 1; //not sure if 0s or 1s are better here, or maybe 0,0,0,1...
		}
	}

	//Unfortunately, stuff like viewer sprite glow assume that these properties persist across draw calls, but ideally we would call this after every one: clearTextureAttributeCaches();

    verticesFilled += vertex_count;
	geometryFilled ++;
	
	//drawAndResetBuffer(0); //DCW DESTRUCTIVE TEST
}


void DrawCache::drawAndResetBuffer(int index) {
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
			} else {
				glDisable(GL_BLEND);
			}
			if(geometry[g].depthTest) {
				glEnable(GL_DEPTH_TEST);
			} else {
				glDisable(GL_DEPTH_TEST);
			}
			glDepthFunc(geometry[g].depthFunction);
			
			//glEnable(GL_BLEND); //We might always want to blend.
			
			//Attach Lights
			int lightsAttached = 0;
			for(int n = 0; n < ACTIVE_LIGHTS_MAX*4; n++) {
				activeLightPositions[n]=0;
				activeLightColors[n]=0;
			}
			GLfloat x,y,z,size, red,green,blue,intensity;
			if (!gatheringLights && !geometry[g].landscapeTexture) {
				for(int l = 0; l < numLightsInScene; l++) {
					x = lightPositions[l*4];
					y = lightPositions[l*4 + 1];
					z = lightPositions[l*4 + 2];
					size = lightPositions[l*4 + 3];
					red = lightColors[l*4];
					green = lightColors[l*4 + 1];
					blue = lightColors[l*4 + 2];
					intensity = lightColors[l*4 + 3];
					
					//Is the light inside the bounding box (plus the light size)?
					if(   x >= (geometry[g].bb_low_x-size)
					   && x <= (geometry[g].bb_high_x+size)
					   && y >= (geometry[g].bb_low_y-size)
					   && y <= (geometry[g].bb_high_y+size)
					   && z >= (geometry[g].bb_low_z-size)
					   && z <= (geometry[g].bb_high_z+size)
					   ) {
						
						//The vertex needs to be in eyespace
						MSI()->transformVertexToEyespace(x, y, z);
						
						//We can only attach up to ACTIVE_LIGHTS_MAX lights.
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
    selfLuminosity = 0;
    pulsate = 0;
    wobble = 0;
    depth = 0;
    glow = 0;
	strictDepthMode = 0;
}
