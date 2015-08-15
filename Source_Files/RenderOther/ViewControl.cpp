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
#include "SoundManager.h"
#include "shell.h"
#include "screen.h"
#include "ViewControl.h"
#include "InfoTree.h"


struct view_settings_definition {
	bool MapActive;
	bool DoFoldEffect;
	bool DoStaticEffect;
	bool DoInterlevelTeleportInEffects;
	bool DoInterlevelTeleportOutEffects;
};

// Defaults:
struct view_settings_definition view_settings = {
	true, // overhead map is active
	true, // do the view folding effect (stretch horizontally, squeeze vertically) when teleporting,
	true,  // also do the static effect / folding effect on viewed teleported objects
	true, // do all effects (and sounds) teleporting into the level
	true  // do all effects (and sounds) teleporting out of the level
};

// Accessors:
bool View_MapActive() {return view_settings.MapActive;}
bool View_DoFoldEffect() {return view_settings.DoFoldEffect;}
bool View_DoStaticEffect() {return view_settings.DoStaticEffect;}
bool View_DoInterlevelTeleportInEffects() { return view_settings.DoInterlevelTeleportInEffects; }
bool View_DoInterlevelTeleportOutEffects() { return view_settings.DoInterlevelTeleportOutEffects; }


// This frame value means that a landscape option will be applied to any frame in a collection:
const int AnyFrame = -1;

// Field-of-view stuff with defaults:
struct FOV_settings_definition {
	float Normal;
	float ExtraVision;
	float TunnelVision;
	float ChangeRate;	// this is 50 degrees/s
	bool FixHorizontalNotVertical;
};

struct FOV_settings_definition FOV_settings = {
	80,
	130,
	30,
	1.66666667F,	// this is 50 degrees/s
	false
};

#define FOV_Normal FOV_settings.Normal
#define FOV_ExtraVision FOV_settings.ExtraVision
#define FOV_TunnelVision FOV_settings.TunnelVision
#define FOV_ChangeRate FOV_settings.ChangeRate
#define FOV_FixHorizontalNotVertical FOV_settings.FixHorizontalNotVertical


#ifdef HAVE_SDL_TTF
static FontSpecifier OnScreenFont = {"Monaco", 12, styleNormal, 0, "mono"};
#else
static FontSpecifier OnScreenFont = {"Monaco", 12, styleNormal, 0, "#4"};
#endif
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

// Indicates whether to fix the horizontal or the vertical field-of-view angle
// (default: fix vertical FOV angle)
bool View_FOV_FixHorizontalNotVertical() {return get_screen_mode()->fix_h_not_v;}

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
struct FOV_settings_definition *original_FOV_settings = NULL;
class XML_FOVParser: public XML_ElementParser
{
public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool ResetValues();

	XML_FOVParser(): XML_ElementParser("fov") {}
};

bool XML_FOVParser::Start()
{
	// backup old values first
	if (!original_FOV_settings) {
		original_FOV_settings = (struct FOV_settings_definition *) malloc(sizeof(struct FOV_settings_definition));
		assert(original_FOV_settings);
		*original_FOV_settings = FOV_settings;
	}
	return true;
}

bool XML_FOVParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"normal"))
	{
		return (ReadBoundedNumericalValue(Value,"%f",FOV_Normal,float(0),float(180)));
	}
	else if (StringsEqual(Tag,"extra"))
	{
		return (ReadBoundedNumericalValue(Value,"%f",FOV_ExtraVision,float(0),float(180)));
	}
	else if (StringsEqual(Tag,"tunnel"))
	{
		return (ReadBoundedNumericalValue(Value,"%f",FOV_TunnelVision,float(0),float(180)));
	}
	else if (StringsEqual(Tag,"rate"))
	{
		return (ReadBoundedNumericalValue(Value,"%f",FOV_ChangeRate,float(0),float(180)));
	}
	else if (StringsEqual(Tag,"fix_h_not_v"))
	{
		return ReadBooleanValueAsBool(Value,FOV_FixHorizontalNotVertical);
	}
	UnrecognizedTag();
	return false;
}

bool XML_FOVParser::ResetValues()
{
	if (original_FOV_settings) {
		FOV_settings = *original_FOV_settings;
		free(original_FOV_settings);
		original_FOV_settings = NULL;
	}
	return true;
}

static XML_FOVParser FOVParser;


// Main view parser
struct view_settings_definition *original_view_settings = NULL;
static FontSpecifier original_OnScreenFont = OnScreenFont;

class XML_ViewParser: public XML_ElementParser
{
public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool ResetValues();

	XML_ViewParser(): XML_ElementParser("view") {}
};

bool XML_ViewParser::Start()
{
	// backup stuff first
	if (!original_view_settings) {
		original_view_settings = (struct view_settings_definition *) malloc(sizeof(struct view_settings_definition));
		assert(original_view_settings);
		*original_view_settings = view_settings;
	}
	Font_SetArray(&OnScreenFont);
	return true;
}

bool XML_ViewParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"map"))
	{
		return ReadBooleanValueAsBool(Value,view_settings.MapActive);
	}
	else if (StringsEqual(Tag,"fold_effect"))
	{
		return ReadBooleanValueAsBool(Value,view_settings.DoFoldEffect);
	}
	else if (StringsEqual(Tag,"static_effect"))
	{
		return ReadBooleanValueAsBool(Value,view_settings.DoStaticEffect);
	}
	else if (StringsEqual(Tag,"interlevel_in_effects"))
	{
		return ReadBooleanValueAsBool(Value, view_settings.DoInterlevelTeleportInEffects);
	}
	else if (StringsEqual(Tag, "interlevel_out_effects"))
	{
		return ReadBooleanValueAsBool(Value, view_settings.DoInterlevelTeleportOutEffects);
	}
	UnrecognizedTag();
	return false;
}

bool XML_ViewParser::ResetValues()
{
	if (original_view_settings) {
		view_settings = *original_view_settings;
		free(original_view_settings);
		original_view_settings = NULL;
	}
	// reset on-screen font and update if needed
	OnScreenFont = original_OnScreenFont;
	if (ScreenFontInited) {
		OnScreenFont.Update();
	}
	
	return true;
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
	bool ResetValues();

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
	if (StringsEqual(Tag,"coll"))
	{
		if (ReadBoundedInt16Value(Value,Collection,short(AnyFrame),short(NUMBER_OF_COLLECTIONS-1)))
		{
			IsPresent = true;
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"frame"))
	{
		return ReadBoundedInt16Value(Value,Frame,0,MAXIMUM_SHAPES_PER_COLLECTION-1);
	}
	else if (StringsEqual(Tag,"horiz_exp"))
	{
		return ReadInt16Value(Value,Data.HorizExp);
	}
	else if (StringsEqual(Tag,"vert_exp"))
	{
		return ReadInt16Value(Value,Data.VertExp);
	}
	else if (StringsEqual(Tag,"vert_repeat"))
	{
		return ReadBooleanValueAsBool(Value,Data.VertRepeat);
	}
	else if (StringsEqual(Tag,"ogl_asprat_exp"))
	{
		return ReadInt16Value(Value,Data.OGL_AspRatExp);
	}
	else if (StringsEqual(Tag,"azimuth"))
	{
		float Azimuth;
		if (ReadFloatValue(Value,Azimuth))
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

bool XML_LandscapeParser::ResetValues()
{
	LODeleteAll();
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

void reset_mml_view()
{
	if (original_view_settings) {
		view_settings = *original_view_settings;
		free(original_view_settings);
		original_view_settings = NULL;
	}
	
	// reset on-screen font and update if needed
	OnScreenFont = original_OnScreenFont;
	if (ScreenFontInited) {
		OnScreenFont.Update();
	}
	
	if (original_FOV_settings) {
		FOV_settings = *original_FOV_settings;
		free(original_FOV_settings);
		original_FOV_settings = NULL;
	}
}

void parse_mml_view(const InfoTree& root)
{
	// backup old values first
	if (!original_view_settings) {
		original_view_settings = (struct view_settings_definition *) malloc(sizeof(struct view_settings_definition));
		assert(original_view_settings);
		*original_view_settings = view_settings;
	}
	
	if (!original_FOV_settings) {
		original_FOV_settings = (struct FOV_settings_definition *) malloc(sizeof(struct FOV_settings_definition));
		assert(original_FOV_settings);
		*original_FOV_settings = FOV_settings;
	}
	
	root.read_attr("map", view_settings.MapActive);
	root.read_attr("fold_effect", view_settings.DoFoldEffect);
	root.read_attr("static_effect", view_settings.DoStaticEffect);
	root.read_attr("interlevel_in_effects", view_settings.DoInterlevelTeleportInEffects);
	root.read_attr("interlevel_out_effects", view_settings.DoInterlevelTeleportOutEffects);
	
	BOOST_FOREACH(InfoTree font, root.children_named("font"))
	{
		font.read_font(OnScreenFont);
	}
	
	BOOST_FOREACH(InfoTree fov, root.children_named("fov"))
	{
		fov.read_attr_bounded<float>("normal", FOV_Normal, 0, 180);
		fov.read_attr_bounded<float>("extra", FOV_ExtraVision, 0, 180);
		fov.read_attr_bounded<float>("tunnel", FOV_TunnelVision, 0, 180);
		fov.read_attr_bounded<float>("rate", FOV_ChangeRate, 0, 180);
		fov.read_attr("fix_h_not_v", FOV_FixHorizontalNotVertical);
	}
}

void reset_mml_landscapes()
{
	LODeleteAll();
}

void parse_mml_landscapes(const InfoTree& root)
{
	BOOST_FOREACH(const InfoTree::value_type &v, root)
	{
		const std::string& name = v.first;
		const InfoTree& child = v.second;
		if (name == "clear")
		{
			int16 coll;
			if (child.read_indexed("coll", coll, NUMBER_OF_COLLECTIONS))
				LODelete(coll);
			else
				LODeleteAll();
		}
		else if (name == "landscape")
		{
			int16 coll;
			if (!child.read_indexed("coll", coll, NUMBER_OF_COLLECTIONS))
				continue;
			
			int16 frame = AnyFrame;
			child.read_indexed("frame", frame, MAXIMUM_SHAPES_PER_COLLECTION);
			
			LandscapeOptions data = DefaultLandscape;
			child.read_attr("horiz_exp", data.HorizExp);
			child.read_attr("vert_exp", data.VertExp);
			child.read_attr("vert_repeat", data.VertRepeat);
			child.read_attr("ogl_asprat_exp", data.OGL_AspRatExp);
			child.read_angle("azimuth", data.Azimuth);
			
			// Check to see if a frame is already accounted for
			bool found = false;
			vector<LandscapeOptionsEntry>& LOL = LOList[coll];
			for (vector<LandscapeOptionsEntry>::iterator LOIter = LOL.begin(); LOIter < LOL.end(); LOIter++)
			{
				if (LOIter->Frame == frame)
				{
					// Replace the data
					LOIter->OptionsData = data;
					found = true;
					break;
				}
			}
			
			// If not, then add a new frame entry
			if (!found)
			{
				LandscapeOptionsEntry DataEntry;
				DataEntry.Frame = frame;
				DataEntry.OptionsData = data;
				LOL.push_back(DataEntry);
			}
		}
	}
}
