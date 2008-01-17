/*
LUA_PLAYER.CPP

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

#include "lua_player.h"
#include "map.h"
#include "player.h"

#ifdef HAVE_LUA

const char *LUA_PLAYER = "Player";
const char *LUA_PLAYER_GET = "";
const char *LUA_PLAYER_SET = "";

extern void L_Error(const char *message);

struct Lua_Player {
	short player_index;
};

static Lua_Player *toLua_Player(lua_State *L, int index)
{
	Lua_Player *p = (Lua_Player *) lua_touserdata(L, index);
	if (!p) luaL_typerror(L, index, LUA_PLAYER);
	return p;
}

static Lua_Player *checkLua_Player(lua_State *L, int index)
{
	Lua_Player *p;
	luaL_checktype(L, index, LUA_TUSERDATA);
	p = (Lua_Player *) luaL_checkudata(L, index, LUA_PLAYER);
	if (!p) luaL_typerror(L, index, LUA_PLAYER);
	return p;
}

static const luaL_reg Lua_Player_methods[] = {
	{0, 0}
};

static int Lua_Player_get_energy(lua_State *L)
{
	Lua_Player *p = (Lua_Player *) lua_touserdata(L, 1);
	lua_pushnumber(L, get_player_data(p->player_index)->suit_energy);
	return 1;
}

static const luaL_reg Lua_Player_get[] = {
	{"energy", Lua_Player_get_energy},
	{"juice", Lua_Player_get_energy},
	{0, 0 }
};

extern void mark_shield_display_as_dirty();

static int Lua_Player_set_energy(lua_State *L)
{
	Lua_Player *p = (Lua_Player *) lua_touserdata(L, 1);
	if (!lua_isnumber(L, 2))
	{
		luaL_error(L, "player.energy: incorrect argument type");
	}

	int energy = static_cast<int>(lua_tonumber(L, 2));
	if (energy > 3 * PLAYER_MAXIMUM_SUIT_ENERGY)
		energy = 3 * PLAYER_MAXIMUM_SUIT_ENERGY;

	get_player_data(p->player_index)->suit_energy = energy;
	mark_shield_display_as_dirty();
	return 0;
}


static const luaL_reg Lua_Player_set[] = {
	{"energy", Lua_Player_set_energy},
	{"juice", Lua_Player_set_energy},
	{0, 0}
};

static int Lua_Player_tostring(lua_State *L)
{
	lua_pushfstring(L, "Player (%i)", toLua_Player(L, 1)->player_index);
	return 1;
}

static int Lua_Player_index(lua_State *L)
{
	if (lua_isstring(L, 2))
	{
		luaL_checktype(L, 1, LUA_TUSERDATA);
		luaL_checkudata(L, 1, LUA_PLAYER);

		// pop the get table
		lua_pushlightuserdata(L, (void *)&LUA_PLAYER_GET);
		lua_gettable(L, LUA_REGISTRYINDEX);
		
		// get the function from that table
		lua_pushvalue(L, 2);
		lua_gettable(L, -2);

		// execute the function with player as our argument
		lua_pushvalue(L, 1);
		if (lua_pcall(L, 1, 1, 0) == LUA_ERRRUN)
			lua_error(L);

	}
	else
	{
		lua_pushnil(L);
	}
	
	return 1;
}

static int Lua_Player_newindex(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	luaL_checkudata(L, 1, LUA_PLAYER);
	
	// pop the get table
	lua_pushlightuserdata(L, (void *)&LUA_PLAYER_SET);
	lua_gettable(L, LUA_REGISTRYINDEX);
	
	// get the function from that table
	lua_pushvalue(L, 2);
	lua_gettable(L, -2);
	
	if (lua_isnil(L, -1))
	{
		luaL_error(L, "player: no such index");
	}
	
	// execute the function with player, value as our arguments
	lua_pushvalue(L, 1);
	lua_pushvalue(L, 3);
	if (lua_pcall(L, 2, 0, 0) == LUA_ERRRUN)
		lua_error(L);

	return 0;
}


static const luaL_reg Lua_Player_meta[] = {
	{"__index", Lua_Player_index},
	{"__newindex", Lua_Player_newindex},
	{"__tostring", Lua_Player_tostring},
	{0, 0}
};

const char *LUA_PLAYERS = "Players";

int Lua_Players_index(lua_State *L)
{
	if (lua_isnumber(L, 2))
	{
		int player_index = static_cast<int>(lua_tonumber(L, 2));
		if (player_index < 1 || player_index > dynamic_world->player_count)
		{
			lua_pushnil(L);
		}
		else
		{
			Lua_Player *p = (Lua_Player *) lua_newuserdata(L, sizeof(Lua_Player));
			luaL_getmetatable(L, LUA_PLAYER);
			lua_setmetatable(L, -2);
			p->player_index = player_index - 1;
		}
	}
	else
	{
		lua_pushnil(L);
	}

	return 1;
}

int Lua_Players_newindex(lua_State *L)
{
	luaL_error(L, "Players is read-only");
}

int Lua_Players_len(lua_State *L)
{
	lua_pushnumber(L, dynamic_world->player_count);
	return 1;
}

static const luaL_reg Lua_Players_meta[] = {
	{"__index", Lua_Players_index},
	{"__newindex", Lua_Players_newindex},
	{"__len", Lua_Players_len},
	{0, 0}
};

static int Lua_Player_load_compatibility(lua_State *L);

int Lua_Player_register (lua_State *L)
{
	luaL_openlib(L, LUA_PLAYER, Lua_Player_methods, 0); // create methods table, add it to the globals 

	luaL_newmetatable(L, LUA_PLAYER); // create metatable, add to the registry
	luaL_openlib(L, 0, Lua_Player_meta, 0); // fill metatable
	
	lua_setmetatable(L, -2);

	lua_newuserdata(L, 0); // Players

	lua_pushvalue(L, -1);
	luaL_newmetatable(L, LUA_PLAYERS);
	luaL_openlib(L, 0, Lua_Players_meta, 0);

	lua_setmetatable(L, -2);

	lua_setglobal(L, LUA_PLAYERS);
	
	// register the get accessors for Player
	lua_pushlightuserdata(L, (void *) &LUA_PLAYER_GET);
	lua_newtable(L);
	luaL_openlib(L, 0, Lua_Player_get, 0);
	lua_settable(L, LUA_REGISTRYINDEX);

	// register the set accessors for Player
	lua_pushlightuserdata(L, (void *) &LUA_PLAYER_SET);
	lua_newtable(L);
	luaL_openlib(L, 0, Lua_Player_set, 0);
	lua_settable(L, LUA_REGISTRYINDEX);

	Lua_Player_load_compatibility(L);
	
	return 0;
}

static const char *compatibility_script = ""
	"function get_life(player) return Players[player + 1].energy end\n"
	"function set_life(player, shield) Players[player + 1].energy = shield end\n";

static int Lua_Player_load_compatibility(lua_State *L)
{
	luaL_loadbuffer(L, compatibility_script, strlen(compatibility_script), "player_compatibility");
	lua_pcall(L, 0, 0, 0);
};

#endif
