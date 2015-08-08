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

	Support for XML scripts in map files
	by Loren Petrich,
	April 16, 2000

	The reason for a separate object is that it will be necessary to execute certain commands
	only on certain levels.
	
Oct 13, 2000 (Loren Petrich)
	Converted the in-memory script data into Standard Template Library vectors

Nov 25, 2000 (Loren Petrich)
	Added support for specifying movies for levels, as Jesse Simko had requested.
	Also added end-of-game support.

Jul 31, 2002 (Loren Petrich):
	Added images.cpp/h accessor for TEXT resources,
	because it can support the M2-Win95 map format.
*/

#include <vector>
#include <sstream>
#include <boost/foreach.hpp>
using namespace std;

#include "cseries.h"
#include "shell.h"
#include "game_wad.h"
#include "Music.h"
#include "ColorParser.h"
#include "XML_DataBlock.h"
#include "XML_LevelScript.h"
#include "XML_ParseTreeRoot.h"
#include "InfoTree.h"
#include "Plugins.h"
#include "Random.h"
#include "images.h"
#include "lua_script.h"
#include "Logging.h"

#include "OGL_LoadScreen.h"

#include "AStream.h"
#include "map.h"

// The "command" is an instruction to process a file/resource in a certain sort of way
struct LevelScriptCommand
{
	// Types of commands
	enum {
		MML,
		Music,
		Movie,
#ifdef HAVE_LUA
		Lua,
#endif /* HAVE_LUA */
#ifdef HAVE_OPENGL
		LoadScreen
#endif
	};
	int Type;
	
	enum {
		UnsetResource = -32768
	};
	
	// Where to read the command data from:
	
	// This is a MacOS resource ID
	short RsrcID;
	
	bool RsrcPresent() {return RsrcID != UnsetResource;}
	
	// This is a Unix-style filespec, with form
	// <dirname>/<dirname>/<filename>
	// with the root directory being the map file's directory
	vector<char> FileSpec;
	
	// Additional data:
	
	// Relative size for a movie (negative means don't use)
	float Size;

	// Additional load screen information:
	short T, L, B, R;
	rgb_color Colors[2];
	bool Stretch;
	bool Scale;
	
	LevelScriptCommand(): RsrcID(UnsetResource), Size(NONE), L(0), T(0), R(0), B(0), Stretch(true), Scale(true) {
		memset(&Colors[0], 0, sizeof(rgb_color));
		memset(&Colors[1], 0xff, sizeof(rgb_color));
	}
};

struct LevelScriptHeader
{
	// Special pseudo-levels: restoration of previous parameters, defaults for this map file,
	// and the end of the game
	enum {
		Restore = -3,
		Default = -2,
		End = -1
	};
	int Level;
	
	vector<LevelScriptCommand> Commands;
	
	// Thanx to the Standard Template Library,
	// the copy constructor and the assignment operator will be automatically implemented
	
	// Whether the music files are to be played in random order;
	// it defaults to false (sequential order)
	bool RandomOrder;
	
	LevelScriptHeader(): Level(Default), RandomOrder(false) {}
};

// Scripts for current map file
static vector<LevelScriptHeader> LevelScripts;

// Current script for adding commands to and for running
static LevelScriptHeader *CurrScriptPtr = NULL;

// Map-file parent directory (where all map-related files are supposed to live)
static DirectorySpecifier MapParentDir;

#ifdef HAVE_LUA
// Same for Lua
static bool LuaFound = false;
#endif /* HAVE_LUA */

// Movie filespec and whether it points to a real file
static FileSpecifier MovieFile;
static bool MovieFileExists = false;
static float MovieSize = NONE;

// For selecting the end-of-game screens --
// what fake level index for them, and how many to display
// (resource numbers increasing in sequence) 
// (defaults from interface.cpp)
short EndScreenIndex = 99;
short NumEndScreens = 1;


// The level-script parsers are separate from the main MML ones,
// because they operate on per-level data.

static XML_DataBlock LSXML_Loader;

// Parse marathon_levels script
static void parse_levels_xml(InfoTree root);

// This is for searching for a script and running it -- works for pseudo-levels
static void GeneralRunScript(int LevelIndex);

// Similar generic function for movies
static void FindMovieInScript(int LevelIndex);

// Defined in images.cpp and 
extern bool get_text_resource_from_scenario(int resource_number, LoadedResource& TextRsrc);

// Loads all those in resource 128 in a map file (or some appropriate equivalent)
void LoadLevelScripts(FileSpecifier& MapFile)
{
	// Because all the files are to live in the map file's parent directory
	MapFile.ToDirectory(MapParentDir);
	
	// Get rid of the previous level script
	// ghs: unless it's the first time, in which case we would be clearing
	// any external level scripts, so don't
	static bool FirstTime = true;
	if (FirstTime)
		FirstTime = false;
	else
		LevelScripts.clear();
	
	// Set these to their defaults before trying to change them
	EndScreenIndex = 99;
	NumEndScreens = 1;
	
	// OpenedResourceFile OFile;
	// if (!MapFile.Open(OFile)) return;
	
	// The script is stored at a special resource ID;
	// simply quit if it could not be found
	LoadedResource ScriptRsrc;
	
	// if (!OFile.Get('T','E','X','T',128,ScriptRsrc)) return;
	if (!get_text_resource_from_scenario(128,ScriptRsrc)) return;
	
	// Load the script
	std::istringstream strm(std::string((char *)ScriptRsrc.GetPointer(), ScriptRsrc.GetLength()));
	try {
		InfoTree root = InfoTree::load_xml(strm).get_child("marathon_levels");
		parse_levels_xml(root);
	} catch (InfoTree::parse_error e) {
		logError2("Error parsing map script in %s: %s", MapFile.GetPath(), e.what());
	} catch (InfoTree::path_error e) {
		logError2("Error parsing map script in %s: %s", MapFile.GetPath(), e.what());
	}
}

void ResetLevelScript()
{
	// For whatever previous music had been playing...
	Music::instance()->FadeOut(MACHINE_TICKS_PER_SECOND/2);
	
	// If no scripts were loaded or none of them had music specified,
	// then don't play any music
	if (!Music::instance()->HasClassicLevelMusic())
		Music::instance()->ClearLevelMusic();

#ifdef HAVE_OPENGL	
	OGL_LoadScreen::instance()->Clear();
#endif

	// reset values to engine defaults first
	ResetAllMMLValues();
	// then load the base stuff (from Scripts folder and whatnot)
	LoadBaseMMLScripts();
	Plugins::instance()->load_mml();
}


// Runs a script for some level
// runs level-specific MML...
void RunLevelScript(int LevelIndex)
{
	// None found just yet...
#ifdef HAVE_LUA
	LuaFound = false;
#endif /* HAVE_LUA */
	
	ResetLevelScript();

	GeneralRunScript(LevelScriptHeader::Default);
	GeneralRunScript(LevelIndex);
	
	Music::instance()->SeedLevelMusic();

}

std::vector<uint8> mmls_chunk;
std::vector<uint8> luas_chunk;

void RunScriptChunks()
{
	int offset = 2;
	while (offset < mmls_chunk.size())
	{
		if (offset + 8 + LEVEL_NAME_LENGTH > mmls_chunk.size())
			break;

		AIStreamBE header(&mmls_chunk[offset], 8 + LEVEL_NAME_LENGTH);
		offset += 8 + LEVEL_NAME_LENGTH;
		
		uint32 flags;
		char name[LEVEL_NAME_LENGTH];
		uint32 length;
		header >> flags;
		header.read(name, LEVEL_NAME_LENGTH);
		name[LEVEL_NAME_LENGTH - 1] = '\0';
		header >> length;
		if (offset + length > mmls_chunk.size())
			break;

		if (length)
		{
			LSXML_Loader.SourceName = name;
			LSXML_Loader.CurrentElement = &RootParser;
			LSXML_Loader.ParseData(reinterpret_cast<char *>(&mmls_chunk[offset]), length);
		}

		offset += length;
	}

	offset = 2;
	while (offset < luas_chunk.size())
	{
		if (offset + 8 + LEVEL_NAME_LENGTH > luas_chunk.size())
			break;

		AIStreamBE header(&luas_chunk[offset], 8 + LEVEL_NAME_LENGTH);
		offset += 8 + LEVEL_NAME_LENGTH;
		
		uint32 flags;
		char name[LEVEL_NAME_LENGTH];
		uint32 length;
		header >> flags;
		header.read(name, LEVEL_NAME_LENGTH);
		name[LEVEL_NAME_LENGTH - 1] = '\0';
		header >> length;
		if (offset + length > luas_chunk.size())
			break;

		LoadLuaScript(reinterpret_cast<char *>(&luas_chunk[offset]), length, _embedded_lua_script);
		offset += length;
	}
}

// Intended to be run at the end of a game
void RunEndScript()
{
	GeneralRunScript(LevelScriptHeader::Default);
	GeneralRunScript(LevelScriptHeader::End);
}

// Intended for restoring old parameter values, because MML sets values at a variety
// of different places, and it may be easier to simply set stuff back to defaults
// by including those defaults in the script.
void RunRestorationScript()
{
	GeneralRunScript(LevelScriptHeader::Default);
	GeneralRunScript(LevelScriptHeader::Restore);
}

// Search for level script and then run it
void GeneralRunScript(int LevelIndex)
{
	// Find the pointer to the current script
	CurrScriptPtr = NULL;
	for (vector<LevelScriptHeader>::iterator ScriptIter = LevelScripts.begin(); ScriptIter < LevelScripts.end(); ScriptIter++)
	{
		if (ScriptIter->Level == LevelIndex)
		{
			CurrScriptPtr = &(*ScriptIter);	// Iterator to pointer
			break;
		}
	}
	if (!CurrScriptPtr) return;
	
	// Insures that this order is the last order set
	Music::instance()->LevelMusicRandom(CurrScriptPtr->RandomOrder);
	
	// OpenedResourceFile OFile;
	// FileSpecifier& MapFile = get_map_file();
	// if (!MapFile.Open(OFile)) return;
	
	for (unsigned k=0; k<CurrScriptPtr->Commands.size(); k++)
	{
		LevelScriptCommand& Cmd = CurrScriptPtr->Commands[k];
		
		// Data to parse
		char *Data = NULL;
		size_t DataLen = 0;
		
		// First, try to load a resource (only for scripts)
		LoadedResource ScriptRsrc;
		switch(Cmd.Type)
		{
		case LevelScriptCommand::MML:
#ifdef HAVE_LUA
		case LevelScriptCommand::Lua:
#endif /* HAVE_LUA */
			// if (Cmd.RsrcPresent() && OFile.Get('T','E','X','T',Cmd.RsrcID,ScriptRsrc))
			if (Cmd.RsrcPresent() && get_text_resource_from_scenario(Cmd.RsrcID,ScriptRsrc))
			{
				Data = (char *)ScriptRsrc.GetPointer();
				DataLen = ScriptRsrc.GetLength();
			}
		}
		
		switch(Cmd.Type)
		{
		case LevelScriptCommand::MML:
			{
				// Skip if not loaded
				if (Data == NULL || DataLen <= 0) break;
				
				// Set to the MML root parser
				char ObjName[256];
				sprintf(ObjName,"[Map Rsrc %hd for Level %d]",Cmd.RsrcID,LevelIndex);
				LSXML_Loader.SourceName = ObjName;
				LSXML_Loader.CurrentElement = &RootParser;
				LSXML_Loader.ParseData(Data,DataLen);
			}
			break;
		
#ifdef HAVE_LUA
                        
			case LevelScriptCommand::Lua:
			{
				// Skip if not loaded
				if (Data == NULL || DataLen <= 0) break;
				
				// Load and indicate whether loading was successful
				if (LoadLuaScript(Data, DataLen, _embedded_lua_script))
					LuaFound = true;
			}
			break;
#endif /* HAVE_LUA */
		
		case LevelScriptCommand::Music:
			{
				FileSpecifier MusicFile;
				if (MusicFile.SetNameWithPath(&Cmd.FileSpec[0]))
					Music::instance()->PushBackLevelMusic(MusicFile);
			}
			break;
#ifdef HAVE_OPENGL
		case LevelScriptCommand::LoadScreen:
		{
			if (Cmd.FileSpec.size() > 0)
			{
				if (Cmd.L || Cmd.T || Cmd.R || Cmd.B)
				{
					OGL_LoadScreen::instance()->Set(Cmd.FileSpec, Cmd.Stretch, Cmd.Scale, Cmd.L, Cmd.T, Cmd.R - Cmd.L, Cmd.B - Cmd.T);
					OGL_LoadScreen::instance()->Colors()[0] = Cmd.Colors[0];
					OGL_LoadScreen::instance()->Colors()[1] = Cmd.Colors[1];
				}
				else 
					OGL_LoadScreen::instance()->Set(Cmd.FileSpec, Cmd.Stretch, Cmd.Scale);
			}
		}
#endif
		// The movie info is handled separately
			
		}
	}
}



// Search for level script and then run it
void FindMovieInScript(int LevelIndex)
{
	// Find the pointer to the current script
	CurrScriptPtr = NULL;
	for (vector<LevelScriptHeader>::iterator ScriptIter = LevelScripts.begin(); ScriptIter < LevelScripts.end(); ScriptIter++)
	{
		if (ScriptIter->Level == LevelIndex)
		{
			CurrScriptPtr = &(*ScriptIter);	// Iterator to pointer
			break;
		}
	}
	if (!CurrScriptPtr) return;
		
	for (unsigned k=0; k<CurrScriptPtr->Commands.size(); k++)
	{
		LevelScriptCommand& Cmd = CurrScriptPtr->Commands[k];
				
		switch(Cmd.Type)
		{
		case LevelScriptCommand::Movie:
			{
				MovieFileExists = MovieFile.SetNameWithPath(&Cmd.FileSpec[0]);

				// Set the size only if there was a movie file here
				if (MovieFileExists)
					MovieSize = Cmd.Size;
			}
			break;
		}
	}
}


// Movie functions

// Finds the level movie and the end movie, to be used in show_movie()
// The first is for some level,
// while the second is for the end of a game
void FindLevelMovie(short index)
{
	MovieFileExists = false;
	MovieSize = NONE;
	FindMovieInScript(LevelScriptHeader::Default);
	FindMovieInScript(index);
}


FileSpecifier *GetLevelMovie(float& Size)
{
	if (MovieFileExists)
	{
		// Set only if the movie-size value is positive
		if (MovieSize >= 0) Size = MovieSize;
		return &MovieFile;
	}
	else
		return NULL;
}

void SetMMLS(uint8* data, size_t length)
{
	if (!length)
	{
		mmls_chunk.clear();
	}
	else
	{
		mmls_chunk.resize(length);
		memcpy(&mmls_chunk[0], data, length);
	}
}

uint8* GetMMLS(size_t& length)
{
	length = mmls_chunk.size();
	return length ? &mmls_chunk[0] : 0;
}

void SetLUAS(uint8* data, size_t length)
{
	if (!length)
	{
		luas_chunk.clear();
	}
	else
	{
		luas_chunk.resize(length);
		memcpy(&luas_chunk[0], data, length);
	}
}

uint8* GetLUAS(size_t& length)
{
	length = luas_chunk.size();
	return length ? &luas_chunk[0] : 0;
}

// Generalized parser for level-script commands; they all use the same format,
// but will be distinguished by their names
class XML_LSCommandParser: public XML_ElementParser
{
	// Need to get an object ID (either resource ID or filename)
	bool ObjectWasFound;
	
	// New script command to compose	
	LevelScriptCommand Cmd;
	
public:
	bool Start();
	bool End();

	// For grabbing the level ID
	bool HandleAttribute(const char *Tag, const char *Value);

	XML_LSCommandParser(const char *_Name, int _CmdType): XML_ElementParser(_Name) {Cmd.Type = _CmdType;}
};

bool XML_LSCommandParser::Start()
{
	ObjectWasFound = false;
	Color_SetArray(Cmd.Colors, 2);
	return true;
}


bool XML_LSCommandParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"resource"))
	{
		short RsrcID;
		if (ReadBoundedInt16Value(Value,RsrcID,0,SHRT_MAX))
		{
			Cmd.RsrcID = RsrcID;
			ObjectWasFound = true;
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"file"))
	{
		size_t vlen = strlen(Value) + 1;
		Cmd.FileSpec.resize(vlen);
		memcpy(&Cmd.FileSpec[0],Value,vlen);
		ObjectWasFound = true;
		return true;
	}
	else if (StringsEqual(Tag, "stretch"))
	{
		return ReadBooleanValueAsBool(Value, Cmd.Stretch);
	}
	else if (StringsEqual(Tag, "scale"))
	{
		return ReadBooleanValueAsBool(Value, Cmd.Scale);
	}
	else if (StringsEqual(Tag,"size"))
	{
		return ReadFloatValue(Value,Cmd.Size);
	}
	else if (StringsEqual(Tag, "progress_top"))
	{
		return ReadInt16Value(Value, Cmd.T);
	}
	else if (StringsEqual(Tag, "progress_bottom"))
	{
		return ReadInt16Value(Value, Cmd.B);
	}
	else if (StringsEqual(Tag, "progress_left"))
	{
		return ReadInt16Value(Value, Cmd.L);
	}
	else if (StringsEqual(Tag, "progress_right"))
	{
		return ReadInt16Value(Value, Cmd.R);
	}
	UnrecognizedTag();
	return false;
}

bool XML_LSCommandParser::End()
{
	if (!ObjectWasFound) return false;

	
	assert(CurrScriptPtr);
	CurrScriptPtr->Commands.push_back(Cmd);
	return true;
}

static XML_LSCommandParser MusicParser("music",LevelScriptCommand::Music);
#ifdef HAVE_OPENGL
static XML_LSCommandParser LoadScreenParser("load_screen", LevelScriptCommand::LoadScreen);
#endif

class XML_RandomOrderParser: public XML_ElementParser
{
public:
	bool HandleAttribute(const char *Tag, const char *Value)
	{
		if (StringsEqual(Tag,"on"))
		{
			bool RandomOrder;
			bool Success = ReadBooleanValueAsBool(Value,RandomOrder);
			if (Success)
			{
				assert(CurrScriptPtr);
				CurrScriptPtr->RandomOrder = RandomOrder;
			}
			return Success;
		}
		UnrecognizedTag();
		return false;
	}
	
	XML_RandomOrderParser(): XML_ElementParser("random_order") {}
};


static XML_RandomOrderParser RandomOrderParser;

	
// Generalized parser for level scripts; for also parsing default and restoration scripts

class XML_GeneralLevelScriptParser: public XML_ElementParser
{
protected:
	// Tell the level script and the parser for its contents what level we are currently doing
	void SetLevel(short Level);
	
public:
	
	XML_GeneralLevelScriptParser(const char *_Name): XML_ElementParser(_Name) {}
};

void XML_GeneralLevelScriptParser::SetLevel(short Level)
{
	// Scan for current level
	CurrScriptPtr = NULL;
	for (vector<LevelScriptHeader>::iterator ScriptIter = LevelScripts.begin(); ScriptIter < LevelScripts.end(); ScriptIter++)
	{
		if (ScriptIter->Level == Level)
		{
			CurrScriptPtr = &(*ScriptIter);	// Iterator to pointer
			break;
		}
	}
	
	// Not found, so add it
	if (!CurrScriptPtr)
	{
		LevelScriptHeader NewHdr;
		NewHdr.Level = Level;
		LevelScripts.push_back(NewHdr);
		CurrScriptPtr = &LevelScripts.back();
	}
}


// For setting up scripting for special pseudo-levels: the default and the restore
class XML_SpecialLevelScriptParser: public XML_GeneralLevelScriptParser
{
	short Level;
	
public:
	bool Start() {SetLevel(Level); return true;}
	
	XML_SpecialLevelScriptParser(const char *_Name, short _Level): XML_GeneralLevelScriptParser(_Name), Level(_Level) {}
};

static  XML_SpecialLevelScriptParser ExternalDefaultScriptParser("default_levels", LevelScriptHeader::Default);

XML_ElementParser *ExternalDefaultLevelScript_GetParser()
{
#ifdef HAVE_OPENGL
	ExternalDefaultScriptParser.AddChild(&LoadScreenParser);
	LoadScreenParser.AddChild(Color_GetParser());
#endif
	ExternalDefaultScriptParser.AddChild(&MusicParser);
	ExternalDefaultScriptParser.AddChild(&RandomOrderParser);
	
	return &ExternalDefaultScriptParser;
}

void parse_level_commands(InfoTree root, int index)
{
	// Find or create command list for this level
	LevelScriptHeader *ls_ptr = NULL;
	for (vector<LevelScriptHeader>::iterator it = LevelScripts.begin(); it < LevelScripts.end(); it++)
	{
		if (it->Level == index)
		{
			ls_ptr = &(*it);
			break;
		}
	}
	
	// Not found, so add it
	if (!ls_ptr)
	{
		LevelScriptHeader new_header;
		new_header.Level = index;
		LevelScripts.push_back(new_header);
		ls_ptr = &LevelScripts.back();
	}

	BOOST_FOREACH(InfoTree::value_type &v, root.equal_range("mml"))
	{
		InfoTree child = v.second;
		LevelScriptCommand cmd;
		cmd.Type = LevelScriptCommand::MML;
		
		if (!child.read_attr_bounded<int16>("resource", cmd.RsrcID, 0, SHRT_MAX))
			continue;
		
		ls_ptr->Commands.push_back(cmd);
	}

#ifdef HAVE_LUA
	BOOST_FOREACH(InfoTree::value_type &v, root.equal_range("lua"))
	{
		InfoTree child = v.second;
		LevelScriptCommand cmd;
		cmd.Type = LevelScriptCommand::Lua;
		
		if (!child.read_attr_bounded<int16>("resource", cmd.RsrcID, 0, SHRT_MAX))
			continue;
		
		ls_ptr->Commands.push_back(cmd);
	}
#endif
	
	BOOST_FOREACH(InfoTree::value_type &v, root.equal_range("music"))
	{
		InfoTree child = v.second;
		LevelScriptCommand cmd;
		cmd.Type = LevelScriptCommand::Music;
		
		std::string filename;
		if (!child.read_attr("file", filename))
			continue;
		cmd.FileSpec = std::vector<char>(filename.begin(), filename.end());
		cmd.FileSpec.resize(filename.size()+1);
		
		ls_ptr->Commands.push_back(cmd);
	}
	
	BOOST_FOREACH(InfoTree::value_type &v, root.equal_range("random_order"))
	{
		InfoTree child = v.second;
		child.read_attr("on", ls_ptr->RandomOrder);
	}
	
	BOOST_FOREACH(InfoTree::value_type &v, root.equal_range("movie"))
	{
		InfoTree child = v.second;
		LevelScriptCommand cmd;
		cmd.Type = LevelScriptCommand::Movie;
		
		std::string filename;
		if (!child.read_attr("file", filename))
			continue;
		cmd.FileSpec = std::vector<char>(filename.begin(), filename.end());
		cmd.FileSpec.resize(filename.size()+1);

		child.read_attr("size", cmd.Size);
		
		ls_ptr->Commands.push_back(cmd);
	}
	
#ifdef HAVE_OPENGL
	BOOST_FOREACH(InfoTree::value_type &v, root.equal_range("load_screen"))
	{
		InfoTree child = v.second;
		LevelScriptCommand cmd;
		cmd.Type = LevelScriptCommand::LoadScreen;
		
		std::string filename;
		if (!child.read_attr("file", filename))
			continue;
		cmd.FileSpec = std::vector<char>(filename.begin(), filename.end());
		cmd.FileSpec.resize(filename.size()+1);
		
		child.read_attr("stretch", cmd.Stretch);
		child.read_attr("scale", cmd.Scale);
		child.read_attr("progress_top", cmd.T);
		child.read_attr("progress_bottom", cmd.B);
		child.read_attr("progress_left", cmd.L);
		child.read_attr("progress_right", cmd.R);
		
		BOOST_FOREACH(InfoTree::value_type &v, child.equal_range("color"))
		{
			InfoTree color = v.second;
			int index = -1;
			if (color.read_attr_bounded("index", index, 0, 1))
			{
				color.read_color(cmd.Colors[index]);
			}
		}
		
		ls_ptr->Commands.push_back(cmd);
	}
#endif
}

void parse_levels_xml(InfoTree root)
{
	boost::optional<InfoTree> ochild;
	
	BOOST_FOREACH(InfoTree::value_type &v, root.equal_range("level"))
	{
		InfoTree lev = v.second;
		int16 index = -1;
		if (lev.read_attr_bounded<int16>("index", index, 0, SHRT_MAX))
		{
			parse_level_commands(lev, index);
		}
	}
	
	if ((ochild = root.get_child_optional("end")))
	{
		parse_level_commands(*ochild, LevelScriptHeader::End);
	}
	if ((ochild = root.get_child_optional("default")))
	{
		parse_level_commands(*ochild, LevelScriptHeader::Default);
	}
	if ((ochild = root.get_child_optional("restore")))
	{
		parse_level_commands(*ochild, LevelScriptHeader::Restore);
	}
	
	if ((ochild = root.get_child_optional("end_screens")))
	{
		ochild->read_attr("index", EndScreenIndex);
		ochild->read_attr_bounded<int16>("count", NumEndScreens, 0, SHRT_MAX);
	}
}
