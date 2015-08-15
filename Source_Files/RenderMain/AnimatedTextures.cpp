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
	
	February 21, 2000 (Loren Petrich)
	
	Animated-textures file. This implements animated wall textures,
	which are now read off of XML configuration files.
	
	May 12, 2000 (Loren Petrich)
	
	Modified to abolish the ATxr resources, and also to add XML support.
	A "select" feature was added, so as to be able to translate only some specified texture.

Oct 13, 2000 (Loren Petrich)
	Converted the animated-texture accounting into Standard Template Library vectors
*/

#include <vector>
#include <string.h>
#include "cseries.h"
#include "AnimatedTextures.h"
#include "interface.h"
#include "InfoTree.h"


class AnimTxtr
{
	// Frame data: private, because the frame list is only supposed
	vector<short> FrameList;
	
	// Control info: private, because they need to stay self-consistent
	// How long the animator stays on a frame;
	// negative means animate in reverse direction
	short NumTicks;
	
	// Phases: which frame, and which tick in a frame
	// These must be in ranges (0 to NumFrames - 1) and (0 to abs(NumTicks) - 1)
	size_t FramePhase, TickPhase;

public:
	// How many frames
	size_t GetNumFrames() {return FrameList.size();}
		
	// Frame ID (writable)
	short& GetFrame(size_t Index) {return FrameList[Index];}
	
	// Clear the list
	void Clear() {FrameList.clear();}
		
	// Load from list:
	void Load(vector<short>& _FrameList);
	
	// Translate the frame; indicate whether the frame was translated.
	// Its argument is the frame ID, which gets changed in place
	bool Translate(short& Frame);
	
	// Set the timing info: number of ticks per frame, and tick and frame phases
	void SetTiming(short _NumTicks, size_t _FramePhase, size_t _TickPhase);
	
	// Update the phases:
	void Update();
	
	// Which texture to select (if -1 [NONE], then select all those in the frame list);
	// the selected texture is the one that gets translated,
	// and if this is set, then it gets translated into the equivalent of the
	// first member of the loop.
	short Select;
	
	// Possible addition:
	// whether the texture is active or not;
	// one could include some extra data to indicate whether to activate
	// the texture or not, as when a switch tag changes state.
	
	// Constructor has defaults of everything
	AnimTxtr()
	{
		NumTicks = 30;	// Once a second
		FramePhase = 0;
		TickPhase = 0;
		Select = -1;
	}
};



void AnimTxtr::Load(vector<short>& _FrameList)
{
	// Quick way to transfer the frame data
	FrameList.swap(_FrameList);
	if (FrameList.empty()) return;
	
	// Just in case it was set to something out-of-range...
	FramePhase = FramePhase % FrameList.size();
}


bool AnimTxtr::Translate(short& Frame)
{
	// Sanity check
	size_t NumFrames = FrameList.size();
	if (NumFrames == 0) return false;

	// Find the index in the loop; default: first one.
	size_t FrameIndex = 0;
	if (Select >= 0)
	{
		if (Frame != Select) return false;
	}
	else
	{
		bool FrameFound = false;
		for (size_t f=0; f<NumFrames; f++)
			if (FrameList[f] == Frame)
			{
				FrameFound = true;
				FrameIndex = f;
			}
		if (!FrameFound) return false;
	}	
	FrameIndex += FramePhase;
	FrameIndex %= NumFrames;
		
	Frame = FrameList[FrameIndex];
	return true;
}


void AnimTxtr::SetTiming(short _NumTicks, size_t _FramePhase, size_t _TickPhase)
{
	NumTicks = _NumTicks;
	FramePhase = _FramePhase;
	TickPhase = _TickPhase;
	
	// Correct for possible overshooting of limits:
	if (NumTicks)
	{
		int TickPhaseFrames = static_cast<int>(TickPhase) / NumTicks;
		if (static_cast<int>(TickPhase) < NumTicks*TickPhaseFrames)
		{
			TickPhase += abs(NumTicks) - NumTicks*TickPhaseFrames;
			TickPhaseFrames--;
		}
		else
		{	TickPhase -= NumTicks*TickPhaseFrames; }

		FramePhase += TickPhaseFrames;
	}
	
	size_t NumFrames = FrameList.size();
	if (NumFrames != 0)
	{	FramePhase = FramePhase % NumFrames; }
}


void AnimTxtr::Update()
{
	// Be careful to wrap around in the appropriate direction
	size_t NumFrames = FrameList.size();
	if (NumTicks > 0)
	{
		if (static_cast<int>(++TickPhase) >= NumTicks)
		{
			TickPhase = 0;
			if (++FramePhase >= NumFrames)
				FramePhase = 0;
		}
	}
	else if (NumTicks < 0)
	{
		if (TickPhase == 0)
		{
			TickPhase = - NumTicks - 1;
			if (FramePhase != 0)
				FramePhase--;
			else
				FramePhase = NumFrames - 1;
		}
		else
		{	TickPhase--; }
	}
}


// Separate animated-texture sequence lists for each collection ID,
// to speed up searching
static vector<AnimTxtr> AnimTxtrList[NUMBER_OF_COLLECTIONS];


// Deletes a collection's animated-texture sequences
static void ATDelete(int c)
{
	AnimTxtrList[c].clear();
}


// Deletes all of them
static void ATDeleteAll()
{
	for (int c=0; c<NUMBER_OF_COLLECTIONS; c++) ATDelete(c);
}


// Updates the animated textures
void AnimTxtr_Update()
{
	for (int c=0; c<NUMBER_OF_COLLECTIONS; c++)
	{
		vector<AnimTxtr>& ATL = AnimTxtrList[c];
		for (vector<AnimTxtr>::iterator ATIter = ATL.begin(); ATIter < ATL.end(); ATIter++)
			ATIter->Update();
	}
}


// Does animated-texture translation in place
shape_descriptor AnimTxtr_Translate(shape_descriptor Texture)
{
	if (Texture == UNONE) return UNONE;
	
	// Pull out frame and collection ID's:
	short Frame = GET_DESCRIPTOR_SHAPE(Texture);
	short CollCT = GET_DESCRIPTOR_COLLECTION(Texture);
	short Collection = GET_COLLECTION(CollCT);
	short ColorTable = GET_COLLECTION_CLUT(CollCT);
	
	// This will assume that the collection is loaded;
	// that could be handled as map preprocessing, by turning
	// all shape descriptors that refer to unloaded shapes to NONE
	
	vector<AnimTxtr>& ATL = AnimTxtrList[Collection];
	for (vector<AnimTxtr>::iterator ATIter = ATL.begin(); ATIter < ATL.end(); ATIter++)
		if (ATIter->Translate(Frame)) break;
	
	// Check the frame for being in range
	if (Frame < 0) return UNONE;
	if (Frame >= get_number_of_collection_frames(Collection)) return UNONE;
	
	// All done:
	CollCT = BUILD_COLLECTION(Collection,ColorTable);
	Texture = BUILD_DESCRIPTOR(Collection,Frame);
	return Texture;
}


// Temporary array for storing frames
// when doing XML parsing
static vector<short> TempFrameList;


class XML_AT_ClearParser: public XML_ElementParser
{
	bool IsPresent;
	short Collection;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();

	XML_AT_ClearParser(): XML_ElementParser("clear") {}
};

bool XML_AT_ClearParser::Start()
{
	IsPresent = false;
	return true;
}

bool XML_AT_ClearParser::HandleAttribute(const char *Tag, const char *Value)
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

bool XML_AT_ClearParser::AttributesDone()
{
	if (IsPresent)
		ATDelete(Collection);
	else
		ATDeleteAll();
	
	return true;
}

static XML_AT_ClearParser AT_ClearParser;


class XML_AT_FrameParser: public XML_ElementParser
{
	bool IsPresent;
	short Index;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_AT_FrameParser(): XML_ElementParser("frame") {}
};

bool XML_AT_FrameParser::Start()
{
	IsPresent = false;
	return true;
}

bool XML_AT_FrameParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"index"))
	{
		if (ReadBoundedInt16Value(Value,Index,0,254))
		{
			IsPresent = true;
			return true;
		}
		else return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_AT_FrameParser::AttributesDone()
{
	// Verify...
	if (!IsPresent)
	{
		AttribsMissing();
		return false;
	}
	TempFrameList.push_back(Index);
	return true;
}

static XML_AT_FrameParser AT_FrameParser;


class XML_AT_SequenceParser: public XML_ElementParser
{
	// Mandatory attributes
	enum {NumberMandatory = 2};
	bool IsPresent[NumberMandatory];
	short Collection, NumTicks;
	
	// Optional Attributes
	uint16 FramePhase, TickPhase;
	short Select;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	bool ResetValues();
	bool End();

	XML_AT_SequenceParser(): XML_ElementParser("sequence") {}
};

bool XML_AT_SequenceParser::Start()
{
	for (int k=0; k<NumberMandatory; k++)
		IsPresent[k] = false;
	FramePhase = TickPhase = 0;	// Default values
	Select = -1;
	TempFrameList.clear();
	
	return true;
}

bool XML_AT_SequenceParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"coll"))
	{
		if (ReadBoundedInt16Value(Value,Collection,0,NUMBER_OF_COLLECTIONS-1))
		{
			IsPresent[0] = true;
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"numticks"))
	{
		if (ReadInt16Value(Value,NumTicks))
		{
			IsPresent[1] = true;
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"framephase"))
	{
		return ReadUInt16Value(Value,FramePhase);
	}
	else if (StringsEqual(Tag,"tickphase"))
	{
		return ReadUInt16Value(Value,TickPhase);
	}
	else if (StringsEqual(Tag,"select"))
	{
		return ReadInt16Value(Value,Select);
	}
	UnrecognizedTag();
	return false;
}

bool XML_AT_SequenceParser::AttributesDone()
{
	// Verify...
	bool AllPresent = true;
	for (int k=0; k<NumberMandatory; k++)
		AllPresent &= IsPresent[k];
	if (!AllPresent)
	{
		AttribsMissing();
		return false;
	}
	return true;
}

static char NoFramesSpecified[] = "no frames specified";

bool XML_AT_SequenceParser::End()
{
	// Verify...
	if (TempFrameList.size() <= 0)
	{
		ErrorString = NoFramesSpecified;
		return false;
	}
	
	// Build a new sequence
	AnimTxtr NewAnim;
	
	NewAnim.Load(TempFrameList);
	NewAnim.SetTiming(NumTicks,FramePhase,TickPhase);
	NewAnim.Select = Select;
	AnimTxtrList[Collection].push_back(NewAnim);
	
	return true;
}

bool XML_AT_SequenceParser::ResetValues()
{
	ATDeleteAll();
	return true;
}

static XML_AT_SequenceParser AT_SequenceParser;


static XML_ElementParser AnimatedTexturesParser("animated_textures");


// XML-parser support
XML_ElementParser *AnimatedTextures_GetParser()
{
	AT_SequenceParser.AddChild(&AT_FrameParser);
	AnimatedTexturesParser.AddChild(&AT_ClearParser);
	AnimatedTexturesParser.AddChild(&AT_SequenceParser);
	
	return &AnimatedTexturesParser;
}


void reset_mml_animated_textures()
{
	ATDeleteAll();
}

void parse_mml_animated_textures(const InfoTree& root)
{
	BOOST_FOREACH(const InfoTree::value_type &v, root)
	{
		std::string name = v.first;
		const InfoTree& child = v.second;
		
		if (v.first == "clear")
		{
			int16 coll = -1;
			if (child.read_indexed("coll", coll, NUMBER_OF_COLLECTIONS))
				ATDelete(coll);
			else
				ATDeleteAll();
		}
		else if (v.first == "sequence")
		{
			int16 coll = -1;
			int16 numticks = -1;
			if (!child.read_indexed("coll", coll, NUMBER_OF_COLLECTIONS) ||
				!child.read_attr("numticks", numticks))
				continue;
			
			vector<short> frames;
			BOOST_FOREACH(InfoTree frame, child.children_named("frame"))
			{
				int16 index = -1;
				if (frame.read_indexed("index", index, 255))
					frames.push_back(index);
			}
			if (!frames.size())
				continue;
			
			uint16 frame_phase = 0;
			child.read_attr("framephase", frame_phase);
			uint16 tick_phase = 0;
			child.read_attr("tickphase", tick_phase);
			int16 select = -1;
			child.read_attr("select", select);
			
			AnimTxtr new_anim;
			new_anim.Load(frames);
			new_anim.SetTiming(numticks, frame_phase, tick_phase);
			new_anim.Select = select;
			AnimTxtrList[coll].push_back(new_anim);
		}
	}
}

