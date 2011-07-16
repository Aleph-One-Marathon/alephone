/*
LUA_SAVED_OBJECTS.H

	Copyright (C) 2010 by Gregory Smith
 
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

	Implements Lua map objects classes
*/

#ifndef __LUA_SAVED_OBJECTS_H
#define __LUA_SAVED_OBJECTS_H

#include "cseries.h"

#ifdef HAVE_LUA
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "map.h"
#include "lua_templates.h"

extern char Lua_Goal_Name[]; // "goal"
typedef L_Class<Lua_Goal_Name> Lua_Goal;

extern char Lua_Goals_Name[]; // "Goals"
typedef L_Container<Lua_Goals_Name, Lua_Goal> Lua_Goals;

extern char Lua_ItemStart_Name[]; // "item_start"
typedef L_Class<Lua_ItemStart_Name> Lua_ItemStart;

extern char Lua_ItemStarts_Name[]; // "ItemStarts"
typedef L_Container<Lua_ItemStarts_Name, Lua_ItemStart> Lua_ItemStarts;

extern char Lua_MonsterStart_Name[]; // "monster_start"
typedef L_Class<Lua_MonsterStart_Name> Lua_MonsterStart;

extern char Lua_MonsterStarts_Name[]; // "MonsterStarts"
typedef L_Container<Lua_MonsterStarts_Name, Lua_MonsterStart> Lua_MonsterStarts;

extern char Lua_PlayerStart_Name[]; // "player_start"
typedef L_Class<Lua_PlayerStart_Name> Lua_PlayerStart;

extern char Lua_PlayerStarts_Name[]; // "PlayerStarts";
typedef L_Container<Lua_PlayerStarts_Name, Lua_PlayerStart> Lua_PlayerStarts;

extern char Lua_SoundObject_Name[]; // "sound_object"
typedef L_Class<Lua_SoundObject_Name> Lua_SoundObject;

extern char Lua_SoundObjects_Name[]; // "SoundObjects"
typedef L_Container<Lua_SoundObjects_Name, Lua_SoundObject> Lua_SoundObjects;

int Lua_Saved_Objects_register(lua_State* L);

#endif

#endif
