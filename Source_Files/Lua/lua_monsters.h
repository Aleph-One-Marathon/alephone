#ifndef __LUA_MONSTERS_H
#define __LUA_MONSTERS_H

/*
LUA_MONSTERS.H

	Copyright (C) 2008 by Gregory Smith
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
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

struct Lua_Monsters {
	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg methods[];
	static int length() { return MAXIMUM_MONSTERS_PER_MAP; }
	static bool valid(int);
};

struct Lua_Monster {
	short index;
	static bool valid(int index) { return Lua_Monsters::valid(index); }

	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];

	static int valid(lua_State *L);

	static int get_action(lua_State *L);
	static int get_active(lua_State *L);
	static int get_mode(lua_State *L);
	static int get_player(lua_State *L);
	static int get_polygon(lua_State *L);
	static int get_type(lua_State *L);
	static int get_vitality(lua_State *L);
	static int set_active(lua_State *L);
	static int set_vitality(lua_State *L);
};

int Lua_Monsters_register(lua_State *L);

#endif

#endif
