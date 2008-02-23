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
	static const luaL_reg methods[];

	static int length() { return dynamic_world->player_count; }
	static bool valid(int index) { return (index >= 0 && index < dynamic_world->player_count); }
};

struct Lua_Player {
	short index;
	static bool valid(int index) { return Lua_Players::valid(index); }

	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];

	static int get_crosshairs(lua_State *L);
	static int get_deaths(lua_State *L);
	static int get_direction(lua_State *L);
	static int get_elevation(lua_State *L);
	static int get_external_velocity(lua_State *L);
	static int get_extravision_duration(lua_State *L);
	static int get_infravision_duration(lua_State *L);
	static int get_internal_velocity(lua_State *L);
	static int get_internal_perpendicular_velocity(lua_State *L);
	static int get_invincibility_duration(lua_State *L);
	static int get_invisibility_duration(lua_State *L);
	static int get_items(lua_State *L);
	static int get_kills(lua_State *L);
	static int get_local(lua_State *L);
	static int get_motion_sensor(lua_State *L);
	static int get_points(lua_State *L);
	static int get_weapons(lua_State *L);
	static int get_x(lua_State *L);
	static int get_y(lua_State *L);
	static int get_z(lua_State *L);
	static int get_zoom(lua_State *L);

	static int set_deaths(lua_State *L);
	static int set_direction(lua_State *L);
	static int set_elevation(lua_State *L);
	static int set_extravision_duration(lua_State *L);
	static int set_infravision_duration(lua_State *L);
	static int set_invincibility_duration(lua_State *L);
	static int set_invisibility_duration(lua_State *L);
	static int set_motion_sensor(lua_State *L);
	static int set_points(lua_State *L);
	static int set_zoom(lua_State *L);

	static int accelerate(lua_State *L);
	static int damage(lua_State *L);
	static int fade_screen(lua_State *L);
	static int position(lua_State *L);
	static int play_sound(lua_State *L);
};

int Lua_Player_register (lua_State *L);

#endif

#endif
