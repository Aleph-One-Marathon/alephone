/*
	February 21, 2000 (Loren Petrich)
	
	Animated-textures file. This implements animated wall textures,
	which are now read off of XML configuration files.
	
	May 12, 2000 (Loren Petrich)
	
	Modified to abolish the ATxr resources, and also to add XML support.
	A "select" feature was added, so as to be able to translate only some specified texture.
*/

#include "cseries.h"
#include "AnimatedTextures.h"
#include "interface.h"
#include "GrowableList.h"
#include <string.h>


class AnimTxtr
{
	// Frame data: private, because the frame list is only supposed
	int NumFrames;
	short *FrameList;
	
	// Control info: private, because they need to stay self-consistent
	// How long the animator stays on a frame;
	// negative means animate in reverse direction
	short NumTicks;
	// Phases: which frame, and which tick in a frame
	// These must be in ranges (0 to NumFrames - 1) and (0 to abs(NumTicks) - 1)
	short FramePhase, TickPhase;

public:
	// How many frames
	int GetNumFrames() {return NumFrames;}
		
	// Frame ID (writable)
	short& GetFrame(int Index) {return FrameList[Index];}
	
	// Clear the list
	void Clear();
		
	// Load from list:
	void Load(int _NumFrames, short *_FrameList);
	
	// Translate the frame; indicate whether the frame was translated.
	// Its argument is the frame ID, which gets changed in place
	bool Translate(short& Frame);
	
	// Set the timing info: number of ticks per frame, and tick and frame phases
	void SetTiming(short _NumTicks, short _FramePhase, short _TickPhase);
	
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
	
	// Next member of linked list
	AnimTxtr *Next;
	
	// Constructor has defaults of everything
	AnimTxtr(): NumFrames(0), FrameList(0), Next(0)
	{
		NumTicks = 30;	// Once a second
		FramePhase = 0;
		TickPhase = 0;
		Select = -1;
	}
	~AnimTxtr() {if (FrameList) delete[]FrameList;}
};


void AnimTxtr::Clear()
{
	if (NumFrames)
	{
		NumFrames = 0;
		delete []FrameList;
		FrameList = 0;
	}
}


void AnimTxtr::Load(int _NumFrames, short *_FrameList)
{
	Clear();
	if (_NumFrames <= 0) return;
	NumFrames = _NumFrames;
	
	FrameList = new short[NumFrames];
	for (int f=0; f<NumFrames; f++)
		FrameList[f] = _FrameList[f];
	
	// Just in case it was set to something out-of-range...
	FramePhase = FramePhase % NumFrames;
}


bool AnimTxtr::Translate(short& Frame)
{
	// Sanity check
	if (NumFrames <= 0) return false;

	// Find the index in the loop; default: first one.
	short FrameIndx = 0;
	if (Select >= 0)
	{
		if (Frame != Select) return false;
	}
	else
	{
		bool FrameFound = false;
		for (int f=0; f<NumFrames; f++)
			if (FrameList[f] == Frame)
			{
				FrameFound = true;
				FrameIndx = f;
			}
		if (!FrameFound) return false;
	}	
	FrameIndx += FramePhase;
	while(FrameIndx >= NumFrames)
		FrameIndx -= NumFrames;
		
	Frame = FrameList[FrameIndx];
	return true;
}


void AnimTxtr::SetTiming(short _NumTicks, short _FramePhase, short _TickPhase)
{
	NumTicks = _NumTicks;
	FramePhase = _FramePhase;
	TickPhase = _TickPhase;
	
	// Correct for possible overshooting of limits:
	if (NumTicks)
	{
		short TickPhaseFrames = TickPhase / NumTicks;
		TickPhase -= NumTicks*TickPhaseFrames;
		if (TickPhase < 0)
		{
			TickPhaseFrames--;
			if (NumTicks > 0)
			{
				TickPhase += NumTicks;
			}
			else if (NumTicks < 0)
			{
				TickPhase -= NumTicks;
			}
		}
		FramePhase += TickPhaseFrames;
	}
	
	if (NumFrames > 0)
	{
		FramePhase = FramePhase % NumFrames;
		if (FramePhase < 0) FramePhase += NumFrames;
	}
}


void AnimTxtr::Update()
{
	// Be careful to wrap around in the appropriate direction
	if (NumTicks > 0)
	{
		if (++TickPhase >= NumTicks)
		{
			TickPhase = 0;
			if (++FramePhase >= NumFrames)
				FramePhase = 0;
		}
	}
	else if (NumTicks < 0)
	{
		if (--TickPhase < 0)
		{
			TickPhase = - NumTicks - 1;
			if (--FramePhase < 0)
				FramePhase = NumFrames - 1;
		}
	}
}


// Separate animated-texture sequence lists for each collection ID,
// to speed up searching
static AnimTxtr *ATRootList[NUMBER_OF_COLLECTIONS] =
{
	NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,
};


// Deletes a collection's animated-texture sequences
static void ATDelete(int c)
{
	AnimTxtr *ATPtr = ATRootList[c];
	while(ATPtr)
	{
		AnimTxtr *NextATPtr = ATPtr->Next;
		delete ATPtr;
		ATPtr = NextATPtr;
	}
	ATRootList[c] = NULL;
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
		AnimTxtr *ATPtr = ATRootList[c];
		while(ATPtr)
		{
			ATPtr->Update();
			ATPtr = ATPtr->Next;
		}
	}
}


// Does animated-texture translation in place
shape_descriptor AnimTxtr_Translate(shape_descriptor Texture)
{
	if (Texture == NONE) return NONE;
	
	// Pull out frame and collection ID's:
	short Frame = GET_DESCRIPTOR_SHAPE(Texture);
	short CollCT = GET_DESCRIPTOR_COLLECTION(Texture);
	short Collection = GET_COLLECTION(CollCT);
	short ColorTable = GET_COLLECTION_CLUT(CollCT);
	
	// This will assume that the collection is loaded;
	// that could be handled as map preprocessing, by turning
	// all shape descriptors that refer to unloaded shapes to NONE
	
	AnimTxtr *ATPtr = ATRootList[Collection];
	while(ATPtr)
	{
		if (ATPtr->Translate(Frame)) break;
		ATPtr = ATPtr->Next;
	}
		
	// Check the frame for being in range
	if (Frame < 0) return NONE;
	if (Frame >= get_number_of_collection_frames(Collection)) return NONE;
	
	// All done:
	CollCT = BUILD_COLLECTION(Collection,ColorTable);
	Texture = BUILD_DESCRIPTOR(Collection,Frame);
	return Texture;
}


// Temporary array for storing frames
// when doing XML parsing
static GrowableList<short> TempFrameList(16);


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
	if (strcmp(Tag,"coll") == 0)
	{
		if (ReadBoundedNumericalValue(Value,"%hd",Collection,short(0),short(NUMBER_OF_COLLECTIONS-1)))
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
	if (strcmp(Tag,"index") == 0)
	{
		if (ReadBoundedNumericalValue(Value,"%hd",Index,short(0),short(254)))
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
	TempFrameList.Add(Index);
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
	short FramePhase, TickPhase, Select;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	bool End();

	XML_AT_SequenceParser(): XML_ElementParser("sequence") {}
};

bool XML_AT_SequenceParser::Start()
{
	for (int k=0; k<NumberMandatory; k++)
		IsPresent[k] = false;
	FramePhase = TickPhase = 0;	// Default values
	Select = -1;
	TempFrameList.ResetLength();
	
	return true;
}

bool XML_AT_SequenceParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (strcmp(Tag,"coll") == 0)
	{
		if (ReadBoundedNumericalValue(Value,"%hd",Collection,short(0),short(NUMBER_OF_COLLECTIONS-1)))
		{
			IsPresent[0] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"numticks") == 0)
	{
		if (ReadNumericalValue(Value,"%hd",NumTicks))
		{
			IsPresent[1] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"framephase") == 0)
	{
		return (ReadNumericalValue(Value,"%hd",FramePhase));
	}
	else if (strcmp(Tag,"tickphase") == 0)
	{
		return (ReadNumericalValue(Value,"%hd",TickPhase));
	}
	else if (strcmp(Tag,"select") == 0)
	{
		return (ReadNumericalValue(Value,"%hd",Select));
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
	if (TempFrameList.GetLength() <= 0)
	{
		ErrorString = NoFramesSpecified;
		return false;
	}
	
	// Build a new sequence
	AnimTxtr *NewSeqPtr = new AnimTxtr;
	
	NewSeqPtr->Load(TempFrameList.GetLength(),TempFrameList.Begin());
	NewSeqPtr->SetTiming(NumTicks,FramePhase,TickPhase);
	NewSeqPtr->Select = Select;
	
	AnimTxtr *Current = ATRootList[Collection];
	if (Current)
	{
		// Search for the end and attach there
		while(Current->Next)
			Current = Current->Next;
		Current->Next = NewSeqPtr;
	}
	else
		ATRootList[Collection] = NewSeqPtr;
	
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
