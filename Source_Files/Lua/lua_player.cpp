/*
LUA_PLAYER.CPP

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

	Implements the Lua Player class
*/

#include "ActionQueues.h"
#include "alephversion.h"
#include "computer_interface.h"
#include "Crosshairs.h"
#include "fades.h"
#include "game_window.h"
#include "interface.h"
#include "lua_map.h"
#include "lua_monsters.h"
#include "lua_objects.h"
#include "lua_hud_objects.h"
#include "lua_player.h"
#include "lua_script.h"
#include "lua_serialize.h"
#include "lua_templates.h"
#include "map.h"
#include "monsters.h"
#include "Music.h"
#include "network.h"
#include "player.h"
#include "projectiles.h"
#include "network_games.h"
#include "Random.h"
#include "screen.h"
#include "shell.h"
#include "SoundManager.h"
#include "ViewControl.h"

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
namespace io = boost::iostreams;

#define DONT_REPEAT_DEFINITIONS
#include "item_definitions.h"
#include "projectile_definitions.h"

#ifdef HAVE_LUA

const float AngleConvert = 360/float(FULL_CIRCLE);

char Lua_Action_Flags_Name[] = "action_flags";
typedef L_Class<Lua_Action_Flags_Name> Lua_Action_Flags;

extern ModifiableActionQueues *GetGameQueue();

template<uint32 flag> 
static int Lua_Action_Flags_Get_t(lua_State *L)
{
	int player_index = Lua_Action_Flags::Index(L, 1);

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
static int Lua_Action_Flags_Set_t(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "action flags: incorrect argument type");
	
	int player_index = Lua_Action_Flags::Index(L, 1);
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

static int Lua_Action_Flags_Set_Microphone(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "action flags: incorrect argument type");

	if (lua_toboolean(L, 2))
		return luaL_error(L, "you can only disable the microphone button flag");

	int player_index = Lua_Action_Flags::Index(L, 1);
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

const luaL_Reg Lua_Action_Flags_Get[] = {
	{"action_trigger", Lua_Action_Flags_Get_t<_action_trigger_state>},
	{"cycle_weapons_backward", Lua_Action_Flags_Get_t<_cycle_weapons_backward>},
	{"cycle_weapons_forward", Lua_Action_Flags_Get_t<_cycle_weapons_forward>},
	{"left_trigger", Lua_Action_Flags_Get_t<_left_trigger_state>},
	{"microphone_button", Lua_Action_Flags_Get_t<_microphone_button>},
	{"right_trigger", Lua_Action_Flags_Get_t<_right_trigger_state>},
	{"toggle_map", Lua_Action_Flags_Get_t<_toggle_map>},
	{0, 0}
};

const luaL_Reg Lua_Action_Flags_Set[] = {
	{"action_trigger", Lua_Action_Flags_Set_t<_action_trigger_state>},
	{"cycle_weapons_backward", Lua_Action_Flags_Set_t<_cycle_weapons_backward>},
	{"cycle_weapons_forward", Lua_Action_Flags_Set_t<_cycle_weapons_forward>},
	{"left_trigger", Lua_Action_Flags_Set_t<_left_trigger_state>},
	{"microphone_button", Lua_Action_Flags_Set_Microphone},
	{"right_trigger", Lua_Action_Flags_Set_t<_right_trigger_state>},
	{"toggle_map", Lua_Action_Flags_Set_t<_toggle_map>},
	{0, 0}
};

extern vector<lua_camera> lua_cameras;

char Lua_Camera_Path_Points_Name[] = "camera_path_points";
typedef L_Class<Lua_Camera_Path_Points_Name> Lua_Camera_Path_Points;

int Lua_Camera_Path_Points_New(lua_State *L)
{
	if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 6))
		return luaL_error(L, "new: incorrect argument type");

	int camera_index = Lua_Camera_Path_Points::Index(L, 1);
	
	int polygon = 0;
	if (lua_isnumber(L, 5))
	{
		polygon = static_cast<int>(lua_tonumber(L, 5));
		if (!Lua_Polygon::Valid(polygon))
			return luaL_error(L, "new: invalid polygon index");
	}
	else if (Lua_Polygon::Is(L, 5))
	{
		polygon = Lua_Polygon::Index(L, 5);
	}
	else
		return luaL_error(L, "new: incorrect argument type");

	world_point3d point = {
		static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE),
		static_cast<world_distance>(lua_tonumber(L, 3) * WORLD_ONE),
		static_cast<world_distance>(lua_tonumber(L, 4) * WORLD_ONE)
	};

	int32 time = static_cast<int32>(lua_tonumber(L, 6));
	int point_index = lua_cameras[camera_index].path.path_points.size();
	lua_cameras[camera_index].path.path_points.resize(point_index+1);
	lua_cameras[camera_index].path.path_points[point_index].polygon = polygon;
	lua_cameras[camera_index].path.path_points[point_index].point = point;
	lua_cameras[camera_index].path.path_points[point_index].delta_time = time;
	return 0;
}

const luaL_Reg Lua_Camera_Path_Points_Get[] = {
	{"new", L_TableFunction<Lua_Camera_Path_Points_New>},
	{0, 0}
};

char Lua_Camera_Path_Angles_Name[] = "camera_path_angles";
typedef L_Class<Lua_Camera_Path_Angles_Name> Lua_Camera_Path_Angles;

int Lua_Camera_Path_Angles_New(lua_State *L)
{
	if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		return luaL_error(L, "new: incorrect argument type");

	int camera_index = Lua_Camera_Path_Angles::Index(L, 1);
	short yaw = static_cast<short>(lua_tonumber(L,2));
	short pitch = static_cast<short>(lua_tonumber(L,3));
	int32 time = static_cast<int32>(lua_tonumber(L,4));
	int angle_index = lua_cameras[camera_index].path.path_angles.size();

	lua_cameras[camera_index].path.path_angles.resize(angle_index+1);
	lua_cameras[camera_index].path.path_angles[angle_index].yaw = static_cast<short>(yaw/AngleConvert);
	lua_cameras[camera_index].path.path_angles[angle_index].pitch = static_cast<short>(pitch/AngleConvert);
	lua_cameras[camera_index].path.path_angles[angle_index].delta_time = time;
	return 0;
}

const luaL_Reg Lua_Camera_Path_Angles_Get[] = {
	{"new", L_TableFunction<Lua_Camera_Path_Angles_New>},
	{0, 0}
};

char Lua_Camera_Name[] = "camera";
typedef L_Class<Lua_Camera_Name> Lua_Camera;


int Lua_Camera_Activate(lua_State *L)
{
	int player_index = -1;
	if (lua_isnumber(L, 2))
	{
		player_index = static_cast<int>(lua_tonumber(L, 2));
	}
	else if (Lua_Player::Is(L, 2))
	{
		player_index = Lua_Player::Index(L, 2);
	} 
	else
		return luaL_error(L, "activate: incorrect argument type");

	if (player_index == local_player_index)
	{
		int camera_index = Lua_Camera::Index(L, 1);
		lua_cameras[camera_index].time_elapsed = 0;
		lua_cameras[camera_index].player_active = player_index;
		lua_cameras[camera_index].path.current_point_index = 0;
		lua_cameras[camera_index].path.current_angle_index = 0;
		lua_cameras[camera_index].path.last_point_time = 0;
		lua_cameras[camera_index].path.last_angle_time = 0;
	}
	
	return 0;
}

int Lua_Camera_Clear(lua_State *L)
{
	int camera_index = Lua_Camera::Index(L, 1);
	lua_cameras[camera_index].path.path_points.resize(0);
	lua_cameras[camera_index].path.path_angles.resize(0);
	return 0;
}

int Lua_Camera_Deactivate(lua_State *L)
{
	int camera_index = Lua_Camera::Index(L, 1);
	lua_cameras[camera_index].time_elapsed = 0;
	lua_cameras[camera_index].player_active = -1;
	lua_cameras[camera_index].path.last_point_time = 0;
	lua_cameras[camera_index].path.last_angle_time = 0;
	return 0;
}

static int Lua_Get_Path_Angles(lua_State *L)
{
	Lua_Camera_Path_Angles::Push(L, Lua_Camera::Index(L, 1));
	return 1;
}

static int Lua_Get_Path_Points(lua_State *L)
{
	Lua_Camera_Path_Points::Push(L, Lua_Camera::Index(L, 1));
	return 1;
}

const luaL_Reg Lua_Camera_Get[] = {
	{"activate", L_TableFunction<Lua_Camera_Activate>},
	{"clear", L_TableFunction<Lua_Camera_Clear>},
	{"deactivate", L_TableFunction<Lua_Camera_Deactivate>},
	{"path_angles", Lua_Get_Path_Angles},
	{"path_points", Lua_Get_Path_Points},
	{0, 0}
};

static int Lua_Camera_Valid(int16 index)
{
	return index >= 0 && index < lua_cameras.size();
}

char Lua_Cameras_Name[] = "Cameras";
typedef L_Container<Lua_Cameras_Name, Lua_Camera> Lua_Cameras;

int Lua_Cameras_New(lua_State *L)
{
	if (lua_cameras.size() == INT16_MAX)
	{
		return 0;
	}

	lua_camera camera;
	camera.index = lua_cameras.size();
	camera.path.index = lua_cameras.size();
	camera.path.current_point_index = 0;
	camera.path.current_angle_index = 0;
	camera.path.last_point_time = 0;
	camera.path.last_angle_time = 0;
	camera.time_elapsed = 0;
	camera.player_active = -1;	
	lua_cameras.push_back(camera);

	Lua_Camera::Push(L, camera.index);

	return 1;
}

const luaL_Reg Lua_Cameras_Methods[] = {
	{"new", L_TableFunction<Lua_Cameras_New>},
	{0, 0}
};

static int16 Lua_Cameras_Length() {
	return lua_cameras.size();
}

char Lua_Crosshairs_Name[] = "crosshairs";
typedef L_Class<Lua_Crosshairs_Name> Lua_Crosshairs;

static int Lua_Crosshairs_Get_Active(lua_State *L)
{
	int player_index = Lua_Crosshairs::Index(L, 1);
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

const luaL_Reg Lua_Crosshairs_Get[] = {
	{"active", Lua_Crosshairs_Get_Active},
	{0, 0}
};

static int Lua_Crosshairs_Set_Active(lua_State *L)
{
	int player_index = Lua_Crosshairs::Index(L, 1);
	if (player_index == local_player_index)
	{
		if (!lua_isboolean(L, 2))
			return luaL_error(L, "active: incorrect argument type");

		Crosshairs_SetActive(lua_toboolean(L, 2));
	}
	
	return 0;
}

const luaL_Reg Lua_Crosshairs_Set[] = {
	{"active", Lua_Crosshairs_Set_Active},
	{0, 0}
};


char Lua_HotkeyBinding_Name[] = "hotkey_binding";
typedef L_Class<Lua_HotkeyBinding_Name> Lua_HotkeyBinding;

extern const char* get_hotkey_binding(int, int);

template<int type>
static int Lua_HotkeyBinding_Get_Binding(lua_State* L)
{
	auto hotkey = Lua_HotkeyBinding::Index(L, 1);
	auto binding = get_hotkey_binding(hotkey, type);

	if (binding[0])
	{
		lua_pushstring(L, binding);
	}
	else
	{
		lua_pushnil(L);
	}

	return 1;
}

// matches w_key::Type
enum {
	type_keyboard,
	type_mouse,
	type_joystick
};

const luaL_Reg Lua_HotkeyBinding_Get[] = {
	{"joystick", Lua_HotkeyBinding_Get_Binding<type_joystick>},
	{"key", Lua_HotkeyBinding_Get_Binding<type_keyboard>},
	{"mouse", Lua_HotkeyBinding_Get_Binding<type_mouse>},
};

char Lua_HotkeyBindings_Name[] = "hotkey_bindings";
typedef L_Class<Lua_HotkeyBindings_Name> Lua_HotkeyBindings;

static int Lua_HotkeyBindings_Get(lua_State* L)
{
	if (lua_isnumber(L, 2))
	{
		auto index = static_cast<int>(lua_tonumber(L, 2));
		if (index >= 1 && index <= 12)
		{
			Lua_HotkeyBinding::Push(L, index);
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

static int Lua_HotkeyBindings_Length(lua_State *L)
{
	lua_pushnumber(L, 12);
	return 1;
}

const luaL_Reg Lua_HotkeyBindings_Metatable[] = {
	{"__index", Lua_HotkeyBindings_Get},
	{"__len", Lua_HotkeyBindings_Length},
	{0, 0}
};

char Lua_OverlayColor_Name[] = "overlay_color";
typedef L_Enum<Lua_OverlayColor_Name> Lua_OverlayColor;

template<char *name>
class PlayerSubtable : public L_Class<name>
{
public:
	int16 m_player_index;

	template<typename instance_t = PlayerSubtable /*or a derived class*/>
	static instance_t *Push(lua_State *L, int16 player_index, int16 index);

	static int16 PlayerIndex(lua_State *L, int index);
};

template<char *name>
template<typename instance_t>
instance_t *PlayerSubtable<name>::Push(lua_State *L, int16 player_index, int16 index)
{
	instance_t *t = 0;

	if (!L_Class<name, int16>::Valid(index) || !Lua_Player::Valid(player_index))
	{
		lua_pushnil(L);
		return 0;
	}

	t = L_Class<name>::template NewInstance<instance_t>(L, index);
	t->m_player_index = player_index;

	return t;
}

template<char *name>
int16 PlayerSubtable<name>::PlayerIndex(lua_State *L, int index)
{
	PlayerSubtable<name> *t = static_cast<PlayerSubtable<name> *>(L_Class<name>::Instance(L, index));
	if (!t) luaL_typerror(L, index, name);
	return t->m_player_index;
}

char Lua_Overlay_Name[] = "overlay";
typedef PlayerSubtable<Lua_Overlay_Name> Lua_Overlay;

int Lua_Overlay_Clear(lua_State *L)
{
        int player = Lua_Overlay::PlayerIndex(L, 1);
	int index = Lua_Overlay::Index(L, 1);
	SetScriptHUDIcon(player, index, 0, 0);
	SetScriptHUDText(player, index, 0);

	return 0;
}

int Lua_Overlay_Fill_Icon(lua_State *L)
{
        int player = Lua_Overlay::PlayerIndex(L, 1);
	int color = Lua_OverlayColor::ToIndex(L, 2);
	SetScriptHUDSquare(player, Lua_Overlay::Index(L, 1), color);

	return 0;
}

const luaL_Reg Lua_Overlay_Get[] = {
	{"clear", L_TableFunction<Lua_Overlay_Clear>},
	{"fill_icon", L_TableFunction<Lua_Overlay_Fill_Icon>},
	{0, 0}
};

static int Lua_Overlay_Set_Icon(lua_State *L)
{
        int player = Lua_Overlay::PlayerIndex(L, 1);
	if (lua_isstring(L, 2))
	{
		SetScriptHUDIcon(player, Lua_Overlay::Index(L, 1), lua_tostring(L, 2), lua_rawlen(L, 2));
	}
	else
	{
		SetScriptHUDIcon(player, Lua_Overlay::Index(L, 1), 0, 0);
	}

	return 0;
}

static int Lua_Overlay_Set_Text(lua_State *L)
{
        int player = Lua_Overlay::PlayerIndex(L, 1);
	const char *text = 0;
	if (lua_isstring(L, 2)) 
		text = lua_tostring(L, 2);
	
	SetScriptHUDText(player, Lua_Overlay::Index(L, 1), text);

	return 0;
}

static int Lua_Overlay_Set_Text_Color(lua_State *L)
{
        int player = Lua_Overlay::PlayerIndex(L, 1);
	int color = Lua_OverlayColor::ToIndex(L, 2);
	SetScriptHUDColor(player, Lua_Overlay::Index(L, 1), color);

	return 0;
}

const luaL_Reg Lua_Overlay_Set[] = {
	{"color", Lua_Overlay_Set_Text_Color},
	{"icon", Lua_Overlay_Set_Icon},
	{"text", Lua_Overlay_Set_Text},
	{0, 0}
};

char Lua_Overlays_Name[] = "overlays";
typedef L_Class<Lua_Overlays_Name> Lua_Overlays;

static int Lua_Overlays_Get(lua_State *L)
{
	if (lua_isnumber(L, 2))
	{
		int player_index = Lua_Overlays::Index(L, 1);
		int index = static_cast<int>(lua_tonumber(L, 2));
		if (Lua_Overlays::Valid(player_index) && index >= 0 && index < MAXIMUM_NUMBER_OF_SCRIPT_HUD_ELEMENTS)
		{
			Lua_Overlay::Push(L, player_index, index);
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

static int Lua_Overlays_Length(lua_State *L)
{
	int player_index = Lua_Overlays::Index(L, 1);
	if (Lua_Overlays::Valid(player_index))
		lua_pushnumber(L, MAXIMUM_NUMBER_OF_SCRIPT_HUD_ELEMENTS);
	else
		lua_pushnumber(L, 0);
	return 1;
}

const luaL_Reg Lua_Overlays_Metatable[] = {
	{"__index", Lua_Overlays_Get},
	{"__len", Lua_Overlays_Length},
	{0, 0}
};

extern bool use_lua_compass[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];
extern world_point2d lua_compass_beacons[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];
extern short lua_compass_states[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];

char Lua_Player_Compass_Name[] = "player_compass";
typedef L_Class<Lua_Player_Compass_Name> Lua_Player_Compass;

template<short state>
int Lua_Player_Compass_All(lua_State *L)
{
	int player_index = Lua_Player_Compass::Index(L, 1);
	lua_compass_states[player_index] = state;
	return 0;
}

static int Lua_Player_Compass_Get_Lua(lua_State *L)
{
	int player_index = Lua_Player_Compass::Index(L, 1);
	lua_pushboolean(L, use_lua_compass[player_index]);
	return 1;
}

template<short state>
static int Lua_Player_Compass_Get_State(lua_State *L)
{
	int player_index = Lua_Player_Compass::Index(L, 1);
	lua_pushboolean(L, lua_compass_states[player_index] & state);
	return 1;
}

static int Lua_Player_Compass_Get_X(lua_State *L)
{
	int player_index = Lua_Player_Compass::Index(L, 1);
	lua_pushnumber(L, static_cast<double>(lua_compass_beacons[player_index].x / WORLD_ONE));
	return 1;
}

static int Lua_Player_Compass_Get_Y(lua_State *L)
{
	int player_index = Lua_Player_Compass::Index(L, 1);
	lua_pushnumber(L, static_cast<double>(lua_compass_beacons[player_index].y / WORLD_ONE));
	return 1;
}

const luaL_Reg Lua_Player_Compass_Get[] = {
	{"all_off", L_TableFunction<Lua_Player_Compass_All<_network_compass_all_off> >},
	{"all_on", L_TableFunction<Lua_Player_Compass_All<_network_compass_all_on> >},
	{"beacon", Lua_Player_Compass_Get_State<_network_compass_use_beacon>},
	{"lua", Lua_Player_Compass_Get_Lua},
	{"ne", Lua_Player_Compass_Get_State<_network_compass_ne>},
	{"northeast", Lua_Player_Compass_Get_State<_network_compass_ne>},
	{"northwest", Lua_Player_Compass_Get_State<_network_compass_nw>},
	{"nw", Lua_Player_Compass_Get_State<_network_compass_nw>},
	{"se", Lua_Player_Compass_Get_State<_network_compass_se>},
	{"southeast", Lua_Player_Compass_Get_State<_network_compass_se>},
	{"southwest", Lua_Player_Compass_Get_State<_network_compass_sw>},
	{"sw", Lua_Player_Compass_Get_State<_network_compass_sw>},
	{"x", Lua_Player_Compass_Get_X},
	{"y", Lua_Player_Compass_Get_Y},
	{0, 0}
};

static int Lua_Player_Compass_Set_Lua(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "lua: incorrect argument type");

	int player_index = Lua_Player_Compass::Index(L, 1);
	use_lua_compass[player_index] = lua_toboolean(L, 2);
	return 0;
}

template<short state>
static int Lua_Player_Compass_Set_State(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "compass: incorrect argument type");
	
	int player_index = Lua_Player_Compass::Index(L, 1);
	if (lua_toboolean(L, 2))
	{
		lua_compass_states[player_index] |= state;
	}
	else
	{
		lua_compass_states[player_index] &= ~state;
	}

	return 0;
}

static int Lua_Player_Compass_Set_X(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "x: incorrect argument type");

	int player_index = Lua_Player_Compass::Index(L, 1);
	lua_compass_beacons[player_index].x = static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE);
	return 0;
}

static int Lua_Player_Compass_Set_Y(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "y: incorrect argument type");

	int player_index = Lua_Player_Compass::Index(L, 1);
	lua_compass_beacons[player_index].y = static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE);
	return 0;
}

const luaL_Reg Lua_Player_Compass_Set[] = {
	{"beacon", Lua_Player_Compass_Set_State<_network_compass_use_beacon>},
	{"lua", Lua_Player_Compass_Set_Lua},
	{"ne", Lua_Player_Compass_Set_State<_network_compass_ne>},
	{"northeast", Lua_Player_Compass_Set_State<_network_compass_ne>},
	{"northwest", Lua_Player_Compass_Set_State<_network_compass_nw>},
	{"nw", Lua_Player_Compass_Set_State<_network_compass_nw>},
	{"se", Lua_Player_Compass_Set_State<_network_compass_se>},
	{"southeast", Lua_Player_Compass_Set_State<_network_compass_se>},
	{"southwest", Lua_Player_Compass_Set_State<_network_compass_sw>},
	{"sw", Lua_Player_Compass_Set_State<_network_compass_sw>},
	{"x", Lua_Player_Compass_Set_X},
	{"y", Lua_Player_Compass_Set_Y},
	{0, 0}
};

char Lua_Player_Items_Name[] = "player_items";
typedef L_Class<Lua_Player_Items_Name> Lua_Player_Items;

static int Lua_Player_Items_Get(lua_State *L)
{
	int player_index = Lua_Player_Items::Index(L, 1);
	int item_type = Lua_ItemType::ToIndex(L, 2);

	player_data *player = get_player_data(player_index);
	int item_count = player->items[item_type];
	if (item_count == NONE) item_count = 0;
	lua_pushnumber(L, item_count);
	return 1;
}

static int Lua_Player_Items_Length(lua_State *L)
{
    lua_pushnumber(L, NUMBER_OF_DEFINED_ITEMS);
    return 1;
}

extern void destroy_players_ball(short player_index);
extern void select_next_best_weapon(short player_index);

static int Lua_Player_Items_Set(lua_State *L)
{
	if (!lua_isnumber(L, 3)) 
		return luaL_error(L, "items: incorrect argument type");

	int player_index = Lua_Player_Items::Index(L, 1);
	player_data *player = get_player_data(player_index);
	int item_type = Lua_ItemType::ToIndex(L, 2);
	int item_count = player->items[item_type];
	item_definition *definition = get_item_definition_external(item_type);
	int new_item_count = static_cast<int>(lua_tonumber(L, 3));

	bool accounting = L_Get_Proper_Item_Accounting(L);
	
	if (new_item_count < 0) 
		luaL_error(L, "items: invalid item count");

	if (item_count == NONE) item_count = 0;
	int real_difference = item_count - new_item_count;
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

			if (accounting)
			{
				for (int i = 0; i < real_difference; ++i)
					object_was_just_destroyed(_object_is_item, item_type);
			}
		}
	}
	else if (new_item_count > item_count)
	{
		while (new_item_count-- > item_count)
		{
			if (try_and_add_player_item(player_index, item_type))
			{
				if (accounting) 
					object_was_just_added(_object_is_item, item_type);
			}
		}
	}

	return 0;
}

const luaL_Reg Lua_Player_Items_Metatable[] = {
	{"__index", Lua_Player_Items_Get},
	{"__newindex", Lua_Player_Items_Set},
	{"__len", Lua_Player_Items_Length},
	{0, 0}
};

char Lua_InternalVelocity_Name[] = "internal_velocity";
typedef L_Class<Lua_InternalVelocity_Name> Lua_InternalVelocity;

static int Lua_InternalVelocity_Get_Forward(lua_State *L)
{
	int player_index = Lua_InternalVelocity::Index(L, 1);
	player_data *player = get_player_data(player_index);
	lua_pushnumber(L, (double) player->variables.velocity / FIXED_ONE);
	return 1;
}

static int Lua_InternalVelocity_Get_Perpendicular(lua_State *L)
{
	int player_index = Lua_InternalVelocity::Index(L, 1);
	player_data *player = get_player_data(player_index);
	lua_pushnumber(L, (double) player->variables.perpendicular_velocity / FIXED_ONE);
	return 1;
}

const luaL_Reg Lua_InternalVelocity_Get[] = {
	{"forward", Lua_InternalVelocity_Get_Forward},
	{"perpendicular", Lua_InternalVelocity_Get_Perpendicular},
	{0, 0}
};

char Lua_ExternalVelocity_Name[] = "external_velocity";
typedef L_Class<Lua_ExternalVelocity_Name> Lua_ExternalVelocity;

static int Lua_ExternalVelocity_Get_I(lua_State *L)
{
	lua_pushnumber(L, (double) get_player_data(Lua_ExternalVelocity::Index(L, 1))->variables.external_velocity.i / WORLD_ONE);
	return 1;
}

static int Lua_ExternalVelocity_Get_J(lua_State *L)
{
	lua_pushnumber(L, (double) get_player_data(Lua_ExternalVelocity::Index(L, 1))->variables.external_velocity.j / WORLD_ONE);
	return 1;
}

static int Lua_ExternalVelocity_Get_K(lua_State *L)
{
	lua_pushnumber(L, (double) get_player_data(Lua_ExternalVelocity::Index(L, 1))->variables.external_velocity.k / WORLD_ONE);
	return 1;
}

const luaL_Reg Lua_ExternalVelocity_Get[] = {
	{"i", Lua_ExternalVelocity_Get_I},
	{"j", Lua_ExternalVelocity_Get_J},
	{"k", Lua_ExternalVelocity_Get_K},
	{"x", Lua_ExternalVelocity_Get_I},
	{"y", Lua_ExternalVelocity_Get_J},
	{"z", Lua_ExternalVelocity_Get_K},
	{0, 0}
};

static int Lua_ExternalVelocity_Set_I(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "i: incorrect argument type");

	int raw_velocity = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	get_player_data(Lua_ExternalVelocity::Index(L, 1))->variables.external_velocity.i = raw_velocity;
	return 0;
}

static int Lua_ExternalVelocity_Set_J(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "j: incorrect argument type");

	int raw_velocity = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	get_player_data(Lua_ExternalVelocity::Index(L, 1))->variables.external_velocity.j = raw_velocity;
	return 0;
}

static int Lua_ExternalVelocity_Set_K(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "k: incorrect argument type");

	int raw_velocity = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	get_player_data(Lua_ExternalVelocity::Index(L, 1))->variables.external_velocity.k = raw_velocity;
	return 0;
}

const luaL_Reg Lua_ExternalVelocity_Set[] = {
	{"i", Lua_ExternalVelocity_Set_I},
	{"j", Lua_ExternalVelocity_Set_J},
	{"k", Lua_ExternalVelocity_Set_K},
	{"x", Lua_ExternalVelocity_Set_I},
	{"y", Lua_ExternalVelocity_Set_J},
	{"z", Lua_ExternalVelocity_Set_K},
	{0, 0}
};

char Lua_FadeType_Name[] = "fade_type";
typedef L_Enum<Lua_FadeType_Name> Lua_FadeType;

char Lua_FadeTypes_Name[] = "FadeTypes";
typedef L_EnumContainer<Lua_FadeTypes_Name, Lua_FadeType> Lua_FadeTypes;

const int MAX_TEXTURE_PALETTE_SIZE = 256;
struct lua_texture {
    shape_descriptor shape;
    short type;
};
typedef struct lua_texture lua_texture;
static std::vector<lua_texture> lua_texture_palette;
static int lua_texture_palette_selected = -1;

void LuaTexturePaletteClear() {
	lua_texture_palette.clear();
}

int LuaTexturePaletteSize() {
	return lua_texture_palette.size();
}

shape_descriptor LuaTexturePaletteTexture(size_t index)
{
	if (index < lua_texture_palette.size())
		return lua_texture_palette[index].shape;
	else
		return UNONE;
}

short LuaTexturePaletteTextureType(size_t index)
{
	if (index < lua_texture_palette.size())
		return lua_texture_palette[index].type;
	else
		return 0;
}

int LuaTexturePaletteSelected()
{
	return lua_texture_palette_selected;
}

char Lua_Texture_Palette_Slot_Name[] = "texture_palette_slot";
typedef PlayerSubtable<Lua_Texture_Palette_Slot_Name> Lua_Texture_Palette_Slot;

int Lua_Texture_Palette_Slot_Clear(lua_State *L)
{
	int player_index = Lua_Texture_Palette_Slot::PlayerIndex(L, 1);
	if (player_index != local_player_index)
		return 0;

    lua_texture blank = { UNONE, 0 };
	lua_texture_palette[Lua_Texture_Palette_Slot::Index(L, 1)] = blank;
	return 0;
}

static int Lua_Texture_Palette_Slot_Get_Collection(lua_State *L)
{
	int player_index = Lua_Texture_Palette_Slot::PlayerIndex(L, 1);
	if (player_index != local_player_index)
		return 0;

	int index = Lua_Texture_Palette_Slot::Index(L, 1);
	if (lua_texture_palette[index].shape == UNONE)
		return 0;

	lua_pushnumber(L, GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(lua_texture_palette[index].shape)));
	return 1;
}

static int Lua_Texture_Palette_Slot_Get_Texture(lua_State *L)
{
	int player_index = Lua_Texture_Palette_Slot::PlayerIndex(L, 1);
	if (player_index != local_player_index)
		return 0;

	int index = Lua_Texture_Palette_Slot::Index(L, 1);
	if (lua_texture_palette[index].shape == UNONE)
		return 0;

	lua_pushnumber(L, GET_DESCRIPTOR_SHAPE(lua_texture_palette[index].shape));
	return 1;
}

static int Lua_Texture_Palette_Slot_Get_Type(lua_State *L)
{
	int player_index = Lua_Texture_Palette_Slot::PlayerIndex(L, 1);
	if (player_index != local_player_index)
		return 0;
    
	int index = Lua_Texture_Palette_Slot::Index(L, 1);
	if (lua_texture_palette[index].shape == UNONE)
		return 0;
    
	lua_pushnumber(L, lua_texture_palette[index].type);
	return 1;
}

const luaL_Reg Lua_Texture_Palette_Slot_Get[] = {
	{"clear", L_TableFunction<Lua_Texture_Palette_Slot_Clear>},
	{"collection", Lua_Texture_Palette_Slot_Get_Collection},
	{"texture_index", Lua_Texture_Palette_Slot_Get_Texture},
    {"type", Lua_Texture_Palette_Slot_Get_Type},
	{0, 0}
};

static int Lua_Texture_Palette_Slot_Set_Collection(lua_State *L)
{
	int player_index = Lua_Texture_Palette_Slot::PlayerIndex(L, 1);
	if (player_index != local_player_index)
		return 0;

	int16 index = Lua_Texture_Palette_Slot::Index(L, 1);
	short collection_index = Lua_Collection::ToIndex(L, 2);

	lua_texture_palette[index].shape = BUILD_DESCRIPTOR(collection_index, GET_DESCRIPTOR_SHAPE(lua_texture_palette[index].shape));
	return 0;
}

static int Lua_Texture_Palette_Slot_Set_Texture(lua_State *L)
{
	int player_index = Lua_Texture_Palette_Slot::PlayerIndex(L, 1);
	if (player_index != local_player_index)
		return 0;

	if (!lua_isnumber(L, 2))
		return luaL_error(L, "texture_index: incorrect argument type");

	int16 index = Lua_Texture_Palette_Slot::Index(L, 1);
	short shape_index = static_cast<short>(lua_tonumber(L, 2));
	if (shape_index < 0 || shape_index >= MAXIMUM_SHAPES_PER_COLLECTION)
		return luaL_error(L, "texture_index: invalid texture index");
	
	lua_texture_palette[index].shape = BUILD_DESCRIPTOR(GET_DESCRIPTOR_COLLECTION(lua_texture_palette[index].shape), shape_index);
	return 0;
}

static int Lua_Texture_Palette_Slot_Set_Type(lua_State *L)
{
	int player_index = Lua_Texture_Palette_Slot::PlayerIndex(L, 1);
	if (player_index != local_player_index)
		return 0;
    
	int16 index = Lua_Texture_Palette_Slot::Index(L, 1);
	short texture_type = Lua_TextureType::ToIndex(L, 2);
    
	lua_texture_palette[index].type = texture_type;
	return 0;
}


const luaL_Reg Lua_Texture_Palette_Slot_Set[] = {
	{"collection", Lua_Texture_Palette_Slot_Set_Collection},
	{"texture_index", Lua_Texture_Palette_Slot_Set_Texture},
    {"type", Lua_Texture_Palette_Slot_Set_Type},
	{0, 0}
};

char Lua_Texture_Palette_Slots_Name[] = "texture_palette_slots";
typedef L_Class<Lua_Texture_Palette_Slots_Name> Lua_Texture_Palette_Slots;

static int Lua_Texture_Palette_Slots_Get(lua_State *L)
{
	if (lua_isnumber(L, 2))
	{
		int player_index = Lua_Texture_Palette_Slots::Index(L, 1);
		int index = static_cast<int>(lua_tonumber(L, 2));
		if (Lua_Texture_Palette_Slots::Valid(player_index) && index >= 0 && index < lua_texture_palette.size())
		{
			Lua_Texture_Palette_Slot::Push(L, player_index, index);
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

static int Lua_Texture_Palette_Slots_Length(lua_State *L)
{
	int player_index = Lua_Texture_Palette_Slots::Index(L, 1);
	if (player_index != local_player_index)
		return 0;

	lua_pushnumber(L, lua_texture_palette.size());
	return 1;
}

const luaL_Reg Lua_Texture_Palette_Slots_Metatable[] = {
	{"__index", Lua_Texture_Palette_Slots_Get},
	{"__len", Lua_Texture_Palette_Slots_Length},
	{0, 0}
};

char Lua_Texture_Palette_Name[] = "texture_palette";
typedef L_Class<Lua_Texture_Palette_Name> Lua_Texture_Palette;

static int Lua_Texture_Palette_Get_Selected(lua_State *L)
{
	int player_index = Lua_Texture_Palette::Index(L, 1);
	if (player_index != local_player_index)
		return 0;

	if (lua_texture_palette_selected == -1)
		return 0;

	lua_pushnumber(L, lua_texture_palette_selected);
	return 1;
}

static int Lua_Texture_Palette_Get_Size(lua_State *L)
{
	int player_index = Lua_Texture_Palette::Index(L, 1);
	if (player_index != local_player_index)
		return 0;

	lua_pushnumber(L, lua_texture_palette.size());
	return 1;
}

static int Lua_Texture_Palette_Get_Slots(lua_State *L)
{
	Lua_Texture_Palette_Slots::Push(L, Lua_Texture_Palette::Index(L, 1));
	return 1;
}

const luaL_Reg Lua_Texture_Palette_Get[] = {
	{"highlight", Lua_Texture_Palette_Get_Selected},
	{"size", Lua_Texture_Palette_Get_Size},
	{"slots", Lua_Texture_Palette_Get_Slots},
	{0, 0}
};

extern void draw_panels();

static int Lua_Texture_Palette_Set_Selected(lua_State *L)
{
	int player_index = Lua_Texture_Palette::Index(L, 1);
	if (player_index != local_player_index)
		return 0;

	if (lua_isnil(L, 2))
		lua_texture_palette_selected = -1;
	else if (lua_isnumber(L, 2))
	{
		int selected = static_cast<int>(lua_tonumber(L, 2));
		if (selected < -1 || selected > lua_texture_palette.size())
			return luaL_error(L, "highlight: invalid slot");

		lua_texture_palette_selected = selected;
		draw_panels();
	}
	else
		return luaL_error(L, "highlight: incorrect argument type");

	return 0;
}


static int Lua_Texture_Palette_Set_Size(lua_State *L)
{
	int player_index = Lua_Texture_Palette::Index(L, 1);
	if (player_index != local_player_index)
		return 0;

	if (!lua_isnumber(L, 2))
		return luaL_error(L, "size: incorrect argument type");

	size_t size = static_cast<size_t>(lua_tonumber(L, 2));
	if (size > MAX_TEXTURE_PALETTE_SIZE)
		return luaL_error(L, "size: Its really big");

    lua_texture blank = { UNONE, 0 };
	lua_texture_palette.resize(size, blank);
	if (lua_texture_palette_selected >= lua_texture_palette.size())
		lua_texture_palette_selected = -1;

	draw_panels();
	return 0;
}

const luaL_Reg Lua_Texture_Palette_Set[] = {
	{"highlight", Lua_Texture_Palette_Set_Selected},
	{"size", Lua_Texture_Palette_Set_Size},
	{0, 0}
};

char Lua_WeaponType_Name[] = "weapon_type";
typedef L_Enum<Lua_WeaponType_Name> Lua_WeaponType;

char Lua_WeaponTypes_Name[] = "WeaponTypes";
typedef L_EnumContainer<Lua_WeaponTypes_Name, Lua_WeaponType> Lua_WeaponTypes;
	
char Lua_Player_Weapon_Trigger_Name[] = "player_weapon_trigger";
class Lua_Player_Weapon_Trigger : public PlayerSubtable<Lua_Player_Weapon_Trigger_Name>
{
public:
	int16 m_weapon_index;

	static Lua_Player_Weapon_Trigger *Push(lua_State *L, int16 player_index, int16 weapon_index, int16 index);
	static int16 WeaponIndex(lua_State *L, int index);
};

Lua_Player_Weapon_Trigger *Lua_Player_Weapon_Trigger::Push(lua_State *L, int16 player_index, int16 weapon_index, int16 index)
{
	Lua_Player_Weapon_Trigger *t = PlayerSubtable::Push<Lua_Player_Weapon_Trigger>(L, player_index, index);
	if (t)
	{
		t->m_weapon_index = weapon_index;
	}

	return t;
}

int16 Lua_Player_Weapon_Trigger::WeaponIndex(lua_State *L, int index)
{
	Lua_Player_Weapon_Trigger *t = static_cast<Lua_Player_Weapon_Trigger*>(Instance(L, index));
	if (!t) luaL_typerror(L, index, Lua_Player_Weapon_Trigger_Name);
	return t->m_weapon_index;
}

static int Lua_Player_Weapon_Trigger_Get_Rounds(lua_State *L)
{
	short rounds = get_player_weapon_ammo_count(
		Lua_Player_Weapon_Trigger::PlayerIndex(L, 1), 
		Lua_Player_Weapon_Trigger::WeaponIndex(L, 1),
		Lua_Player_Weapon_Trigger::Index(L, 1));
	lua_pushnumber(L, rounds);
	return 1;
}

const luaL_Reg Lua_Player_Weapon_Trigger_Get[] = {
	{"rounds", Lua_Player_Weapon_Trigger_Get_Rounds},
	{0, 0}
};

char Lua_Player_Weapon_Name[] = "player_weapon";
typedef PlayerSubtable<Lua_Player_Weapon_Name> Lua_Player_Weapon;

template<int trigger>
static int get_weapon_trigger(lua_State *L)
{
	Lua_Player_Weapon_Trigger::Push(L, Lua_Player_Weapon::PlayerIndex(L, 1), Lua_Player_Weapon::Index(L, 1), trigger);
	return 1;
}

static int Lua_Player_Weapon_Get_Type(lua_State *L)
{
	Lua_WeaponType::Push(L, Lua_Player_Weapon::Index(L, 1));
	return 1;
}

extern bool ready_weapon(short player_index, short weapon_index);

int Lua_Player_Weapon_Select(lua_State *L)
{
	ready_weapon(Lua_Player_Weapon::PlayerIndex(L, 1), Lua_Player_Weapon::Index(L, 1));
	return 0;
}

const luaL_Reg Lua_Player_Weapon_Get[] = { 
	{"primary", get_weapon_trigger<_primary_weapon>},
	{"secondary", get_weapon_trigger<_secondary_weapon>},
	{"select", L_TableFunction<Lua_Player_Weapon_Select>},
	{"type", Lua_Player_Weapon_Get_Type},
	{0, 0} 
};

extern player_weapon_data *get_player_weapon_data(const short player_index);
extern bool player_has_valid_weapon(short player_index);

char Lua_Player_Weapons_Name[] = "player_weapons";
typedef L_Class<Lua_Player_Weapons_Name> Lua_Player_Weapons;

extern bool can_wield_weapons[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];

static int Lua_Player_Weapons_Get(lua_State *L)
{
	int player_index = Lua_Player_Weapons::Index(L, 1);
	bool string_arg = lua_isstring(L, 2) && !lua_isnumber(L, 2);
	if (string_arg && (strcmp(lua_tostring(L, 2), "current") == 0))
	{
	    if (player_has_valid_weapon(player_index))
	    {
	        player_weapon_data *weapon_data = get_player_weapon_data(player_index);
	        Lua_Player_Weapon::Push(L, player_index, weapon_data->current_weapon);
	    }
	    else
	    {
	        lua_pushnil(L);
	    }
	}
	else if (string_arg && (strcmp(lua_tostring(L, 2), "desired") == 0))
	{
	    player_weapon_data *weapon_data = get_player_weapon_data(player_index);
	    if (weapon_data->desired_weapon != NONE)
	    {
	        Lua_Player_Weapon::Push(L, player_index, weapon_data->desired_weapon);
	    }
	    else
	    {
	        lua_pushnil(L);
	    }
	}
	else if (string_arg && (strcmp(lua_tostring(L, 2), "active") == 0))
	{
	    lua_pushboolean(L, can_wield_weapons[player_index]);
	}
	else
	{
		int index = Lua_WeaponType::ToIndex(L, 2);
		Lua_Player_Weapon::Push(L, player_index, index);
	}

	return 1;
}

static int Lua_Player_Weapons_Length(lua_State *L)
{
	lua_pushnumber(L, MAXIMUM_NUMBER_OF_WEAPONS);
	return 1;
}

static int Lua_Player_Weapons_Set(lua_State *L)
{
	if (lua_isstring(L, 2) && strcmp(lua_tostring(L, 2), "active") == 0)
	{
		if (!lua_isboolean(L, 3))
			return luaL_error(L, "can_wield: incorrect argument type");
		can_wield_weapons[Lua_Player_Weapons::Index(L, 1)] = lua_toboolean(L, 3);
		return 0;
	}
	else
		return luaL_error(L, "no such index");
}

const luaL_Reg Lua_Player_Weapons_Metatable[] = {
	{"__index", Lua_Player_Weapons_Get},
	{"__newindex", Lua_Player_Weapons_Set},
	{"__len", Lua_Player_Weapons_Length},
	{0, 0}
};

char Lua_Player_Kills_Name[] = "player_kills";
typedef L_Class<Lua_Player_Kills_Name> Lua_Player_Kills;

static int Lua_Player_Kills_Get(lua_State *L)
{
	int player_index = Lua_Player_Kills::Index(L, 1);
	int slain_player_index = Lua_Player::Index(L, 2);
	
	player_data *slain_player = get_player_data(slain_player_index);

	lua_pushnumber(L, slain_player->damage_taken[player_index].kills);
	return 1;
}			

static int Lua_Player_Kills_Length(lua_State *L)
{
    lua_pushnumber(L, dynamic_world->player_count);
    return 1;
}

static int Lua_Player_Kills_Set(lua_State *L)
{
	if (!lua_isnumber(L, 3))
		return luaL_error(L, "kills: incorrect argument type");

	int player_index = Lua_Player_Kills::Index(L, 1);
	int slain_player_index = Lua_Player::Index(L, 2);	
	int kills = static_cast<int>(lua_tonumber(L, 3));

	player_data *player = get_player_data(player_index);
	player_data *slain_player = get_player_data(slain_player_index);

	int kills_award = kills - slain_player->damage_taken[player_index].kills;
	if (kills_award)
	{
		slain_player->damage_taken[player_index].kills += kills_award;
		team_damage_taken[slain_player->team].kills += kills_award;

		if (player_index != slain_player_index)
		{
			player->total_damage_given.kills += kills_award;
			team_damage_given[player->team].kills += kills_award;
		}
		if (slain_player->team == player->team)
		{
			team_friendly_fire[slain_player->team].kills += kills_award;
		}
		mark_player_network_stats_as_dirty(current_player_index);
	}
	return 0;
}

const luaL_Reg Lua_Player_Kills_Metatable[] = {
	{"__index", Lua_Player_Kills_Get},
	{"__newindex", Lua_Player_Kills_Set},
	{"__len", Lua_Player_Kills_Length},
	{0, 0}
};

char Lua_PlayerColor_Name[] = "player_color";

char Lua_PlayerColors_Name[] = "PlayerColors";

char Lua_Player_Name[] = "player";

// methods

// accelerate(direction, velocity, vertical_velocity)
int Lua_Player_Accelerate(lua_State *L)
{
	if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		return luaL_error(L, "accelerate: incorrect argument type");

	player_data *player = get_player_data(Lua_Player::Index(L, 1));
	double direction = static_cast<double>(lua_tonumber(L, 2));
	double velocity = static_cast<double>(lua_tonumber(L, 3));
	double vertical_velocity = static_cast<double>(lua_tonumber(L, 4));

	accelerate_player(player->monster_index, static_cast<int>(vertical_velocity * WORLD_ONE), static_cast<int>(direction/AngleConvert), static_cast<int>(velocity * WORLD_ONE));
	return 0;
}

int Lua_Player_Activate_Terminal(lua_State *L)
{
	int16 text_index = NONE;
	if (lua_isnumber(L, 2))
		text_index = static_cast<int16>(lua_tonumber(L, 2));
	else if (Lua_Terminal::Is(L, 2))
		text_index = Lua_Terminal::Index(L, 2);
	else
		return luaL_error(L, "activate_terminal: invalid terminal index");

	enter_computer_interface(Lua_Player::Index(L, 1), text_index, calculate_level_completion_state());
	return 0;
}

int Lua_Player_Find_Action_Key_Target(lua_State *L)
{
	short target_type;
	short object_index = find_action_key_target(Lua_Player::Index(L, 1), MAXIMUM_ACTIVATION_RANGE, &target_type, film_profile.find_action_key_target_has_side_effects);

	if (object_index != NONE)
	{
		switch (target_type)
		{
		case _target_is_platform:
			Lua_Platform::Push(L, object_index);
			break;

		case _target_is_control_panel:
			Lua_Side::Push(L, object_index);
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

extern projectile_definition *get_projectile_definition(short type);

int Lua_Player_Find_Target(lua_State *L)
{
	// find the origin of projectiles (don't move left/right)
	player_data *player = get_player_data(Lua_Player::Index(L, 1));
	world_point3d origin = player->camera_location;
	world_point3d destination = origin;

	translate_point3d(&destination, WORLD_ONE, player->facing, player->elevation);
	short old_polygon = get_object_data(player->object_index)->polygon;
	short new_polygon;
	short obstruction_index;
	short line_index;

	projectile_definition *definition = get_projectile_definition(0);
	bool was_pass_transparent = definition->flags & _usually_pass_transparent_side;
	if (!was_pass_transparent)
		definition->flags |= _usually_pass_transparent_side;

	// preflight a projectile, 1 WU at a time (because of projectile speed bug)
	uint16 flags = translate_projectile(0, &origin, old_polygon, &destination, &new_polygon, player->monster_index, &obstruction_index, &line_index, true, NONE);

	while (!(flags & _projectile_hit))
	{
		origin = destination;
		old_polygon = new_polygon;

		translate_point3d(&destination, WORLD_ONE, player->facing, player->elevation);
		flags = translate_projectile(0, &origin, old_polygon, &destination, &new_polygon, player->monster_index, &obstruction_index, &line_index, true, NONE);
	}

	if (!was_pass_transparent) 
		definition->flags &= ~_usually_pass_transparent_side;

	if (flags & _projectile_hit_monster)
	{
		object_data *object = get_object_data(obstruction_index);
		Lua_Monster::Push(L, object->permutation);
	}
	else if (flags & _projectile_hit_floor)
	{
		Lua_Polygon_Floor::Push(L, new_polygon);
	}
	else if (flags & _projectile_hit_media)
	{
		Lua_Polygon::Push(L, new_polygon);
	}
	else if (flags & _projectile_hit_scenery)
	{
		Lua_Scenery::Push(L, obstruction_index);
	}
	else if (obstruction_index != NONE)
	{
		Lua_Polygon_Ceiling::Push(L, new_polygon);
	}
	else
	{
		short side_index = find_adjacent_side(new_polygon, line_index);
		Lua_Side::Push(L, side_index);
	}

	lua_pushnumber(L, (double) destination.x / WORLD_ONE);
	lua_pushnumber(L, (double) destination.y / WORLD_ONE);
	lua_pushnumber(L, (double) destination.z / WORLD_ONE);
	Lua_Polygon::Push(L, new_polygon);

	return 5;
}
	

int Lua_Player_Damage(lua_State *L)
{
	int args = lua_gettop(L);
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "damage: incorrect argument type");
	
	player_data *player = get_player_data(Lua_Player::Index(L, 1));
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

int Lua_Player_Fade_Screen(lua_State *L)
{
	short player_index = Lua_Player::Index(L, 1);
	if (player_index == local_player_index)
	{
		int fade_index = Lua_FadeType::ToIndex(L, 2);
		start_fade(fade_index);
	}
	return 0;
}

int Lua_Player_Play_Sound(lua_State *L)
{
	int args = lua_gettop(L);
	
	int player_index = Lua_Player::Index(L, 1);
	int sound_index = Lua_Sound::ToIndex(L, 2);
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

extern bool mute_lua;

int Lua_Player_Print(lua_State *L)
{
	if (mute_lua) return 0;

	if (lua_gettop(L) != 2) 
		return luaL_error(L, "print: incorrect argument type");

	int player_index = Lua_Player::Index(L, 1);
	if (player_index == (IsScriptHUDNonlocal() ? current_player_index : local_player_index))
	{
		lua_getglobal(L, "tostring");
		lua_insert(L, -2);
		lua_pcall(L, 1, 1, 0);
		if (lua_tostring(L, -1))
		{
			screen_printf("%s", lua_tostring(L, -1));
		}
		lua_pop(L, 1);
	}
	
	return 0;
}

extern struct physics_constants *get_physics_constants_for_model(short physics_model, uint32 action_flags);
extern void instantiate_physics_variables(struct physics_constants *constants, struct physics_variables *variables, short player_index, bool first_time, bool take_action);

int Lua_Player_Position(lua_State *L)
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

	int player_index = Lua_Player::Index(L, 1);
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

int Lua_Player_Teleport(lua_State *L)
{
	if (!lua_isnumber(L, 2) && !Lua_Polygon::Is(L, 2))
		return luaL_error(L, "teleport(): incorrect argument type");

	int destination = -1;
	if (lua_isnumber(L, 2))
		destination = static_cast<int>(lua_tonumber(L, 2));
	else 
		destination = Lua_Polygon::Index(L, 2);

	int player_index = Lua_Player::Index(L, 1);
	
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

int Lua_Player_Teleport_To_Level(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "teleport_to_level(): incorrect argument type");

	int level = static_cast<int>(lua_tonumber(L, 2));
	int player_index = Lua_Player::Index(L, 1);
	
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

extern short current_player_index;

int Lua_Player_Get_Viewed_Player(lua_State *L)
{
	int player_index = Lua_Player::Index(L, 1);
	if (player_index != local_player_index)
		return 0;
	Lua_Player::Push(L, current_player_index);
	return 1;
}

int Lua_Player_View_Player(lua_State *L)
{
	int player_index = Lua_Player::Index(L, 1);
	if (player_index != local_player_index)
		return 0;

	int view_player_index;
	if (lua_isnumber(L, 2))
	{
		view_player_index = static_cast<int>(lua_tonumber(L, 2));
		if (view_player_index < 0 || view_player_index >= dynamic_world->player_count)
			return luaL_error(L, "view_player(): invalid player index");
	}
	else if (Lua_Player::Is(L, 2))
		view_player_index = Lua_Player::Index(L, 2);
	else
		return luaL_error(L, "view_player(): incorrect argument type");
	
	if (view_player_index != current_player_index)
	{
		set_current_player_index(view_player_index);
		update_interface(NONE);
		dirty_terminal_view(player_index);
	}

	return 0;
		
}

// get accessors

static int Lua_Player_Get_Action_Flags(lua_State *L)
{
	Lua_Action_Flags::Push(L, Lua_Player::Index(L, 1));
	return 1;
}

static int Lua_Player_Get_Color(lua_State *L)
{
	Lua_PlayerColor::Push(L, get_player_data(Lua_Player::Index(L, 1))->color);
	return 1;
}

static int Lua_Player_Get_Compass(lua_State *L)
{
	Lua_Player_Compass::Push(L, Lua_Player::Index(L, 1));
	return 1;
}

static int Lua_Player_Get_Crosshairs(lua_State *L)
{
	Lua_Crosshairs::Push(L, Lua_Player::Index(L, 1));
	return 1;
}

static int Lua_Player_Get_Dead(lua_State *L)
{
	player_data *player = get_player_data(Lua_Player::Index(L, 1));
	lua_pushboolean(L, (PLAYER_IS_DEAD(player) || PLAYER_IS_TOTALLY_DEAD(player)));
	return 1;
}

static int Lua_Player_Get_Deaths(lua_State *L)
{
	player_data *player = get_player_data(Lua_Player::Index(L, 1));
	lua_pushnumber(L, player->monster_damage_taken.kills);
	return 1;
}

static int Lua_Player_Get_Energy(lua_State *L)
{
	lua_pushnumber(L, get_player_data(Lua_Player::Index(L, 1))->suit_energy);
	return 1;
}

static int Lua_Player_Get_Elevation(lua_State *L)
{
	double angle = FIXED_INTEGERAL_PART(get_player_data(Lua_Player::Index(L, 1))->variables.elevation) * AngleConvert;
	lua_pushnumber(L, angle);
	return 1;
}

static int Lua_Player_Get_Direction(lua_State *L)
{
	double angle = FIXED_INTEGERAL_PART(get_player_data(Lua_Player::Index(L, 1))->variables.direction) * AngleConvert;
	lua_pushnumber(L, angle);
	return 1;
}

static int Lua_Player_Get_Head_Direction(lua_State *L)
{
	player_data *pdata = get_player_data(Lua_Player::Index(L, 1));
	double angle = FIXED_INTEGERAL_PART(pdata->variables.direction + pdata->variables.head_direction) * AngleConvert;
	if (angle >= 360.0) { angle -= 360.0; }
	if (angle <    0.0) { angle += 360.0; }
	lua_pushnumber(L, angle);
	return 1;
}

static int Lua_Player_Get_External_Velocity(lua_State *L)
{
	Lua_ExternalVelocity::Push(L, Lua_Player::Index(L, 1));
	return 1;
}

static int Lua_Player_Get_Extravision_Duration(lua_State *L)
{
	lua_pushnumber(L, get_player_data(Lua_Player::Index(L, 1))->extravision_duration);
	return 1;
}

template<uint16 flag>
static int Lua_Player_Get_Flag(lua_State *L)
{
	player_data *player = get_player_data(Lua_Player::Index(L, 1));
	lua_pushboolean(L, player->variables.flags & flag);
	return 1;
}

static int Lua_Player_Get_Hotkey(lua_State* L)
{
	int player_index = Lua_Player::Index(L, 1);
	if (GetGameQueue()->countActionFlags(player_index))
	{
		auto player = get_player_data(player_index);
		lua_pushnumber(L, player->hotkey);
	}
	else
	{
		return luaL_error(L, "hotkey is only accessible in idle()");
	}

	return 1;
}

static int Lua_Player_Get_Hotkey_Bindings(lua_State* L)
{
	auto player_index = Lua_Player::Index(L, 1);
	if (player_index == local_player_index)
	{
		Lua_HotkeyBindings::Push(L, Lua_Player::Index(L, 1));
		return 1;
	}
	else
	{
		return 0;
	}
}

static int Lua_Player_Get_Infravision_Duration(lua_State *L)
{
	lua_pushnumber(L, get_player_data(Lua_Player::Index(L, 1))->infravision_duration);
	return 1;
}

static int Lua_Player_Get_Internal_Velocity(lua_State *L)
{
	Lua_InternalVelocity::Push(L, Lua_Player::Index(L, 1));
	return 1;
}

static int Lua_Player_Get_Invincibility_Duration(lua_State *L)
{
	lua_pushnumber(L, get_player_data(Lua_Player::Index(L, 1))->invincibility_duration);
	return 1;
}

static int Lua_Player_Get_Invisibility_Duration(lua_State *L)
{
	lua_pushnumber(L, get_player_data(Lua_Player::Index(L, 1))->invisibility_duration);
	return 1;
}

static int Lua_Player_Get_Items(lua_State *L)
{
	Lua_Player_Items::Push(L, Lua_Player::Index(L, 1));
	return 1;
}

static int Lua_Player_Get_Kills(lua_State *L)
{
	Lua_Player_Kills::Push(L, Lua_Player::Index(L, 1));
	return 1;
}

static int Lua_Player_Get_Local(lua_State *L)
{
	lua_pushboolean(L, Lua_Player::Index(L, 1) == local_player_index);
	return 1;
}

extern bool MotionSensorActive;

static int Lua_Player_Get_Motion_Sensor(lua_State *L)
{
	short player_index = Lua_Player::Index(L, 1);
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

static int Lua_Player_Get_Monster(lua_State *L)
{
	Lua_Monster::Push(L, get_player_data(Lua_Player::Index(L, 1))->monster_index);
	return 1;
}

static int Lua_Player_Get_Name(lua_State *L)
{
	lua_pushstring(L, get_player_data(Lua_Player::Index(L, 1))->name);
	return 1;
}

static int Lua_Player_Get_Netdead(lua_State *L)
{
	lua_pushboolean(L, get_player_data(Lua_Player::Index(L, 1))->netdead);
	return 1;
}

static int Lua_Player_Get_Overlays(lua_State *L)
{
	Lua_Overlays::Push(L, Lua_Player::Index(L, 1));
	return 1;
}

static int Lua_Player_Get_Oxygen(lua_State *L)
{
	lua_pushnumber(L, get_player_data(Lua_Player::Index(L, 1))->suit_oxygen);
	return 1;
}

static int Lua_Player_Get_Points(lua_State *L)
{
	lua_pushnumber(L, get_player_data(Lua_Player::Index(L, 1))->netgame_parameters[0]);
	return 1;
}

static int Lua_Player_Get_Polygon(lua_State *L)
{
	Lua_Polygon::Push(L, get_player_data(Lua_Player::Index(L, 1))->supporting_polygon_index);
	return 1;
}

static int Lua_Player_Get_Team(lua_State *L)
{
	Lua_PlayerColor::Push(L, get_player_data(Lua_Player::Index(L, 1))->team);
	return 1;
}

static int Lua_Player_Get_Texture_Palette(lua_State *L)
{
	Lua_Texture_Palette::Push(L, Lua_Player::Index(L, 1));
	return 1;
}

static int Lua_Player_Get_Totally_Dead(lua_State* L)
{
	auto player = get_player_data(Lua_Player::Index(L, 1));
	lua_pushboolean(L, PLAYER_IS_TOTALLY_DEAD(player));
	return 1;
}

static int Lua_Player_Get_Weapons(lua_State *L)
{
	Lua_Player_Weapons::Push(L, Lua_Player::Index(L, 1));
	return 1;
}

static int Lua_Player_Get_X(lua_State *L)
{
	lua_pushnumber(L, (double) get_player_data(Lua_Player::Index(L, 1))->location.x / WORLD_ONE);
	return 1;
}

static int Lua_Player_Get_Y(lua_State *L)
{
	lua_pushnumber(L, (double) get_player_data(Lua_Player::Index(L, 1))->location.y / WORLD_ONE);
	return 1;
}

static int Lua_Player_Get_Z(lua_State *L)
{
	lua_pushnumber(L, (double) get_player_data(Lua_Player::Index(L, 1))->location.z / WORLD_ONE);
	return 1;
}

static int Lua_Player_Get_Zoom(lua_State *L)
{
	short player_index = Lua_Player::Index(L, 1);
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

extern void revive_player(int16_t);

static int Lua_Player_Revive(lua_State* L)
{
	short player_index = Lua_Player::Index(L, 1);
	auto player = get_player_data(player_index);
	if (!PLAYER_IS_TOTALLY_DEAD(player))
	{
		return luaL_error(L, "revive: you can only revive totally dead players");
	}

	revive_player(player_index);
	return 0;
}

const luaL_Reg Lua_Player_Get[] = {
	{"accelerate", L_TableFunction<Lua_Player_Accelerate>},
	{"action_flags", Lua_Player_Get_Action_Flags},
	{"activate_terminal", L_TableFunction<Lua_Player_Activate_Terminal>},
	{"color", Lua_Player_Get_Color},
	{"compass", Lua_Player_Get_Compass},
	{"crosshairs", Lua_Player_Get_Crosshairs},
	{"damage", L_TableFunction<Lua_Player_Damage>},
	{"dead", Lua_Player_Get_Dead},
	{"deaths", Lua_Player_Get_Deaths},
	{"direction", Lua_Player_Get_Direction},
	{"disconnected", Lua_Player_Get_Netdead},
	{"energy", Lua_Player_Get_Energy},
	{"elevation", Lua_Player_Get_Elevation},
	{"external_velocity", Lua_Player_Get_External_Velocity},
	{"extravision_duration", Lua_Player_Get_Extravision_Duration},
	{"feet_below_media", Lua_Player_Get_Flag<_FEET_BELOW_MEDIA_BIT>},
	{"fade_screen", L_TableFunction<Lua_Player_Fade_Screen>},
	{"find_action_key_target", L_TableFunction<Lua_Player_Find_Action_Key_Target>},
	{"find_target", L_TableFunction<Lua_Player_Find_Target>},
	{"head_below_media", Lua_Player_Get_Flag<_HEAD_BELOW_MEDIA_BIT>},
	{"head_direction", Lua_Player_Get_Head_Direction},
	{"hotkey", Lua_Player_Get_Hotkey},
	{"hotkey_bindings", Lua_Player_Get_Hotkey_Bindings},
	{"infravision_duration", Lua_Player_Get_Infravision_Duration},
	{"internal_velocity", Lua_Player_Get_Internal_Velocity},
	{"invincibility_duration", Lua_Player_Get_Invincibility_Duration},
	{"invisibility_duration", Lua_Player_Get_Invisibility_Duration},
	{"items", Lua_Player_Get_Items},
	{"local_", Lua_Player_Get_Local},
	{"juice", Lua_Player_Get_Energy},
	{"kills", Lua_Player_Get_Kills},
	{"life", Lua_Player_Get_Energy},
	{"monster", Lua_Player_Get_Monster},
	{"motion_sensor_active", Lua_Player_Get_Motion_Sensor},
	{"name", Lua_Player_Get_Name},
	{"overlays", Lua_Player_Get_Overlays},
	{"oxygen", Lua_Player_Get_Oxygen},
	{"pitch", Lua_Player_Get_Elevation},
	{"print", L_TableFunction<Lua_Player_Print>},
	{"play_sound", L_TableFunction<Lua_Player_Play_Sound>},
	{"points", Lua_Player_Get_Points},
	{"polygon", Lua_Player_Get_Polygon},
	{"position", L_TableFunction<Lua_Player_Position>},
	{"revive", L_TableFunction<Lua_Player_Revive>},
	{"team", Lua_Player_Get_Team},
	{"teleport", L_TableFunction<Lua_Player_Teleport>},
	{"teleport_to_level", L_TableFunction<Lua_Player_Teleport_To_Level>},
	{"texture_palette", Lua_Player_Get_Texture_Palette},
	{"totally_dead", Lua_Player_Get_Totally_Dead},
	{"view_player", L_TableFunction<Lua_Player_View_Player>},
	{"viewed_player", Lua_Player_Get_Viewed_Player},
	{"weapons", Lua_Player_Get_Weapons},
	{"x", Lua_Player_Get_X},
	{"y", Lua_Player_Get_Y},
	{"yaw", Lua_Player_Get_Direction},
	{"z", Lua_Player_Get_Z},
	{"zoom_active", Lua_Player_Get_Zoom},
	{0, 0}
};

extern void mark_shield_display_as_dirty();

static int Lua_Player_Set_Color(lua_State *L)
{
	int color = Lua_PlayerColor::ToIndex(L, 2);
	get_player_data(Lua_Player::Index(L, 1))->color = color;
	
	return 0;
}

static int Lua_Player_Set_Deaths(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "deaths: incorrect argument type");

	player_data *player = get_player_data(Lua_Player::Index(L, 1));
	int kills = static_cast<int>(lua_tonumber(L, 2));
	if (player->monster_damage_taken.kills != kills)
	{
		team_monster_damage_taken[player->team].kills += (kills - player->monster_damage_taken.kills);
		player->monster_damage_taken.kills = kills;
		mark_player_network_stats_as_dirty(current_player_index);
	}

	return 0;
}

static int Lua_Player_Set_Direction(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "direction: incorrect argument type");

	double facing = static_cast<double>(lua_tonumber(L, 2));
	int player_index = Lua_Player::Index(L, 1);
	player_data *player = get_player_data(player_index);
	player->variables.direction = INTEGER_TO_FIXED((int)(facing/AngleConvert));
	instantiate_physics_variables(get_physics_constants_for_model(static_world->physics_model, 0), &player->variables, player_index, false, false);
	
	// Lua control locks virtual aim to physical aim
	if (player_index == local_player_index)
		resync_virtual_aim();
	
	return 0;
}

static int Lua_Player_Set_Head_Direction(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "head_direction: incorrect argument type");
	
	double facing = static_cast<double>(lua_tonumber(L, 2));
	int player_index = Lua_Player::Index(L, 1);
	player_data *player = get_player_data(player_index);
	player->variables.head_direction = INTEGER_TO_FIXED((int)(facing/AngleConvert)) - player->variables.direction;
	while (player->variables.head_direction >= INTEGER_TO_FIXED(HALF_CIRCLE)) {
		player->variables.head_direction -= INTEGER_TO_FIXED(FULL_CIRCLE);
	}
	while (player->variables.head_direction < -1*INTEGER_TO_FIXED(HALF_CIRCLE)) {
		player->variables.head_direction += INTEGER_TO_FIXED(FULL_CIRCLE);
	}
	instantiate_physics_variables(get_physics_constants_for_model(static_world->physics_model, 0), &player->variables, player_index, false, false);
	
	return 0;
}

static int Lua_Player_Set_Hotkey(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "hotkey: incorrect argument type");

	int hotkey = static_cast<int>(lua_tonumber(L, 2));
	int player_index = Lua_Player::Index(L, 1);
	if (GetGameQueue()->countActionFlags(player_index))
	{
		auto player = get_player_data(player_index);
		player->hotkey = hotkey;
	}
	else
	{
		return luaL_error(L, "hotkey is only accessible in idle()");
	}

	return 0;
}

static int Lua_Player_Set_Elevation(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "elevation: incorrect argument type");
	
	double elevation = static_cast<double>(lua_tonumber(L, 2));
	if (elevation > 180) elevation -= 360.0;
	int player_index = Lua_Player::Index(L, 1);
	player_data *player = get_player_data(player_index);
	player->variables.elevation = INTEGER_TO_FIXED((int)(elevation/AngleConvert));
	instantiate_physics_variables(get_physics_constants_for_model(static_world->physics_model, 0), &player->variables, player_index, false, false);
	
	// Lua control locks virtual aim to physical aim
	if (player_index == local_player_index)
		resync_virtual_aim();
	
	return 0;
}

static int Lua_Player_Set_Infravision_Duration(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "extravision: incorrect argument type");

	player_data *player = get_player_data(Lua_Player::Index(L, 1));
	player->infravision_duration = static_cast<int>(lua_tonumber(L, 2));
	return 0;
}

static int Lua_Player_Set_Invincibility_Duration(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "extravision: incorrect argument type");

	player_data *player = get_player_data(Lua_Player::Index(L, 1));
	player->invincibility_duration = static_cast<int>(lua_tonumber(L, 2));
	return 0;
}

static int Lua_Player_Set_Invisibility_Duration(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "extravision: incorrect argument type");

	player_data *player = get_player_data(Lua_Player::Index(L, 1));
	player->invisibility_duration = static_cast<int>(lua_tonumber(L, 2));
	return 0;
}

static int Lua_Player_Set_Energy(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "energy: incorrect argument type");

	int energy = static_cast<int>(lua_tonumber(L, 2));
	if (energy > 3 * PLAYER_MAXIMUM_SUIT_ENERGY)
		energy = 3 * PLAYER_MAXIMUM_SUIT_ENERGY;

	get_player_data(Lua_Player::Index(L, 1))->suit_energy = energy;
	mark_shield_display_as_dirty();

	return 0;
}

static int Lua_Player_Set_Extravision_Duration(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "extravision: incorrect argument type");

	int player_index = Lua_Player::Index(L, 1);
	player_data *player = get_player_data(player_index);
	short extravision_duration = static_cast<short>(lua_tonumber(L, 2));
	if ((player_index == local_player_index) && (extravision_duration == 0) != (player->extravision_duration == 0))
	{
		start_extravision_effect(extravision_duration);
	}
	player->extravision_duration = static_cast<int>(lua_tonumber(L, 2));
	return 0;
}

static int Lua_Player_Set_Motion_Sensor(lua_State *L)
{
	short player_index = Lua_Player::Index(L, 1);
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

static int Lua_Player_Set_Oxygen(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "oxygen: incorrect argument type");
	
	int oxygen = static_cast<int>(lua_tonumber(L, 2));
	if (oxygen > PLAYER_MAXIMUM_SUIT_OXYGEN)
		oxygen = PLAYER_MAXIMUM_SUIT_OXYGEN;

	get_player_data(Lua_Player::Index(L, 1))->suit_oxygen = oxygen;
	mark_shield_display_as_dirty();

	return 0;
}

static int Lua_Player_Set_Points(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "points: incorrect argument type");

	int points = static_cast<int>(lua_tonumber(L, 2));

	player_data *player = get_player_data(Lua_Player::Index(L, 1));
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

static int Lua_Player_Set_Team(lua_State *L)
{
	int team = Lua_PlayerColor::ToIndex(L, 2);
	get_player_data(Lua_Player::Index(L, 1))->team = team;

	return 0;
}

static int Lua_Player_Set_Zoom(lua_State *L)
{
	short player_index = Lua_Player::Index(L, 1);
	if (player_index == local_player_index)
	{
		if (!lua_isboolean(L, 2))
			return luaL_error(L, "zoom_active: incorrect argument type");
		
		SetTunnelVision(lua_toboolean(L, 2));
	}

	return 0;
}

const luaL_Reg Lua_Player_Set[] = {
	{"color", Lua_Player_Set_Color},
	{"deaths", Lua_Player_Set_Deaths},
	{"direction", Lua_Player_Set_Direction},
	{"head_direction", Lua_Player_Set_Head_Direction},
	{"hotkey", Lua_Player_Set_Hotkey},
	{"elevation", Lua_Player_Set_Elevation},
	{"energy", Lua_Player_Set_Energy},
	{"extravision_duration", Lua_Player_Set_Extravision_Duration},
	{"infravision_duration", Lua_Player_Set_Infravision_Duration},
	{"invincibility_duration", Lua_Player_Set_Invincibility_Duration},
	{"invisibility_duration", Lua_Player_Set_Invisibility_Duration},
	{"juice", Lua_Player_Set_Energy},
	{"life", Lua_Player_Set_Energy},
	{"motion_sensor_active", Lua_Player_Set_Motion_Sensor},
	{"oxygen", Lua_Player_Set_Oxygen},
	{"pitch", Lua_Player_Set_Elevation},
	{"points", Lua_Player_Set_Points},
	{"team", Lua_Player_Set_Team},
	{"yaw", Lua_Player_Set_Direction},
	{"zoom_active", Lua_Player_Set_Zoom},
	{0, 0}
};

bool Lua_Player_Valid(int16 index)
{
	return index >= 0 && index < dynamic_world->player_count;
}

char Lua_Players_Name[] = "Players";

int Lua_Players_Print(lua_State *L)
{
	if (mute_lua) return 0;

	if (lua_gettop(L) != 1) 
		return luaL_error(L, "print: incorrect argument type");

	lua_getglobal(L, "tostring");
	lua_insert(L, -2);
	lua_pcall(L, 1, 1, 0);
	if (lua_tostring(L, -1))
	{
		screen_printf("%s", lua_tostring(L, -1));
	}
	lua_pop(L, 1);

	return 0;
}

int Lua_Players_Get_Local_Player(lua_State *L)
{
	Lua_Player::Push(L, local_player_index);
	return 1;
}

const luaL_Reg Lua_Players_Get[] = {
	{"local_player", Lua_Players_Get_Local_Player},
	{"print", L_TableFunction<Lua_Players_Print>},
	{0, 0}
};

int16 Lua_Players_Length() {
	return dynamic_world->player_count;
}

char Lua_DifficultyType_Name[] = "difficulty_type";
typedef L_Enum<Lua_DifficultyType_Name> Lua_DifficultyType;

char Lua_DifficultyTypes_Name[] = "DifficultyTypes";
typedef L_EnumContainer<Lua_DifficultyTypes_Name, Lua_DifficultyType> Lua_DifficultyTypes;

char Lua_GameType_Name[] = "game_type";
typedef L_Enum<Lua_GameType_Name> Lua_GameType;

char Lua_GameTypes_Name[] = "GameTypes";
typedef L_EnumContainer<Lua_GameTypes_Name, Lua_GameType> Lua_GameTypes;

char Lua_Game_Name[] = "Game";
typedef L_Class<Lua_Game_Name> Lua_Game;

char Lua_ScoringMode_Name[] = "scoring_mode";
typedef L_Enum<Lua_ScoringMode_Name> Lua_ScoringMode;

char Lua_ScoringModes_Name[] = "ScoringModes";
typedef L_Container<Lua_ScoringModes_Name, Lua_ScoringMode> Lua_ScoringModes;

static int Lua_Game_Get_Dead_Players_Drop_Items(lua_State *L)
{
	lua_pushboolean(L, !(GET_GAME_OPTIONS() & _burn_items_on_death));
	return 1;
}

static int Lua_Game_Get_Difficulty(lua_State *L)
{
	Lua_DifficultyType::Push(L, dynamic_world->game_information.difficulty_level);
	return 1;
}

static int Lua_Game_Get_Kill_Limit(lua_State *L)
{
	lua_pushnumber(L, dynamic_world->game_information.kill_limit);
	return 1;
}

static int Lua_Game_Get_Monsters_Replenish(lua_State* L)
{
	lua_pushboolean(L, dynamic_world->game_information.game_options & _monsters_replenish);
	return 1;
}

static int Lua_Game_Get_Proper_Item_Accounting(lua_State* L)
{
	lua_pushboolean(L, L_Get_Proper_Item_Accounting(L));
	return 1;
}

static int Lua_Game_Get_Nonlocal_Overlays(lua_State* L)
{
	lua_pushboolean(L, L_Get_Nonlocal_Overlays(L));
	return 1;
}

static int Lua_Game_Get_Time_Remaining(lua_State* L)
{
  if(dynamic_world->game_information.game_time_remaining > 999 * 30)
    lua_pushnil(L);
  else
    lua_pushnumber(L, dynamic_world->game_information.game_time_remaining);
  return 1;
}

static int Lua_Game_Get_Ticks(lua_State *L)
{
	lua_pushnumber(L, dynamic_world->tick_count);
	return 1;
}

static int Lua_Game_Get_Type(lua_State *L)
{
	Lua_GameType::Push(L, GET_GAME_TYPE());
	return 1;
}

extern int game_end_condition;
extern int game_scoring_mode;

static int Lua_Game_Get_Scoring_Mode(lua_State *L)
{
	Lua_ScoringMode::Push(L, game_scoring_mode);
	return 1;
}

static int Lua_Game_Get_Version(lua_State *L)
{
	lua_pushstring(L, A1_DATE_VERSION);
	return 1;
}

static int Lua_Game_Set_View_Player(lua_State *L)
{

	int view_player_index;
	if (lua_isnumber(L, 2))
	{
		view_player_index = static_cast<int>(lua_tonumber(L, 2));
		if (view_player_index < 0 || view_player_index >= dynamic_world->player_count)
			return luaL_error(L, "view_player: invalid player index");
	}
	else if (Lua_Player::Is(L, 2))
		view_player_index = Lua_Player::Index(L, 2);
	else
		return luaL_error(L, "view_player: incorrect argument type");
	
	if (view_player_index != current_player_index)
	{
		set_current_player_index(view_player_index);
		update_interface(NONE);
		dirty_terminal_view(local_player_index);
	}

	return 0;
		
}

static int Lua_Game_Set_Proper_Item_Accounting(lua_State* L)
{
	if (!lua_isboolean(L, 2))
		luaL_error(L, "proper_item_accounting: incorrect argument type");
	L_Set_Proper_Item_Accounting(L, lua_toboolean(L, 2));
	return 0;
}

static int Lua_Game_Set_Nonlocal_Overlays(lua_State* L)
{
	if (!lua_isboolean(L, 2))
		luaL_error(L, "nonlocal_overlays: incorrect argument type");
	L_Set_Nonlocal_Overlays(L, lua_toboolean(L, 2));
	return 0;
}

static int Lua_Game_Set_Scoring_Mode(lua_State *L)
{
  int mode = Lua_ScoringMode::ToIndex(L, 2);
  game_scoring_mode = mode;
  // TODO: set network stats to dirty
  return 0;
}

static int Lua_Game_Set_Dead_Players_Drop_Items(lua_State* L)
{
	if (!lua_isboolean(L, 2))
		luaL_error(L, "dead_players_drop_items: incorrect argument type");

	bool dpdi = lua_toboolean(L, 2);
	if (dpdi)
	{
		GET_GAME_OPTIONS() &= ~_burn_items_on_death;
	} 
	else
	{
		GET_GAME_OPTIONS() |= _burn_items_on_death;
	}

	return 0;
}

static int Lua_Game_Set_Monsters_Replenish(lua_State* L)
{
	if (!lua_isboolean(L, 2))
		luaL_error(L, "monsters_replenish: incorrect argument type");

	bool replenish = lua_toboolean(L, 2);
	if (replenish)
	{
		dynamic_world->game_information.game_options |= _monsters_replenish;
	} 
	else
	{
		dynamic_world->game_information.game_options &= ~_monsters_replenish;
	}
	return 0;
}

static int Lua_Game_Set_Over(lua_State *L)
{
  if(lua_isnil(L, 2)) game_end_condition = _game_normal_end_condition;
  else game_end_condition = lua_toboolean(L, 2) ? _game_end_now_condition : _game_no_end_condition;
  return 0;
}

extern GM_Random lua_random_generator;
extern GM_Random lua_random_local_generator;

int Lua_Game_Better_Random(lua_State *L)
{
	if (lua_isnumber(L, 1))
	{
		lua_pushnumber(L, lua_random_generator.KISS() % static_cast<uint32>(lua_tonumber(L, 1)));
	}
	else
	{
		lua_pushnumber(L, lua_random_generator.KISS());
	}
	return 1;
}

int Lua_Game_Deserialize(lua_State* L)
{
        if (!lua_isstring(L, 1))
        {
                lua_pushstring(L, "Game.deserialize: incorrect argument type");
                lua_error(L);
        }
    
        size_t len;
        auto s = lua_tolstring(L, 1, &len);

        io::stream_buffer<io::array_source> sb(s, len);

        if (lua_restore(L, &sb))
        {
                return 1;
        }
        else
        {
                return 0;
        }
}

int Lua_Game_Global_Random(lua_State *L)
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

int Lua_Game_Local_Random(lua_State *L)
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

int Lua_Game_Random_Local(lua_State *L)
{
	if (lua_isnumber(L, 1))
	{
		lua_pushnumber(L, lua_random_local_generator.KISS() % static_cast<uint32>(lua_tonumber(L, 1)));
	}
	else
	{
		lua_pushnumber(L, lua_random_local_generator.KISS());
	}
	return 1;
}

int Lua_Game_Save(lua_State *L)
{
	if (!game_is_networked)
		save_game();
	
	return 0;
}

int Lua_Game_Serialize(lua_State* L)
{
        std::stringbuf sb;
        if (lua_save(L, &sb))
        {
                lua_pushlstring(L, sb.str().data(), sb.str().size());
                return 1;
        }
        else
        {
                return 0;
        }
}

extern int L_Restore_Passed(lua_State *);
extern int L_Restore_Saved(lua_State *);

const luaL_Reg Lua_Game_Get[] = {
	{"dead_players_drop_items", Lua_Game_Get_Dead_Players_Drop_Items},
	{"deserialize", L_TableFunction<Lua_Game_Deserialize>},
	{"difficulty", Lua_Game_Get_Difficulty},
	{"global_random", L_TableFunction<Lua_Game_Global_Random>},
	{"kill_limit", Lua_Game_Get_Kill_Limit},
	{"time_remaining", Lua_Game_Get_Time_Remaining},
	{"local_random", L_TableFunction<Lua_Game_Local_Random>},
	{"monsters_replenish", Lua_Game_Get_Monsters_Replenish},
	{"proper_item_accounting", Lua_Game_Get_Proper_Item_Accounting},
	{"nonlocal_overlays", Lua_Game_Get_Nonlocal_Overlays},
	{"random", L_TableFunction<Lua_Game_Better_Random>},
	{"random_local", L_TableFunction<Lua_Game_Random_Local>},
	{"restore_passed", L_TableFunction<L_Restore_Passed>},
	{"restore_saved", L_TableFunction<L_Restore_Saved>},
	{"ticks", Lua_Game_Get_Ticks},
	{"type", Lua_Game_Get_Type},
	{"save", L_TableFunction<Lua_Game_Save>},
	{"serialize", L_TableFunction<Lua_Game_Serialize>},
	{"scoring_mode", Lua_Game_Get_Scoring_Mode},
	{"version", Lua_Game_Get_Version},
	{0, 0}
};

const luaL_Reg Lua_Game_Set[] = {
	{"dead_players_drop_items", Lua_Game_Set_Dead_Players_Drop_Items},
	{"monsters_replenish", Lua_Game_Set_Monsters_Replenish},
	{"proper_item_accounting", Lua_Game_Set_Proper_Item_Accounting},
	{"nonlocal_overlays", Lua_Game_Set_Nonlocal_Overlays},
	{"scoring_mode", Lua_Game_Set_Scoring_Mode},
	{"over", Lua_Game_Set_Over},
	{0, 0}
};

char Lua_Music_Name[] = "Music";
typedef L_Class<Lua_Music_Name> Lua_Music;

int Lua_Music_Clear(lua_State *L)
{
	Music::instance()->ClearLevelMusic();
	return 0;
}

int Lua_Music_Fade(lua_State *L)
{
	int duration;
	if (!lua_isnumber(L, 1))
		duration = 1000;
	else
		duration = static_cast<int>(lua_tonumber(L, 1) * 1000);
	Music::instance()->FadeOut(duration);
	Music::instance()->ClearLevelMusic();
	return 0;
}

int Lua_Music_Play(lua_State *L)
{
	bool restart_music;
	restart_music = !Music::instance()->IsLevelMusicActive() && !Music::instance()->Playing();
	for (int n = 1; n <= lua_gettop(L); n++)
	{
		if (!lua_isstring(L, n))
			return luaL_error(L, "play: invalid file specifier");

		std::string search_path = L_Get_Search_Path(L);

		FileSpecifier file;
		if (search_path.size())
		{
			if (!file.SetNameWithPath(lua_tostring(L, n), search_path))
				Music::instance()->PushBackLevelMusic(file);
		}
		else
		{
			if (file.SetNameWithPath(lua_tostring(L, n)))
				Music::instance()->PushBackLevelMusic(file);
		}
	}

	if (restart_music)
		Music::instance()->PreloadLevelMusic();
	return 0;
}

int Lua_Music_Stop(lua_State *L)
{
	Music::instance()->ClearLevelMusic();
	Music::instance()->StopLevelMusic();

	return 0;
}

int Lua_Music_Valid(lua_State* L) {
	int top = lua_gettop(L);
	for(int n = 1; n <= top; n++) {
		if(!lua_isstring(L, n))
			return luaL_error(L, "valid: invalid file specifier");
		FileSpecifier path;
		if(path.SetNameWithPath(lua_tostring(L, n))) {
			StreamDecoder* stream = StreamDecoder::Get(path);
			if(stream) {
				lua_pushboolean(L, true);
				delete stream;
			} else lua_pushboolean(L, false);
		} else lua_pushboolean(L, false);
	}
	return top;
}

const luaL_Reg Lua_Music_Get[] = {
	{"clear", L_TableFunction<Lua_Music_Clear>},
	{"fade", L_TableFunction<Lua_Music_Fade>},
	{"play", L_TableFunction<Lua_Music_Play>},
	{"stop", L_TableFunction<Lua_Music_Stop>},
	{"valid", L_TableFunction<Lua_Music_Valid>},
	{0, 0}
};

static void Lua_Player_load_compatibility(lua_State *L);

int Lua_Player_register (lua_State *L)
{
	Lua_Action_Flags::Register(L, Lua_Action_Flags_Get, Lua_Action_Flags_Set);

	Lua_Camera_Path_Angles::Register(L, Lua_Camera_Path_Angles_Get);
	Lua_Camera_Path_Points::Register(L, Lua_Camera_Path_Points_Get);
	Lua_Camera::Register(L, Lua_Camera_Get);
	Lua_Camera::Valid = Lua_Camera_Valid;

	Lua_Cameras::Register(L, Lua_Cameras_Methods);
	Lua_Cameras::Length = Lua_Cameras_Length;
	
	Lua_Crosshairs::Register(L, Lua_Crosshairs_Get, Lua_Crosshairs_Set);

	Lua_HotkeyBindings::Register(L, 0, 0, Lua_HotkeyBindings_Metatable);
	Lua_HotkeyBinding::Register(L, Lua_HotkeyBinding_Get);
	
	Lua_Player_Compass::Register(L, Lua_Player_Compass_Get, Lua_Player_Compass_Set);
	Lua_Player_Items::Register(L, 0, 0, Lua_Player_Items_Metatable);
	Lua_Player_Kills::Register(L, 0, 0, Lua_Player_Kills_Metatable);

	Lua_InternalVelocity::Register(L, Lua_InternalVelocity_Get);
	Lua_ExternalVelocity::Register(L, Lua_ExternalVelocity_Get, Lua_ExternalVelocity_Set);
	Lua_FadeType::Register(L, 0, 0, 0, Lua_FadeType_Mnemonics);
	Lua_FadeType::Valid = Lua_FadeType::ValidRange(NUMBER_OF_FADE_TYPES);
	
	Lua_FadeTypes::Register(L);
	Lua_FadeTypes::Length = Lua_FadeTypes::ConstantLength((int16) NUMBER_OF_FADE_TYPES);
	
	Lua_Texture_Palette_Slot::Register(L, Lua_Texture_Palette_Slot_Get, Lua_Texture_Palette_Slot_Set);
	Lua_Texture_Palette_Slots::Register(L, 0, 0, Lua_Texture_Palette_Slots_Metatable);
	Lua_Texture_Palette::Register(L, Lua_Texture_Palette_Get, Lua_Texture_Palette_Set);

	Lua_WeaponType::Register(L, 0, 0, 0, Lua_WeaponType_Mnemonics);
	Lua_WeaponType::Valid = Lua_WeaponType::ValidRange(MAXIMUM_NUMBER_OF_WEAPONS);

	Lua_WeaponTypes::Register(L);
	Lua_WeaponTypes::Length = Lua_WeaponTypes::ConstantLength((int16) MAXIMUM_NUMBER_OF_WEAPONS);

	Lua_Player_Weapon::Register(L, Lua_Player_Weapon_Get);
	Lua_Player_Weapon::Valid = Lua_Player_Weapon::ValidRange(MAXIMUM_NUMBER_OF_WEAPONS);

	Lua_Player_Weapons::Register(L, 0, 0, Lua_Player_Weapons_Metatable);
	Lua_Player_Weapons::Valid = Lua_Player_Valid;

	Lua_Player_Weapon_Trigger::Register(L, Lua_Player_Weapon_Trigger_Get);
	Lua_Player_Weapon_Trigger::Valid = Lua_Player_Weapon_Trigger::ValidRange((int) _secondary_weapon + 1);

	Lua_OverlayColor::Register(L, 0, 0, 0, Lua_OverlayColor_Mnemonics);
	Lua_OverlayColor::Valid = Lua_OverlayColor::ValidRange(8);

	Lua_Overlays::Register(L, 0, 0, Lua_Overlays_Metatable);
	Lua_Overlays::Valid = Lua_Player_Valid;

	Lua_Overlay::Register(L, Lua_Overlay_Get, Lua_Overlay_Set);
	Lua_Overlay::Valid = Lua_Overlay::ValidRange(MAXIMUM_NUMBER_OF_SCRIPT_HUD_ELEMENTS);

	Lua_PlayerColor::Register(L, 0, 0, 0, Lua_PlayerColor_Mnemonics);
	Lua_PlayerColor::Valid = Lua_PlayerColor::ValidRange(NUMBER_OF_TEAM_COLORS);

	Lua_PlayerColors::Register(L);
	Lua_PlayerColors::Length = Lua_PlayerColors::ConstantLength((int16) NUMBER_OF_TEAM_COLORS);

	Lua_Player::Register(L, Lua_Player_Get, Lua_Player_Set);
	Lua_Player::Valid = Lua_Player_Valid;
	
	Lua_Players::Register(L, Lua_Players_Get);
	Lua_Players::Length = Lua_Players_Length;

	Lua_Game::Register(L, Lua_Game_Get, Lua_Game_Set);

	Lua_GameType::Register(L, 0, 0, 0, Lua_GameType_Mnemonics);
	Lua_GameType::Valid = Lua_GameType::ValidRange(NUMBER_OF_GAME_TYPES);

	Lua_GameTypes::Register(L);
	Lua_GameTypes::Length = Lua_GameTypes::ConstantLength(NUMBER_OF_GAME_TYPES);

	Lua_ScoringMode::Register(L, 0, 0, 0, Lua_ScoringMode_Mnemonics);
	Lua_ScoringMode::Valid = Lua_ScoringMode::ValidRange(NUMBER_OF_GAME_SCORING_MODES);

	Lua_ScoringModes::Register(L);
	Lua_ScoringModes::Length = Lua_ScoringModes::ConstantLength(NUMBER_OF_GAME_SCORING_MODES);

	Lua_DifficultyType::Register(L, 0, 0, 0, Lua_DifficultyType_Mnemonics);
	Lua_DifficultyType::Valid = Lua_DifficultyType::ValidRange(NUMBER_OF_GAME_DIFFICULTY_LEVELS);

	Lua_DifficultyTypes::Register(L);
	Lua_DifficultyTypes::Length = Lua_DifficultyTypes::ConstantLength(NUMBER_OF_GAME_DIFFICULTY_LEVELS);

	Lua_TextureType::Register(L, 0, 0, 0, Lua_TextureType_Mnemonics);
	Lua_TextureType::Valid = Lua_TextureType::ValidRange(NUMBER_OF_LUA_TEXTURE_TYPES);
    
	Lua_TextureTypes::Register(L);
	Lua_TextureTypes::Length = Lua_TextureTypes::ConstantLength(NUMBER_OF_LUA_TEXTURE_TYPES);

	Lua_Music::Register(L, Lua_Music_Get);

	// register one Game userdatum globally
	Lua_Game::Push(L, 0);
	lua_setglobal(L, Lua_Game_Name);

	// register one Music userdatum
	Lua_Music::Push(L, 0);
	lua_setglobal(L, Lua_Music_Name);
	
	Lua_Player_load_compatibility(L);
	
	return 0;
}

static const char *compatibility_script = ""
	"function accelerate_player(player, vertical_velocity, direction, velocity) Players[player]:accelerate(direction, velocity, vertical_velocity) end\n"
	"function activate_camera(player, camera) Cameras[camera]:activate(player) end\n"
	"function activate_terminal(player, text) Players[player]:activate_terminal(text) end\n"
	"function add_item(player, item_type) Players[player].items[item_type] = Players[player].items[item_type] + 1 end\n"
	"function add_path_angle(camera, yaw, pitch, time) Cameras[camera].path_angles:new(yaw, pitch, time) end\n"
	"function add_path_point(camera, polygon, x, y, z, time) Cameras[camera].path_points:new(x, y, z, polygon, time) end\n"
	"function award_kills(player, slain_player, amount) if player == -1 then Players[slain_player].deaths = Players[slain_player].deaths + amount else Players[player].kills[slain_player] = Players[player].kills[slain_player] + amount end end\n"
	"function add_to_player_external_velocity(player, x, y, z) Players[player].external_velocity.i = Players[player].external_velocity.i + x Players[player].external_velocity.j = Players[player].external_velocity.j + y Players[player].external_velocity.k = Players[player].external_velocity.k + z end\n"
	"function award_points(player, amount) Players[player].points = Players[player].points + amount end\n"
	"function better_random() return Game.random() end\n"
	"function clear_camera(camera) Cameras[camera]:clear() end\n"
	"function clear_music() Music.clear() end\n"
	"function count_item(player, item_type) return Players[player].items[item_type] end\n"
	"function create_camera() return Cameras.new().index end\n"
	"function crosshairs_active(player) return Players[player].crosshairs.active end\n"
	"function deactivate_camera(camera) Cameras[camera]:deactivate() end\n"
	"function destroy_ball(player) for i in ItemTypes() do if i.ball then Players[player].items[i] = 0 end end end\n"
	"function fade_music(duration) if duration then Music.fade(duration * 60 / 1000) else Music.fade(60 / 1000) end end\n"
	"function get_game_difficulty() return Game.difficulty.index end\n"
	"function get_game_type() return Game.type.index end\n"
	"function get_kills(player, slain_player) if player == -1 then return Players[slain_player].deaths else return Players[player].kills[slain_player] end end\n"
	"function get_kill_limit() return Game.kill_limit end\n"
	"function get_life(player) return Players[player].energy end\n"
	"function get_motion_sensor_state(player) return Players[player].motion_sensor_active end\n"
	"function get_oxygen(player) return Players[player].oxygen end\n"
	"function get_player_angle(player) return Players[player].yaw, Players[player].pitch end\n"
	"function get_player_color(player) return Players[player].color.index end\n"
	"function get_player_external_velocity(player) return Players[player].external_velocity.i * 1024, Players[player].external_velocity.j * 1024, Players[player].external_velocity.k * 1024 end\n"
	"function get_player_internal_velocity(player) return Players[player].internal_velocity.forward * 65536, Players[player].internal_velocity.perpendicular * 65536 end\n"
	"function get_player_name(player) return Players[player].name end\n"
	"function get_player_polygon(player) return Players[player].polygon.index end\n"
	"function get_player_position(player) return Players[player].x, Players[player].y, Players[player].z end\n"
	"function get_player_powerup_duration(player, powerup) if powerup == _powerup_invisibility then return Players[player].invisibility_duration elseif powerup == _powerup_invincibility then return Players[player].invincibility_duration elseif powerup == _powerup_infravision then return Players[player].infravision_duration elseif powerup == _powerup_extravision then return Players[player].extravision_duration end end\n"
	"function get_player_team(player) return Players[player].team.index end\n"
	"function get_player_weapon(player) if Players[player].weapons.current then return Players[player].weapons.current.index else return nil end end\n"
	"function get_points(player) return Players[player].points end\n"
	"function global_random() return Game.global_random() end\n"
	"function inflict_damage(player, amount, type) if (type) then Players[player]:damage(amount, type) else Players[player]:damage(amount) end end\n"
	"function local_player_index() for p in Players() do if p.local_ then return p.index end end end\n"
	"function local_random() return Game.local_random() end\n"
	"function number_of_players() return # Players end\n"
	"function play_music(...) Music.play(...) end\n"
	"function player_is_dead(player) return Players[player].dead end\n"
	"function player_media(player) if Players[player].head_below_media then return Players[player].polygon.media.index else return nil end end\n"
	"function player_to_monster_index(player) return Players[player].monster.index end\n"
	"function play_sound(player, sound, pitch) Players[player]:play_sound(sound, pitch) end\n"
	"function remove_item(player, item_type) if Players[player].items[item_type] > 0 then Players[player].items[item_type] = Players[player].items[item_type] - 1 end end\n"
	"function screen_fade(player, fade) if fade then Players[player]:fade_screen(fade) else for p in Players() do p:fade_screen(player) end end end\n"
	"function screen_print(player, message) if message then if Players[player] then Players[player]:print(message) end else Players.print(player) end end\n"
	"function select_weapon(player, weapon) Players[player].weapons[weapon]:select() end\n"
	"function set_crosshairs_state(player, state) Players[player].crosshairs.active = state end\n"
	"function set_kills(player, slain_player, amount) if player == -1 then Players[slain_player].deaths = amount else Players[player].kills[slain_player] = amount end end\n"
	"function set_life(player, shield) Players[player].energy = shield end\n"
	"function set_lua_compass_beacon(player, x, y) Players[player].compass.x = x Players[player].compass.y = y end\n"
	"function set_lua_compass_state(player, state) if state > 15 then Players[player].compass.beacon = true state = state % 16 else Players[player].compass.beacon = false end if state > 7 then Players[player].compass.se = true state = state - 8 else Players[player].compass.se = false end if state > 3 then Players[player].compass.sw = true state = state - 4 else Players[player].compass.sw = false end if state > 1 then Players[player].compass.ne = true state = state - 2 else Players[player].compass.ne = false end if state == 1 then Players[player].compass.nw = true else Players[player].compass.nw = false end end\n"
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
	"function stop_music() Music.stop() end\n"
	"function set_zoom_state(player, state) Players[player].zoom_active = state end\n"
	"function teleport_player(player, polygon) Players[player]:teleport(polygon) end\n"
	"function teleport_player_to_level(player, level) Players[player]:teleport_to_level(level) end\n"
	"function use_lua_compass(player, state) if state ~= nil then Players[player].compass.lua = state else for p in Players() do p.compass.lua = player end end end\n"
	"function zoom_active(player) return Players[player].zoom_active end\n"
	;

static void Lua_Player_load_compatibility(lua_State *L)
{
	luaL_loadbuffer(L, compatibility_script, strlen(compatibility_script), "player_compatibility");
	lua_pcall(L, 0, 0, 0);
}

#endif
