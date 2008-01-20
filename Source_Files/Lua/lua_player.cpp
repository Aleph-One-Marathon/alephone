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
#include "monsters.h"
#include "player.h"
#include "screen.h"
#include "SoundManager.h"
#include "ViewControl.h"

#ifdef HAVE_LUA

const char *LUA_PLAYER = "Player";
const char *LUA_PLAYER_GET = LUA_PLAYER + 1;
const char *LUA_PLAYER_SET = LUA_PLAYER + 2;

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

static int Lua_Player_index(lua_State *L, int index)
{
	Lua_Player *p = toLua_Player(L, index);
	return p->player_index;
}

// methods

static int Lua_Player_teleport(lua_State *L)
{
	Lua_Player *p = toLua_Player(L, 1);
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "player:teleport(): incorrect argument type");

	int destination = static_cast<int>(lua_tonumber(L, 2));
	
	player_data *player = get_player_data(p->player_index);
	monster_data *monster = get_monster_data(player->monster_index);

	SET_PLAYER_TELEPORTING_STATUS(player, true);
	monster->action = _monster_is_teleporting;
	player->teleporting_phase = 0;
	player->delay_before_teleport = 0;

	player->teleporting_destination = destination;
	if (local_player_index == p->player_index)
		start_teleporting_effect(true);
	play_object_sound(player->object_index, Sound_TeleportOut());
	return 0;
}

static int Lua_Player_teleport_to_level(lua_State *L)
{
	Lua_Player *p = toLua_Player(L, 1);
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "player:teleport_to_level(): incorrect argument type");

	int level = static_cast<int>(lua_tonumber(L, 2));
	
	player_data *player = get_player_data(p->player_index);
	monster_data *monster = get_monster_data(player->monster_index);
	
	SET_PLAYER_TELEPORTING_STATUS(player, true);
	monster->action = _monster_is_teleporting;
	player->teleporting_phase = 0;
	player->delay_before_teleport = 0;

	player->teleporting_destination = -level - 1;
	if (View_DoInterlevelTeleportOutEffects()) {
		start_teleporting_effect(true);
		play_object_sound(player->object_index, Sound_TeleportOut());
	}
	return 0;
}

// get accessors

static int Lua_Player_get_color(lua_State *L)
{
	lua_pushnumber(L, get_player_data(Lua_Player_index(L, 1))->color);
	return 1;
}

static int Lua_Player_get_dead(lua_State *L)
{
	player_data *player = get_player_data(Lua_Player_index(L, 1));
	lua_pushboolean(L, (PLAYER_IS_DEAD(player) || PLAYER_IS_TOTALLY_DEAD(player)));
	return 1;
}

static int Lua_Player_get_energy(lua_State *L)
{
	lua_pushnumber(L, get_player_data(Lua_Player_index(L, 1))->suit_energy);
	return 1;
}

static int Lua_Player_get_index(lua_State *L)
{
	lua_pushnumber(L, Lua_Player_index(L, 1));
	return 1;
}

static int Lua_Player_get_name(lua_State *L)
{
	lua_pushstring(L, get_player_data(Lua_Player_index(L, 1))->name);
	return 1;
}

static int Lua_Player_get_oxygen(lua_State *L)
{
	lua_pushnumber(L, get_player_data(Lua_Player_index(L, 1))->suit_oxygen);
	return 1;
}

static int Lua_Player_get_team(lua_State *L)
{
	lua_pushnumber(L, get_player_data(Lua_Player_index(L, 1))->team);
	return 1;
}

static int Lua_Player_get_teleport(lua_State *L)
{
	lua_pushcfunction(L, Lua_Player_teleport);
	return 1;
}

static int Lua_Player_get_teleport_to_level(lua_State *L)
{
	lua_pushcfunction(L, Lua_Player_teleport_to_level);
	return 1;
}

static const luaL_reg Lua_Player_get[] = {
	{"color", Lua_Player_get_color},
	{"dead", Lua_Player_get_dead},
	{"energy", Lua_Player_get_energy},
	{"index", Lua_Player_get_index},
	{"juice", Lua_Player_get_energy},
	{"life", Lua_Player_get_energy},
	{"name", Lua_Player_get_name},
	{"oxygen", Lua_Player_get_oxygen},
	{"team", Lua_Player_get_team},
	{"teleport", Lua_Player_get_teleport},
	{"teleport_to_level", Lua_Player_get_teleport_to_level},
	{0, 0 }
};

extern void mark_shield_display_as_dirty();

static int Lua_Player_set_color(lua_State *L)
{
	if (!lua_isnumber(L, 2))
	{
		return luaL_error(L, "player.color: incorrect argument type");
	}
	

	int color = static_cast<int>(lua_tonumber(L, 2));
	if (color < 0 || color > NUMBER_OF_TEAM_COLORS)
	{
		luaL_error(L, "player.color: invalid color");
	}
	get_player_data(Lua_Player_index(L, 1))->color = color;
	
	return 0;
}

static int Lua_Player_set_energy(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "player.energy: incorrect argument type");

	int energy = static_cast<int>(lua_tonumber(L, 2));
	if (energy > 3 * PLAYER_MAXIMUM_SUIT_ENERGY)
		energy = 3 * PLAYER_MAXIMUM_SUIT_ENERGY;

	get_player_data(Lua_Player_index(L, 1))->suit_energy = energy;
	mark_shield_display_as_dirty();

	return 0;
}

static int Lua_Player_set_oxygen(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "player.oxygen: incorrect argument type");
	
	int oxygen = static_cast<int>(lua_tonumber(L, 2));
	if (oxygen > PLAYER_MAXIMUM_SUIT_OXYGEN)
		oxygen = PLAYER_MAXIMUM_SUIT_OXYGEN;

	get_player_data(Lua_Player_index(L, 1))->suit_oxygen = oxygen;
	mark_shield_display_as_dirty();

	return 0;
}

static int Lua_Player_set_team(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "player.team: incorrect argument type");

	int team = static_cast<int>(lua_tonumber(L, 2));
	if (team < 0 || team >= NUMBER_OF_TEAM_COLORS)
	{
		luaL_error(L, "player.team: invalid team");
	}
	get_player_data(Lua_Player_index(L, 1))->team = team;

	return 0;
}

static const luaL_reg Lua_Player_set[] = {
	{"color", Lua_Player_set_color},
	{"energy", Lua_Player_set_energy},
	{"juice", Lua_Player_set_energy},
	{"life", Lua_Player_set_energy},
	{"oxygen", Lua_Player_set_oxygen},
	{"team", Lua_Player_set_team},
	{0, 0}
};

static int Lua_Player_tostring(lua_State *L)
{
	lua_pushfstring(L, "Player %d", toLua_Player(L, 1)->player_index);
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

		if (lua_isfunction(L, -1))
		{
			// execute the function with player as our argument
			lua_pushvalue(L, 1);
			if (lua_pcall(L, 1, 1, 0) == LUA_ERRRUN)
			{
				// report the error as being on this line
				luaL_where(L, 1);
				lua_pushvalue(L, -2);
				lua_concat(L, 2);
				lua_error(L);
			}
		}
		else
		{
			lua_pushnil(L);
		}
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
	
	// pop the set table
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
	{
		// report the error as being on this line
		luaL_where(L, 1);
		lua_pushvalue(L, -2);
		lua_concat(L, 2);
		lua_error(L);
	}

	return 0;
}


static const luaL_reg Lua_Player_meta[] = {
	{"__index", Lua_Player_index},
	{"__newindex", Lua_Player_newindex},
	{"__tostring", Lua_Player_tostring},
	{0, 0}
};

const char *LUA_PLAYERS = "Players";

static int Lua_Players_iterator(lua_State *L);

static int Lua_Players_call(lua_State *L)
{
	lua_pushnumber(L, 0); // player index
	lua_pushcclosure(L, Lua_Players_iterator, 1);
	return 1;
}

static int Lua_Players_iterator(lua_State *L)
{
	int player_index = static_cast<int>(lua_tonumber(L, lua_upvalueindex(1)));
	if (player_index < dynamic_world->player_count)
	{
		Lua_Player *p = (Lua_Player *) lua_newuserdata(L, sizeof(Lua_Player));
		luaL_getmetatable(L, LUA_PLAYER);
		lua_setmetatable(L, -2);
		p->player_index = player_index;
		
		lua_pushnumber(L, ++player_index);
		lua_replace(L, lua_upvalueindex(1));
	}
	else
	{
		lua_pushnil(L);
	}

	return 1;
}

int Lua_Players_index(lua_State *L)
{
	if (lua_isnumber(L, 2))
	{
		int player_index = static_cast<int>(lua_tonumber(L, 2));
		if (player_index < 0 || player_index >= dynamic_world->player_count)
		{
			lua_pushnil(L);
		}
		else
		{
			Lua_Player *p = (Lua_Player *) lua_newuserdata(L, sizeof(Lua_Player));
			luaL_getmetatable(L, LUA_PLAYER);
			lua_setmetatable(L, -2);
			p->player_index = player_index;
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
	{"__call", Lua_Players_call},
	{0, 0}
};

static int Lua_Player_load_compatibility(lua_State *L);

int Lua_Player_register (lua_State *L)
{
	luaL_newmetatable(L, LUA_PLAYER); // create Player metatable, add to the registry
	luaL_openlib(L, 0, Lua_Player_meta, 0); // fill metatable
	
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
	"function get_life(player) return Players[player].energy end\n"
	"function get_oxygen(player) return Players[player].oxygen end\n"
	"function get_player_color(player) return Players[player].color end\n"
	"function get_player_name(player) return Players[player].name end\n"
	"function get_player_team(player) return Players[player].team end\n"
	"function set_life(player, shield) Players[player].energy = shield end\n"
	"function number_of_players() return # Players end\n"
	"function player_is_dead(player) return Players[player].dead end\n"
	"function set_oxygen(player, oxygen) Players[player].oxygen = oxygen end\n"
	"function set_player_color(player, color) Players[player].color = color end\n"
	"function set_player_team(player, team) Players[player].team = team end\n"
	"function teleport_player(player, polygon) Players[player]:teleport(polygon) end\n"
	"function teleport_player_to_level(player, level) Players[player]:teleport_to_level(level) end\n"
	;

static int Lua_Player_load_compatibility(lua_State *L)
{
	luaL_loadbuffer(L, compatibility_script, strlen(compatibility_script), "player_compatibility");
	lua_pcall(L, 0, 0, 0);
};

#endif
