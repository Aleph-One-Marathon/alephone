#ifndef __LUA_MAP_H
#define __LUA_MAP_H

/*
LUA_MAP.H

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

	Implements Lua map classes
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
#include "lightsource.h"

struct Lua_DamageTypes {
	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg methods[];
	static int length() { return NUMBER_OF_DAMAGE_TYPES; }
	static bool valid(int index) { return index >= 0 && index < NUMBER_OF_DAMAGE_TYPES; }
};

struct Lua_DamageType {
	short index;
	static bool valid(int index) { return Lua_DamageTypes::valid(index); }
	
	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];
};

struct Lua_Polygons {
	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg methods[];
	static int length() { return dynamic_world->polygon_count; }
	static bool valid(int index) { return (index >= 0 && index < dynamic_world->polygon_count); }
};

struct Lua_Platform {
	short index;
	static bool valid(int index) { return (index >= 0 && index < dynamic_world->platform_count); }

	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];

	static int get_active(lua_State *L);
	static int get_ceiling_height(lua_State *L);
	static int get_contracting(lua_State *L);
	static int get_extending(lua_State *L);
	static int get_floor_height(lua_State *L);
	static int get_monster_controllable(lua_State *L);
	static int get_player_controllable(lua_State *L);
	static int get_polygon(lua_State *L);
	static int get_speed(lua_State *L);
	static int set_active(lua_State *L);
	static int set_ceiling_height(lua_State *L);
	static int set_contracting(lua_State *L);
	static int set_extending(lua_State *L);
	static int set_floor_height(lua_State *L);
	static int set_monster_controllable(lua_State *L);
	static int set_player_controllable(lua_State *L);
	static int set_speed(lua_State *L);
};

struct Lua_Polygon {
	short index;
	static const char *name;
	static bool valid(int index) { return Lua_Polygons::valid(index); }

	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];

	static int get_ceiling(lua_State *L);
	static int get_floor(lua_State *L);
	static int get_platform(lua_State *L);
	static int get_x(lua_State *L);
	static int get_y(lua_State *L);
	static int get_z(lua_State *L); // shortcut for floor_height
	static int get_type(lua_State *L);
	static int set_type(lua_State *L);
};

struct Lua_Lights {
	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg methods[];
	static bool valid(int index) { return index >= 0 && index < MAXIMUM_LIGHTS_PER_MAP; }
	static int length() { return MAXIMUM_LIGHTS_PER_MAP; }
};

struct Lua_Light {
	short index;
	static const char *name;
	static bool valid(int index) { return Lua_Lights::valid(index); }

	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];

	static int get_active(lua_State *);
	static int set_active(lua_State *);
};

struct Lua_Tags {
	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg methods[];
	static bool valid(int index) { return index >= 0; }
};

struct Lua_Tag {
	short index;
	static const char *name;
	static bool valid(int index) { return Lua_Tags::valid(index); }
	
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];

	static int get_active(lua_State *);
	static int set_active(lua_State *);
};

int Lua_Map_register (lua_State *L);

#endif

#endif
