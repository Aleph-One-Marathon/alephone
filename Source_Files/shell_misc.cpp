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

	Created for non-duplication of code between mac and SDL ports.
	(Loren Petrich and others)

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Disabled network_speaker_idle_proc under Carbon

Mar 08, 2002 (Woody Zenfell):
    Enabled network_microphone_idle_proc (in global_idle_proc()) under SDL
*/

bool chat_input_mode = false;

#include "cseries.h"

#include "XML_ParseTreeRoot.h"
#include "interface.h"
#include "world.h"
#include "screen.h"
#include "map.h"
#include "shell.h"
#include "preferences.h"
#include "vbl.h"
#include "player.h"
#include "Music.h"
#include "items.h"
#include "TextStrings.h"
#include "InfoTree.h"

#include <ctype.h>


extern void process_new_item_for_reloading(short player_index, short item_type);
extern bool try_and_add_player_item(short player_index,	short type);
extern void mark_shield_display_as_dirty();
extern void mark_oxygen_display_as_dirty();
extern void accelerate_monster(short monster_index,	world_distance vertical_velocity, 
							   angle direction, world_distance velocity);
extern void update_interface(short time_elapsed);


/*
 *  Called regularly during event loops
 */

void global_idle_proc(void)
{
	Music::instance()->Idle();
	SoundManager::instance()->Idle();
}

/*
 *  Special version of malloc() used for level transitions, frees some
 *  memory if possible
 */

void *level_transition_malloc(
	size_t size)
{
	void *ptr= malloc(size);
	if (!ptr)
	{
		SoundManager::instance()->UnloadAllSounds();
		
		ptr= malloc(size);
		if (!ptr)
		{
			unload_all_collections();
			
			ptr= malloc(size);
		}
	}
	
	return ptr;
}
