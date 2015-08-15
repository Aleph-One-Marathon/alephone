#ifndef _OGL_MODEL_DEF_
#define _OGL_MODEL_DEF_
/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
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
	
	OpenGL Model-Definition File
	by Loren Petrich,
	May 11, 2003

	This contains the definitions of all the OpenGL models and skins
*/

#include "OGL_Texture_Def.h"
#include "Model3D.h"


#ifdef HAVE_OPENGL

#include "OGL_Headers.h"

// 3D-Model and Skin Support

// Model-skin options
struct OGL_SkinData: public OGL_TextureOptionsBase
{
	short CLUT;				// Which color table is this skin for? (-1 is all)
	
	int GetMaxSize();
	OGL_SkinData(): CLUT(ALL_CLUTS) {}
};

// Manages skins, in case we decide to have separate static and animated models
struct OGL_SkinManager
{
	// List of skins that a model will "own"
	vector<OGL_SkinData> SkinData;
	
	// OpenGL skin ID's (one for each possible
	// Copied from TextureState in OGL_Textures	
	// Which member textures?
	enum
	{
		Normal,		// Used for all normally-shaded and shadeless textures
		Glowing,	// Used for self-luminous textures
		Bump,		// Bump map for textures
		NUMBER_OF_TEXTURES
	};
	GLuint IDs[NUMBER_OF_OPENGL_BITMAP_SETS][NUMBER_OF_TEXTURES];		// Texture ID's
	bool IDsInUse[NUMBER_OF_OPENGL_BITMAP_SETS][NUMBER_OF_TEXTURES];	// Which ID's are being used?
		
	void Reset(bool Clear_OGL_Txtrs);		// Resets the skins so that they may be reloaded;
											// indicate whether to clear OpenGL textures
	
	OGL_SkinData *GetSkin(short CLUT);		// Gets a pointer to a skin-data object; NULL for no skin available
	bool Use(short CLUT, short Which);		// Uses a skin; returns whether to load one	
	
	// For convenience
	void Load();
	void Unload();
};


// Mode
enum
{
	OGL_MLight_Fast,			// Fast method -- only one miner's-light calculation
	OGL_MLight_Fast_NoFade,		// Like above, but miner's light doesn't fade toward sides
	OGL_MLight_Indiv,			// Miner's light calculated for each vertex
	OGL_MLight_Indiv_NoFade,	// Like above, but miner's light doesn't fade toward sides
	NUMBER_OF_MODEL_LIGHT_TYPES
};


// Static 3D-Model Data and Options
class OGL_ModelData: public OGL_SkinManager
{
public:
	// Name of the model file;
	// there are two extra names here for handling ggadwa's Dim3 multiple files
	FileSpecifier ModelFile, ModelFile1, ModelFile2;
	
	// Type of model-file data (guess the model-file type if empty)
	vector<char> ModelType;
	
	// Preprocessing: rotation scaling, shifting
	// Scaling and rotation are applied before shifting
	// Scaling can be negative, thus producing mirroring
	float Scale;					// From model units to engine internal units (not World Units)
	float XRot, YRot, ZRot;			// In degrees
	float XShift, YShift, ZShift;	// In internal units
	short Sidedness;				// Which side of the polygons is visible?
									// (+: clockwise, -: counterclockwise, 0: both)
	short NormalType;				// What type of normals?
	float NormalSplit;				// Threshold for splitting the vertex normals 
	short LightType;				// What type of lighting?
	short DepthType;				// What sort of depth reference to use?
									// (+: farthest point, -: nearest point, 0: center point)
	bool  ForceSpriteDepth;			// Force sprites to be depth-sorted between model polys? (shader only)
	
	// Should a rotation rate be included, in order to get that Quake look?
	
	// The model itself (static, single-skin [only one skin at a time])
	Model3D Model;
	bool ModelPresent() {return !Model.VertIndices.empty();}
	
	// For convenience
	void Load();
	void Unload();
	
	OGL_ModelData():
		Scale(1), XRot(0), YRot(0), ZRot(0), XShift(0), YShift(0), ZShift(0), Sidedness(1),
			NormalType(1), NormalSplit(0.5), LightType(0), DepthType(0), ForceSpriteDepth(false) {}
};


// Returns NULL if a collectiona and sequence do not have an associated model;
// also returns which model sequence was found (
OGL_ModelData *OGL_GetModelData(short Collection, short Sequence, short& ModelSequence);

// Resets all model skins; arg is whether to clear OpenGL textures
void OGL_ResetModelSkins(bool Clear_OGL_Txtrs);

// for managing the model loading and unloading;
int OGL_CountModels(short Collection);
void OGL_LoadModels(short Collection);
void OGL_UnloadModels(short Collection);

// for managing the sprite depth-buffer override (see ForceSpriteDepth above)
void OGL_ResetForceSpriteDepth();  // to clear before calling OGL_LoadModels
bool OGL_ForceSpriteDepth();

class InfoTree;
void parse_mml_opengl_model(const InfoTree& root);
void reset_mml_opengl_model();
void parse_mml_opengl_model_clear(const InfoTree& root);

#endif // def HAVE_OPENGL

#endif
