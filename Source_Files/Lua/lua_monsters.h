#ifndef __LUA_MONSTERS_H
#define __LUA_MONSTERS_H

/*
LUA_MONSTERS.H

	Copyright (C) 2008 by Gregory Smith
 
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

	Implements Lua monster classes
*/

#include "cseries.h"

#ifdef HAVE_LUA
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "map.h"
#include "monsters.h"
#include "lua_templates.h"

extern char Lua_Monster_Name[]; // "monster"
typedef L_Class<Lua_Monster_Name> Lua_Monster;

extern char Lua_Monsters_Name[]; // "Monsters"
typedef L_Container<Lua_Monsters_Name, Lua_Monster> Lua_Monsters;

extern char Lua_MonsterAction_Name[]; // "monster_action"
typedef L_Enum<Lua_MonsterAction_Name> Lua_MonsterAction;

extern char Lua_MonsterType_Name[]; // "monster_type"
typedef L_Enum<Lua_MonsterType_Name> Lua_MonsterType;

int Lua_Monsters_register(lua_State *L);

#endif

#endif
