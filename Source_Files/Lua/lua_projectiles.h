#ifndef __LUA_PROJECTILES_H
#define __LUA_PROJECTILES_H
/*
LUA_PROJECTILES.H

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

	Implements the Lua projectile class
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
#include "projectiles.h"

struct Lua_Projectiles {
	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg methods[];
	static int length() { return MAXIMUM_PROJECTILES_PER_MAP; }
	static bool valid(int);

	// methods
	static int new_projectile(lua_State *L);
};

struct Lua_Projectile {
	short index;
	static bool valid(int index) { return Lua_Projectiles::valid(index); }
	
	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];

	static int get_damage_scale(lua_State *L);
	static int get_elevation(lua_State *L);
	static int get_facing(lua_State *L);
	static int get_gravity(lua_State *L);
	static int get_owner(lua_State *L);
	static int get_polygon(lua_State *L);
	static int get_target(lua_State *L);
	static int get_type(lua_State *L);
	static int get_x(lua_State *L);
	static int get_y(lua_State *L);
	static int get_z(lua_State *L);
	static int set_damage_scale(lua_State *L);
	static int set_elevation(lua_State *L);
	static int set_facing(lua_State *L);
	static int set_gravity(lua_State *L);
	static int set_owner(lua_State *L);
	static int set_polygon(lua_State *L);
	static int set_target(lua_State *L);
	static int set_x(lua_State *L);
	static int set_y(lua_State *L);
	static int set_z(lua_State *L);

	static int position(lua_State *L);
};

int Lua_Projectiles_register(lua_State *L);

#endif

#endif
