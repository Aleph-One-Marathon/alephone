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

#include "cseries.h"
#include "OGL_Model_Def.h"
#include "OGL_Setup.h"

#ifdef HAVE_OPENGL

#include <cmath>

#include "Dim3_Loader.h"
#include "StudioLoader.h"
#include "WavefrontLoader.h"
#include "QD3D_Loader.h"
#include "InfoTree.h"


// Model-data stuff;
// defaults for whatever might need them
// Including skin stuff for convenience here
static OGL_ModelData DefaultModelData;
static OGL_SkinData DefaultSkinData;


// For mapping Marathon-physics sequences onto model sequences
struct SequenceMapEntry
{
	int16 Sequence;
	int16 ModelSequence;
	bool operator==(const SequenceMapEntry& other) const
	{
		return (Sequence == other.Sequence && ModelSequence == other.ModelSequence);
	}
	bool operator<(const SequenceMapEntry& other) const
	{
		return (Sequence < other.Sequence || ModelSequence < other.ModelSequence);
	}
};


// Store model-data stuff in a set of STL vectors
struct ModelDataEntry
{
	// Which Marathon-engine sequence gets translated into this model,
	// if static, or the neutral sequence, if dynamic
	short Sequence;
	
	vector<SequenceMapEntry> SequenceMap;
	
	// Make a member for more convenient access
	OGL_ModelData ModelData;
	
	ModelDataEntry(): Sequence(NONE) {}
};


// Separate model-data sequence lists for each collection ID,
// to speed up searching
static vector<ModelDataEntry> MdlList[NUMBER_OF_COLLECTIONS];

// Will look up both the model index and its sequence
struct ModelHashEntry
{
	int16 ModelIndex;
	int16 ModelSeqTabIndex;
};

// Model-data hash table for extra-fast searching:
static vector<ModelHashEntry> MdlHash[NUMBER_OF_COLLECTIONS];

// Hash-table size and function
const int MdlHashSize = 1 << 8;
const int MdlHashMask = MdlHashSize - 1;
inline uint8 MdlHashFunc(short Sequence)
{
	// E-Z
	return (uint8)(Sequence & MdlHashMask);
}


// Deletes a collection's model-data sequences
static void MdlDelete(short Collection)
{
	int c = Collection;
	MdlList[c].clear();
	MdlHash[c].clear();
}

// Deletes all of them
static void MdlDeleteAll()
{
	for (int c=0; c<NUMBER_OF_COLLECTIONS; c++) MdlDelete(c);
}


OGL_ModelData *OGL_GetModelData(short Collection, short Sequence, short& ModelSequence)
{
	// Model is neutral unless specified otherwise
	ModelSequence = NONE;
	
	// Initialize the hash table if necessary
	if (MdlHash[Collection].empty())
	{
		MdlHash[Collection].resize(MdlHashSize);
		objlist_set(&MdlHash[Collection][0],NONE,MdlHashSize);
	}
	
	// Set up a *reference* to the appropriate hashtable entry;
	// this makes setting this entry a bit more convenient
	ModelHashEntry& HashVal = MdlHash[Collection][MdlHashFunc(Sequence)];
	
	// Check to see if the model-data entry is correct;
	// if it is, then we're done.
	if (HashVal.ModelIndex != NONE)
	{
		// First, check in the sequence-map table
		vector<ModelDataEntry>::iterator MdlIter = MdlList[Collection].begin() + HashVal.ModelIndex;
		size_t MSTIndex = static_cast<size_t>(HashVal.ModelSeqTabIndex);  // Cast only safe b/c of following check
		if (MSTIndex < MdlIter->SequenceMap.size())
		{
			vector<SequenceMapEntry>::iterator SMIter = MdlIter->SequenceMap.begin() + MSTIndex;
			if (SMIter->Sequence == Sequence)
			{
				ModelSequence = SMIter->ModelSequence;
				return MdlIter->ModelData.ModelPresent() ? &MdlIter->ModelData : NULL;
			}
		}
		
		// Now check the neutral sequence
		if (MdlIter->Sequence == Sequence)
		{
			return MdlIter->ModelData.ModelPresent() ? &MdlIter->ModelData : NULL;
		}
	}
	
	// Fallback for the case of a hashtable miss;
	// do a linear search and then update the hash entry appropriately.
	vector<ModelDataEntry>& ML = MdlList[Collection];
	int16 Indx = 0;
	for (vector<ModelDataEntry>::iterator MdlIter = ML.begin(); MdlIter < ML.end(); MdlIter++, Indx++)
	{
		// First, search the sequence-map table
		int16 SMIndx = 0;
		vector<SequenceMapEntry>& SM = MdlIter->SequenceMap;
		for (vector<SequenceMapEntry>::iterator SMIter = SM.begin(); SMIter < SM.end(); SMIter++, SMIndx++)
		{
			if (SMIter->Sequence == Sequence)
			{
				HashVal.ModelIndex = Indx;
				HashVal.ModelSeqTabIndex = SMIndx;
				ModelSequence = SMIter->ModelSequence;
				return MdlIter->ModelData.ModelPresent() ? &MdlIter->ModelData : NULL;
			}
		}
		
		// Now check the neutral sequence
		if (MdlIter->Sequence == Sequence)
		{
			HashVal.ModelIndex = Indx;
			HashVal.ModelSeqTabIndex = NONE;
			return MdlIter->ModelData.ModelPresent() ? &MdlIter->ModelData : NULL;
		}
	}
	
	// None found!
	return NULL;
}

int OGL_SkinData::GetMaxSize()
{
	return Get_OGL_ConfigureData().ModelConfig.MaxSize;
}


// Any easy STL ways of doing this mapping of functions onto members of arrays?

void OGL_SkinManager::Load()
{
	for (vector<OGL_SkinData>::iterator SkinIter = SkinData.begin(); SkinIter < SkinData.end(); SkinIter++)
		SkinIter->Load();
}


void OGL_SkinManager::Unload()
{
	for (vector<OGL_SkinData>::iterator SkinIter = SkinData.begin(); SkinIter < SkinData.end(); SkinIter++)
		SkinIter->Unload();
}


void OGL_SkinManager::Reset(bool Clear_OGL_Txtrs)
{
	if (Clear_OGL_Txtrs)
	{
		for (int k=0; k<NUMBER_OF_OPENGL_BITMAP_SETS; k++)
			for (int l=0; l<NUMBER_OF_TEXTURES; l++)
			{
				if (IDsInUse[k][l])
					glDeleteTextures(1,&IDs[k][l]);
			}
	}
	
	// Mass clearing
	objlist_clear(IDsInUse[0],NUMBER_OF_OPENGL_BITMAP_SETS*NUMBER_OF_TEXTURES);
}


OGL_SkinData *OGL_SkinManager::GetSkin(short CLUT)
{
	for (unsigned k=0; k<SkinData.size(); k++)
	{
		OGL_SkinData& Skin = SkinData[k];
		if (Skin.CLUT == CLUT)
			return &Skin;
	}

	if (IsInfravisionTable(CLUT) && CLUT != INFRAVISION_BITMAP_SET)
	{
        for (unsigned k=0; k<SkinData.size(); k++)
        {
            OGL_SkinData& Skin = SkinData[k];
            if (Skin.CLUT == INFRAVISION_BITMAP_SET)
                return &Skin;
        }
    }
    
	if (IsSilhouetteTable(CLUT) && CLUT != SILHOUETTE_BITMAP_SET)
	{
        for (unsigned k=0; k<SkinData.size(); k++)
        {
            OGL_SkinData& Skin = SkinData[k];
            if (Skin.CLUT == SILHOUETTE_BITMAP_SET)
                return &Skin;
        }
    }

    for (unsigned k=0; k<SkinData.size(); k++)
	{
		OGL_SkinData& Skin = SkinData[k];
		if (Skin.CLUT == ALL_CLUTS)
			return &Skin;
	}

	return NULL;
}


bool OGL_SkinManager::Use(short CLUT, short Which)
{
	// References so they can be written into
	GLuint& TxtrID = IDs[CLUT][Which];
	bool& InUse = IDsInUse[CLUT][Which];
	bool LoadSkin = false;
	if (!InUse)
	{
		glGenTextures(1,&TxtrID);
		InUse = true;
		LoadSkin = true;
	}
	glBindTexture(GL_TEXTURE_2D,TxtrID);
	return LoadSkin;
}


// Circle constants
const double TWO_PI = 8*atan(1.0);
const double Degree2Radian = TWO_PI/360;			// A circle is 2*pi radians

// Matrix manipulation (can't trust OpenGL to be active, so won't be using OpenGL matrix stuff)

// Src -> Dest
static void MatCopy(const GLfloat SrcMat[3][3], GLfloat DestMat[3][3])
{
	objlist_copy(DestMat[0],SrcMat[0],9);
}

// Sets the arg to that matrix
static void MatIdentity(GLfloat Mat[3][3])
{
	const GLfloat IMat[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
	MatCopy(IMat,Mat);
}

// Src1, Src2 -> Dest (cannot be one of the sources)
static void MatMult(const GLfloat Mat1[3][3], const GLfloat Mat2[3][3], GLfloat DestMat[3][3])
{
	for (int k=0; k<3; k++)
		for (int l=0; l<3; l++)
		{
			GLfloat Sum = 0;
			for (int m=0; m<3; m++)
				Sum += Mat1[k][m]*Mat2[m][l];
			DestMat[k][l] = Sum;
		}
}

// Alters the matrix in place
static void MatScalMult(GLfloat Mat[3][3], const GLfloat Scale)
{
	for (int k=0; k<3; k++)
	{
		GLfloat *MatRow = Mat[k];
		for (int l=0; l<3; l++)
			MatRow[l] *= Scale;
	}
}

// Src -> Dest vector (cannot be the same location)
static void MatVecMult(const GLfloat Mat[3][3], const GLfloat *SrcVec, GLfloat *DestVec)
{
	for (int k=0; k<3; k++)
	{
		const GLfloat *MatRow = Mat[k];
		GLfloat Sum = 0;
		for (int l=0; l<3; l++)
			Sum += MatRow[l]*SrcVec[l];
		DestVec[k] = Sum;
	}
}


inline bool StringPresent(vector<char>& String)
{
	return (String.size() > 1);
}


void OGL_ModelData::Load()
{
	// Already loaded?
	if (ModelPresent()) return;
	
	// Load the model
	Model.Clear();

	if (ModelFile == FileSpecifier()) return;
	if (!ModelFile.Exists()) return;

	bool Success = false;
	
	char *Type = &ModelType[0];
	if (StringsEqual(Type,"wave",4))
	{
		// Alias|Wavefront, backward compatible version
		Success = LoadModel_Wavefront(ModelFile, Model);
	}
	else if (StringsEqual(Type,"obj",3))
	{
		// Alias|Wavefront, but with coordinate system conversion.
		Success = LoadModel_Wavefront_RightHand(ModelFile, Model);
	}
	else if (StringsEqual(Type,"3ds",3))
	{
		// 3D Studio Max, backward compatible version
		Success = LoadModel_Studio(ModelFile, Model);
	}
	else if (StringsEqual(Type,"max",3))
	{
		// 3D Studio Max, but with coordinate system conversion.
		Success = LoadModel_Studio_RightHand(ModelFile, Model);
	}
	else if (StringsEqual(Type,"dim3",4))
	{
		// Brian Barnes's "Dim3" model format (first pass: model geometry)
		Success = LoadModel_Dim3(ModelFile, Model, LoadModelDim3_First);
		
		// Second and third passes: frames and sequences
		try
		{
			if (ModelFile1 == FileSpecifier()) throw 0;
			if (!ModelFile1.Exists()) throw 0;
			if (!LoadModel_Dim3(ModelFile1, Model, LoadModelDim3_Rest)) throw 0;
		}
		catch(...)
		{}
		//
		try
		{
			if (ModelFile2 == FileSpecifier()) throw 0;
			if (!ModelFile2.Exists()) throw 0;
			if (!LoadModel_Dim3(ModelFile2, Model, LoadModelDim3_Rest)) throw 0;
		}
		catch(...)
		{}
	}
#if HAVE_QUESA
	else if (StringsEqual(Type,"qd3d") || StringsEqual(Type,"3dmf") || StringsEqual(Type,"quesa"))
	{
		// QuickDraw 3D / Quesa
		Success = LoadModel_QD3D(ModelFile, Model);
	}
#endif
	
	if (!Success)
	{
		Model.Clear();
		return;
	}
	
	// Calculate transformation matrix
	GLfloat Angle, Cosine, Sine;
	GLfloat RotMatrix[3][3], NewRotMatrix[3][3], IndivRotMatrix[3][3];
	MatIdentity(RotMatrix);
	
	MatIdentity(IndivRotMatrix);
	Angle = (float)Degree2Radian*XRot;
	Cosine = (float)cos(Angle);
	Sine = (float)sin(Angle);
	IndivRotMatrix[1][1] = Cosine;
	IndivRotMatrix[1][2] = - Sine;
	IndivRotMatrix[2][1] = Sine;
	IndivRotMatrix[2][2] = Cosine;
	MatMult(IndivRotMatrix,RotMatrix,NewRotMatrix);
	MatCopy(NewRotMatrix,RotMatrix);
	
	MatIdentity(IndivRotMatrix);
	Angle = (float)Degree2Radian*YRot;
	Cosine = (float)cos(Angle);
	Sine = (float)sin(Angle);
	IndivRotMatrix[2][2] = Cosine;
	IndivRotMatrix[2][0] = - Sine;
	IndivRotMatrix[0][2] = Sine;
	IndivRotMatrix[0][0] = Cosine;
	MatMult(IndivRotMatrix,RotMatrix,NewRotMatrix);
	MatCopy(NewRotMatrix,RotMatrix);
	
	MatIdentity(IndivRotMatrix);
	Angle = (float)Degree2Radian*ZRot;
	Cosine = (float)cos(Angle);
	Sine = (float)sin(Angle);
	IndivRotMatrix[0][0] = Cosine;
	IndivRotMatrix[0][1] = - Sine;
	IndivRotMatrix[1][0] = Sine;
	IndivRotMatrix[1][1] = Cosine;
	MatMult(IndivRotMatrix,RotMatrix,NewRotMatrix);
	MatCopy(NewRotMatrix,RotMatrix);
	
	MatScalMult(NewRotMatrix,Scale);			// For the position vertices
	if (Scale < 0) MatScalMult(RotMatrix,-1);	// For the normals
	
	// Is model animated or static?
	// Test by trying to find neutral positions (useful for working with the normals later on)
	if (Model.FindPositions_Neutral(false))
	{
		// Copy over the vector and normal transformation matrices:
		for (int k=0; k<3; k++)
			for (int l=0; l<3; l++)
			{
				Model.TransformPos.M[k][l] = NewRotMatrix[k][l];
				Model.TransformNorm.M[k][l] = RotMatrix[k][l];
			}
		
		Model.TransformPos.M[0][3] = XShift;
		Model.TransformPos.M[1][3] = YShift;
		Model.TransformPos.M[2][3] = ZShift;
		
		// Find the transformed bounding box:
		bool RestOfCorners = false;
		GLfloat NewBoundingBox[2][3];
		// The indices i1, i2, and i3 are for selecting which of the box's two principal corners
		// to get coordinates from
		for (int i1=0; i1<2; i1++)
		{
			GLfloat X = Model.BoundingBox[i1][0];
			for (int i2=0; i2<2; i2++)
			{
				GLfloat Y = Model.BoundingBox[i2][0];
				for (int i3=0; i3<2; i3++)
				{
					GLfloat Z = Model.BoundingBox[i3][0];
					
					GLfloat Corner[3];
					for (int ic=0; ic<3; ic++)
					{
						GLfloat *Row = Model.TransformPos.M[ic];
						Corner[ic] = Row[0]*X + Row[1]*Y + Row[2]*Z + Row[3];
					}
					
					if (RestOfCorners)
					{
						// Find minimum and maximum for each coordinate
						for (int ic=0; ic<3; ic++)
						{
							NewBoundingBox[0][ic] = min(NewBoundingBox[0][ic],Corner[ic]);
							NewBoundingBox[1][ic] = max(NewBoundingBox[1][ic],Corner[ic]);
						}
					}
					else
					{
						// Simply copy it in:
						for (int ic=0; ic<3; ic++)
							NewBoundingBox[0][ic] = NewBoundingBox[1][ic] = Corner[ic];
						RestOfCorners = true;
					}
				}
			}
		}
		
		for (int ic=0; ic<2; ic++)
			objlist_copy(Model.BoundingBox[ic],NewBoundingBox[ic],3);
	}
	else
	{
		// Static model
		size_t NumVerts = Model.Positions.size()/3;
		
		for (size_t k=0; k<NumVerts; k++)
		{
			GLfloat *Pos = Model.PosBase() + 3*k;
			GLfloat NewPos[3];
			MatVecMult(NewRotMatrix,Pos,NewPos);	// Has the scaling
			Pos[0] = NewPos[0] + XShift;
			Pos[1] = NewPos[1] + YShift;
			Pos[2] = NewPos[2] + ZShift;
		}
		
		size_t NumNorms = Model.Normals.size()/3;
		for (size_t k=0; k<NumNorms; k++)
		{
			GLfloat *Norms = Model.NormBase() + 3*k;
			GLfloat NewNorms[3];
			MatVecMult(RotMatrix,Norms,NewNorms);	// Not scaled
			objlist_copy(Norms,NewNorms,3);
		}	
	
		// So as to be consistent with the new points
		Model.FindBoundingBox();
	}	
	
	Model.AdjustNormals(NormalType,NormalSplit);
	Model.CalculateTangents();
	
	// Don't forget the skins
	OGL_SkinManager::Load();
}


void OGL_ModelData::Unload()
{
	Model.Clear();
	OGL_ResetForceSpriteDepth();
	
	// Don't forget the skins
	OGL_SkinManager::Unload();
}

int OGL_CountModels(short Collection)
{
	return MdlList[Collection].size();
}

extern void OGL_ProgressCallback(int);

static bool ForcingSpriteDepth = false;
void OGL_ResetForceSpriteDepth() { ForcingSpriteDepth = false; }
bool OGL_ForceSpriteDepth() { return ForcingSpriteDepth; }

// for managing the model and image loading and unloading
void OGL_LoadModels(short Collection)
{
	vector<ModelDataEntry>& ML = MdlList[Collection];
	for (vector<ModelDataEntry>::iterator MdlIter = ML.begin(); MdlIter < ML.end(); MdlIter++)
	{
		MdlIter->ModelData.Load();
		if (MdlIter->ModelData.ForceSpriteDepth)
		{
			ForcingSpriteDepth = true;
		}
		OGL_ProgressCallback(1);
	}
}

void OGL_UnloadModels(short Collection)
{
	vector<ModelDataEntry>& ML = MdlList[Collection];
	for (vector<ModelDataEntry>::iterator MdlIter = ML.begin(); MdlIter < ML.end(); MdlIter++)
	{
		MdlIter->ModelData.Unload();
	}
}


// Reset model skins; used in OGL_ResetTextures() in OGL_Textures.cpp
void OGL_ResetModelSkins(bool Clear_OGL_Txtrs)
{
	for (int ic=0; ic<MAXIMUM_COLLECTIONS; ic++)
	{
		vector<ModelDataEntry>& ML = MdlList[ic];
		for (vector<ModelDataEntry>::iterator MdlIter = ML.begin(); MdlIter < ML.end(); MdlIter++)
		{
			MdlIter->ModelData.Reset(Clear_OGL_Txtrs);
		}
	}
}

class XML_SkinDataParser: public XML_ElementParser
{
	short CLUT_Variant;
	
	OGL_SkinData Data;
public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	vector<OGL_SkinData> *SkinDataPtr;
	
	XML_SkinDataParser(): XML_ElementParser("skin"), SkinDataPtr(NULL) {}
};

bool XML_SkinDataParser::Start()
{
	Data = DefaultSkinData;
    CLUT_Variant = CLUT_VARIANT_NORMAL;
	
	return true;
}

bool XML_SkinDataParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"clut"))
	{
		return ReadBoundedInt16Value(Value,Data.CLUT,short(ALL_CLUTS),short(SILHOUETTE_BITMAP_SET));
	}
	else if (StringsEqual(Tag,"clut_variant"))
	{
		return ReadBoundedInt16Value(Value,CLUT_Variant,short(ALL_CLUT_VARIANTS),short(NUMBER_OF_CLUT_VARIANTS)-1);
	}
	else if (StringsEqual(Tag,"opac_type"))
	{
		return ReadBoundedInt16Value(Value,Data.OpacityType,0,OGL_NUMBER_OF_OPACITY_TYPES-1);
	}
	else if (StringsEqual(Tag,"opac_scale"))
	{
		return ReadFloatValue(Value,Data.OpacityScale);
	}
	else if (StringsEqual(Tag,"opac_shift"))
	{
		return ReadFloatValue(Value,Data.OpacityShift);
	}
	else if (StringsEqual(Tag,"normal_image"))
	{
		Data.NormalColors.SetNameWithPath(Value);
		return true;
	}
	else if (StringsEqual(Tag,"offset_image"))
	{
		Data.OffsetMap.SetNameWithPath(Value);
		return true;
	}
	else if (StringsEqual(Tag,"normal_mask"))
	{
		Data.NormalMask.SetNameWithPath(Value);
		return true;
	}
	else if (StringsEqual(Tag,"glow_image"))
	{
		Data.GlowColors.SetNameWithPath(Value);
		return true;
	}
	else if (StringsEqual(Tag,"glow_mask"))
	{
		Data.GlowMask.SetNameWithPath(Value);
		return true;
	}
	else if (StringsEqual(Tag,"normal_blend"))
	{
		return ReadBoundedInt16Value(Value,Data.NormalBlend,0,OGL_NUMBER_OF_BLEND_TYPES-1);
	}
	else if (StringsEqual(Tag,"glow_blend"))
	{
		return ReadBoundedInt16Value(Value,Data.GlowBlend,0,OGL_NUMBER_OF_BLEND_TYPES-1);
	}
	else if (StringsEqual(Tag,"normal_bloom_scale"))
	{
		return ReadFloatValue(Value,Data.BloomScale);
	}
	else if (StringsEqual(Tag,"normal_bloom_shift"))
	{
		return ReadFloatValue(Value,Data.BloomShift);
	}
	else if (StringsEqual(Tag,"glow_bloom_scale"))
	{
		return ReadFloatValue(Value,Data.GlowBloomScale);
	}
	else if (StringsEqual(Tag,"glow_bloom_shift"))
	{
		return ReadFloatValue(Value,Data.GlowBloomShift);
	}
	else if (StringsEqual(Tag,"minimum_glow_intensity"))
	{
		return ReadFloatValue(Value,Data.MinGlowIntensity);
	}
	UnrecognizedTag();
	return false;
}

bool XML_SkinDataParser::AttributesDone()
{
	assert(SkinDataPtr);
	vector<OGL_SkinData>& SkinData = *SkinDataPtr;
    
    short CLUT = Data.CLUT;
    // translate deprecated clut options
	if (CLUT == INFRAVISION_BITMAP_SET)
	{
		CLUT = ALL_CLUTS;
		CLUT_Variant = CLUT_VARIANT_INFRAVISION;
	}
	else if (CLUT == SILHOUETTE_BITMAP_SET)
	{
		CLUT = ALL_CLUTS;
		CLUT_Variant = CLUT_VARIANT_SILHOUETTE;
	}
	
	// loop so we can apply "all variants" mode if needed
	for (short var = CLUT_VARIANT_NORMAL; var < NUMBER_OF_CLUT_VARIANTS; var++)
	{
		if (CLUT_Variant != ALL_CLUT_VARIANTS && CLUT_Variant != var)
			continue;
		
		// translate clut+variant to internal clut number
		short actual_clut = CLUT;
		if (var == CLUT_VARIANT_INFRAVISION)
		{
			if (CLUT == ALL_CLUTS)
				actual_clut = INFRAVISION_BITMAP_SET;
			else
				actual_clut = INFRAVISION_BITMAP_CLUTSPECIFIC + CLUT;
		}
		else if (var == CLUT_VARIANT_SILHOUETTE)
		{
			if (CLUT == ALL_CLUTS)
				actual_clut = SILHOUETTE_BITMAP_SET;
			else
				actual_clut = SILHOUETTE_BITMAP_CLUTSPECIFIC + CLUT;
		}
        
        Data.CLUT = actual_clut;

        // Check to see if a frame is already accounted for
        bool found = false;
        for (vector<OGL_SkinData>::iterator SDIter = SkinData.begin(); SDIter < SkinData.end(); SDIter++)
        {
            if (SDIter->CLUT == Data.CLUT)
            {
                // Replace the data
                *SDIter = Data;
                found = true;
                break;
            }
        }
        if (!found)
        {
            // If not, then add a new frame entry
            SkinData.push_back(Data);
        }
    }
	
	return true;
}

static XML_SkinDataParser SkinDataParser;


// For mapping Marathon-engine sequences onto model sequences
class XML_SequenceMapParser: public XML_ElementParser
{
	bool SeqIsPresent, ModelSeqIsPresent;
	SequenceMapEntry Data;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	vector<SequenceMapEntry> *SeqMapPtr;

	XML_SequenceMapParser(): XML_ElementParser("seq_map") {}
};

bool XML_SequenceMapParser::Start()
{
	Data.Sequence = Data.ModelSequence = NONE;
	SeqIsPresent = ModelSeqIsPresent = false;		
	return true;
}

bool XML_SequenceMapParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"seq"))
	{
		if (ReadBoundedInt16Value(Value,Data.Sequence,0,MAXIMUM_SHAPES_PER_COLLECTION-1))
		{
			SeqIsPresent = true;
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"model_seq"))
	{
		if (ReadBoundedInt16Value(Value,Data.ModelSequence,NONE,MAXIMUM_SHAPES_PER_COLLECTION-1))
		{
			ModelSeqIsPresent = true;
			return true;
		}
		else return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_SequenceMapParser::AttributesDone()
{
	// Verify...
	if (!SeqIsPresent || !ModelSeqIsPresent)
	{
		AttribsMissing();
		return false;
	}
	
	// Add the entry
	vector<SequenceMapEntry>& SeqMap = *SeqMapPtr;
	SeqMap.push_back(Data);
	return true;
}

static XML_SequenceMapParser SequenceMapParser;


class XML_MdlClearParser: public XML_ElementParser
{
	bool IsPresent;
	short Collection;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();

	XML_MdlClearParser(): XML_ElementParser("model_clear") {}
};

bool XML_MdlClearParser::Start()
{
	IsPresent = false;
	return true;
}

bool XML_MdlClearParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"coll"))
	{
		if (ReadBoundedInt16Value(Value,Collection,0,NUMBER_OF_COLLECTIONS-1))
		{
			IsPresent = true;
			return true;
		}
		else return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_MdlClearParser::AttributesDone()
{
	if (IsPresent)
		MdlDelete(Collection);
	else
		MdlDeleteAll();
	
	return true;
}

static XML_MdlClearParser Mdl_ClearParser;


class XML_ModelDataParser: public XML_ElementParser
{
	bool CollIsPresent;
	short Collection, Sequence;
	vector<SequenceMapEntry> SequenceMap;
	
	OGL_ModelData Data;

public:
	bool Start();
	bool ReadSignValue(const char *String, int16& Value);
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	bool ResetValues();
	bool End();
	
	XML_ModelDataParser(): XML_ElementParser("model") {}
};

bool XML_ModelDataParser::Start()
{
	Data = DefaultModelData;
	CollIsPresent = false;
	Sequence = NONE;
	SequenceMap.clear();
	
	// For doing the model skins
	SkinDataParser.SkinDataPtr = &Data.SkinData;
	
	// For doing the sequence mapping
	SequenceMapParser.SeqMapPtr = &SequenceMap;
		
	return true;
}

bool XML_ModelDataParser::ReadSignValue(const char *String, int16& Value)
{
	if (StringsEqual(String, "+"))
	{
		Value = 1;
		return true;
	}
	else if (StringsEqual(String, "-"))
	{
		Value = -1;
		return true;
	}
	return ReadInt16Value(String, Value);
}

bool XML_ModelDataParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"coll"))
	{
		if (ReadBoundedInt16Value(Value,Collection,0,NUMBER_OF_COLLECTIONS-1))
		{
			CollIsPresent = true;
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"seq"))
	{
		return (ReadBoundedInt16Value(Value,Sequence,0,MAXIMUM_SHAPES_PER_COLLECTION-1));
	}
	else if (StringsEqual(Tag,"scale"))
	{
		return ReadFloatValue(Value,Data.Scale);
	}
	else if (StringsEqual(Tag,"x_rot"))
	{
		return ReadFloatValue(Value,Data.XRot);
	}
	else if (StringsEqual(Tag,"y_rot"))
	{
		return ReadFloatValue(Value,Data.YRot);
	}
	else if (StringsEqual(Tag,"z_rot"))
	{
		return ReadFloatValue(Value,Data.ZRot);
	}
	else if (StringsEqual(Tag,"x_shift"))
	{
		return ReadFloatValue(Value,Data.XShift);
	}
	else if (StringsEqual(Tag,"y_shift"))
	{
		return ReadFloatValue(Value,Data.YShift);
	}
	else if (StringsEqual(Tag,"z_shift"))
	{
		return ReadFloatValue(Value,Data.ZShift);
	}
	else if (StringsEqual(Tag,"side"))
	{
		return ReadSignValue(Value,Data.Sidedness);
	}
	else if (StringsEqual(Tag,"norm_type"))
	{
		return ReadBoundedInt16Value(Value,Data.NormalType,0,Model3D::NUMBER_OF_NORMAL_TYPES-1);
	}
	else if (StringsEqual(Tag,"norm_split"))
	{
		return ReadFloatValue(Value,Data.NormalSplit);
	}
	else if (StringsEqual(Tag,"light_type"))
	{
		return ReadBoundedInt16Value(Value,Data.LightType,0,NUMBER_OF_MODEL_LIGHT_TYPES-1);
	}
	else if (StringsEqual(Tag,"depth_type"))
	{
		return ReadSignValue(Value,Data.DepthType);
	}
	else if (StringsEqual(Tag,"force_sprite_depth"))
	{
		return ReadBooleanValue(Value,Data.ForceSpriteDepth);
	}
	else if (StringsEqual(Tag,"file"))
	{
		Data.ModelFile.SetNameWithPath(Value);
		return true;
	}
	else if (StringsEqual(Tag,"file1"))
	{
		Data.ModelFile1.SetNameWithPath(Value);
		return true;
	}
	else if (StringsEqual(Tag,"file2"))
	{
		Data.ModelFile2.SetNameWithPath(Value);
		return true;
	}
	else if (StringsEqual(Tag,"type"))
	{
		size_t nchars = strlen(Value)+1;
		Data.ModelType.resize(nchars);
		memcpy(&Data.ModelType[0],Value,nchars);
		return true;
	}
	UnrecognizedTag();
	return false;
}

bool XML_ModelDataParser::AttributesDone()
{
	// Verify...
	if (!CollIsPresent)
	{
		AttribsMissing();
		return false;
	}
	return true;
}

bool XML_ModelDataParser::ResetValues()
{
	MdlDeleteAll();
	return true;
}

bool XML_ModelDataParser::End()
{
	// Do this at the end because the model data will then include the skin data
	
	// Check to see if a frame is already accounted for
	vector<ModelDataEntry>& ML = MdlList[Collection];
	for (vector<ModelDataEntry>::iterator MdlIter = ML.begin(); MdlIter < ML.end(); MdlIter++)
	{
		// Run a gauntlet of equality tests
		if (MdlIter->Sequence != Sequence) continue;
		
		if (MdlIter->SequenceMap.size() != SequenceMap.size()) continue;
		
		// Ought to sort, then compare, for correct results
		bool AllEqual = true;
		for (size_t q=0; q<SequenceMap.size(); q++)
		{
			SequenceMapEntry& MS = MdlIter->SequenceMap[q];
			SequenceMapEntry& S = SequenceMap[q];
			if (MS.Sequence != S.Sequence || MS.ModelSequence != S.ModelSequence)
			{
				AllEqual = false;
				break;
			}
		}
		if (!AllEqual) continue;
		
		// Replace the data; it passed the tests
		MdlIter->ModelData = Data;
		return true;
	}
	
	// If not, then add a new frame entry
	ModelDataEntry DataEntry;
	DataEntry.Sequence = Sequence;
	DataEntry.SequenceMap.swap(SequenceMap);	// Quick transfer of contents in desired direction
	DataEntry.ModelData = Data;
	ML.push_back(DataEntry);
	
	return true;
}

static XML_ModelDataParser ModelDataParser;


// XML-parser support:
XML_ElementParser *ModelData_GetParser()
{
	ModelDataParser.AddChild(&SkinDataParser);
	ModelDataParser.AddChild(&SequenceMapParser);
	
	return &ModelDataParser;
}
XML_ElementParser *Mdl_Clear_GetParser() {return &Mdl_ClearParser;}

void reset_mml_opengl_model()
{
	MdlDeleteAll();
}

void parse_mml_opengl_model_clear(const InfoTree& root)
{
	int16 coll;
	if (root.read_indexed("coll", coll, NUMBER_OF_COLLECTIONS))
		MdlDelete(coll);
	else
		MdlDeleteAll();
}

static bool read_sign_val(const InfoTree& root, std::string key, int16& val)
{
	std::string sign;
	if (!root.read_attr(key, sign))
		return false;
	if (sign == "+") {
		val = 1;
		return true;
	} else if (sign == "-") {
		val = -1;
		return true;
	} else if (sign == "0") {
		val = 0;
		return true;
	}
	return root.read_attr(key, val);
}

void parse_mml_opengl_model(const InfoTree& root)
{
	int16 coll;
	if (!root.read_indexed("coll", coll, NUMBER_OF_COLLECTIONS))
		return;
	
	ModelDataEntry entry;
	entry.ModelData = DefaultModelData;
	entry.Sequence = NONE;
	entry.SequenceMap.clear();
	
	root.read_indexed("seq", entry.Sequence, MAXIMUM_SHAPES_PER_COLLECTION);
		
	OGL_ModelData& def = entry.ModelData;
	root.read_attr("scale", def.Scale);
	root.read_attr("x_rot", def.XRot);
	root.read_attr("y_rot", def.YRot);
	root.read_attr("z_rot", def.ZRot);
	root.read_attr("x_shift", def.XShift);
	root.read_attr("y_shift", def.YShift);
	root.read_attr("z_shift", def.ZShift);
	read_sign_val(root, "side", def.Sidedness);
	root.read_indexed("norm_type", def.NormalType, Model3D::NUMBER_OF_NORMAL_TYPES);
	root.read_attr("norm_split", def.NormalSplit);
	root.read_indexed("light_type", def.LightType, NUMBER_OF_MODEL_LIGHT_TYPES);
	read_sign_val(root, "depth_type", def.DepthType);
	root.read_attr("force_sprite_depth", def.ForceSpriteDepth);
	root.read_path("file", def.ModelFile);
	root.read_path("file1", def.ModelFile1);
	root.read_path("file2", def.ModelFile2);
	
	std::string mtype;
	if (root.read_attr("type", mtype))
	{
		def.ModelType.assign(mtype.begin(), mtype.end());
		def.ModelType.push_back('\0');
	}
	
	BOOST_FOREACH(InfoTree seqmap, root.children_named("seq_map"))
	{
		SequenceMapEntry e;
		if (!seqmap.read_indexed("seq", e.Sequence, MAXIMUM_SHAPES_PER_COLLECTION))
			continue;
		if (!seqmap.read_indexed("model_seq", e.ModelSequence, MAXIMUM_SHAPES_PER_COLLECTION, true))
			continue;
		entry.SequenceMap.push_back(e);
	}
	
	BOOST_FOREACH(InfoTree skin, root.children_named("skin"))
	{
		int16 clut = ALL_CLUTS;
		skin.read_attr_bounded<int16>("clut", clut, ALL_CLUTS, SILHOUETTE_BITMAP_SET);
		
		int16 clut_variant = CLUT_VARIANT_NORMAL;
		skin.read_attr_bounded<int16>("clut_variant", clut_variant, ALL_CLUT_VARIANTS, NUMBER_OF_CLUT_VARIANTS-1);
		
		OGL_SkinData sdef = DefaultSkinData;
		sdef.CLUT = clut;
		skin.read_indexed("opac_type", sdef.OpacityType, OGL_NUMBER_OF_OPACITY_TYPES);
		skin.read_attr("opac_scale", sdef.OpacityScale);
		skin.read_attr("opac_shift", sdef.OpacityShift);
		skin.read_path("normal_image", sdef.NormalColors);
		skin.read_path("offset_image", sdef.OffsetMap);
		skin.read_path("normal_mask", sdef.NormalMask);
		skin.read_path("glow_image", sdef.GlowColors);
		skin.read_path("glow_mask", sdef.GlowMask);
		skin.read_indexed("normal_blend", sdef.NormalBlend, OGL_NUMBER_OF_BLEND_TYPES);
		skin.read_indexed("glow_blend", sdef.GlowBlend, OGL_NUMBER_OF_BLEND_TYPES);
		skin.read_attr("normal_bloom_scale", sdef.BloomScale);
		skin.read_attr("normal_bloom_shift", sdef.BloomShift);
		skin.read_attr("glow_bloom_scale", sdef.GlowBloomScale);
		skin.read_attr("glow_bloom_shift", sdef.GlowBloomShift);
		skin.read_attr("minimum_glow_intensity", sdef.MinGlowIntensity);
		
		// translate deprecated clut options
		if (clut == INFRAVISION_BITMAP_SET)
		{
			clut = ALL_CLUTS;
			clut_variant = CLUT_VARIANT_INFRAVISION;
		}
		else if (clut == SILHOUETTE_BITMAP_SET)
		{
			clut = ALL_CLUTS;
			clut_variant = CLUT_VARIANT_SILHOUETTE;
		}
		
		// loop so we can apply "all variants" mode if needed
		for (short var = CLUT_VARIANT_NORMAL; var < NUMBER_OF_CLUT_VARIANTS; var++)
		{
			if (clut_variant != ALL_CLUT_VARIANTS && clut_variant != var)
				continue;
			
			// translate clut+variant to internal clut number
			short actual_clut = clut;
			if (var == CLUT_VARIANT_INFRAVISION)
			{
				if (clut == ALL_CLUTS)
					actual_clut = INFRAVISION_BITMAP_SET;
				else
					actual_clut = INFRAVISION_BITMAP_CLUTSPECIFIC + clut;
			}
			else if (var == CLUT_VARIANT_SILHOUETTE)
			{
				if (clut == ALL_CLUTS)
					actual_clut = SILHOUETTE_BITMAP_SET;
				else
					actual_clut = SILHOUETTE_BITMAP_CLUTSPECIFIC + clut;
			}
			
			bool found = false;
			for (vector<OGL_SkinData>::iterator it = def.SkinData.begin(); it != def.SkinData.end(); ++it)
			{
				if (it->CLUT == clut)
				{
					*it = sdef;
					found = true;
					break;
				}
			}
			if (!found)
			{
				def.SkinData.push_back(sdef);
			}
		}
	}
	
	// Add parsed model to list
	// Check to see if a frame is already accounted for
	bool found = false;
	vector<ModelDataEntry>& ML = MdlList[coll];
	for (vector<ModelDataEntry>::iterator MdlIter = ML.begin(); MdlIter < ML.end(); MdlIter++)
	{
		// Sequence and map must match
		if (MdlIter->Sequence != entry.Sequence) continue;
		if (MdlIter->SequenceMap.size() != entry.SequenceMap.size()) continue;
		
		// Sort and compare sequence maps
		std::vector<SequenceMapEntry> omap = MdlIter->SequenceMap;
		std::vector<SequenceMapEntry> nmap = entry.SequenceMap;
		std::sort(omap.begin(), omap.end());
		std::sort(nmap.begin(), nmap.end());
		if (omap != nmap) continue;
		
		// Replace the data; it passed the tests
		MdlIter->ModelData = def;
		found = true;
		break;
	}
	
	// If not, then add a new frame entry
	if (!found)
	{
		ML.push_back(entry);
	}
}

#endif
