/*
	May 22, 2000 (Loren Petrich)
	
	The work of the view controller.

Oct 13, 2000 (Loren Petrich)
	Using the STL for the landscape-option-data container

Nov 29, 2000 (Loren Petrich):
	Added making view-folding effect optional
	Added making teleport static/fold effect optional

Dec 17, 2000 (Loren Petrich:
	Added teleport-sound control for Marathon 1 compatibility
*/

#include <vector>
#include <string.h>
#include "cseries.h"
#include "world.h"
#include "mysound.h"
#include "ViewControl.h"


// Is the overhead map active?
static bool MapActive = true;

// Accessor:
bool View_MapActive() {return MapActive;}

// This frame value means that a landscape option will be applied to any frame in a collection:
const int AnyFrame = -1;

// Field-of-view stuff with defaults:
static float FOV_Normal = 80;
static float FOV_ExtraVision = 130;
static float FOV_TunnelVision = 30;
static float FOV_ChangeRate = 1.66666667;	// this is 50 degrees/s

// Defaults:
// do the view folding effect (stretch horizontally, squeeze vertically) when teleporting,
// also do the static effect / folding effect on viewed teleported objects
static bool DoFoldEffect = true;
static bool DoStaticEffect = true;

static FontSpecifier OnScreenFont = {"Monaco", 12, styleNormal, "#4"};
static bool ScreenFontInited = false;

// Accessors:
float View_FOV_Normal() {return FOV_Normal;}
float View_FOV_ExtraVision() {return FOV_ExtraVision;}
float View_FOV_TunnelVision() {return FOV_TunnelVision;}

FontSpecifier& GetOnScreenFont()
{
	// Init the font the first time through; accessor functions are very convenient :-)
	if (!ScreenFontInited)
	{
		OnScreenFont.Init();
		ScreenFontInited = true;
	}
	return OnScreenFont;
}

// Move field-of-view value closer to some target value:
bool View_AdjustFOV(float& FOV, float FOV_Target)
{
	bool Changed = false;
	if (FOV_ChangeRate < 0) FOV_ChangeRate *= -1;
	
	if (FOV > FOV_Target)
	{
		FOV -= FOV_ChangeRate;
		FOV = MAX(FOV,FOV_Target);
		Changed = true;
	}
	else if (FOV < FOV_Target)
	{
		FOV += FOV_ChangeRate;
		FOV = MIN(FOV,FOV_Target);
		Changed = true;
	}
	
	return Changed;
}

// Indicates whether to do fold-in/fold-out effect when one is teleporting
bool View_DoFoldEffect() {return DoFoldEffect;}


// Indicates whether to do the "static" effect when one is teleporting
extern bool View_DoStaticEffect() {return DoStaticEffect;}


// Landscape stuff: this is for being able to return a pointer to the default one
static LandscapeOptions DefaultLandscape;


// Store landscape stuff as a vector member
struct LandscapeOptionsEntry
{
	// Which frame to apply to (default: 0, since there is usually only one)
	short Frame;
	
	// Make a member for more convenient access
	LandscapeOptions OptionsData;
	
	LandscapeOptionsEntry(): Frame(0) {}
};

// Separate landscape-texture sequence lists for each collection ID,
// to speed up searching.
static vector<LandscapeOptionsEntry> LOList[NUMBER_OF_COLLECTIONS];

// Deletes a collection's landscape-texture sequences
static void LODelete(int c)
{
	LOList[c].clear();
}

// Deletes all of them
static void LODeleteAll()
{
	for (int c=0; c<NUMBER_OF_COLLECTIONS; c++) LODelete(c);
}


LandscapeOptions *View_GetLandscapeOptions(shape_descriptor Desc)
{
	// Pull out frame and collection ID's:
	short Frame = GET_DESCRIPTOR_SHAPE(Desc);
	short CollCT = GET_DESCRIPTOR_COLLECTION(Desc);
	short Collection = GET_COLLECTION(CollCT);
	
	vector<LandscapeOptionsEntry>& LOL = LOList[Collection];
	for (vector<LandscapeOptionsEntry>::iterator LOIter = LOL.begin(); LOIter < LOL.end(); LOIter++)
	{
		if (LOIter->Frame == Frame || LOIter->Frame == AnyFrame)
		{
			// Get a pointer from the iterator in order to return it
			return &(LOIter->OptionsData);
		}
	}
	
	// Return the default if no matching entry was found
	return &DefaultLandscape;
}


// Field-of-view parser
class XML_FOVParser: public XML_ElementParser
{
public:
	bool HandleAttribute(const char *Tag, const char *Value);

	XML_FOVParser(): XML_ElementParser("fov") {}
};

bool XML_FOVParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (strcmp(Tag,"normal") == 0)
	{
		return (ReadBoundedNumericalValue(Value,"%f",FOV_Normal,float(0),float(180)));
	}
	else if (strcmp(Tag,"extra") == 0)
	{
		return (ReadBoundedNumericalValue(Value,"%f",FOV_ExtraVision,float(0),float(180)));
	}
	else if (strcmp(Tag,"tunnel") == 0)
	{
		return (ReadBoundedNumericalValue(Value,"%f",FOV_TunnelVision,float(0),float(180)));
	}
	else if (strcmp(Tag,"rate") == 0)
	{
		return (ReadBoundedNumericalValue(Value,"%f",FOV_ChangeRate,float(0),float(180)));
	}
	UnrecognizedTag();
	return false;
}

static XML_FOVParser FOVParser;


// Main view parser: has one attribute: whether or not to show the overhead map
class XML_ViewParser: public XML_ElementParser
{
public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);

	XML_ViewParser(): XML_ElementParser("view") {}
};

bool XML_ViewParser::Start()
{
	Font_SetArray(&OnScreenFont);
	return true;
}

bool XML_ViewParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (strcmp(Tag,"map") == 0)
	{
		return (ReadBooleanValue(Value,MapActive));
	}
	else if (strcmp(Tag,"fold_effect") == 0)
	{
		return (ReadBooleanValue(Value,DoFoldEffect));
	}
	else if (strcmp(Tag,"static_effect") == 0)
	{
		return (ReadBooleanValue(Value,DoStaticEffect));
	}
	UnrecognizedTag();
	return false;
}

static XML_ViewParser ViewParser;


// XML-parser support
XML_ElementParser *View_GetParser()
{
	ViewParser.AddChild(&FOVParser);
	ViewParser.AddChild(Font_GetParser());
	
	return &ViewParser;
}



class XML_LO_ClearParser: public XML_ElementParser
{
	bool IsPresent;
	short Collection;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();

	XML_LO_ClearParser(): XML_ElementParser("clear") {}
};

bool XML_LO_ClearParser::Start()
{
	IsPresent = false;
	return true;
}

bool XML_LO_ClearParser::HandleAttribute(const char *Tag, const char *Value)
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

bool XML_LO_ClearParser::AttributesDone()
{
	if (IsPresent)
		LODelete(Collection);
	else
		LODeleteAll();
	
	return true;
}

static XML_LO_ClearParser LO_ClearParser;


class XML_LandscapeParser: public XML_ElementParser
{
	bool IsPresent;
	short Collection, Frame;
	LandscapeOptions Data;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_LandscapeParser(): XML_ElementParser("landscape") {}
};

bool XML_LandscapeParser::Start()
{
	Data = DefaultLandscape;
	Frame = AnyFrame;
		
	return true;
}

bool XML_LandscapeParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (strcmp(Tag,"coll") == 0)
	{
		if (ReadBoundedNumericalValue(Value,"%hd",Collection,short(AnyFrame),short(NUMBER_OF_COLLECTIONS-1)))
		{
			IsPresent = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"frame") == 0)
	{
		return (ReadBoundedNumericalValue(Value,"%hd",Frame,short(0),short(MAXIMUM_SHAPES_PER_COLLECTION-1)));
	}
	else if (strcmp(Tag,"horiz_exp") == 0)
	{
		return (ReadNumericalValue(Value,"%hd",Data.HorizExp));
	}
	else if (strcmp(Tag,"vert_exp") == 0)
	{
		return (ReadNumericalValue(Value,"%hd",Data.VertExp));
	}
	else if (strcmp(Tag,"vert_repeat") == 0)
	{
		return (ReadBooleanValue(Value,Data.VertRepeat));
	}
	else if (strcmp(Tag,"ogl_asprat_exp") == 0)
	{
		return (ReadNumericalValue(Value,"%hd",Data.OGL_AspRatExp));
	}
	else if (strcmp(Tag,"azimuth") == 0)
	{
		float Azimuth;
		if (ReadNumericalValue(Value,"%f",Azimuth))
		{
			Azimuth = Azimuth - 360*int(Azimuth/360);
			if (Azimuth < 0) Azimuth += 360;
			Data.Azimuth = angle(FULL_CIRCLE*(Azimuth/360) + 0.5);
			return true;
		}
		return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_LandscapeParser::AttributesDone()
{
	// Verify...
	if (!IsPresent)
	{
		AttribsMissing();
		return false;
	}
	
	// Check to see if a frame is already accounted for
	vector<LandscapeOptionsEntry>& LOL = LOList[Collection];
	for (vector<LandscapeOptionsEntry>::iterator LOIter = LOL.begin(); LOIter < LOL.end(); LOIter++)
	{
		if (LOIter->Frame == Frame)
		{
			// Replace the data
			LOIter->OptionsData = Data;
			return true;
		}
	}
	
	// If not, then add a new frame entry
	LandscapeOptionsEntry DataEntry;
	DataEntry.Frame = Frame;
	DataEntry.OptionsData = Data;
	LOL.push_back(DataEntry);
	
	return true;
}

static XML_LandscapeParser LandscapeParser;


static XML_ElementParser LandscapesParser("landscapes");


// XML-parser support
XML_ElementParser *Landscapes_GetParser()
{
	LandscapesParser.AddChild(&LandscapeParser);
	LandscapesParser.AddChild(&LO_ClearParser);
	
	return &LandscapesParser;
}
