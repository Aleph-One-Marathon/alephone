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
#include <map>
#include <sstream>

#include "cseries.h"
#include "shell.h"
#include "game_wad.h"
#include "Music.h"
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
		Lua,
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
	std::string FileSpec;
	
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
	
	std::vector<LevelScriptCommand> Commands;
	
	// Thanx to the Standard Template Library,
	// the copy constructor and the assignment operator will be automatically implemented
	
	// Whether the music files are to be played in random order;
	// it defaults to false (sequential order)
	bool RandomOrder;
	
	LevelScriptHeader(): RandomOrder(false) {}
};

// Scripts for current map file
static std::map<int, LevelScriptHeader> LevelScripts;

// Current script for adding commands to and for running
static LevelScriptHeader *CurrScriptPtr = NULL;

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
	} catch (const InfoTree::parse_error& e) {
		logError("Error parsing map script in %s: %s", MapFile.GetPath(), e.what());
	} catch (const InfoTree::path_error& e) {
		logError("Error parsing map script in %s: %s", MapFile.GetPath(), e.what());
	} catch (const InfoTree::data_error& e) {
		logError("Error parsing map script in %s: %s", MapFile.GetPath(), e.what());
	} catch (const InfoTree::unexpected_error& e) {
		logError("Error parsing map script in %s: %s", MapFile.GetPath(), e.what());
	}
}

void ResetLevelScript()
{
	// For whatever previous music had been playing...
	Music::instance()->Fade(0, MACHINE_TICKS_PER_SECOND/2);
	
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
	LoadBaseMMLScripts(false);
	Plugins::instance()->load_mml(false);
}


// Runs a script for some level
// runs level-specific MML...
void RunLevelScript(int LevelIndex)
{
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
			ParseMMLFromData(reinterpret_cast<char *>(&mmls_chunk[offset]), length);
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
	if (LevelScripts.find(LevelIndex) == LevelScripts.end()) return;
	CurrScriptPtr = &(LevelScripts[LevelIndex]);
	
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
		case LevelScriptCommand::Lua:
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
//				char ObjName[256];
//				sprintf(ObjName,"[Map Rsrc %hd for Level %d]",Cmd.RsrcID,LevelIndex);
				ParseMMLFromData(Data, DataLen);
			}
			break;

		case LevelScriptCommand::Lua:
		{
			// Skip if not loaded
			if (Data == NULL || DataLen <= 0) break;
				
			LoadLuaScript(Data, DataLen, _embedded_lua_script);
		}
		break;
		
		case LevelScriptCommand::Music:
			{
				FileSpecifier MusicFile;
				if (MusicFile.SetNameWithPath(Cmd.FileSpec.c_str()))
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
					OGL_LoadScreen::instance()->Set(Cmd.FileSpec.c_str(), Cmd.Stretch, Cmd.Scale, Cmd.L, Cmd.T, Cmd.R - Cmd.L, Cmd.B - Cmd.T);
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
	if (LevelScripts.find(LevelIndex) == LevelScripts.end()) return;
	CurrScriptPtr = &(LevelScripts[LevelIndex]);
	
	for (unsigned k=0; k<CurrScriptPtr->Commands.size(); k++)
	{
		LevelScriptCommand& Cmd = CurrScriptPtr->Commands[k];
				
		switch(Cmd.Type)
		{
		case LevelScriptCommand::Movie:
			{
				MovieFileExists = MovieFile.SetNameWithPath(Cmd.FileSpec.c_str());

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


void reset_mml_default_levels()
{
	// no reset
}

void parse_mml_default_levels(const InfoTree& root)
{
	LevelScriptHeader *ls_ptr = &(LevelScripts[LevelScriptHeader::Default]);
	
	for (const InfoTree &child : root.children_named("music"))
	{
		LevelScriptCommand cmd;
		cmd.Type = LevelScriptCommand::Music;
		
		if (!child.read_attr("file", cmd.FileSpec))
			continue;
		
		ls_ptr->Commands.push_back(cmd);
	}
	
	for (const InfoTree &child : root.children_named("random_order"))
	{
		child.read_attr("on", ls_ptr->RandomOrder);
	}
	
#ifdef HAVE_OPENGL
	for (const InfoTree &child : root.children_named("load_screen"))
	{
		LevelScriptCommand cmd;
		cmd.Type = LevelScriptCommand::LoadScreen;
		
		if (!child.read_attr("file", cmd.FileSpec))
			continue;
		
		// expand relative file spec now (we may be in scoped search path)
		FileSpecifier f;
		if (f.SetNameWithPath(cmd.FileSpec.c_str()))
		{
			std::string base, part;
			f.SplitPath(base, part);
			cmd.FileSpec = base + '/' + part;
		}
		
		child.read_attr("stretch", cmd.Stretch);
		child.read_attr("scale", cmd.Scale);
		child.read_attr("progress_top", cmd.T);
		child.read_attr("progress_bottom", cmd.B);
		child.read_attr("progress_left", cmd.L);
		child.read_attr("progress_right", cmd.R);
		
		for (const InfoTree &color : child.children_named("color"))
		{
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

void parse_level_commands(InfoTree root, int index)
{
	// Find or create command list for this level
	LevelScriptHeader *ls_ptr = &(LevelScripts[index]);

	for (const InfoTree &child : root.children_named("mml"))
	{
		LevelScriptCommand cmd;
		cmd.Type = LevelScriptCommand::MML;
		
		if (!child.read_attr_bounded<int16>("resource", cmd.RsrcID, 0, SHRT_MAX))
			continue;
		
		ls_ptr->Commands.push_back(cmd);
	}

	for (const InfoTree &child : root.children_named("lua"))
	{
		LevelScriptCommand cmd;
		cmd.Type = LevelScriptCommand::Lua;
		
		if (!child.read_attr_bounded<int16>("resource", cmd.RsrcID, 0, SHRT_MAX))
			continue;
		
		ls_ptr->Commands.push_back(cmd);
	}
	
	for (const InfoTree &child : root.children_named("music"))
	{
		LevelScriptCommand cmd;
		cmd.Type = LevelScriptCommand::Music;
		
		if (!child.read_attr("file", cmd.FileSpec))
			continue;
		
		ls_ptr->Commands.push_back(cmd);
	}
	
	for (const InfoTree &child : root.children_named("random_order"))
	{
		child.read_attr("on", ls_ptr->RandomOrder);
	}
	
	for (const InfoTree &child : root.children_named("movie"))
	{
		LevelScriptCommand cmd;
		cmd.Type = LevelScriptCommand::Movie;
		
		if (!child.read_attr("file", cmd.FileSpec))
			continue;

		child.read_attr("size", cmd.Size);
		
		ls_ptr->Commands.push_back(cmd);
	}
	
#ifdef HAVE_OPENGL
	for (const InfoTree &child : root.children_named("load_screen"))
	{
		LevelScriptCommand cmd;
		cmd.Type = LevelScriptCommand::LoadScreen;
		
		if (!child.read_attr("file", cmd.FileSpec))
			continue;
		
		child.read_attr("stretch", cmd.Stretch);
		child.read_attr("scale", cmd.Scale);
		child.read_attr("progress_top", cmd.T);
		child.read_attr("progress_bottom", cmd.B);
		child.read_attr("progress_left", cmd.L);
		child.read_attr("progress_right", cmd.R);
		
		for (const InfoTree &color : child.children_named("color"))
		{
			int16 index;
			if (color.read_indexed("index", index, 2))
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
	for (const InfoTree &lev : root.children_named("level"))
	{
		int16 index;
		if (lev.read_indexed("index", index, SHRT_MAX+1))
		{
			parse_level_commands(lev, index);
		}
	}
	
	for (const InfoTree &child : root.children_named("end"))
	{
		parse_level_commands(child, LevelScriptHeader::End);
	}
	for (const InfoTree &child : root.children_named("default"))
	{
		parse_level_commands(child, LevelScriptHeader::Default);
	}
	for (const InfoTree &child : root.children_named("restore"))
	{
		parse_level_commands(child, LevelScriptHeader::Restore);
	}
	
	for (const InfoTree &child : root.children_named("end_screens"))
	{
		child.read_attr("index", EndScreenIndex);
		child.read_indexed("count", NumEndScreens, SHRT_MAX+1);
	}
}
