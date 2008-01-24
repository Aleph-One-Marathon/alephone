#ifndef __LUA_PLAYER_H
#define __LUA_PLAYER_H

/*
LUA_PLAYER.H

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

	Implements the Lua Player class
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

struct Lua_Players
{
	static const char *name;
	static const luaL_reg metatable[];

	static int length() { return dynamic_world->player_count; }
	static bool valid(int index) { return (index >= 0 && index < dynamic_world->player_count); }
};

int Lua_Player_register (lua_State *L);

#endif

#endif
