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

#include "ActionQueues.h"
#include "game_window.h"
#include "lua_map.h"
#include "lua_monsters.h"
#include "lua_objects.h"
#include "lua_player.h"
#include "lua_templates.h"
#include "map.h"
#include "monsters.h"
#include "player.h"
#include "screen.h"
#include "SoundManager.h"
#include "ViewControl.h"

#define DONT_REPEAT_DEFINITIONS
#include "item_definitions.h"

#ifdef HAVE_LUA

const float AngleConvert = 360/float(FULL_CIRCLE);

struct Lua_Action_Flags {
	short index;
	static const char *name;
	static bool valid(int index) { return Lua_Players::valid(index); }
	
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];
};

const char *Lua_Action_Flags::name = "action_flags";

extern ModifiableActionQueues *GetGameQueue();

template<uint32 flag> 
static int get_action_flag_T(lua_State *L)
{
	int player_index = L_Index<Lua_Action_Flags>(L, 1);

	if (GetGameQueue()->countActionFlags(player_index))
	{
		uint32 flags = GetGameQueue()->peekActionFlags(player_index, 0);
		lua_pushboolean(L, flags & flag);
	}
	else
	{
		return luaL_error(L, "action flags are only accessible in idle()");
	}

	return 1;
}

template<uint32 flag> 
static int set_action_flag_T(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "action flags: incorrect argument type");
	
	int player_index = L_Index<Lua_Action_Flags>(L, 1);
	if (GetGameQueue()->countActionFlags(player_index))
	{
		if (lua_toboolean(L, 2))
		{
			GetGameQueue()->modifyActionFlags(player_index, flag, flag);
		}
		else
		{
			GetGameQueue()->modifyActionFlags(player_index, 0, flag);
		}
	}
	else
	{
		return luaL_error(L, "action flags are only accessible in idle()");
	}

	return 0;
}

static int set_microphone_action_flag(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "action flags: incorrect argument type");

	if (lua_toboolean(L, 2))
		return luaL_error(L, "you can only disable the microphone button flag");

	int player_index = L_Index<Lua_Action_Flags>(L, 1);
	if (GetGameQueue()->countActionFlags(player_index))
	{
		GetGameQueue()->modifyActionFlags(player_index, 0, _microphone_button);
	}
	else
	{
		return luaL_error(L, "action flags are only accessible in idle()");
	}

	return 0;
}

const luaL_reg Lua_Action_Flags::index_table[] = {
	{"action_trigger", get_action_flag_T<_action_trigger_state>},
	{"cycle_weapons_backward", get_action_flag_T<_cycle_weapons_backward>},
	{"cycle_weapons_forward", get_action_flag_T<_cycle_weapons_forward>},
	{"left_trigger", get_action_flag_T<_left_trigger_state>},
	{"microphone_button", get_action_flag_T<_microphone_button>},
	{"right_trigger", get_action_flag_T<_right_trigger_state>},
	{"toggle_map", get_action_flag_T<_toggle_map>},
	{0, 0}
};

const luaL_reg Lua_Action_Flags::newindex_table[] = {
	{"action_trigger", set_action_flag_T<_action_trigger_state>},
	{"cycle_weapons_backward", set_action_flag_T<_cycle_weapons_backward>},
	{"cycle_weapons_forward", set_action_flag_T<_cycle_weapons_forward>},
	{"left_trigger", set_action_flag_T<_left_trigger_state>},
	{"microphone_button", set_microphone_action_flag},
	{"right_trigger", set_action_flag_T<_right_trigger_state>},
	{"toggle_map", set_action_flag_T<_toggle_map>},
	{0, 0}
};

const luaL_reg Lua_Action_Flags::metatable[] = {
	{"__index", L_TableGet<Lua_Action_Flags>},
	{"__newindex", L_TableSet<Lua_Action_Flags>},
	{0, 0}
};

struct Lua_Side {
	short index;
	static bool valid(int index) { return true; }

	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];
};

const char *Lua_Side::name = "side";

const luaL_reg Lua_Side::index_table[] = {
	{"index", L_TableIndex<Lua_Side>},
	{0, 0}
};

const luaL_reg Lua_Side::newindex_table[] = {
	{0, 0}
};

const luaL_reg Lua_Side::metatable[] = {
	{"__index", L_TableGet<Lua_Side>},
	{"__newindex", L_TableSet<Lua_Side>},
	{0, 0}
};

struct Lua_Player_Items {
	short index;
	static bool valid(int index) { return Lua_Players::valid(index); }
	
	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];

	static int get(lua_State *L);
	static int set(lua_State *L);
};

const char *Lua_Player_Items::name = "player_items";

const luaL_reg Lua_Player_Items::index_table[] = {
	{0, 0}
};

const luaL_reg Lua_Player_Items::newindex_table[] = {
	{0, 0}
};

const luaL_reg Lua_Player_Items::metatable[] = {
	{"__index", Lua_Player_Items::get},
	{"__newindex", Lua_Player_Items::set},
	{0, 0}
};

int Lua_Player_Items::get(lua_State *L)
{
	int player_index = L_Index<Lua_Player_Items>(L, 1);
	int item_type = L_ToIndex<Lua_ItemType>(L, 2);

	player_data *player = get_player_data(player_index);
	int item_count = player->items[item_type];
	if (item_count == NONE) item_count = 0;
	lua_pushnumber(L, item_count);
	return 1;
}

extern void destroy_players_ball(short player_index);
extern void select_next_best_weapon(short player_index);

int Lua_Player_Items::set(lua_State *L)
{
	if (!lua_isnumber(L, 3)) 
		return luaL_error(L, "items: incorrect argument type");

	int player_index = L_Index<Lua_Player_Items>(L, 1);
	player_data *player = get_player_data(player_index);
	int item_type = L_ToIndex<Lua_ItemType>(L, 2);
	int item_count = player->items[item_type];
	item_definition *definition = get_item_definition_external(item_type);
	int new_item_count = static_cast<int>(lua_tonumber(L, 3));
	
	if (new_item_count < 0) 
		luaL_error(L, "items: invalid item count");

	if (item_count == NONE) item_count = 0;
	if (new_item_count == 0) new_item_count = NONE;

	if (new_item_count < item_count)
	{
		if (definition->item_kind == _ball)
		{
			if (find_player_ball_color(player_index) != NONE)
				destroy_players_ball(player_index);
		}
		else
		{
			player->items[item_type] = new_item_count;
			mark_player_inventory_as_dirty(player_index, item_type);
			if (definition->item_kind == _weapon && player->items[item_type] == NONE)
			{
				select_next_best_weapon(player_index);
			}
		}
	}
	else if (new_item_count > item_count)
	{
		while (new_item_count-- > item_count)
		{
			try_and_add_player_item(player_index, item_type);
		}
	}

	return 0;
}
				
const char *Lua_Player::name = "player";

// methods

int Lua_Player_find_action_key_target(lua_State *L)
{
	// no arguments
	short target_type;
	short object_index = find_action_key_target(L_Index<Lua_Player>(L, 1), MAXIMUM_ACTIVATION_RANGE, &target_type);

	if (object_index != NONE)
	{
		switch (target_type)
		{
		case _target_is_platform:
			L_Push<Lua_Platform>(L, object_index);
			break;

		case _target_is_control_panel:
			L_Push<Lua_Side>(L, object_index);
			break;

		default:
			lua_pushnil(L);
			break;
		}
	}
	else
	{
		lua_pushnil(L);
	}

	return 1;
}

int Lua_Player::damage(lua_State *L)
{
	int args = lua_gettop(L);
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "damage: incorrect argument type");
	
	player_data *player = get_player_data(L_Index<Lua_Player>(L, 1));
	if (PLAYER_IS_DEAD(player) || PLAYER_IS_TOTALLY_DEAD(player))
		return 0;

	damage_definition damage;
	damage.type = _damage_crushing;
	damage.base = static_cast<int>(lua_tonumber(L, 2));
	damage.random = 0;
	damage.scale = FIXED_ONE;

	if (args > 2)
	{
		if (lua_isnumber(L, 3))
		{
			damage.type = static_cast<int>(lua_tonumber(L, 3));
		}
		else
			return luaL_error(L, "damage: incorrect type");
	}

	damage_player(player->monster_index, NONE, NONE, &damage, NONE);
	return 0;
}

int Lua_Player::play_sound(lua_State *L)
{
	int args = lua_gettop(L);
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "play_sound: incorrect argument type");
	
	int player_index = L_Index<Lua_Player>(L, 1);
	int sound_index = static_cast<int>(lua_tonumber(L, 2));
	float pitch = 1.0;
	if (args > 2)
	{
		if (lua_isnumber(L, 3))
			pitch = static_cast<float>(lua_tonumber(L, 3));
		else
			return luaL_error(L, "play_sound: incorrect argument type");
	}

	if (local_player_index != player_index)
		return 0;

	SoundManager::instance()->PlaySound(sound_index, NULL, NONE, _fixed(FIXED_ONE * pitch));
	return 0;
}

extern struct physics_constants *get_physics_constants_for_model(short physics_model, uint32 action_flags);
extern void instantiate_physics_variables(struct physics_constants *constants, struct physics_variables *variables, short player_index, bool first_time, bool take_action);

int Lua_Player::position(lua_State *L)
{
	if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		return luaL_error(L, ("position: incorrect argument type"));

	int polygon_index = 0;
	if (lua_isnumber(L, 5))
	{
		polygon_index = static_cast<int>(lua_tonumber(L, 5));
		if (!Lua_Polygons::valid(polygon_index))
			return luaL_error(L, ("position: invalid polygon index"));
	}
	else if (L_Is<Lua_Polygon>(L, 5))
	{
		polygon_index = L_Index<Lua_Polygon>(L, 5);
	}
	else
		return luaL_error(L, ("position: incorrect argument type"));

	int player_index = L_Index<Lua_Player>(L, 1);
	player_data *player = get_player_data(player_index);
	object_data *object = get_object_data(player->object_index);

	world_point3d location;
	location.x = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	location.y = static_cast<int>(lua_tonumber(L, 3) * WORLD_ONE);
	location.z = static_cast<int>(lua_tonumber(L, 4) * WORLD_ONE);
	
	translate_map_object(player->object_index, &location, polygon_index);
	player->variables.position.x = WORLD_TO_FIXED(object->location.x);
	player->variables.position.y = WORLD_TO_FIXED(object->location.y);
	player->variables.position.z = WORLD_TO_FIXED(object->location.z);
	
	instantiate_physics_variables(get_physics_constants_for_model(static_world->physics_model, 0), &player->variables, player_index, false, false);
	return 0;
}

int Lua_Player_teleport(lua_State *L)
{
	if (!lua_isnumber(L, 2) && !L_Is<Lua_Polygon>(L, 2))
		return luaL_error(L, "teleport(): incorrect argument type");

	int destination = -1;
	if (lua_isnumber(L, 2))
		destination = static_cast<int>(lua_tonumber(L, 2));
	else 
		destination = L_Index<Lua_Polygon>(L, 2);

	int player_index = L_Index<Lua_Player>(L, 1);
	
	player_data *player = get_player_data(player_index);
	monster_data *monster = get_monster_data(player->monster_index);

	SET_PLAYER_TELEPORTING_STATUS(player, true);
	monster->action = _monster_is_teleporting;
	player->teleporting_phase = 0;
	player->delay_before_teleport = 0;

	player->teleporting_destination = destination;
	if (local_player_index == player_index)
		start_teleporting_effect(true);
	play_object_sound(player->object_index, Sound_TeleportOut());
	return 0;
}

int Lua_Player_teleport_to_level(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "teleport_to_level(): incorrect argument type");

	int level = static_cast<int>(lua_tonumber(L, 2));
	int player_index = L_Index<Lua_Player>(L, 1);
	
	player_data *player = get_player_data(player_index);
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

static int Lua_Player_get_action_flags(lua_State *L)
{
	L_Push<Lua_Action_Flags>(L, L_Index<Lua_Player>(L, 1));
	return 1;
}

static int Lua_Player_get_color(lua_State *L)
{
	lua_pushnumber(L, get_player_data(L_Index<Lua_Player>(L, 1))->color);
	return 1;
}

static int Lua_Player_get_dead(lua_State *L)
{
	player_data *player = get_player_data(L_Index<Lua_Player>(L, 1));
	lua_pushboolean(L, (PLAYER_IS_DEAD(player) || PLAYER_IS_TOTALLY_DEAD(player)));
	return 1;
}

static int Lua_Player_get_energy(lua_State *L)
{
	lua_pushnumber(L, get_player_data(L_Index<Lua_Player>(L, 1))->suit_energy);
	return 1;
}

int Lua_Player::get_elevation(lua_State *L)
{
	double angle = FIXED_INTEGERAL_PART(get_player_data(L_Index<Lua_Player>(L, 1))->variables.elevation) * AngleConvert;
	lua_pushnumber(L, angle);
	return 1;
}

int Lua_Player::get_direction(lua_State *L)
{
	double angle = FIXED_INTEGERAL_PART(get_player_data(L_Index<Lua_Player>(L, 1))->variables.direction) * AngleConvert;
	lua_pushnumber(L, angle);
	return 1;
}

int Lua_Player::get_items(lua_State *L)
{
	fprintf(stderr, "pushing items\n");
	L_Push<Lua_Player_Items>(L, L_Index<Lua_Player>(L, 1));
	return 1;
}

int Lua_Player::get_local(lua_State *L)
{
	lua_pushboolean(L, L_Index<Lua_Player>(L, 1) == local_player_index);
	return 1;
}

static int Lua_Player_get_monster(lua_State *L)
{
	L_Push<Lua_Monster>(L, get_player_data(L_Index<Lua_Player>(L, 1))->monster_index);
	return 1;
}

static int Lua_Player_get_name(lua_State *L)
{
	lua_pushstring(L, get_player_data(L_Index<Lua_Player>(L, 1))->name);
	return 1;
}

static int Lua_Player_get_oxygen(lua_State *L)
{
	lua_pushnumber(L, get_player_data(L_Index<Lua_Player>(L, 1))->suit_oxygen);
	return 1;
}

static int Lua_Player_get_polygon(lua_State *L)
{
	L_Push<Lua_Polygon>(L, get_player_data(L_Index<Lua_Player>(L, 1))->supporting_polygon_index);
	return 1;
}

static int Lua_Player_get_team(lua_State *L)
{
	lua_pushnumber(L, get_player_data(L_Index<Lua_Player>(L, 1))->team);
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

int Lua_Player::get_x(lua_State *L)
{
	lua_pushnumber(L, (double) get_player_data(L_Index<Lua_Player>(L, 1))->location.x / WORLD_ONE);
	return 1;
}

int Lua_Player::get_y(lua_State *L)
{
	lua_pushnumber(L, (double) get_player_data(L_Index<Lua_Player>(L, 1))->location.y / WORLD_ONE);
	return 1;
}

int Lua_Player::get_z(lua_State *L)
{
	lua_pushnumber(L, (double) get_player_data(L_Index<Lua_Player>(L, 1))->location.z / WORLD_ONE);
	return 1;
}

const luaL_reg Lua_Player::index_table[] = {
	{"action_flags", Lua_Player_get_action_flags},
	{"color", Lua_Player_get_color},
	{"damage", L_TableFunction<Lua_Player::damage>},
	{"dead", Lua_Player_get_dead},
	{"direction", Lua_Player::get_direction},
	{"energy", Lua_Player_get_energy},
	{"elevation", Lua_Player::get_elevation},
	{"find_action_key_target", L_TableFunction<Lua_Player_find_action_key_target>},
	{"index", L_TableIndex<Lua_Player>},
	{"items", Lua_Player::get_items},
	{"local_", Lua_Player::get_local},
	{"juice", Lua_Player_get_energy},
	{"life", Lua_Player_get_energy},
	{"monster", Lua_Player_get_monster},
	{"name", Lua_Player_get_name},
	{"oxygen", Lua_Player_get_oxygen},
	{"pitch", Lua_Player::get_elevation},
	{"play_sound", L_TableFunction<Lua_Player::play_sound>},
	{"polygon", Lua_Player_get_polygon},
	{"position", L_TableFunction<Lua_Player::position>},
	{"team", Lua_Player_get_team},
	{"teleport", L_TableFunction<Lua_Player_teleport>},
	{"teleport_to_level", L_TableFunction<Lua_Player_teleport_to_level>},
	{"x", Lua_Player::get_x},
	{"y", Lua_Player::get_y},
	{"yaw", Lua_Player::get_direction},
	{"z", Lua_Player::get_z},
	{0, 0}
};

extern void mark_shield_display_as_dirty();

static int Lua_Player_set_color(lua_State *L)
{
	if (!lua_isnumber(L, 2))
	{
		return luaL_error(L, "color: incorrect argument type");
	}
	

	int color = static_cast<int>(lua_tonumber(L, 2));
	if (color < 0 || color > NUMBER_OF_TEAM_COLORS)
	{
		luaL_error(L, "player.color: invalid color");
	}
	get_player_data(L_Index<Lua_Player>(L, 1))->color = color;
	
	return 0;
}

int Lua_Player::set_direction(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "direction: incorrect argument type");

	double facing = static_cast<double>(lua_tonumber(L, 2));
	int player_index = L_Index<Lua_Player>(L, 1);
	player_data *player = get_player_data(player_index);
	player->variables.direction = INTEGER_TO_FIXED((int)(facing/AngleConvert));
	instantiate_physics_variables(get_physics_constants_for_model(static_world->physics_model, 0), &player->variables, player_index, false, false);
	return 0;
}

int Lua_Player::set_elevation(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "elevation: incorrect argument type");
	
	double elevation = static_cast<double>(lua_tonumber(L, 2));
	if (elevation > 180) elevation -= 360.0;
	int player_index = L_Index<Lua_Player>(L, 1);
	player_data *player = get_player_data(player_index);
	player->variables.elevation = INTEGER_TO_FIXED((int)(elevation/AngleConvert));
	instantiate_physics_variables(get_physics_constants_for_model(static_world->physics_model, 0), &player->variables, player_index, false, false);
	return 0;
}

static int Lua_Player_set_energy(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "energy: incorrect argument type");

	int energy = static_cast<int>(lua_tonumber(L, 2));
	if (energy > 3 * PLAYER_MAXIMUM_SUIT_ENERGY)
		energy = 3 * PLAYER_MAXIMUM_SUIT_ENERGY;

	get_player_data(L_Index<Lua_Player>(L, 1))->suit_energy = energy;
	mark_shield_display_as_dirty();

	return 0;
}

static int Lua_Player_set_oxygen(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "oxygen: incorrect argument type");
	
	int oxygen = static_cast<int>(lua_tonumber(L, 2));
	if (oxygen > PLAYER_MAXIMUM_SUIT_OXYGEN)
		oxygen = PLAYER_MAXIMUM_SUIT_OXYGEN;

	get_player_data(L_Index<Lua_Player>(L, 1))->suit_oxygen = oxygen;
	mark_shield_display_as_dirty();

	return 0;
}

static int Lua_Player_set_team(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "team: incorrect argument type");

	int team = static_cast<int>(lua_tonumber(L, 2));
	if (team < 0 || team >= NUMBER_OF_TEAM_COLORS)
	{
		luaL_error(L, "player.team: invalid team");
	}
	get_player_data(L_Index<Lua_Player>(L, 1))->team = team;

	return 0;
}

const luaL_reg Lua_Player::newindex_table[] = {
	{"color", Lua_Player_set_color},
	{"direction", Lua_Player::set_direction},
	{"elevation", Lua_Player::set_elevation},
	{"energy", Lua_Player_set_energy},
	{"juice", Lua_Player_set_energy},
	{"life", Lua_Player_set_energy},
	{"oxygen", Lua_Player_set_oxygen},
	{"pitch", Lua_Player::set_elevation},
	{"position", Lua_Player::position},
	{"team", Lua_Player_set_team},
	{"yaw", Lua_Player::set_direction},
	{0, 0}
};

static int Lua_Player_tostring(lua_State *L)
{
	lua_pushfstring(L, "Player %d", L_Index<Lua_Player>(L, 1));
	return 1;
}

const luaL_reg Lua_Player::metatable[] = {
	{"__index", L_TableGet<Lua_Player>},
	{"__newindex", L_TableSet<Lua_Player>},
	{"__tostring", Lua_Player_tostring},
	{0, 0}
};

const char* Lua_Players::name = "Players";

const luaL_reg Lua_Players::methods[] = {
	{0, 0}
};

const luaL_reg Lua_Players::metatable[] = {
	{"__index", L_GlobalIndex<Lua_Players, Lua_Player>},
	{"__newindex", L_GlobalNewindex<Lua_Players>},
	{"__len", L_GlobalLength<Lua_Players>},
	{"__call", L_GlobalCall<Lua_Players, Lua_Player>},
	{0, 0}
};

static int Lua_Player_load_compatibility(lua_State *L);

int Lua_Player_register (lua_State *L)
{
	L_Register<Lua_Action_Flags>(L);
	L_Register<Lua_Player_Items>(L);
	L_Register<Lua_Player>(L);
	L_Register<Lua_Side>(L);

	L_GlobalRegister<Lua_Players>(L);
	
	Lua_Player_load_compatibility(L);
	
	return 0;
}

static const char *compatibility_script = ""
	"function add_item(player, item_type) Players[player].items[item_type] = Players[player].items[item_type] + 1 end\n"
	"function count_item(player, item_type) return Players[player].items[item_type] end\n"
	"function destroy_ball(player) for i in ItemTypes() do if i.ball then Players[player].items[i] = 0 end end end\n"
	"function get_life(player) return Players[player].energy end\n"
	"function get_oxygen(player) return Players[player].oxygen end\n"
	"function get_player_angle(player) return Players[player].yaw, Players[player].pitch end\n"
	"function get_player_color(player) return Players[player].color end\n"
	"function get_player_name(player) return Players[player].name end\n"
	"function get_player_polygon(player) return Players[player].polygon.index end\n"
	"function get_player_position(player) return Players[player].x, Players[player].y, Players[player].z end\n"
	"function get_player_team(player) return Players[player].team end\n"
	"function inflict_damage(player, amount, type) if (type) then Players[player]:damage(amount, type) else Players[player]:damage(amount) end end\n"
	"function local_player_index() for p in Players() do if p.local_ then return p.index end end end\n"
	"function number_of_players() return # Players end\n"
	"function player_is_dead(player) return Players[player].dead end\n"
	"function player_to_monster_index(player) return Players[player].monster.index end\n"
	"function play_sound(player, sound, pitch) Players[player]:play_sound(sound, pitch) end\n"
	"function remove_item(player, item_type) if Players[player].items[item_type] > 0 then Players[player].items[item_type] = Players[player].items[item_type] - 1 end end\n"
	"function set_life(player, shield) Players[player].energy = shield end\n"
	"function set_oxygen(player, oxygen) Players[player].oxygen = oxygen end\n"
	"function set_player_angle(player, yaw, pitch) Players[player].yaw = yaw Players[player].pitch = pitch + 360.0 end\n"
	"function set_player_color(player, color) Players[player].color = color end\n"
	"function set_player_position(player, x, y, z, polygon) Players[player]:position(x, y, z, polygon) end\n"
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
