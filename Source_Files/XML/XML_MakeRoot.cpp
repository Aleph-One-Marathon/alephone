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

	Creator of root of XML-Parser Tree
	by Loren Petrich,
	April 16, 2000

	This is for setting up the absolute root element; this element has as its children
	the possible root elements of the Marathon XML files, which is here only "marathon"
*/

#include "cseries.h"
#include "XML_ParseTreeRoot.h"
#include "TextStrings.h"
#include "interface.h"
#include "game_window.h"
#include "PlayerName.h"
#include "motion_sensor.h"
#include "world.h"
#include "overhead_map.h"
#include "dynamic_limits.h"
#include "AnimatedTextures.h"
#include "player.h"
#include "items.h"
#include "media.h"
#include "map.h"
#include "platforms.h"
#include "scenery.h"
#include "fades.h"
#include "ViewControl.h"
#include "weapons.h"
#include "OGL_Setup.h"
#include "shell.h"
#include "SoundManager.h"
#include "vbl.h"
#include "monsters.h"
#include "Logging.h"
#include "Scenario.h"
#include "SW_Texture_Extras.h"
#include "Console.h"
#include "XML_LevelScript.h"


// The absolute root element is a global, of course
XML_ElementParser RootParser("");

// This is the canonical root element in the XML setup files:
XML_ElementParser MarathonParser("marathon");


void SetupParseTree()
{
	// Add the only recognized XML-document-root object here
	RootParser.AddChild(&MarathonParser);

	// Add all its subobjects
	MarathonParser.AddChild(TS_GetParser());		// Text strings
	MarathonParser.AddChild(Interface_GetParser());
	MarathonParser.AddChild(PlayerName_GetParser());
	MarathonParser.AddChild(Infravision_GetParser());
	MarathonParser.AddChild(MotionSensor_GetParser());
	MarathonParser.AddChild(OverheadMap_GetParser());
	MarathonParser.AddChild(DynamicLimits_GetParser());
	MarathonParser.AddChild(AnimatedTextures_GetParser());
	MarathonParser.AddChild(Player_GetParser());
	MarathonParser.AddChild(Items_GetParser());
	MarathonParser.AddChild(ControlPanels_GetParser());
	MarathonParser.AddChild(Liquids_GetParser());
	MarathonParser.AddChild(Sounds_GetParser());
	MarathonParser.AddChild(Platforms_GetParser());
	MarathonParser.AddChild(Scenery_GetParser());
	MarathonParser.AddChild(Faders_GetParser());
	MarathonParser.AddChild(View_GetParser());
	MarathonParser.AddChild(Landscapes_GetParser());
	MarathonParser.AddChild(Weapons_GetParser());
	MarathonParser.AddChild(OpenGL_GetParser());
	MarathonParser.AddChild(Cheats_GetParser());
	MarathonParser.AddChild(TextureLoading_GetParser());
	MarathonParser.AddChild(Keyboard_GetParser());
	MarathonParser.AddChild(DamageKicks_GetParser());
	MarathonParser.AddChild(Monsters_GetParser());
	MarathonParser.AddChild(Logging_GetParser());
	MarathonParser.AddChild(Scenario_GetParser());
	MarathonParser.AddChild(SW_Texture_Extras_GetParser());
	MarathonParser.AddChild(Console_GetParser());
	MarathonParser.AddChild(ExternalDefaultLevelScript_GetParser());
}

// This will reset all values changed by MML scripts which implement ResetValues() method
// and are part of the master MarathonParser tree.
void ResetAllMMLValues()
{
  MarathonParser.ResetChildrenValues();
}
