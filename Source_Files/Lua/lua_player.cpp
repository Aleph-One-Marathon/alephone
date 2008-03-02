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
#include "Crosshairs.h"
#include "fades.h"
#include "game_window.h"
#include "lua_map.h"
#include "lua_monsters.h"
#include "lua_objects.h"
#include "lua_player.h"
#include "lua_templates.h"
#include "map.h"
#include "monsters.h"
#include "player.h"
#include "network_games.h"
#include "Random.h"
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

struct Lua_Crosshairs {
	short index;
	static const char *name;
	static bool valid(int index) { return Lua_Players::valid(index); }
	
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];

	static int get_active(lua_State *L);

	static int set_active(lua_State *L);
};

const char *Lua_Crosshairs::name = "crosshairs";

int Lua_Crosshairs::get_active(lua_State *L)
{
	int player_index = L_Index<Lua_Crosshairs>(L, 1);
	if (player_index == local_player_index)
	{
		lua_pushboolean(L, Crosshairs_IsActive());
		return 1;
	}
	else
	{
		return 0;
	}
}

const luaL_reg Lua_Crosshairs::index_table[] = {
	{"active", Lua_Crosshairs::get_active},
	{0, 0}
};

int Lua_Crosshairs::set_active(lua_State *L)
{
	int player_index = L_Index<Lua_Crosshairs>(L, 1);
	if (player_index == local_player_index)
	{
		if (!lua_isboolean(L, 2))
			return luaL_error(L, "active: incorrect argument type");

		Crosshairs_SetActive(lua_toboolean(L, 2));
	}
	
	return 0;
}

const luaL_reg Lua_Crosshairs::newindex_table[] = {
	{"active", Lua_Crosshairs::set_active},
	{0, 0}
};

const luaL_reg Lua_Crosshairs::metatable[] = {
	{"__index", L_TableGet<Lua_Crosshairs>},
	{"__newindex", L_TableSet<Lua_Crosshairs>},
	{0, 0}
};

struct Lua_OverlayColor {
	short index;
	static bool valid(int index) { return index >= 0 && index < 8; }
	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];
};

const char *Lua_OverlayColor::name = "overlay_color";
const luaL_reg Lua_OverlayColor::index_table[] = {
	{"index", L_TableIndex<Lua_OverlayColor>},
	{0, 0}
};

const luaL_reg Lua_OverlayColor::newindex_table[] = { {0, 0} };
const luaL_reg Lua_OverlayColor::metatable[] = {
	{"__eq", L_Equals<Lua_OverlayColor>},
	{"__index", L_TableGet<Lua_OverlayColor>},
	{"__newindex", L_TableSet<Lua_OverlayColor>},
	{0, 0}
};

struct Lua_Overlays {
	short index;
	static bool valid(int index) { return Lua_Players::valid(index); }
	
	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];

	static int get(lua_State *L);
};

struct Lua_Overlay {
	short index;
	short player_index;

	static bool valid(int index) { return Lua_Overlays::valid(index); }

	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];

	static int set_icon(lua_State *L);
	static int set_text(lua_State *L);
	static int set_text_color(lua_State *L);

	static int clear(lua_State *L);
	static int fill_icon(lua_State *L);
};

const char *Lua_Overlay::name = "overlay";

int Lua_Overlay::clear(lua_State *L)
{
	Lua_Overlay *t = L_To<Lua_Overlay>(L, 1);
	if (t->player_index == local_player_index)
	{
		SetScriptHUDIcon(t->index, 0, 0);
		SetScriptHUDText(t->index, 0);
	}

	return 0;
}

int Lua_Overlay::fill_icon(lua_State *L)
{
	Lua_Overlay *t = L_To<Lua_Overlay>(L, 1);
	if (t->player_index == local_player_index)
	{
		int color = L_ToIndex<Lua_OverlayColor>(L, 2);
		SetScriptHUDSquare(t->index, color);
	}

	return 0;
}

const luaL_reg Lua_Overlay::index_table[] = {
	{"clear", L_TableFunction<Lua_Overlay::clear>},
	{"fill_icon", L_TableFunction<Lua_Overlay::fill_icon>},
	{0, 0}
};

int Lua_Overlay::set_icon(lua_State *L)
{
	Lua_Overlay *t = L_To<Lua_Overlay>(L, 1);
	if (t->player_index == local_player_index)
	{
		if (lua_isstring(L, 2))
		{
			SetScriptHUDIcon(t->index, lua_tostring(L, 2), lua_strlen(L, 2));
		}
		else
		{
			SetScriptHUDIcon(t->index, 0, 0);
		}
	}

	return 0;
}

int Lua_Overlay::set_text(lua_State *L)
{
	Lua_Overlay *t = L_To<Lua_Overlay>(L, 1);
	if (t->player_index == local_player_index)
	{
		const char *text = 0;
		if (lua_isstring(L, 2)) 
			text = lua_tostring(L, 2);
		
		SetScriptHUDText(t->index, text);
	}

	return 0;
}

int Lua_Overlay::set_text_color(lua_State *L)
{
	Lua_Overlay *t = L_To<Lua_Overlay>(L, 1);
	if (t->player_index == local_player_index)
	{
		int color = L_ToIndex<Lua_OverlayColor>(L, 2);
		SetScriptHUDColor(t->index, color);
	}

	return 0;
}

const luaL_reg Lua_Overlay::newindex_table[] = {
	{"color", Lua_Overlay::set_text_color},
	{"icon", Lua_Overlay::set_icon},
	{"text", Lua_Overlay::set_text},
	{0, 0}
};

const luaL_reg Lua_Overlay::metatable[] = {
	{"__index", L_TableGet<Lua_Overlay>},
	{"__newindex", L_TableSet<Lua_Overlay>},
	{0, 0}
};

const char *Lua_Overlays::name = "overlays";
const luaL_reg Lua_Overlays::index_table[] = { {0, 0} };
const luaL_reg Lua_Overlays::newindex_table[] = { {0, 0} };
const luaL_reg Lua_Overlays::metatable[] = {
	{"__index", Lua_Overlays::get},
	{0, 0}
};

int Lua_Overlays::get(lua_State *L)
{
	if (lua_isnumber(L, 2))
	{
		int player_index = L_Index<Lua_Overlays>(L, 1);
		int index = static_cast<int>(lua_tonumber(L, 2));
		if (Lua_Overlays::valid(player_index) && index >= 0 && index < MAXIMUM_NUMBER_OF_SCRIPT_HUD_ELEMENTS)
		{
			Lua_Overlay *t = L_PushNew<Lua_Overlay>(L);
			t->index = index;
			t->player_index = player_index;
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
	int item_type = Lua_ItemType::ToIndex(L, 2);

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
	int item_type = Lua_ItemType::ToIndex(L, 2);
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

struct Lua_InternalVelocity {
	short index;
	static const char *name;
	static bool valid(int index) { return Lua_Players::valid(index); }
	
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];

	static int get_forward(lua_State *L);
	static int get_perpendicular(lua_State *L);
};

const char *Lua_InternalVelocity::name = "internal_velocity";

int Lua_InternalVelocity::get_forward(lua_State *L)
{
	int player_index = L_Index<Lua_InternalVelocity>(L, 1);
	player_data *player = get_player_data(player_index);
	lua_pushnumber(L, (double) player->variables.velocity / FIXED_ONE);
	return 1;
}

int Lua_InternalVelocity::get_perpendicular(lua_State *L)
{
	int player_index = L_Index<Lua_InternalVelocity>(L, 1);
	player_data *player = get_player_data(player_index);
	lua_pushnumber(L, (double) player->variables.perpendicular_velocity / FIXED_ONE);
	return 1;
}

const luaL_reg Lua_InternalVelocity::index_table[] = {
	{"forward", Lua_InternalVelocity::get_forward},
	{"perpendicular", Lua_InternalVelocity::get_perpendicular},
	{0, 0}
};

const luaL_reg Lua_InternalVelocity::newindex_table[] = { {0, 0} };

const luaL_reg Lua_InternalVelocity::metatable[] = {
	{"__index", L_TableGet<Lua_InternalVelocity>},
	{"__newindex", L_TableSet<Lua_InternalVelocity>},
	{0, 0}
};

struct Lua_ExternalVelocity {
	short index;
	static const char* name;
	static bool valid(int index) { return Lua_Players::valid(index); }
	
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];

	static int get_i(lua_State *);
	static int get_j(lua_State *);
	static int get_k(lua_State *);

	static int set_i(lua_State *);
	static int set_j(lua_State *);
	static int set_k(lua_State *);
};

const char *Lua_ExternalVelocity::name = "external_velocity";

int Lua_ExternalVelocity::get_i(lua_State *L)
{
	lua_pushnumber(L, (double) get_player_data(L_Index<Lua_ExternalVelocity>(L, 1))->variables.external_velocity.i / WORLD_ONE);
	return 1;
}

int Lua_ExternalVelocity::get_j(lua_State *L)
{
	lua_pushnumber(L, (double) get_player_data(L_Index<Lua_ExternalVelocity>(L, 1))->variables.external_velocity.j / WORLD_ONE);
	return 1;
}

int Lua_ExternalVelocity::get_k(lua_State *L)
{
	lua_pushnumber(L, (double) get_player_data(L_Index<Lua_ExternalVelocity>(L, 1))->variables.external_velocity.k / WORLD_ONE);
	return 1;
}

const luaL_reg Lua_ExternalVelocity::index_table[] = {
	{"i", Lua_ExternalVelocity::get_i},
	{"j", Lua_ExternalVelocity::get_j},
	{"k", Lua_ExternalVelocity::get_k},
	{"x", Lua_ExternalVelocity::get_i},
	{"y", Lua_ExternalVelocity::get_j},
	{"z", Lua_ExternalVelocity::get_k},
	{0, 0}
};

int Lua_ExternalVelocity::set_i(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "i: incorrect argument type");

	int raw_velocity = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	get_player_data(L_Index<Lua_ExternalVelocity>(L, 1))->variables.external_velocity.i = raw_velocity;
}

int Lua_ExternalVelocity::set_j(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "j: incorrect argument type");

	int raw_velocity = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	get_player_data(L_Index<Lua_ExternalVelocity>(L, 1))->variables.external_velocity.j = raw_velocity;
}

int Lua_ExternalVelocity::set_k(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "k: incorrect argument type");

	int raw_velocity = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	get_player_data(L_Index<Lua_ExternalVelocity>(L, 1))->variables.external_velocity.k = raw_velocity;
}

const luaL_reg Lua_ExternalVelocity::newindex_table[] = {
	{"i", Lua_ExternalVelocity::set_i},
	{"j", Lua_ExternalVelocity::set_j},
	{"k", Lua_ExternalVelocity::set_k},
	{"x", Lua_ExternalVelocity::set_i},
	{"y", Lua_ExternalVelocity::set_j},
	{"z", Lua_ExternalVelocity::set_k},
	{0, 0}
};

const luaL_reg Lua_ExternalVelocity::metatable[] = {
	{"__index", L_TableGet<Lua_ExternalVelocity>},
	{"__newindex", L_TableSet<Lua_ExternalVelocity>},
	{0, 0}
};

struct Lua_FadeTypes {
	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg methods[];
	static int length() { return NUMBER_OF_FADE_TYPES; }
	static bool valid(int index) { return index >= 0 && index < NUMBER_OF_FADE_TYPES; }
};

struct Lua_FadeType {
	short index;
	static const char *name;
	static bool valid(int index) { return Lua_FadeTypes::valid(index); }
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];
};

const char *Lua_FadeType::name = "fade_type";
const luaL_reg Lua_FadeType::index_table[] = {
	{"index", L_TableIndex<Lua_FadeType>},
	{0, 0}
};

const luaL_reg Lua_FadeType::newindex_table[] = { {0, 0} };

const luaL_reg Lua_FadeType::metatable[] = {
	{"__eq", L_Equals<Lua_FadeType>},
	{"__index", L_TableGet<Lua_FadeType>},
	{"__newindex", L_TableSet<Lua_FadeType>},
	{0, 0}
};

const char *Lua_FadeTypes::name = "FadeTypes";
const luaL_reg Lua_FadeTypes::metatable[] = {
	{"__index", L_GlobalIndex<Lua_FadeTypes, Lua_FadeType>},
	{"__newindex", L_GlobalNewindex<Lua_FadeTypes>},
	{"__call", L_GlobalCall<Lua_FadeTypes, Lua_FadeType>},
	{0, 0}
};

const luaL_reg Lua_FadeTypes::methods[] = { {0, 0} };

struct Lua_WeaponTypes {
	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg methods[];
	static int length() { return MAXIMUM_NUMBER_OF_WEAPONS; }
	static bool valid(int index) { return index >= 0 && index < MAXIMUM_NUMBER_OF_WEAPONS; }
};

struct Lua_WeaponType {
	short index;
	static bool valid(int index) { return Lua_WeaponTypes::valid(index); }
	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];
};

const char *Lua_WeaponType::name = "weapon_type";
const luaL_reg Lua_WeaponType::index_table[] = {
	{"index", L_TableIndex<Lua_WeaponType>},
	{0, 0}
};

const luaL_reg Lua_WeaponType::newindex_table[] = { {0, 0 } };

const luaL_reg Lua_WeaponType::metatable[] = {
	{"__eq", L_Equals<Lua_WeaponType>},
	{"__index", L_TableGet<Lua_WeaponType>},
	{"__newindex", L_TableSet<Lua_WeaponType>},
	{0, 0}
};

const char *Lua_WeaponTypes::name = "WeaponTypes";
const luaL_reg Lua_WeaponTypes::metatable[] = {
	{"__index", L_GlobalIndex<Lua_WeaponTypes, Lua_WeaponType>},
	{"__newindex", L_GlobalNewindex<Lua_WeaponTypes>},
	{"__call", L_GlobalCall<Lua_WeaponTypes, Lua_WeaponType>},
	{0, 0}
};
	
const luaL_reg Lua_WeaponTypes::methods[] = { {0, 0 } };
	
struct Lua_Player_Weapon_Trigger {
	short index;
	short player_index;
	short trigger_index;

	static bool valid(int index) { return Lua_WeaponTypes::valid(index); }

	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];

	static int get_rounds(lua_State *L);
};

const char *Lua_Player_Weapon_Trigger::name = "player_weapon_trigger";

int Lua_Player_Weapon_Trigger::get_rounds(lua_State *L)
{
	Lua_Player_Weapon_Trigger *trigger = L_To<Lua_Player_Weapon_Trigger>(L, 1);
	short rounds = get_player_weapon_ammo_count(trigger->player_index, trigger->index, trigger->trigger_index);
	lua_pushnumber(L, rounds);
	return 1;
}

const luaL_reg Lua_Player_Weapon_Trigger::index_table[] = {
	{"rounds", Lua_Player_Weapon_Trigger::get_rounds},
	{0, 0}
};

const luaL_reg Lua_Player_Weapon_Trigger::newindex_table[] = { {0, 0} };

const luaL_reg Lua_Player_Weapon_Trigger::metatable[] = {
	{"__index", L_TableGet<Lua_Player_Weapon_Trigger>},
	{"__newindex", L_TableSet<Lua_Player_Weapon_Trigger>},
	{0, 0}
};

struct Lua_Player_Weapons {
	short index;
	static bool valid(int index) { return Lua_Players::valid(index); }
	
	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];

	static int get(lua_State *L);
};

struct Lua_Player_Weapon {
	short index;
	short player_index;

	static bool valid(int index) { return Lua_WeaponTypes::valid(index); }

	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];

	static int get_type(lua_State *L);

	static int select(lua_State *L);
};

const char *Lua_Player_Weapon::name = "player_weapon";

template<int trigger>
static int get_weapon_trigger(lua_State *L)
{
	Lua_Player_Weapon *weapon = L_To<Lua_Player_Weapon>(L, 1);
	Lua_Player_Weapon_Trigger *t = L_PushNew<Lua_Player_Weapon_Trigger>(L);
	t->index = weapon->index;
	t->player_index = weapon->player_index;
	t->trigger_index = trigger;
	return 1;
}

int Lua_Player_Weapon::get_type(lua_State *L)
{
	L_Push<Lua_WeaponType>(L, L_Index<Lua_Player_Weapon>(L, 1));
	return 1;
}

extern bool ready_weapon(short player_index, short weapon_index);

int Lua_Player_Weapon::select(lua_State *L)
{
	Lua_Player_Weapon *weapon = L_To<Lua_Player_Weapon>(L, 1);
	ready_weapon(weapon->player_index, weapon->index);
	return 0;
}

const luaL_reg Lua_Player_Weapon::index_table[] = { 
	{"index", L_TableIndex<Lua_Player_Weapon>},
	{"primary", get_weapon_trigger<_primary_weapon>},
	{"secondary", get_weapon_trigger<_secondary_weapon>},
	{"select", L_TableFunction<Lua_Player_Weapon::select>},
	{"type", Lua_Player_Weapon::get_type},
	{0, 0} 
};
const luaL_reg Lua_Player_Weapon::newindex_table[] = { {0, 0} };
const luaL_reg Lua_Player_Weapon::metatable[] = {
	{"__index", L_TableGet<Lua_Player_Weapon>},
	{"__newindex", L_TableSet<Lua_Player_Weapon>},
	{0, 0}
};

const char *Lua_Player_Weapons::name = "player_weapons";
const luaL_reg Lua_Player_Weapons::index_table[] = { {0, 0} };
const luaL_reg Lua_Player_Weapons::newindex_table[] = { {0, 0} };
const luaL_reg Lua_Player_Weapons::metatable[] = {
	{"__index", Lua_Player_Weapons::get},
	{0, 0}
};

extern player_weapon_data *get_player_weapon_data(const short player_index);
extern bool player_has_valid_weapon(short player_index);

int Lua_Player_Weapons::get(lua_State *L)
{
	if (lua_isnumber(L, 2) || L_Is<Lua_WeaponType>(L, 2))
	{
		int player_index = L_Index<Lua_Player_Weapons>(L, 1);
		int index = L_ToIndex<Lua_WeaponType>(L, 2);
		if (!Lua_Player_Weapons::valid(player_index) || !Lua_WeaponTypes::valid(index))
		{
			lua_pushnil(L);
		}
		else
		{
			Lua_Player_Weapon *t = L_PushNew<Lua_Player_Weapon>(L);
			t->index = index;
			t->player_index = player_index;
		}
	}
	else if (lua_isstring(L, 2))
	{
		if (strcmp(lua_tostring(L, 2), "current") == 0)
		{
			int player_index = L_Index<Lua_Player_Weapons>(L, 1);
			if (player_has_valid_weapon(player_index))
			{
				player_weapon_data *weapon_data = get_player_weapon_data(player_index);
				player_data *player = get_player_data(player_index);
				
				Lua_Player_Weapon *t = L_PushNew<Lua_Player_Weapon>(L);
				t->index = weapon_data->current_weapon;
				t->player_index = player_index;
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
	}
	else
	{
		lua_pushnil(L);
	}

	return 1;
}

struct Lua_Player_Kills {
	short index;
	static bool valid(int index) { return Lua_Players::valid(index); }

	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];

	static int get(lua_State *L);
	static int set(lua_State *L);
};

const char *Lua_Player_Kills::name = "player_kills";

const luaL_reg Lua_Player_Kills::index_table[] = {
	{0, 0}
};

const luaL_reg Lua_Player_Kills::newindex_table[] = {
	{0, 0}
};

const luaL_reg Lua_Player_Kills::metatable[] = {
	{"__index", Lua_Player_Kills::get},
	{"__newindex", Lua_Player_Kills::set},
	{0, 0}
};

int Lua_Player_Kills::get(lua_State *L)
{
	int player_index = L_Index<Lua_Player_Kills>(L, 1);
	int slain_player_index = L_ToIndex<Lua_Player>(L, 2);
	
	player_data *slain_player = get_player_data(slain_player_index);

	lua_pushnumber(L, slain_player->damage_taken[player_index].kills);
	return 1;
}			

int Lua_Player_Kills::set(lua_State *L)
{
	if (!lua_isnumber(L, 3))
		return luaL_error(L, "kills: incorrect argument type");

	int player_index = L_Index<Lua_Player_Kills>(L, 1);
	int slain_player_index = L_ToIndex<Lua_Player>(L, 2);	
	int kills = static_cast<int>(lua_tonumber(L, 3));

	player_data *player = get_player_data(player_index);
	player_data *slain_player = get_player_data(slain_player_index);

	if (slain_player->damage_taken[player_index].kills != kills)
	{
		team_damage_taken[slain_player->team].kills += kills - slain_player->damage_taken[player_index].kills;
		if (slain_player_index != player_index)
		{
			team_damage_given[player->team].kills += kills - slain_player->damage_taken[player_index].kills;
		}
		if (slain_player->team == player->team)
		{
			team_friendly_fire[slain_player->team].kills += kills - slain_player->damage_taken[player_index].kills;
		}
		slain_player->damage_taken[player_index].kills = kills;
		mark_player_network_stats_as_dirty(current_player_index);
	}
	return 0;
}

const char *Lua_Player::name = "player";

// methods

// accelerate(direction, velocity, vertical_velocity)
int Lua_Player::accelerate(lua_State *L)
{
	if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		return luaL_error(L, "accelerate: incorrect argument type");

	player_data *player = get_player_data(L_Index<Lua_Player>(L, 1));
	double direction = static_cast<double>(lua_tonumber(L, 2));
	double velocity = static_cast<double>(lua_tonumber(L, 3));
	double vertical_velocity = static_cast<double>(lua_tonumber(L, 4));

	accelerate_player(player->monster_index, static_cast<int>(vertical_velocity * WORLD_ONE), static_cast<int>(direction/AngleConvert), static_cast<int>(velocity * WORLD_ONE));
	return 0;
}

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
			Lua_Platform::Push(L, object_index);
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
		damage.type = Lua_DamageType::ToIndex(L, 3);
	}

	damage_player(player->monster_index, NONE, NONE, &damage, NONE);
	return 0;
}

int Lua_Player::fade_screen(lua_State *L)
{
	short player_index = L_Index<Lua_Player>(L, 1);
	if (player_index == local_player_index)
	{
		int fade_index = L_ToIndex<Lua_FadeType>(L, 2);
		start_fade(fade_index);
	}
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
		if (!Lua_Polygon::Valid(polygon_index))
			return luaL_error(L, ("position: invalid polygon index"));
	}
	else if (Lua_Polygon::Is(L, 5))
	{
		polygon_index = Lua_Polygon::Index(L, 5);
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
	if (!lua_isnumber(L, 2) && !Lua_Polygon::Is(L, 2))
		return luaL_error(L, "teleport(): incorrect argument type");

	int destination = -1;
	if (lua_isnumber(L, 2))
		destination = static_cast<int>(lua_tonumber(L, 2));
	else 
		destination = Lua_Polygon::Index(L, 2);

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

int Lua_Player::get_crosshairs(lua_State *L)
{
	L_Push<Lua_Crosshairs>(L, L_Index<Lua_Player>(L, 1));
	return 1;
}

static int Lua_Player_get_dead(lua_State *L)
{
	player_data *player = get_player_data(L_Index<Lua_Player>(L, 1));
	lua_pushboolean(L, (PLAYER_IS_DEAD(player) || PLAYER_IS_TOTALLY_DEAD(player)));
	return 1;
}

int Lua_Player::get_deaths(lua_State *L)
{
	player_data *player = get_player_data(L_Index<Lua_Player>(L, 1));
	lua_pushnumber(L, player->monster_damage_taken.kills);
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

int Lua_Player::get_external_velocity(lua_State *L)
{
	L_Push<Lua_ExternalVelocity>(L, L_Index<Lua_Player>(L, 1));
	return 1;
}

int Lua_Player::get_extravision_duration(lua_State *L)
{
	lua_pushnumber(L, get_player_data(L_Index<Lua_Player>(L, 1))->extravision_duration);
	return 1;
}

int Lua_Player::get_infravision_duration(lua_State *L)
{
	lua_pushnumber(L, get_player_data(L_Index<Lua_Player>(L, 1))->infravision_duration);
	return 1;
}

int Lua_Player::get_internal_velocity(lua_State *L)
{
	L_Push<Lua_InternalVelocity>(L, L_Index<Lua_Player>(L, 1));
	return 1;
}

int Lua_Player::get_invincibility_duration(lua_State *L)
{
	lua_pushnumber(L, get_player_data(L_Index<Lua_Player>(L, 1))->invincibility_duration);
	return 1;
}

int Lua_Player::get_invisibility_duration(lua_State *L)
{
	lua_pushnumber(L, get_player_data(L_Index<Lua_Player>(L, 1))->invisibility_duration);
	return 1;
}

int Lua_Player::get_items(lua_State *L)
{
	L_Push<Lua_Player_Items>(L, L_Index<Lua_Player>(L, 1));
	return 1;
}

int Lua_Player::get_kills(lua_State *L)
{
	L_Push<Lua_Player_Kills>(L, L_Index<Lua_Player>(L, 1));
	return 1;
}

int Lua_Player::get_local(lua_State *L)
{
	lua_pushboolean(L, L_Index<Lua_Player>(L, 1) == local_player_index);
	return 1;
}

extern bool MotionSensorActive;

int Lua_Player::get_motion_sensor(lua_State *L)
{
	short player_index = L_Index<Lua_Player>(L, 1);
	if (player_index == local_player_index)
	{
		lua_pushboolean(L, MotionSensorActive);
		return 1;
	}
	else
	{
		return 0;
	}
}

static int Lua_Player_get_monster(lua_State *L)
{
	Lua_Monster::Push(L, get_player_data(L_Index<Lua_Player>(L, 1))->monster_index);
	return 1;
}

static int Lua_Player_get_name(lua_State *L)
{
	lua_pushstring(L, get_player_data(L_Index<Lua_Player>(L, 1))->name);
	return 1;
}

int Lua_Player::get_overlays(lua_State *L)
{
	L_Push<Lua_Overlays>(L, L_Index<Lua_Player>(L, 1));
	return 1;
}

static int Lua_Player_get_oxygen(lua_State *L)
{
	lua_pushnumber(L, get_player_data(L_Index<Lua_Player>(L, 1))->suit_oxygen);
	return 1;
}

int Lua_Player::get_points(lua_State *L)
{
	lua_pushnumber(L, get_player_data(L_Index<Lua_Player>(L, 1))->netgame_parameters[0]);
	return 1;
}

static int Lua_Player_get_polygon(lua_State *L)
{
	Lua_Polygon::Push(L, get_player_data(L_Index<Lua_Player>(L, 1))->supporting_polygon_index);
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

int Lua_Player::get_weapons(lua_State *L)
{
	L_Push<Lua_Player_Weapons>(L, L_Index<Lua_Player>(L, 1));
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

int Lua_Player::get_zoom(lua_State *L)
{
	short player_index = L_Index<Lua_Player>(L, 1);
	if (player_index == local_player_index)
	{
		lua_pushboolean(L, GetTunnelVision());
		return 1;
	}
	else
	{
		return 0;
	}
}

const luaL_reg Lua_Player::index_table[] = {
	{"accelerate", L_TableFunction<Lua_Player::accelerate>},
	{"action_flags", Lua_Player_get_action_flags},
	{"color", Lua_Player_get_color},
	{"crosshairs", Lua_Player::get_crosshairs},
	{"damage", L_TableFunction<Lua_Player::damage>},
	{"dead", Lua_Player_get_dead},
	{"deaths", Lua_Player::get_deaths},
	{"direction", Lua_Player::get_direction},
	{"energy", Lua_Player_get_energy},
	{"elevation", Lua_Player::get_elevation},
	{"external_velocity", Lua_Player::get_external_velocity},
	{"extravision_duration", Lua_Player::get_extravision_duration},
	{"fade_screen", L_TableFunction<Lua_Player::fade_screen>},
	{"find_action_key_target", L_TableFunction<Lua_Player_find_action_key_target>},
	{"index", L_TableIndex<Lua_Player>},
	{"infravision_duration", Lua_Player::get_infravision_duration},
	{"internal_velocity", Lua_Player::get_internal_velocity},
	{"invincibility_duration", Lua_Player::get_invincibility_duration},
	{"invisibility_duration", Lua_Player::get_invisibility_duration},
	{"items", Lua_Player::get_items},
	{"local_", Lua_Player::get_local},
	{"juice", Lua_Player_get_energy},
	{"kills", Lua_Player::get_kills},
	{"life", Lua_Player_get_energy},
	{"monster", Lua_Player_get_monster},
	{"motion_sensor_active", Lua_Player::get_motion_sensor},
	{"name", Lua_Player_get_name},
	{"overlays", Lua_Player::get_overlays},
	{"oxygen", Lua_Player_get_oxygen},
	{"pitch", Lua_Player::get_elevation},
	{"play_sound", L_TableFunction<Lua_Player::play_sound>},
	{"points", Lua_Player::get_points},
	{"polygon", Lua_Player_get_polygon},
	{"position", L_TableFunction<Lua_Player::position>},
	{"team", Lua_Player_get_team},
	{"teleport", L_TableFunction<Lua_Player_teleport>},
	{"teleport_to_level", L_TableFunction<Lua_Player_teleport_to_level>},
	{"weapons", Lua_Player::get_weapons},
	{"x", Lua_Player::get_x},
	{"y", Lua_Player::get_y},
	{"yaw", Lua_Player::get_direction},
	{"z", Lua_Player::get_z},
	{"zoom_active", Lua_Player::get_zoom},
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

int Lua_Player::set_deaths(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "deaths: incorrect argument type");

	player_data *player = get_player_data(L_Index<Lua_Player>(L, 1));
	int kills = static_cast<int>(lua_tonumber(L, 2));
	if (player->monster_damage_taken.kills != kills)
	{
		team_monster_damage_taken[player->team].kills += (kills - player->monster_damage_taken.kills);
		player->monster_damage_taken.kills = kills;
		mark_player_network_stats_as_dirty(current_player_index);
	}

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

int Lua_Player::set_infravision_duration(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "extravision: incorrect argument type");

	player_data *player = get_player_data(L_Index<Lua_Player>(L, 1));
	player->infravision_duration = static_cast<int>(lua_tonumber(L, 2));
	return 0;
}

int Lua_Player::set_invincibility_duration(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "extravision: incorrect argument type");

	player_data *player = get_player_data(L_Index<Lua_Player>(L, 1));
	player->invincibility_duration = static_cast<int>(lua_tonumber(L, 2));
	return 0;
}

int Lua_Player::set_invisibility_duration(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "extravision: incorrect argument type");

	player_data *player = get_player_data(L_Index<Lua_Player>(L, 1));
	player->invisibility_duration = static_cast<int>(lua_tonumber(L, 2));
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

int Lua_Player::set_extravision_duration(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "extravision: incorrect argument type");

	int player_index = L_Index<Lua_Player>(L, 1);
	player_data *player = get_player_data(player_index);
	short extravision_duration = static_cast<short>(lua_tonumber(L, 2));
	if ((player_index == local_player_index) && (extravision_duration == 0) != (player->extravision_duration == 0))
	{
		start_extravision_effect(extravision_duration);
	}
	player->extravision_duration = static_cast<int>(lua_tonumber(L, 2));
	return 0;
}

extern void draw_panels();

int Lua_Player::set_motion_sensor(lua_State *L)
{
	short player_index = L_Index<Lua_Player>(L, 1);
	if (player_index == local_player_index)
	{
		if (!lua_isboolean(L, 2))
			return luaL_error(L, "motion_sensor: incorrect argument type");
		bool state = lua_toboolean(L, 2);
		if (MotionSensorActive != state)
		{
			MotionSensorActive = lua_toboolean(L, 2);
			draw_panels();
		}
	}
	
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

int Lua_Player::set_points(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "points: incorrect argument type");

	int points = static_cast<int>(lua_tonumber(L, 2));

	player_data *player = get_player_data(L_Index<Lua_Player>(L, 1));
	if (player->netgame_parameters[0] != points)
	{
#if !defined(DISABLE_NETWORKING)
		team_netgame_parameters[player->team][0] += points - player->netgame_parameters[0];
#endif
		player->netgame_parameters[0] = points;
		mark_player_network_stats_as_dirty(current_player_index);
	}

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

int Lua_Player::set_zoom(lua_State *L)
{
	short player_index = L_Index<Lua_Player>(L, 1);
	if (player_index == local_player_index)
	{
		if (!lua_isboolean(L, 2))
			return luaL_error(L, "zoom_active: incorrect argument type");
		
		SetTunnelVision(lua_toboolean(L, 2));
	}

	return 0;
}

const luaL_reg Lua_Player::newindex_table[] = {
	{"color", Lua_Player_set_color},
	{"deaths", Lua_Player::set_deaths},
	{"direction", Lua_Player::set_direction},
	{"elevation", Lua_Player::set_elevation},
	{"energy", Lua_Player_set_energy},
	{"extravision_duration", Lua_Player::set_extravision_duration},
	{"infravision_duration", Lua_Player::set_infravision_duration},
	{"invincibility_duration", Lua_Player::set_invincibility_duration},
	{"invisibility_duration", Lua_Player::set_invisibility_duration},
	{"juice", Lua_Player_set_energy},
	{"life", Lua_Player_set_energy},
	{"motion_sensor_active", Lua_Player::set_motion_sensor},
	{"oxygen", Lua_Player_set_oxygen},
	{"pitch", Lua_Player::set_elevation},
	{"points", Lua_Player::set_points},
	{"position", Lua_Player::position},
	{"team", Lua_Player_set_team},
	{"yaw", Lua_Player::set_direction},
	{"zoom_active", Lua_Player::set_zoom},
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

struct Lua_DifficultyType {
	short index;
	static bool valid(int index) { return index >= 0 && index < NUMBER_OF_GAME_DIFFICULTY_LEVELS; }
	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];
};

const char *Lua_DifficultyType::name = "difficulty_type";
const luaL_reg Lua_DifficultyType::index_table[] = {
	{"index", L_TableIndex<Lua_DifficultyType>},
	{0, 0}
};

const luaL_reg Lua_DifficultyType::newindex_table[] = { {0, 0} };

const luaL_reg Lua_DifficultyType::metatable[] = {
	{"__eq", L_Equals<Lua_DifficultyType>},
	{"__index", L_TableGet<Lua_DifficultyType>},
	{"__newindex", L_TableSet<Lua_DifficultyType>},
	{0, 0}
};

struct Lua_GameType {
	short index;
	static bool valid(int index) { return index >= 0 && index < NUMBER_OF_GAME_TYPES; }
	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];
};

const char *Lua_GameType::name = "game_type";
const luaL_reg Lua_GameType::index_table[] = {
	{"index", L_TableIndex<Lua_GameType>},
	{0, 0}
};

const luaL_reg Lua_GameType::newindex_table[] = { {0, 0} };

const luaL_reg Lua_GameType::metatable[] = {
	{"__eq", L_Equals<Lua_GameType>},
	{"__index", L_TableGet<Lua_GameType>},
	{"__newindex", L_TableSet<Lua_GameType>},
	{0, 0}
};

struct Lua_Game {
	short index;
	static const char *name;

	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];

	static bool valid(int) { return true; }

	static int get_difficulty(lua_State *L);
	static int get_kill_limit(lua_State *L);
	static int get_type(lua_State *L);

	static int better_random(lua_State *L);
	static int global_random(lua_State *L);
	static int local_random(lua_State *L);
};

const char *Lua_Game::name = "Game";

int Lua_Game::get_difficulty(lua_State *L)
{
	L_Push<Lua_DifficultyType>(L, dynamic_world->game_information.difficulty_level);
	return 1;
}

int Lua_Game::get_kill_limit(lua_State *L)
{
	lua_pushnumber(L, dynamic_world->game_information.kill_limit);
	return 1;
}

int Lua_Game::get_type(lua_State *L)
{
	L_Push<Lua_GameType>(L, GET_GAME_TYPE());
	return 1;
}

extern GM_Random lua_random_generator;

int Lua_Game::better_random(lua_State *L)
{
	if (lua_isnumber(L, 1))
	{
		lua_pushnumber(L, lua_random_generator.KISS() % static_cast<uint32>(lua_tonumber(L, 1)));
	}
	{
		lua_pushnumber(L, lua_random_generator.KISS());
	}
	return 1;
}

int Lua_Game::global_random(lua_State *L)
{
	if (lua_isnumber(L, 1))
	{
		lua_pushnumber(L, ::global_random() % static_cast<uint16>(lua_tonumber(L, 1)));
	}
	else
	{
		lua_pushnumber(L, ::global_random());
	}
	return 1;
}

int Lua_Game::local_random(lua_State *L)
{
	if (lua_isnumber(L, 1))
	{
		lua_pushnumber(L, ::local_random() % static_cast<uint16>(lua_tonumber(L, 1)));
	}
	else
	{
		lua_pushnumber(L, ::local_random());
	}
	return 1;
}

const luaL_reg Lua_Game::index_table[] = {
	{"difficulty", Lua_Game::get_difficulty},
	{"global_random", L_TableFunction<Lua_Game::global_random>},
	{"kill_limit", Lua_Game::get_kill_limit},
	{"local_random", L_TableFunction<Lua_Game::local_random>},
	{"random", L_TableFunction<Lua_Game::better_random>},
	{"type", Lua_Game::get_type},
	{0, 0}
};

const luaL_reg Lua_Game::newindex_table[] = {
	{0, 0}
};

const luaL_reg Lua_Game::metatable[] = {
	{"__index", L_TableGet<Lua_Game>},
	{"__newindex", L_TableSet<Lua_Game>},
	{0, 0}
};
	

static int Lua_Player_load_compatibility(lua_State *L);

int Lua_Player_register (lua_State *L)
{
	L_Register<Lua_Action_Flags>(L);
	L_Register<Lua_Crosshairs>(L);
	L_Register<Lua_Player_Items>(L);
	L_Register<Lua_Player_Kills>(L);
	L_Register<Lua_Player>(L);
	L_Register<Lua_Side>(L);
	L_Register<Lua_InternalVelocity>(L);
	L_Register<Lua_ExternalVelocity>(L);
	L_Register<Lua_FadeType>(L);
	L_GlobalRegister<Lua_FadeTypes>(L);
	L_Register<Lua_WeaponType>(L);
	L_GlobalRegister<Lua_WeaponTypes>(L);
	L_Register<Lua_Player_Weapon>(L);
	L_Register<Lua_Player_Weapons>(L);
	L_Register<Lua_Player_Weapon_Trigger>(L);
	L_Register<Lua_OverlayColor>(L);
	L_Register<Lua_Overlays>(L);
	L_Register<Lua_Overlay>(L);

	L_GlobalRegister<Lua_Players>(L);

	L_Register<Lua_Game>(L);
	L_Register<Lua_GameType>(L);
	L_Register<Lua_DifficultyType>(L);
	// register one Game userdatum globally
	L_Push<Lua_Game>(L, 0);
	lua_setglobal(L, Lua_Game::name);
	
	Lua_Player_load_compatibility(L);
	
	return 0;
}

static const char *compatibility_script = ""
	"function accelerate_player(player, vertical_velocity, direction, velocity) Players[player]:accelerate(direction, velocity, vertical_velocity) end\n"
	"function add_item(player, item_type) Players[player].items[item_type] = Players[player].items[item_type] + 1 end\n"
	"function award_kills(player, slain_player, amount) if player == -1 then Players[slain_player].deaths = Players[slain_player].deaths + amount else Players[player].kills[slain_player] = Players[player].kills[slain_player] + amount end end\n"
	"function add_to_player_external_velocity(player, x, y, z) Players[player].external_velocity.i = Players[player].external_velocity.i + x Players[player].external_velocity.j = Players[player].external_velocity.j + y Players[player].external_velocity.k = Players[player].external_velocity.k + z end\n"
	"function award_points(player, amount) Players[player].points = Players[player].points + amount end\n"
	"function better_random() return Game.random() end\n"
	"function count_item(player, item_type) return Players[player].items[item_type] end\n"
	"function crosshairs_active(player) return Players[player].crosshairs.active end\n"
	"function destroy_ball(player) for i in ItemTypes() do if i.ball then Players[player].items[i] = 0 end end end\n"
	"function get_game_difficulty() return Game.difficulty.index end\n"
	"function get_game_type() return Game.type.index end\n"
	"function get_kills(player, slain_player) if player == -1 then return Players[slain_player].deaths else return Players[player].kills[slain_player] end end\n"
	"function get_kill_limit() return Game.kill_limit end\n"
	"function get_life(player) return Players[player].energy end\n"
	"function get_motion_sensor_state(player) return Players[player].motion_sensor_active end\n"
	"function get_oxygen(player) return Players[player].oxygen end\n"
	"function get_player_angle(player) return Players[player].yaw, Players[player].pitch end\n"
	"function get_player_color(player) return Players[player].color end\n"
	"function get_player_external_velocity(player) return Players[player].external_velocity.i * 1024, Players[player].external_velocity.j * 1024, Players[player].external_velocity.k * 1024 end\n"
	"function get_player_internal_velocity(player) return Players[player].internal_velocity.forward * 65536, Players[player].internal_velocity.perpendicular * 65536 end\n"
	"function get_player_name(player) return Players[player].name end\n"
	"function get_player_polygon(player) return Players[player].polygon.index end\n"
	"function get_player_position(player) return Players[player].x, Players[player].y, Players[player].z end\n"
	"function get_player_powerup_duration(player, powerup) if powerup == _powerup_invisibility then return Players[player].invisibility_duration elseif powerup == _powerup_invincibility then return Players[player].invincibility_duration elseif powerup == _powerup_infravision then return Players[player].infravision_duratiohn elseif powerup == _powerup_extravision then return Players[player].extravision_duration end end\n"
	"function get_player_team(player) return Players[player].team end\n"
	"function get_player_weapon(player) if Players[player].weapons.current then return Players[player].weapons.current.index else return nil end end\n"
	"function get_points(player) return Players[player].points end\n"
	"function global_random() return Game.global_random() end\n"
	"function inflict_damage(player, amount, type) if (type) then Players[player]:damage(amount, type) else Players[player]:damage(amount) end end\n"
	"function local_player_index() for p in Players() do if p.local_ then return p.index end end end\n"
	"function local_random() return Game.local_random() end\n"
	"function number_of_players() return # Players end\n"
	"function player_is_dead(player) return Players[player].dead end\n"
	"function player_to_monster_index(player) return Players[player].monster.index end\n"
	"function play_sound(player, sound, pitch) Players[player]:play_sound(sound, pitch) end\n"
	"function remove_item(player, item_type) if Players[player].items[item_type] > 0 then Players[player].items[item_type] = Players[player].items[item_type] - 1 end end\n"
	"function screen_fade(player, fade) if fade then Players[player]:fade_screen(fade) else for p in Players() do p:fade_screen(player) end end end\n"
	"function select_weapon(player, weapon) Players[player].weapons[weapon]:select() end\n"
	"function set_crosshairs_active(player, state) Players[player].crosshairs.active = state end\n"
	"function set_kills(player, slain_player, amount) if player == -1 then Players[slain_player].deaths = amount else Players[player].kills[slain_player] = amount end end\n"
	"function set_life(player, shield) Players[player].energy = shield end\n"
	"function set_motion_sensor_state(player, state) Players[player].motion_sensor_active = state end\n"
	"function set_overlay_color(overlay, color) for p in Players() do if p.local_ then p.overlays[overlay].color = color end end end\n"
	"function set_overlay_icon(overlay, icon) for p in Players() do if p.local_ then p.overlays[overlay].icon = icon end end end\n"
	"function set_overlay_icon_by_color(overlay, color) for p in Players() do if p.local_ then p.overlays[overlay]:fill_icon(color) end end end\n"
	"function set_overlay_text(overlay, text) for p in Players() do if p.local_ then p.overlays[overlay].text = text end end end\n"
	"function set_oxygen(player, oxygen) Players[player].oxygen = oxygen end\n"
	"function set_player_angle(player, yaw, pitch) Players[player].yaw = yaw Players[player].pitch = pitch + 360.0 end\n"
	"function set_player_color(player, color) Players[player].color = color end\n"
	"function set_player_external_velocity(player, x, y, z) Players[player].external_velocity.i = x / 1024 Players[player].external_velocity.j = y / 1024 Players[player].external_velocity.k = z / 1024 end\n"
	"function set_player_position(player, x, y, z, polygon) Players[player]:position(x, y, z, polygon) end\n"
	"function set_player_powerup_duration(player, powerup, duration) if powerup == _powerup_invisibility then Players[player].invisibility_duration = duration elseif powerup == _powerup_invincibility then Players[player].invincibility_duration = duration elseif powerup == _powerup_infravision then Players[player].infravision_duration = duration elseif powerup == _powerup_extravision then Players[player].extravision_duration = duration end end\n"
	"function set_player_team(player, team) Players[player].team = team end\n"
	"function set_points(player, amount) Players[player].points = amount end\n"
	"function set_zoom_state(player, state) Players[player].zoom_active = state end\n"
	"function teleport_player(player, polygon) Players[player]:teleport(polygon) end\n"
	"function teleport_player_to_level(player, level) Players[player]:teleport_to_level(level) end\n"
	"function zoom_active(player) return Players[player].zoom_active end\n"
	;

static int Lua_Player_load_compatibility(lua_State *L)
{
	luaL_loadbuffer(L, compatibility_script, strlen(compatibility_script), "player_compatibility");
	lua_pcall(L, 0, 0, 0);
};

#endif
