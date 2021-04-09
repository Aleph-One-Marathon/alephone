/*
LUA_MAP.CPP

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

	Implements Lua map classes
*/

#include <unordered_map>

#include "interface.h" // get_game_state
#include "network.h"   // game_info
#include "lua_map.h"
#include "lua_monsters.h"
#include "lua_objects.h"
#include "lua_player.h"
#include "lua_templates.h"
#include "lightsource.h"
#include "map.h"
#include "media.h"
#include "platforms.h"
#include "player.h"
#include "projectile_definitions.h"
#include "projectiles.h"
#include "OGL_Setup.h"
#include "SoundManager.h"

#include "collection_definition.h"


#include <boost/bind.hpp>

#ifdef HAVE_LUA

char Lua_AmbientSound_Name[] = "ambient_sound";
char Lua_AmbientSounds_Name[] = "AmbientSounds";

extern collection_definition *get_collection_definition(short);

char Lua_Collection_Name[] = "collection";

static int Lua_Collection_Get_Bitmap_Count(lua_State *L)
{
	collection_definition *collection = get_collection_definition(Lua_Collection::Index(L, 1));
	lua_pushnumber(L, collection->bitmap_count);
	return 1;
}

const luaL_Reg Lua_Collection_Get[] = {
	{"bitmap_count", Lua_Collection_Get_Bitmap_Count},
	{0, 0},
};

char Lua_Collections_Name[] = "Collections";

char Lua_ControlPanelClass_Name[] = "control_panel_class";
char Lua_ControlPanelClasses_Name[] = "ControlPanelClasses";

char Lua_ControlPanelType_Name[] = "control_panel_type";

extern short get_panel_class(short panel_type);

// no devices.h, so copy this from devices.cpp:

enum // control panel sounds
{
	_activating_sound,
	_deactivating_sound,
	_unusuable_sound,
	
	NUMBER_OF_CONTROL_PANEL_SOUNDS
};

struct control_panel_definition
{
	int16 _class;
	uint16 flags;
	
	int16 collection;
	int16 active_shape, inactive_shape;

	int16 sounds[NUMBER_OF_CONTROL_PANEL_SOUNDS];
	_fixed sound_frequency;
	
	int16 item;
};

extern control_panel_definition* get_control_panel_definition(int16);

static int Lua_ControlPanelType_Get_Active_Texture_Index(lua_State *L)
{
	control_panel_definition *definition = get_control_panel_definition(Lua_ControlPanelType::Index(L, 1));
	lua_pushnumber(L, GET_DESCRIPTOR_SHAPE(definition->active_shape));
	return 1;
}

static int Lua_ControlPanelType_Get_Inactive_Texture_Index(lua_State *L)
{
	control_panel_definition *definition = get_control_panel_definition(Lua_ControlPanelType::Index(L, 1));
	lua_pushnumber(L, GET_DESCRIPTOR_SHAPE(definition->inactive_shape));
	return 1;
}

static int Lua_ControlPanelType_Get_Class(lua_State *L)
{
	Lua_ControlPanelClass::Push(L, get_panel_class(Lua_ControlPanelType::Index(L, 1)));
	return 1;
}

static int Lua_ControlPanelType_Get_Collection(lua_State *L)
{
	control_panel_definition *definition = get_control_panel_definition(Lua_ControlPanelType::Index(L, 1));
	Lua_Collection::Push(L, definition->collection);
	return 1;
}

const luaL_Reg Lua_ControlPanelType_Get[] = {
	{"active_texture_index", Lua_ControlPanelType_Get_Active_Texture_Index},
	{"class", Lua_ControlPanelType_Get_Class},
	{"collection", Lua_ControlPanelType_Get_Collection},
	{"inactive_texture_index", Lua_ControlPanelType_Get_Inactive_Texture_Index},
	{0, 0}
};

char Lua_ControlPanelTypes_Name[] = "ControlPanelTypes";

char Lua_DamageType_Name[] = "damage_type";
char Lua_DamageTypes_Name[] = "DamageTypes";

char Lua_Endpoint_Name[] = "endpoint";
typedef L_Class<Lua_Endpoint_Name> Lua_Endpoint;

static int Lua_Endpoint_Get_X(lua_State *L)
{
	endpoint_data *endpoint = get_endpoint_data(Lua_Endpoint::Index(L, 1));
	lua_pushnumber(L, (double) endpoint->vertex.x / WORLD_ONE);
	return 1;
}

static int Lua_Endpoint_Get_Y(lua_State *L)
{
	endpoint_data *endpoint = get_endpoint_data(Lua_Endpoint::Index(L, 1));
	lua_pushnumber(L, (double) endpoint->vertex.y / WORLD_ONE);
	return 1;
}

static bool Lua_Endpoint_Valid(int16 index)
{
	return index >= 0 && index < EndpointList.size();
}

const luaL_Reg Lua_Endpoint_Get[] = {
	{"x", Lua_Endpoint_Get_X},
	{"y", Lua_Endpoint_Get_Y},
	{0, 0}
};

char Lua_Endpoints_Name[] = "Endpoints";
typedef L_Container<Lua_Endpoints_Name, Lua_Endpoint> Lua_Endpoints;
int16 Lua_Endpoints_Length() { return EndpointList.size(); }

char Lua_Line_Endpoints_Name[] = "line_endpoints";
typedef L_Class<Lua_Line_Endpoints_Name> Lua_Line_Endpoints;

static int Lua_Line_Endpoints_Get(lua_State *L)
{
	if (lua_isnumber(L, 2))
	{
		short line_index = Lua_Line::Index(L, 1);
		line_data *line = get_line_data(line_index);
		int endpoint_index = static_cast<int>(lua_tonumber(L, 2));
		if (endpoint_index == 0 || endpoint_index == 1)
		{
			Lua_Endpoint::Push(L, line->endpoint_indexes[endpoint_index]);
		}
		else
		{
			lua_pushnil(L);
		}
	}

	return 1;
}

static int Lua_Line_Endpoints_Length(lua_State *L)
{
	lua_pushnumber(L, 2);
	return 1;
}

const luaL_Reg Lua_Line_Endpoints_Metatable[] = {
	{"__index", Lua_Line_Endpoints_Get},
	{"__len", Lua_Line_Endpoints_Length},
	{0, 0}
};

char Lua_Line_Name[] = "line";

static int Lua_Line_Get_Clockwise_Polygon(lua_State *L)
{
	Lua_Polygon::Push(L, get_line_data(Lua_Line::Index(L, 1))->clockwise_polygon_owner);
	return 1;
}

static int Lua_Line_Get_Counterclockwise_Polygon(lua_State *L)
{
	Lua_Polygon::Push(L, get_line_data(Lua_Line::Index(L, 1))->counterclockwise_polygon_owner);
	return 1;
}

static int Lua_Line_Get_Clockwise_Side(lua_State *L)
{
	Lua_Side::Push(L, get_line_data(Lua_Line::Index(L, 1))->clockwise_polygon_side_index);
	return 1;
}

static int Lua_Line_Get_Counterclockwise_Side(lua_State *L)
{
	Lua_Side::Push(L, get_line_data(Lua_Line::Index(L, 1))->counterclockwise_polygon_side_index);
	return 1;
}

static int Lua_Line_Get_Decorative(lua_State* L)
{
	lua_pushboolean(L, get_line_data(Lua_Line::Index(L, 1))->is_decorative());
	return 1;
}

static int Lua_Line_Get_Endpoints(lua_State *L)
{
	Lua_Line_Endpoints::Push(L, Lua_Line::Index(L, 1));
	return 1;
}

static int Lua_Line_Get_Has_Transparent_Side(lua_State *L)
{
	lua_pushboolean(L, LINE_HAS_TRANSPARENT_SIDE(get_line_data(Lua_Line::Index(L, 1))));
	return 1;
}

static int Lua_Line_Get_Highest_Adjacent_Floor(lua_State *L)
{
	lua_pushnumber(L, (double) get_line_data(Lua_Line::Index(L, 1))->highest_adjacent_floor / WORLD_ONE);
	return 1;
}

static int Lua_Line_Get_Length(lua_State *L)
{
	lua_pushnumber(L, (double) get_line_data(Lua_Line::Index(L, 1))->length / WORLD_ONE);
	return 1;
}

static int Lua_Line_Get_Lowest_Adjacent_Ceiling(lua_State *L)
{
	lua_pushnumber(L, (double) get_line_data(Lua_Line::Index(L, 1))->lowest_adjacent_ceiling / WORLD_ONE);
	return 1;
}

static int Lua_Line_Get_Solid(lua_State *L)
{
	line_data *line = get_line_data(Lua_Line::Index(L, 1));
	lua_pushboolean(L, LINE_IS_SOLID(line));
	return 1;
}

static int Lua_Line_Get_Visible_On_Automap(lua_State *L)
{
	lua_pushboolean(L, LINE_IS_IN_AUTOMAP(Lua_Line::Index(L, 1)));
	return 1;
}

static int Lua_Line_Set_Decorative(lua_State* L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, ("decorative: incorrect argument type"));

	get_line_data(Lua_Line::Index(L, 1))->set_decorative(lua_toboolean(L, 2));
	return 0;
}

static int Lua_Line_Set_Visible_On_Automap(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, ("visible_on_automap: incorrect argument type"));
	
	if (lua_toboolean(L, 2))
		ADD_LINE_TO_AUTOMAP(Lua_Line::Index(L, 1));
	else
		CLEAR_LINE_FROM_AUTOMAP(Lua_Line::Index(L, 1));
	return 0;
}


const luaL_Reg Lua_Line_Get[] = {
	{"cw_polygon", Lua_Line_Get_Clockwise_Polygon},
	{"ccw_polygon", Lua_Line_Get_Counterclockwise_Polygon},
	{"cw_side", Lua_Line_Get_Clockwise_Side},
	{"ccw_side", Lua_Line_Get_Counterclockwise_Side},
	{"clockwise_polygon", Lua_Line_Get_Clockwise_Polygon},
	{"clockwise_side", Lua_Line_Get_Clockwise_Side},
	{"counterclockwise_polygon", Lua_Line_Get_Counterclockwise_Polygon},
	{"counterclockwise_side", Lua_Line_Get_Counterclockwise_Side},
	{"decorative", Lua_Line_Get_Decorative},
	{"endpoints", Lua_Line_Get_Endpoints},
	{"has_transparent_side", Lua_Line_Get_Has_Transparent_Side},
	{"highest_adjacent_floor", Lua_Line_Get_Highest_Adjacent_Floor},
	{"length", Lua_Line_Get_Length},
	{"lowest_adjacent_ceiling", Lua_Line_Get_Lowest_Adjacent_Ceiling},
	{"solid", Lua_Line_Get_Solid},
	{"visible_on_automap", Lua_Line_Get_Visible_On_Automap},
	{0, 0}
};

const luaL_Reg Lua_Line_Set[] = {
	{"decorative", Lua_Line_Set_Decorative},
	{"visible_on_automap", Lua_Line_Set_Visible_On_Automap},
	{0, 0}
};

static bool Lua_Line_Valid(int16 index)
{
	return index >= 0 && index < LineList.size();
}

char Lua_Lines_Name[] = "Lines";
static int16 Lua_Lines_Length() { return LineList.size(); }

char Lua_PlatformType_Name[] = "platform_type";
char Lua_PlatformTypes_Name[] = "PlatformTypes";

char Lua_Platform_Name[] = "platform";
bool Lua_Platform_Valid(int16 index)
{
	return index >= 0 && index < dynamic_world->platform_count;
}

template<int flag_bit> int 
Lua_Platform_Get_Dynamic_Flag(lua_State* L)
{
	platform_data* platform = get_platform_data(Lua_Platform::Index(L, 1));
	lua_pushboolean(L, platform->dynamic_flags & (1 << flag_bit));
	return 1;
}

template<int flag_bit> int
Lua_Platform_Set_Dynamic_Flag(lua_State* L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "platform: incorrect argument type");

	platform_data* platform = get_platform_data(Lua_Platform::Index(L, 1));
	bool flag = lua_toboolean(L, 2);
	if (flag)
		platform->dynamic_flags |= (1 << flag_bit);
	else
		platform->dynamic_flags &= ~(1 << flag_bit);

	return 0;
}


template<int flag_bit> int 
Lua_Platform_Get_Static_Flag(lua_State* L)
{
	platform_data* platform = get_platform_data(Lua_Platform::Index(L, 1));
	lua_pushboolean(L, platform->static_flags & (1 << flag_bit));
	return 1;
}

template<int flag_bit> int
Lua_Platform_Set_Static_Flag(lua_State* L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "platform: incorrect argument type");

	platform_data* platform = get_platform_data(Lua_Platform::Index(L, 1));
	bool flag = lua_toboolean(L, 2);
	if (flag)
		platform->static_flags |= (1 << flag_bit);
	else
		platform->static_flags &= ~(1 << flag_bit);

	return 0;
}

static int Lua_Platform_Get_Active(lua_State *L)
{
	platform_data *platform = get_platform_data(Lua_Platform::Index(L, 1));
	lua_pushboolean(L, PLATFORM_IS_ACTIVE(platform));
	return 1;
}

static int Lua_Platform_Get_Ceiling_Height(lua_State *L)
{
	platform_data *platform = get_platform_data(Lua_Platform::Index(L, 1));
	lua_pushnumber(L, (double) platform->ceiling_height / WORLD_ONE);
	return 1;
}

static int Lua_Platform_Get_Contracting(lua_State *L)
{
	platform_data *platform = get_platform_data(Lua_Platform::Index(L, 1));
	lua_pushboolean(L, !PLATFORM_IS_EXTENDING(platform));
	return 1;
}

static int Lua_Platform_Get_Extending(lua_State *L)
{
	platform_data *platform = get_platform_data(Lua_Platform::Index(L, 1));
	lua_pushboolean(L, PLATFORM_IS_EXTENDING(platform));
	return 1;
}

static int Lua_Platform_Get_Floor_Height(lua_State *L)
{
	platform_data *platform = get_platform_data(Lua_Platform::Index(L, 1));
	lua_pushnumber(L, (double) platform->floor_height / WORLD_ONE);
	return 1;
}

static int Lua_Platform_Get_Polygon(lua_State *L)
{
	Lua_Polygon::Push(L, get_platform_data(Lua_Platform::Index(L, 1))->polygon_index);
	return 1;
}

static int Lua_Platform_Get_Speed(lua_State *L)
{
	platform_data *platform = get_platform_data(Lua_Platform::Index(L, 1));
	lua_pushnumber(L, (double) platform->speed / WORLD_ONE);
	return 1;
}

static int Lua_Platform_Get_Type(lua_State* L)
{
	platform_data* platform = get_platform_data(Lua_Platform::Index(L, 1));
	Lua_PlatformType::Push(L, platform->type);
	return 1;
}	

extern bool set_platform_state(short, bool, short);

static int Lua_Platform_Set_Active(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "active: incorrect argument type");

	short platform_index = Lua_Platform::Index(L, 1);
	set_platform_state(platform_index, lua_toboolean(L, 2), NONE);
	return 0;
}

static int Lua_Platform_Set_Ceiling_Height(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "ceiling_height: incorrect argument type");
	
	short platform_index = Lua_Platform::Index(L, 1);
	platform_data *platform = get_platform_data(platform_index);

	platform->ceiling_height = static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE);
	adjust_platform_endpoint_and_line_heights(platform_index);
	adjust_platform_for_media(platform_index, false);
	
	return 0;
}	

static int Lua_Platform_Set_Contracting(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "contracting: incorrect argument type");

	platform_data *platform = get_platform_data(Lua_Platform::Index(L, 1));
	bool contracting = lua_toboolean(L, 2);
	if (contracting)
		SET_PLATFORM_IS_CONTRACTING(platform);
	else
		SET_PLATFORM_IS_EXTENDING(platform);
	return 0;
}

static int Lua_Platform_Set_Extending(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "extending: incorrect argument type");

	platform_data *platform = get_platform_data(Lua_Platform::Index(L, 1));
	bool extending = lua_toboolean(L, 2);
	if (extending)
		SET_PLATFORM_IS_EXTENDING(platform);
	else
		SET_PLATFORM_IS_CONTRACTING(platform);
	return 0;
}

static int Lua_Platform_Set_Floor_Height(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "ceiling_height: incorrect argument type");
	
	short platform_index = Lua_Platform::Index(L, 1);
	platform_data *platform = get_platform_data(platform_index);

	platform->floor_height = static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE);
	adjust_platform_endpoint_and_line_heights(platform_index);
	adjust_platform_for_media(platform_index, false);
	
	return 0;
}	

static int Lua_Platform_Set_Speed(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "speed: incorrect argument type");
	
	platform_data *platform = get_platform_data(Lua_Platform::Index(L, 1));
	platform->speed = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	return 0;
}

static int Lua_Platform_Set_Type(lua_State* L)
{
	platform_data* platform = get_platform_data(Lua_Platform::Index(L, 1));
	platform->type = Lua_PlatformType::ToIndex(L, 2);
	return 0;
}
	

const luaL_Reg Lua_Platform_Get[] = {
	{"activates_adjacent_platforms_at_each_level", Lua_Platform_Get_Static_Flag<_platform_activates_adjacent_platforms_at_each_level>},
	{"activates_adjacent_platforms_when_activating", Lua_Platform_Get_Static_Flag<_platform_activates_adjacent_platforms_when_activating>},
	{"activates_adjacent_platforms_when_deactivating", Lua_Platform_Get_Static_Flag<_platform_activates_adjacent_platforms_when_deactivating>},
	{"activates_light", Lua_Platform_Get_Static_Flag<_platform_activates_light>},
	{"activates_only_once", Lua_Platform_Get_Static_Flag<_platform_activates_only_once>},
	{"active", Lua_Platform_Get_Active},
	{"cannot_be_externally_deactivated", Lua_Platform_Get_Static_Flag<_platform_cannot_be_externally_deactivated>},
	{"causes_damage", Lua_Platform_Get_Static_Flag<_platform_causes_damage>},
	{"ceiling_height", Lua_Platform_Get_Ceiling_Height},
	{"comes_from_ceiling", Lua_Platform_Get_Static_Flag<_platform_comes_from_ceiling>},
	{"comes_from_floor", Lua_Platform_Get_Static_Flag<_platform_comes_from_floor>},
	{"contracting", Lua_Platform_Get_Contracting},
	{"contracts_slower", Lua_Platform_Get_Static_Flag<_platform_contracts_slower>},
	{"deactivates_adjacent_platforms_when_activating", Lua_Platform_Get_Static_Flag<_platform_deactivates_adjacent_platforms_when_activating>},
	{"deactivates_adjacent_platforms_when_deactivating", Lua_Platform_Get_Static_Flag<_platform_deactivates_adjacent_platforms_when_deactivating>},
	{"deactivates_at_each_level", Lua_Platform_Get_Static_Flag<_platform_deactivates_at_each_level>},
	{"deactivates_at_initial_level", Lua_Platform_Get_Static_Flag<_platform_deactivates_at_initial_level>},
	{"deactivates_light", Lua_Platform_Get_Static_Flag<_platform_deactivates_light>},
	{"delays_before_activation", Lua_Platform_Get_Static_Flag<_platform_delays_before_activation>},
	{"does_not_activate_parent", Lua_Platform_Get_Static_Flag<_platform_does_not_activate_parent>},
	{"door", Lua_Platform_Get_Static_Flag<_platform_is_door>},
	{"extending", Lua_Platform_Get_Extending},
	{"extends_floor_to_ceiling", Lua_Platform_Get_Static_Flag<_platform_extends_floor_to_ceiling>},
	{"floor_height", Lua_Platform_Get_Floor_Height},
	{"has_been_activated", Lua_Platform_Get_Dynamic_Flag<_platform_has_been_activated>},
	{"initially_active", Lua_Platform_Get_Static_Flag<_platform_is_initially_active>},
	{"initially_extended", Lua_Platform_Get_Static_Flag<_platform_is_initially_extended>},
	{"locked", Lua_Platform_Get_Static_Flag<_platform_is_locked>},
	{"monster_controllable", Lua_Platform_Get_Static_Flag<_platform_is_monster_controllable>},
	{"reverses_direction_when_obstructed", Lua_Platform_Get_Static_Flag<_platform_reverses_direction_when_obstructed>},
	{"player_controllable", Lua_Platform_Get_Static_Flag<_platform_is_player_controllable>},
	{"polygon", Lua_Platform_Get_Polygon},
	{"secret", Lua_Platform_Get_Static_Flag<_platform_is_secret>},
	{"speed", Lua_Platform_Get_Speed},
	{"type", Lua_Platform_Get_Type},
	{"uses_native_polygon_heights", Lua_Platform_Get_Static_Flag<_platform_uses_native_polygon_heights>},
	{0, 0}
};

const luaL_Reg Lua_Platform_Set[] = {
	{"activates_adjacent_platforms_at_each_level", Lua_Platform_Set_Static_Flag<_platform_activates_adjacent_platforms_at_each_level>},
	{"activates_adjacent_platforms_when_activating", Lua_Platform_Set_Static_Flag<_platform_activates_adjacent_platforms_when_activating>},
	{"activates_adjacent_platforms_when_deactivating", Lua_Platform_Set_Static_Flag<_platform_activates_adjacent_platforms_when_deactivating>},
	{"activates_light", Lua_Platform_Set_Static_Flag<_platform_activates_light>},
	{"activates_only_once", Lua_Platform_Set_Static_Flag<_platform_activates_only_once>},
	{"active", Lua_Platform_Set_Active},
	{"cannot_be_externally_deactivated", Lua_Platform_Set_Static_Flag<_platform_cannot_be_externally_deactivated>},
	{"causes_damage", Lua_Platform_Set_Static_Flag<_platform_causes_damage>},
	{"ceiling_height", Lua_Platform_Set_Ceiling_Height},
	{"contracting", Lua_Platform_Set_Contracting},
	{"contracts_slower", Lua_Platform_Set_Static_Flag<_platform_contracts_slower>},
	{"deactivates_adjacent_platforms_when_activating", Lua_Platform_Set_Static_Flag<_platform_deactivates_adjacent_platforms_when_activating>},
	{"deactivates_adjacent_platforms_when_deactivating", Lua_Platform_Set_Static_Flag<_platform_deactivates_adjacent_platforms_when_deactivating>},
	{"deactivates_at_each_level", Lua_Platform_Set_Static_Flag<_platform_deactivates_at_each_level>},
	{"deactivates_at_initial_level", Lua_Platform_Set_Static_Flag<_platform_deactivates_at_initial_level>},
	{"deactivates_light", Lua_Platform_Set_Static_Flag<_platform_deactivates_light>},
	{"delays_before_activation", Lua_Platform_Set_Static_Flag<_platform_delays_before_activation>},
	{"does_not_activate_parent", Lua_Platform_Set_Static_Flag<_platform_does_not_activate_parent>},
	{"door", Lua_Platform_Set_Static_Flag<_platform_is_door>},
	{"extending", Lua_Platform_Set_Extending},
	{"floor_height", Lua_Platform_Set_Floor_Height},
	{"has_been_activated", Lua_Platform_Get_Dynamic_Flag<_platform_has_been_activated>},
	{"locked", Lua_Platform_Set_Static_Flag<_platform_is_locked>},
	{"monster_controllable", Lua_Platform_Set_Static_Flag<_platform_is_monster_controllable>},
	{"player_controllable", Lua_Platform_Set_Static_Flag<_platform_is_player_controllable>},
	{"reverses_direction_when_obstructed", Lua_Platform_Set_Static_Flag<_platform_reverses_direction_when_obstructed>},
	{"secret", Lua_Platform_Set_Static_Flag<_platform_is_secret>},
	{"speed", Lua_Platform_Set_Speed},
	{"type", Lua_Platform_Set_Type},
	{0, 0}
};

char Lua_Platforms_Name[] = "Platforms";
int16 Lua_Platforms_Length() {
	return dynamic_world->platform_count;
}

char Lua_Polygon_Floor_Name[] = "polygon_floor";

static int Lua_Polygon_Floor_Get_Collection(lua_State *L)
{
	Lua_Collection::Push(L, GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(get_polygon_data(Lua_Polygon_Floor::Index(L, 1))->floor_texture)));
	return 1;
}

static int Lua_Polygon_Floor_Get_Height(lua_State *L)
{
	lua_pushnumber(L, (double) (get_polygon_data(Lua_Polygon_Floor::Index(L, 1))->floor_height) / WORLD_ONE);
	return 1;
}

static int Lua_Polygon_Floor_Get_Light(lua_State *L)
{
	Lua_Light::Push(L, get_polygon_data(Lua_Polygon_Floor::Index(L, 1))->floor_lightsource_index);
	return 1;
}

static int Lua_Polygon_Floor_Get_Texture_Index(lua_State *L)
{
	lua_pushnumber(L, GET_DESCRIPTOR_SHAPE(get_polygon_data(Lua_Polygon_Floor::Index(L, 1))->floor_texture));
	return 1;
}

static int Lua_Polygon_Floor_Get_Texture_X(lua_State *L)
{
	lua_pushnumber(L, (double) (get_polygon_data(Lua_Polygon_Floor::Index(L, 1))->floor_origin.x) / WORLD_ONE);
	return 1;
}

static int Lua_Polygon_Floor_Get_Texture_Y(lua_State *L)
{
	lua_pushnumber(L, (double) (get_polygon_data(Lua_Polygon_Floor::Index(L, 1))->floor_origin.y) / WORLD_ONE);
	return 1;
}

static int Lua_Polygon_Floor_Get_Transfer_Mode(lua_State *L)
{
	Lua_TransferMode::Push(L, get_polygon_data(Lua_Polygon_Floor::Index(L, 1))->floor_transfer_mode);
	return 1;
}

static int Lua_Polygon_Floor_Set_Collection(lua_State *L)
{
	short polygon_index = Lua_Polygon_Floor::Index(L, 1);
	short collection_index = Lua_Collection::ToIndex(L, 2);

	polygon_data *polygon = get_polygon_data(polygon_index);
	polygon->floor_texture = BUILD_DESCRIPTOR(collection_index, GET_DESCRIPTOR_SHAPE(polygon->floor_texture));
	return 0;
}

static int Lua_Polygon_Floor_Set_Height(lua_State *L)
{
	if (!lua_isnumber(L, 2))
	{
		luaL_error(L, "height: incorrect argument type");
	}

	struct polygon_data *polygon = get_polygon_data(Lua_Polygon_Floor::Index(L, 1));
	polygon->floor_height = static_cast<world_distance>(lua_tonumber(L,2)*WORLD_ONE);
	for (short i = 0; i < polygon->vertex_count; ++i)
	{
		recalculate_redundant_endpoint_data(polygon->endpoint_indexes[i]);
		recalculate_redundant_line_data(polygon->line_indexes[i]);
	}
	return 0;
}

static int Lua_Polygon_Floor_Set_Light(lua_State *L)
{
	short light_index;
	if (lua_isnumber(L, 2))
	{
		light_index = static_cast<short>(lua_tonumber(L, 2));
		if (light_index < 0 || light_index >= MAXIMUM_LIGHTS_PER_MAP)
			return luaL_error(L, "light: invalid light index");
	}
	else
	{
		light_index = Lua_Light::Index(L, 2);
	}
	
	get_polygon_data(Lua_Polygon_Floor::Index(L, 1))->floor_lightsource_index = light_index;
	return 0;
}

static int Lua_Polygon_Floor_Set_Texture_Index(lua_State *L)
{
	polygon_data *polygon = get_polygon_data(Lua_Polygon_Floor::Index(L, 1));
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "texture_index: incorrect argument type");

	short shape_index = static_cast<short>(lua_tonumber(L, 2));
	if (shape_index < 0 || shape_index >= MAXIMUM_SHAPES_PER_COLLECTION)
		return luaL_error(L, "texture_index: invalid texture index");
	
	polygon->floor_texture = BUILD_DESCRIPTOR(GET_DESCRIPTOR_COLLECTION(polygon->floor_texture), shape_index);
	return 0;
}

static int Lua_Polygon_Floor_Set_Texture_X(lua_State *L)
{
	polygon_data *polygon = get_polygon_data(Lua_Polygon_Floor::Index(L, 1));
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "texture_x: incorrect argument type");

	polygon->floor_origin.x = static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE);
	return 0;
}

static int Lua_Polygon_Floor_Set_Texture_Y(lua_State *L)
{
	polygon_data *polygon = get_polygon_data(Lua_Polygon_Floor::Index(L, 1));
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "texture_y: incorrect argument type");

	polygon->floor_origin.y = static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE);
	return 0;
}

static int Lua_Polygon_Floor_Set_Transfer_Mode(lua_State *L)
{
	polygon_data *polygon = get_polygon_data(Lua_Polygon_Floor::Index(L, 1));
	polygon->floor_transfer_mode = Lua_TransferMode::ToIndex(L, 2);
	return 0;
}

const luaL_Reg Lua_Polygon_Floor_Get[] = {
	{"collection", Lua_Polygon_Floor_Get_Collection},
	{"height", Lua_Polygon_Floor_Get_Height},
	{"light", Lua_Polygon_Floor_Get_Light},
	{"texture_index", Lua_Polygon_Floor_Get_Texture_Index},
	{"texture_x", Lua_Polygon_Floor_Get_Texture_X},
	{"texture_y", Lua_Polygon_Floor_Get_Texture_Y},
	{"transfer_mode", Lua_Polygon_Floor_Get_Transfer_Mode},
	{"z", Lua_Polygon_Floor_Get_Height},
	{0, 0}
};

const luaL_Reg Lua_Polygon_Floor_Set[] = {
	{"collection", Lua_Polygon_Floor_Set_Collection},
	{"height", Lua_Polygon_Floor_Set_Height},
	{"light", Lua_Polygon_Floor_Set_Light},
	{"texture_index", Lua_Polygon_Floor_Set_Texture_Index},
	{"texture_x", Lua_Polygon_Floor_Set_Texture_X},
	{"texture_y", Lua_Polygon_Floor_Set_Texture_Y},
	{"transfer_mode", Lua_Polygon_Floor_Set_Transfer_Mode},
	{"z", Lua_Polygon_Floor_Set_Height},
	{0, 0}
};

char Lua_Polygon_Ceiling_Name[] = "polygon_ceiling";

static int Lua_Polygon_Ceiling_Get_Collection(lua_State *L)
{
	Lua_Collection::Push(L, GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(get_polygon_data(Lua_Polygon_Ceiling::Index(L, 1))->ceiling_texture)));
	return 1;
}

static int Lua_Polygon_Ceiling_Get_Height(lua_State *L)
{
	lua_pushnumber(L, (double) (get_polygon_data(Lua_Polygon_Ceiling::Index(L, 1))->ceiling_height) / WORLD_ONE);
	return 1;
}

static int Lua_Polygon_Ceiling_Get_Light(lua_State *L)
{
	Lua_Light::Push(L, get_polygon_data(Lua_Polygon_Ceiling::Index(L, 1))->ceiling_lightsource_index);
	return 1;
}

static int Lua_Polygon_Ceiling_Get_Texture_Index(lua_State *L)
{
	lua_pushnumber(L, GET_DESCRIPTOR_SHAPE(get_polygon_data(Lua_Polygon_Ceiling::Index(L, 1))->ceiling_texture));
	return 1;
}

static int Lua_Polygon_Ceiling_Get_Texture_X(lua_State *L)
{
	lua_pushnumber(L, (double) (get_polygon_data(Lua_Polygon_Ceiling::Index(L, 1))->ceiling_origin.x) / WORLD_ONE);
	return 1;
}

static int Lua_Polygon_Ceiling_Get_Texture_Y(lua_State *L)
{
	lua_pushnumber(L, (double) (get_polygon_data(Lua_Polygon_Ceiling::Index(L, 1))->ceiling_origin.y) / WORLD_ONE);
	return 1;
}

static int Lua_Polygon_Ceiling_Get_Transfer_Mode(lua_State *L)
{
	Lua_TransferMode::Push(L, get_polygon_data(Lua_Polygon_Ceiling::Index(L, 1))->ceiling_transfer_mode);
	return 1;
}

static int Lua_Polygon_Ceiling_Set_Collection(lua_State *L)
{
	short polygon_index = Lua_Polygon_Ceiling::Index(L, 1);
	short collection_index = Lua_Collection::ToIndex(L, 2);

	polygon_data *polygon = get_polygon_data(polygon_index);
	polygon->ceiling_texture = BUILD_DESCRIPTOR(collection_index, GET_DESCRIPTOR_SHAPE(polygon->ceiling_texture));
	return 0;
}

static int Lua_Polygon_Ceiling_Set_Height(lua_State *L)
{
	if (!lua_isnumber(L, 2))
	{
		luaL_error(L, "height: incorrect argument type");
	}

	struct polygon_data *polygon = get_polygon_data(Lua_Polygon_Ceiling::Index(L, 1));
	polygon->ceiling_height = static_cast<world_distance>(lua_tonumber(L,2)*WORLD_ONE);
	for (short i = 0; i < polygon->vertex_count; ++i)
	{
		recalculate_redundant_endpoint_data(polygon->endpoint_indexes[i]);
		recalculate_redundant_line_data(polygon->line_indexes[i]);
	}
	return 0;
}

static int Lua_Polygon_Ceiling_Set_Light(lua_State *L)
{
	short light_index;
	if (lua_isnumber(L, 2))
	{
		light_index = static_cast<short>(lua_tonumber(L, 2));
		if (light_index < 0 || light_index >= MAXIMUM_LIGHTS_PER_MAP)
			return luaL_error(L, "light: invalid light index");
	}
	else
	{
		light_index = Lua_Light::Index(L, 2);
	}
	
	get_polygon_data(Lua_Polygon_Ceiling::Index(L, 1))->ceiling_lightsource_index = light_index;
	return 0;
}

static int Lua_Polygon_Ceiling_Set_Texture_Index(lua_State *L)
{
	polygon_data *polygon = get_polygon_data(Lua_Polygon_Ceiling::Index(L, 1));
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "texture_index: incorrect argument type");

	short shape_index = static_cast<short>(lua_tonumber(L, 2));
	if (shape_index < 0 || shape_index >= MAXIMUM_SHAPES_PER_COLLECTION)
		return luaL_error(L, "texture_index: invalid texture index");
	
	polygon->ceiling_texture = BUILD_DESCRIPTOR(GET_DESCRIPTOR_COLLECTION(polygon->ceiling_texture), shape_index);
	return 0;
}

static int Lua_Polygon_Ceiling_Set_Texture_X(lua_State *L)
{
	polygon_data *polygon = get_polygon_data(Lua_Polygon_Ceiling::Index(L, 1));
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "texture_x: incorrect argument type");

	polygon->ceiling_origin.x = static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE);
	return 0;
}

static int Lua_Polygon_Ceiling_Set_Texture_Y(lua_State *L)
{
	polygon_data *polygon = get_polygon_data(Lua_Polygon_Ceiling::Index(L, 1));
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "texture_y: incorrect argument type");

	polygon->ceiling_origin.y = static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE);
	return 0;
}

static int Lua_Polygon_Ceiling_Set_Transfer_Mode(lua_State *L)
{
	polygon_data *polygon = get_polygon_data(Lua_Polygon_Ceiling::Index(L, 1));
	polygon->ceiling_transfer_mode = Lua_TransferMode::ToIndex(L, 2);
	return 0;
}

const luaL_Reg Lua_Polygon_Ceiling_Get[] = {
	{"collection", Lua_Polygon_Ceiling_Get_Collection},
	{"height", Lua_Polygon_Ceiling_Get_Height},
	{"light", Lua_Polygon_Ceiling_Get_Light},
	{"texture_index", Lua_Polygon_Ceiling_Get_Texture_Index},
	{"texture_x", Lua_Polygon_Ceiling_Get_Texture_X},
	{"texture_y", Lua_Polygon_Ceiling_Get_Texture_Y},
	{"transfer_mode", Lua_Polygon_Ceiling_Get_Transfer_Mode},
	{"z", Lua_Polygon_Ceiling_Get_Height},
	{0, 0}
};

const luaL_Reg Lua_Polygon_Ceiling_Set[] = {
	{"collection", Lua_Polygon_Ceiling_Set_Collection},
	{"height", Lua_Polygon_Ceiling_Set_Height},
	{"light", Lua_Polygon_Ceiling_Set_Light},
	{"texture_index", Lua_Polygon_Ceiling_Set_Texture_Index},
	{"texture_x", Lua_Polygon_Ceiling_Set_Texture_X},
	{"texture_y", Lua_Polygon_Ceiling_Set_Texture_Y},
	{"transfer_mode", Lua_Polygon_Ceiling_Set_Transfer_Mode},
	{"z", Lua_Polygon_Ceiling_Set_Height},
	{0, 0}
};

char Lua_PolygonType_Name[] = "polygon_type";
typedef L_Enum<Lua_PolygonType_Name> Lua_PolygonType;

char Lua_PolygonTypes_Name[] = "PolygonTypes";
typedef L_EnumContainer<Lua_PolygonTypes_Name, Lua_PolygonType> Lua_PolygonTypes;

char Lua_Polygon_Name[] = "polygon";

char Lua_Adjacent_Polygons_Name[] = "adjacent_polygons";
typedef L_Class<Lua_Adjacent_Polygons_Name> Lua_Adjacent_Polygons;

static int Lua_Adjacent_Polygons_Iterator(lua_State *L)
{
	int index = static_cast<int>(lua_tonumber(L, lua_upvalueindex(1)));
	short polygon_index = Lua_Adjacent_Polygons::Index(L, lua_upvalueindex(2));
	polygon_data *polygon = get_polygon_data(polygon_index);

	while (index < polygon->vertex_count)
	{
		if (polygon->adjacent_polygon_indexes[index] != NONE)
		{
			Lua_Polygon::Push(L, polygon->adjacent_polygon_indexes[index]);
			lua_pushnumber(L, ++index);
			lua_replace(L, lua_upvalueindex(1));
			return 1;
		}
		else
		{
			index++;
		}
	}

	lua_pushnil(L);
	return 1;
}

static int Lua_Adjacent_Polygons_Call(lua_State *L)
{
	lua_pushnumber(L, 0);
	lua_pushvalue(L, 1);
	lua_pushcclosure(L, Lua_Adjacent_Polygons_Iterator, 2);
	return 1;
}

static int Lua_Adjacent_Polygons_Get(lua_State *L)
{
	if (lua_isnumber(L, 2))
	{
		short polygon_index = Lua_Adjacent_Polygons::Index(L, 1);
		polygon_data *polygon = get_polygon_data(polygon_index);
		int adjacent_polygon_index = static_cast<int>(lua_tonumber(L, 2));
		if (adjacent_polygon_index >= 0 && adjacent_polygon_index < polygon->vertex_count)
		{
			Lua_Polygon::Push(L, polygon->adjacent_polygon_indexes[adjacent_polygon_index]);
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

static int Lua_Adjacent_Polygons_Length(lua_State *L)
{
	lua_pushnumber(L, get_polygon_data(Lua_Adjacent_Polygons::Index(L, 1))->vertex_count);
	return 1;
}

const luaL_Reg Lua_Adjacent_Polygons_Metatable[] = {
	{"__index", Lua_Adjacent_Polygons_Get},
	{"__call", Lua_Adjacent_Polygons_Call},
	{"__len", Lua_Adjacent_Polygons_Length},
	{0, 0}
};

char Lua_Polygon_Endpoints_Name[] = "polygon_endpoints";
typedef L_Class<Lua_Polygon_Endpoints_Name> Lua_Polygon_Endpoints;

static int Lua_Polygon_Endpoints_Iterator(lua_State *L)
{
	int index = static_cast<int>(lua_tonumber(L, lua_upvalueindex(1)));
	short polygon_index = Lua_Polygon_Endpoints::Index(L, lua_upvalueindex(2));
	polygon_data *polygon = get_polygon_data(polygon_index);
	
	while (index < polygon->vertex_count)
	{
		if (polygon->endpoint_indexes[index] != NONE)
		{
			Lua_Endpoint::Push(L, polygon->endpoint_indexes[index]);
			lua_pushnumber(L, ++index);
			lua_replace(L, lua_upvalueindex(1));
			return 1;
		}
		else
		{
			index++;
		}
	}

	lua_pushnil(L);
	return 1;
}

static int Lua_Polygon_Endpoints_Call(lua_State *L)
{
	lua_pushnumber(L, 0);
	lua_pushvalue(L, 1);
	lua_pushcclosure(L, Lua_Polygon_Endpoints_Iterator, 2);
	return 1;
}

static int Lua_Polygon_Endpoints_Get(lua_State *L)
{
	if (lua_isnumber(L, 2))
	{
		short polygon_index = Lua_Polygon_Endpoints::Index(L, 1);
		polygon_data *polygon = get_polygon_data(polygon_index);
		int endpoint_index = static_cast<int>(lua_tonumber(L, 2));
		if (endpoint_index >= 0 && endpoint_index < polygon->vertex_count)
		{
			Lua_Endpoint::Push(L, polygon->endpoint_indexes[endpoint_index]);
		}
		else
		{
			lua_pushnil(L);
		}
	}

	return 1;
}

static int Lua_Polygon_Endpoints_Length(lua_State *L)
{
	lua_pushnumber(L, get_polygon_data(Lua_Polygon_Endpoints::Index(L, 1))->vertex_count);
	return 1;
}

const luaL_Reg Lua_Polygon_Endpoints_Metatable[] = {
	{"__index", Lua_Polygon_Endpoints_Get},
	{"__call", Lua_Polygon_Endpoints_Call},
	{"__len", Lua_Polygon_Endpoints_Length},
	{0, 0}
};

char Lua_Polygon_Lines_Name[] = "polygon_lines";
typedef L_Class<Lua_Polygon_Lines_Name> Lua_Polygon_Lines;

static int Lua_Polygon_Lines_Iterator(lua_State *L)
{
	int index = static_cast<int>(lua_tonumber(L, lua_upvalueindex(1)));
	short polygon_index = Lua_Polygon_Lines::Index(L, lua_upvalueindex(2));
	polygon_data *polygon = get_polygon_data(polygon_index);
	
	while (index < polygon->vertex_count)
	{
		if (polygon->line_indexes[index] != NONE)
		{
			Lua_Line::Push(L, polygon->line_indexes[index]);
			lua_pushnumber(L, ++index);
			lua_replace(L, lua_upvalueindex(1));
			return 1;
		}
		else
		{
			index++;
		}
	}

	lua_pushnil(L);
	return 1;
}

static int Lua_Polygon_Lines_Call(lua_State *L)
{
	lua_pushnumber(L, 0);
	lua_pushvalue(L, 1);
	lua_pushcclosure(L, Lua_Polygon_Lines_Iterator, 2);
	return 1;
}

static int Lua_Polygon_Lines_Get(lua_State *L)
{
	if (lua_isnumber(L, 2))
	{
		short polygon_index = Lua_Polygon_Lines::Index(L, 1);
		polygon_data *polygon = get_polygon_data(polygon_index);
		int line_index = static_cast<int>(lua_tonumber(L, 2));
		if (line_index >= 0 && line_index < polygon->vertex_count)
		{
			Lua_Line::Push(L, polygon->line_indexes[line_index]);
		}
		else
		{
			lua_pushnil(L);
		}
	}

	return 1;
}

static int Lua_Polygon_Lines_Length(lua_State *L)
{
	lua_pushnumber(L, get_polygon_data(Lua_Polygon_Lines::Index(L, 1))->vertex_count);
	return 1;
}

const luaL_Reg Lua_Polygon_Lines_Metatable[] = {
	{"__index", Lua_Polygon_Lines_Get},
	{"__call", Lua_Polygon_Lines_Call},
	{"__len", Lua_Polygon_Lines_Length},
	{0, 0}
};

char Lua_Polygon_Sides_Name[] = "polygon_sides";
typedef L_Class<Lua_Polygon_Sides_Name> Lua_Polygon_Sides;

static int Lua_Polygon_Sides_Iterator(lua_State *L)
{
	int index = static_cast<int>(lua_tonumber(L, lua_upvalueindex(1)));
	short polygon_index = Lua_Polygon_Sides::Index(L, lua_upvalueindex(2));
	polygon_data *polygon = get_polygon_data(polygon_index);
	
	while (index < polygon->vertex_count)
	{
		if (polygon->side_indexes[index] != NONE)
		{
			Lua_Side::Push(L, polygon->side_indexes[index]);
			lua_pushnumber(L, ++index);
			lua_replace(L, lua_upvalueindex(1));
			return 1;
		}
		else
		{
			index++;
		}
	}

	lua_pushnil(L);
	return 1;
}

static int Lua_Polygon_Sides_Call(lua_State *L)
{
	lua_pushnumber(L, 0);
	lua_pushvalue(L, 1);
	lua_pushcclosure(L, Lua_Polygon_Sides_Iterator, 2);
	return 1;
}

static int Lua_Polygon_Sides_Get(lua_State *L)
{
	if (lua_isnumber(L, 2))
	{
		short polygon_index = Lua_Polygon_Sides::Index(L, 1);
		polygon_data *polygon = get_polygon_data(polygon_index);
		int side_index = static_cast<int>(lua_tonumber(L, 2));
		if (side_index >= 0 && side_index < polygon->vertex_count)
		{
			Lua_Side::Push(L, polygon->side_indexes[side_index]);
		}
		else
		{
			lua_pushnil(L);
		}
	}

	return 1;
}

static int Lua_Polygon_Sides_Length(lua_State *L)
{
	lua_pushnumber(L, get_polygon_data(Lua_Polygon_Sides::Index(L, 1))->vertex_count);
	return 1;
}

const luaL_Reg Lua_Polygon_Sides_Metatable[] = {
	{"__index", Lua_Polygon_Sides_Get},
	{"__call", Lua_Polygon_Sides_Call},
	{"__len", Lua_Polygon_Sides_Length},
	{0, 0}
};

int Lua_Polygon_Change_Height(lua_State* L)
{
	if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		return luaL_error(L, ("change_height: incorrect argument type"));

	short polygon_index = Lua_Polygon::Index(L, 1);

	auto floor_height = lua_tonumber(L, 2) * WORLD_ONE;
	auto ceiling_height = lua_tonumber(L, 3) * WORLD_ONE;

	auto success = change_polygon_height(polygon_index, floor_height, ceiling_height, nullptr);

	if (success)
	{
		auto polygon = get_polygon_data(polygon_index);
		for (auto i = 0; i < polygon->vertex_count; ++i)
		{
			recalculate_redundant_line_data(polygon->line_indexes[i]);
			recalculate_redundant_endpoint_data(polygon->endpoint_indexes[i]);
		}
	}

	lua_pushboolean(L, success);
	return 1;
}

extern projectile_definition* get_projectile_definition(short);

// p, x1, y1, z1, owner, x2, y2, z2, [stop_at_objects], [stop_at_media]
int Lua_Polygon_Check_Collision(lua_State* L)
{
	if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3) ||
		!lua_isnumber(L, 4) || !lua_isnumber(L, 6) ||
		!lua_isnumber(L, 7) || !lua_isnumber(L, 8) ||
		(lua_gettop(L) >= 9 && !(lua_isnil(L, 9) || lua_isboolean(L, 9))) ||
		(lua_gettop(L) >= 10 && !(lua_isnil(L, 10) || lua_isboolean(L, 10))))
	{
		return luaL_error(L, ("check_collision: incorrect argument type"));
	}

	short owner = NONE;
	if (Lua_Monster::Is(L, 5))
	{
		owner = Lua_Monster::Index(L, 5);
	}
	else if (Lua_Player::Is(L, 5))
	{
		auto player = get_player_data(Lua_Player::Index(L, 5));
		owner = player->monster_index;
	}
	else if (!lua_isnil(L, 5))
		return luaL_error(L, ("check_collision: incorrect argument type"));

	world_point3d origin = {
		static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE),
		static_cast<world_distance>(lua_tonumber(L, 3) * WORLD_ONE),
		static_cast<world_distance>(lua_tonumber(L, 4) * WORLD_ONE)
	};
	
	world_point3d destination = {
		static_cast<world_distance>(lua_tonumber(L, 6) * WORLD_ONE),
		static_cast<world_distance>(lua_tonumber(L, 7) * WORLD_ONE),
		static_cast<world_distance>(lua_tonumber(L, 8) * WORLD_ONE)
	};

	auto stop_at_objects = lua_toboolean(L, 9);
	auto stop_at_media = lua_toboolean(L, 10);

	short polygon_index = Lua_Polygon::Index(L, 1);

	// preflight a projectile 1 WU at a time (because of the speed bug)
	world_distance distance = distance2d(
		reinterpret_cast<world_point2d*>(&origin),
		reinterpret_cast<world_point2d*>(&destination));

	int32_t chunks = (distance + WORLD_ONE - 1) / WORLD_ONE;

	int32_t dx = destination.x - origin.x;
	int32_t dy = destination.y - origin.y;
	int32_t dz = destination.z - origin.z;

	world_point3d p0;
	world_point3d p1 = origin;
	short old_polygon;
	short new_polygon = polygon_index;
	short obstruction_index;
	short line_index;
	uint16_t flags = 0;

	auto projectile_definition = get_projectile_definition(0);
	auto projectile_flags = projectile_definition->flags;

	projectile_definition->flags =
		_usually_pass_transparent_side |
		_sometimes_pass_transparent_side |
		(!stop_at_objects ? _passes_through_objects : 0) |
		(!stop_at_media ? _penetrates_media : 0);

	for (auto i = 0; i < chunks && !(flags & _projectile_hit); ++i)
	{
		old_polygon = new_polygon;
		p0 = p1;

		p1 = {
			static_cast<world_distance>(origin.x + dx * (i + 1) / chunks),
			static_cast<world_distance>(origin.y + dy * (i + 1) / chunks),
			static_cast<world_distance>(origin.z + dz * (i + 1) / chunks)
		};

		flags = translate_projectile(0, &p0, old_polygon,
									 &p1, &new_polygon, owner,
									 &obstruction_index, &line_index, true, NONE);
	}

	projectile_definition->flags = projectile_flags;

	if (flags & _projectile_hit_monster)
	{
		auto object = get_object_data(obstruction_index);
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
	else if (flags & _projectile_hit)
	{
		auto side_index = find_adjacent_side(new_polygon, line_index);
		Lua_Side::Push(L, side_index);
	}
	else
	{
		lua_pushnil(L);
	}

	lua_pushnumber(L, static_cast<double>(p1.x) / WORLD_ONE);
	lua_pushnumber(L, static_cast<double>(p1.y) / WORLD_ONE);
	lua_pushnumber(L, static_cast<double>(p1.z) / WORLD_ONE);
	Lua_Polygon::Push(L, new_polygon);

	return 5;
}

// contains(x, y, z)
int Lua_Polygon_Contains(lua_State *L)
{
	if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		return luaL_error(L, ("contains: incorrect argument type"));

	short polygon_index = Lua_Polygon::Index(L, 1);
	polygon_data *polygon = get_polygon_data(polygon_index);

	world_point2d p;
	p.x = static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE);
	p.y = static_cast<world_distance>(lua_tonumber(L, 3) * WORLD_ONE);

	world_distance z;
	if (lua_gettop(L) == 4)
	{
		if (lua_isnumber(L, 4))
			z = static_cast<world_distance>(lua_tonumber(L, 4) * WORLD_ONE);
		else
			return luaL_error(L, "contains: incorrect argument type");
	}
	else
	{
		z = polygon->floor_height;
	}

	lua_pushboolean(L, point_in_polygon(polygon_index, &p) && z >= polygon->floor_height && z <= polygon->ceiling_height);
	return 1;
}

static int Lua_Polygon_Find_Polygon(lua_State* L)
{
	if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3) ||
		!lua_isnumber(L, 4) || !lua_isnumber(L, 5))
		return luaL_error(L, "find_polygon: incorrect argument type");

	world_point2d origin;
	origin.x = static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE);
	origin.y = static_cast<world_distance>(lua_tonumber(L, 3) * WORLD_ONE);

	world_point2d destination;
	destination.x = static_cast<world_distance>(lua_tonumber(L, 4) * WORLD_ONE);
	destination.y = static_cast<world_distance>(lua_tonumber(L, 5) * WORLD_ONE);

	auto polygon_index = find_new_object_polygon(&origin, &destination, Lua_Polygon::Index(L, 1));
	if (polygon_index != NONE)
	{
		Lua_Polygon::Push(L, polygon_index);
	}
	else
	{
		lua_pushnil(L);
	}

	return 1;
}

// line_crossed_leaving(x1, y1, x2, y2)
int Lua_Polygon_Find_Line_Crossed_Leaving(lua_State *L)
{
	if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 5))
		return luaL_error(L, ("find_line_crossed_leaving: incorrect argument type"));
	
	world_point2d origin;
	origin.x = static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE);
	origin.y = static_cast<world_distance>(lua_tonumber(L, 3) * WORLD_ONE);

	world_point2d destination;
	destination.x = static_cast<world_distance>(lua_tonumber(L, 4) * WORLD_ONE);
	destination.y = static_cast<world_distance>(lua_tonumber(L, 5) * WORLD_ONE);

	short line_index = find_line_crossed_leaving_polygon(Lua_Polygon::Index(L, 1), &origin, &destination);
	if (line_index != NONE)
	{
		Lua_Line::Push(L, line_index);
	}
	else
	{
		lua_pushnil(L);
	}

	return 1;
}

static int Lua_Polygon_Monsters_Iterator(lua_State *L)
{
	int index = static_cast<int>(lua_tonumber(L, lua_upvalueindex(1)));
	lua_pushvalue(L, lua_upvalueindex(2));
	lua_pushnumber(L, index);
	lua_gettable(L, -2);
	lua_remove(L, -2);

	lua_pushnumber(L, ++index);
	lua_replace(L, lua_upvalueindex(1));

	return 1;
}	

int Lua_Polygon_Monsters(lua_State *L)
{
	polygon_data *polygon = get_polygon_data(Lua_Polygon::Index(L, 1));
	
	int table_index = 1;
	short object_index = polygon->first_object;

	lua_pushnumber(L, 1);
	lua_newtable(L);
	while (object_index != NONE)
	{
		object_data *object = get_object_data(object_index);
		if (GET_OBJECT_OWNER(object) == _object_is_monster)
		{
			lua_pushnumber(L, table_index++);
			Lua_Monster::Push(L, object->permutation);
			lua_settable(L, -3);
		}

		object_index = object->next_object;
	}
	
	lua_pushcclosure(L, Lua_Polygon_Monsters_Iterator, 2);
	return 1;
}

// play_sound(sound) or play_sound(x, y, z, sound, [pitch])
int Lua_Polygon_Play_Sound(lua_State *L)
{
	if (lua_gettop(L) == 2)
	{
		short sound_code = Lua_Sound::ToIndex(L, 2);
		play_polygon_sound(Lua_Polygon::Index(L, 1), sound_code);
	}
	else 
	{
		if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
			return luaL_error(L, "play_sound: incorrect argument type");
		world_location3d source;
		source.point.x = static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE);
		source.point.y = static_cast<world_distance>(lua_tonumber(L, 3) * WORLD_ONE);
		source.point.z = static_cast<world_distance>(lua_tonumber(L, 4) * WORLD_ONE);
		source.polygon_index = Lua_Polygon::Index(L, 1);
		short sound_code = Lua_Sound::ToIndex(L, 5);
		_fixed pitch = FIXED_ONE;
		if (lua_gettop(L) == 6) 
		{
			if (!lua_isnumber(L, 6))
				return luaL_error(L, "play_sound: incorrect argument type");
			pitch = static_cast<_fixed>(lua_tonumber(L, 6) * FIXED_ONE);
		}

		SoundManager::instance()->PlaySound(sound_code, &source, NONE, pitch);
	}

	return 0;
}

static int Lua_Polygon_Get_Adjacent(lua_State *L)
{
	Lua_Adjacent_Polygons::Push(L, Lua_Polygon::Index(L, 1));
	return 1;
}

static int Lua_Polygon_Get_Area(lua_State *L)
{
	lua_pushnumber(L, (double) get_polygon_data(Lua_Polygon::Index(L, 1))->area / WORLD_ONE / WORLD_ONE);
	return 1;
}

static int Lua_Polygon_Get_Ceiling(lua_State *L)
{
	Lua_Polygon_Ceiling::Push(L, Lua_Polygon::Index(L, 1));
	return 1;
}

static int Lua_Polygon_Get_Endpoints(lua_State *L)
{
	Lua_Polygon_Endpoints::Push(L, Lua_Polygon::Index(L, 1));
	return 1;
}

static int Lua_Polygon_Get_Floor(lua_State *L)
{
	Lua_Polygon_Floor::Push(L, Lua_Polygon::Index(L, 1));
	return 1;
}

static int Lua_Polygon_Get_Lines(lua_State *L)
{
	Lua_Polygon_Lines::Push(L, Lua_Polygon::Index(L, 1));
	return 1;
}

static int Lua_Polygon_Get_Media(lua_State *L)
{
	polygon_data *polygon = get_polygon_data(Lua_Polygon::Index(L, 1));
	if (polygon->media_index != NONE)
	{
		Lua_Media::Push(L, polygon->media_index);
	}
	else
	{
		lua_pushnil(L);
	}

	return 1;
}

static int Lua_Polygon_Get_Permutation(lua_State *L)
{
	polygon_data *polygon = get_polygon_data(Lua_Polygon::Index(L, 1));
	lua_pushnumber(L, polygon->permutation);
	return 1;
}

static int Lua_Polygon_Get_Sides(lua_State *L)
{
	Lua_Polygon_Sides::Push(L, Lua_Polygon::Index(L, 1));
	return 1;
}

static int Lua_Polygon_Get_Type(lua_State *L)
{
	Lua_PolygonType::Push(L, get_polygon_data(Lua_Polygon::Index(L, 1))->type);
	return 1;
}

static int Lua_Polygon_Get_Platform(lua_State *L)
{
	polygon_data *polygon = get_polygon_data(Lua_Polygon::Index(L, 1));
	if (polygon->type == _polygon_is_platform)
	{
		Lua_Platform::Push(L, polygon->permutation);
	}
	else
	{
		lua_pushnil(L);
	}

	return 1;
}

static int Lua_Polygon_Get_X(lua_State *L)
{
	lua_pushnumber(L, (double) get_polygon_data(Lua_Polygon::Index(L, 1))->center.x / WORLD_ONE);
	return 1;
}

static int Lua_Polygon_Get_Y(lua_State *L)
{
	lua_pushnumber(L, (double) get_polygon_data(Lua_Polygon::Index(L, 1))->center.y / WORLD_ONE);
	return 1;
}

static int Lua_Polygon_Get_Z(lua_State *L)
{
	lua_pushnumber(L, (double) get_polygon_data(Lua_Polygon::Index(L, 1))->floor_height / WORLD_ONE);
	return 1;
}

static int Lua_Polygon_Get_Visible_On_Automap(lua_State *L)
{
	lua_pushboolean(L, POLYGON_IS_IN_AUTOMAP(Lua_Polygon::Index(L, 1)));
	return 1;
}

static int Lua_Polygon_Set_Media(lua_State *L)
{
	polygon_data *polygon = get_polygon_data(Lua_Polygon::Index(L, 1));
	short media_index = NONE;
	if (lua_isnumber(L, 2))
	{
		media_index = static_cast<short>(lua_tonumber(L, 2));
		if (media_index < 0 || media_index > MAXIMUM_MEDIAS_PER_MAP)
			return luaL_error(L, "media: invalid media index");

	} 
	else if (Lua_Media::Is(L, 2))
	{
		media_index = Lua_Media::Index(L, 2);
	}
	else if (!lua_isnil(L, 2))
	{
		return luaL_error(L, "media: incorrect argument type");
	}

	polygon->media_index = media_index;
	return 0;
}
		
static int Lua_Polygon_Set_Permutation(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, ("type: incorrect argument type"));
	
	int permutation = static_cast<int>(lua_tonumber(L, 2));
	get_polygon_data(Lua_Polygon::Index(L, 1))->permutation = permutation;
	return 0;
}

static int Lua_Polygon_Set_Type(lua_State *L)
{
	polygon_data* polygon = get_polygon_data(Lua_Polygon::Index(L, 1));
	polygon->type = Lua_PolygonType::ToIndex(L, 2);
	return 0;
}

static int Lua_Polygon_Set_Visible_On_Automap(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, ("visible_on_automap: incorrect argument type"));
	
	if (lua_toboolean(L, 2))
		ADD_POLYGON_TO_AUTOMAP(Lua_Polygon::Index(L, 1));
	else
		CLEAR_POLYGON_FROM_AUTOMAP(Lua_Polygon::Index(L, 1));
	return 0;
}

static bool Lua_Polygon_Valid(int16 index)
{
	return index >= 0 && index < dynamic_world->polygon_count;
}

const luaL_Reg Lua_Polygon_Get[] = {
	{"adjacent_polygons", Lua_Polygon_Get_Adjacent},
	{"area", Lua_Polygon_Get_Area},
	{"ceiling", Lua_Polygon_Get_Ceiling},
	{"change_height", L_TableFunction<Lua_Polygon_Change_Height>},
	{"check_collision", L_TableFunction<Lua_Polygon_Check_Collision>},
	{"contains", L_TableFunction<Lua_Polygon_Contains>},
	{"endpoints", Lua_Polygon_Get_Endpoints},
	{"find_polygon", L_TableFunction<Lua_Polygon_Find_Polygon>},
	{"find_line_crossed_leaving", L_TableFunction<Lua_Polygon_Find_Line_Crossed_Leaving>},
	{"floor", Lua_Polygon_Get_Floor},
	{"lines", Lua_Polygon_Get_Lines},
	{"media", Lua_Polygon_Get_Media},
	{"monsters", L_TableFunction<Lua_Polygon_Monsters>},
	{"permutation", Lua_Polygon_Get_Permutation},
	{"platform", Lua_Polygon_Get_Platform},
	{"play_sound", L_TableFunction<Lua_Polygon_Play_Sound>},
	{"sides", Lua_Polygon_Get_Sides},
	{"type", Lua_Polygon_Get_Type},
	{"visible_on_automap", Lua_Polygon_Get_Visible_On_Automap},
	{"x", Lua_Polygon_Get_X},
	{"y", Lua_Polygon_Get_Y},
	{"z", Lua_Polygon_Get_Z},
	{0, 0}
};

const luaL_Reg Lua_Polygon_Set[] = {
	{"media", Lua_Polygon_Set_Media},
	{"permutation", Lua_Polygon_Set_Permutation},
	{"type", Lua_Polygon_Set_Type},
	{"visible_on_automap", Lua_Polygon_Set_Visible_On_Automap},
	{0, 0}
};

char Lua_Polygons_Name[] = "Polygons";

int16 Lua_Polygons_Length() {
	return dynamic_world->polygon_count;
}

char Lua_Side_ControlPanel_Name[] = "side_control_panel";
typedef L_Class<Lua_Side_ControlPanel_Name> Lua_Side_ControlPanel;

template<int16 flag>
static int Lua_Side_ControlPanel_Get_Flag(lua_State *L)
{
	side_data *side = get_side_data(Lua_Side_ControlPanel::Index(L, 1));
	lua_pushboolean(L, side->flags & flag);
	return 1;
}

template<int16 flag>
static int Lua_Side_ControlPanel_Set_Flag(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "control_panel: incorrect argument type");
	
	side_data *side = get_side_data(Lua_Side_ControlPanel::Index(L, 1));
	if (lua_toboolean(L, 2))
		side->flags |= flag;
	else
		side->flags &= ~flag;
	return 0;
}

static int Lua_Side_ControlPanel_Get_Type(lua_State *L)
{
	Lua_ControlPanelType::Push(L, get_side_data(Lua_Side_ControlPanel::Index(L, 1))->control_panel_type);
	return 1;
}

static int Lua_Side_ControlPanel_Get_Permutation(lua_State *L)
{
	lua_pushnumber(L, get_side_data(Lua_Side_ControlPanel::Index(L, 1))->control_panel_permutation);
	return 1;
}

static int Lua_Side_ControlPanel_Get_UsesItem(lua_State* L)
{
	control_panel_definition* definition = get_control_panel_definition(get_side_data(Lua_Side_ControlPanel::Index(L, 1))->control_panel_type);
	lua_pushboolean(L, definition->item != NONE);
	return 1;
}

const luaL_Reg Lua_Side_ControlPanel_Get[] = {
	{"can_be_destroyed", Lua_Side_ControlPanel_Get_Flag<_side_switch_can_be_destroyed>},
	{"light_dependent", Lua_Side_ControlPanel_Get_Flag<_side_is_lighted_switch>},
	{"only_toggled_by_weapons", Lua_Side_ControlPanel_Get_Flag<_side_switch_can_only_be_hit_by_projectiles>},
	{"repair", Lua_Side_ControlPanel_Get_Flag<_side_is_repair_switch>},
	{"status", Lua_Side_ControlPanel_Get_Flag<_control_panel_status>},
	{"type", Lua_Side_ControlPanel_Get_Type},
	{"permutation", Lua_Side_ControlPanel_Get_Permutation},
	{"uses_item", Lua_Side_ControlPanel_Get_UsesItem},
	{0, 0}
};

static int Lua_Side_ControlPanel_Set_Permutation(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "permutation: incorrect argument type");

	side_data *side = get_side_data(Lua_Side_ControlPanel::Index(L, 1));
	side->control_panel_permutation = static_cast<int16>(lua_tonumber(L, 2));
	return 0;
}

extern void set_control_panel_texture(side_data *);

static int Lua_Side_ControlPanel_Set_Type(lua_State *L)
{
	side_data *side = get_side_data(Lua_Side_ControlPanel::Index(L, 1));
	side->control_panel_type = Lua_ControlPanelType::ToIndex(L, 2);
	set_control_panel_texture(side);
	return 0;
}

// the old version set a useless flag; instead, do nothing
static int Lua_Side_ControlPanel_Set_UsesItem(lua_State*)
{
    return 0;
}

const luaL_Reg Lua_Side_ControlPanel_Set[] = {
	{"can_be_destroyed", Lua_Side_ControlPanel_Set_Flag<_side_switch_can_be_destroyed>},
	{"light_dependent", Lua_Side_ControlPanel_Set_Flag<_side_is_lighted_switch>},
	{"only_toggled_by_weapons", Lua_Side_ControlPanel_Set_Flag<_side_switch_can_only_be_hit_by_projectiles>},
	{"permutation", Lua_Side_ControlPanel_Set_Permutation},
	{"repair", Lua_Side_ControlPanel_Set_Flag<_side_is_repair_switch>},
	{"status", Lua_Side_ControlPanel_Set_Flag<_control_panel_status>},
	{"type", Lua_Side_ControlPanel_Set_Type},
	{"uses_item", Lua_Side_ControlPanel_Set_UsesItem},
	{0, 0}
};

char Lua_Primary_Side_Name[] = "primary_side";
typedef L_Class<Lua_Primary_Side_Name> Lua_Primary_Side;

static int Lua_Primary_Side_Get_Collection(lua_State *L)
{
	Lua_Collection::Push(L, GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(get_side_data(Lua_Primary_Side::Index(L, 1))->primary_texture.texture)));
	return 1;
}

static int Lua_Primary_Side_Get_Light(lua_State *L)
{
	Lua_Light::Push(L, get_side_data(Lua_Primary_Side::Index(L, 1))->primary_lightsource_index);
	return 1;
}

static int Lua_Primary_Side_Get_Texture_Index(lua_State *L)
{
	lua_pushnumber(L, GET_DESCRIPTOR_SHAPE(get_side_data(Lua_Primary_Side::Index(L, 1))->primary_texture.texture));
	return 1;
}

static int Lua_Primary_Side_Get_Texture_X(lua_State *L)
{
	lua_pushnumber(L, (double) (get_side_data(Lua_Primary_Side::Index(L, 1))->primary_texture.x0) / WORLD_ONE);
	return 1;
}

static int Lua_Primary_Side_Get_Texture_Y(lua_State *L)
{
	lua_pushnumber(L, (double) (get_side_data(Lua_Primary_Side::Index(L, 1))->primary_texture.y0) / WORLD_ONE);
	return 1;
}

static int Lua_Primary_Side_Get_Transfer_Mode(lua_State *L)
{
	Lua_TransferMode::Push(L, get_side_data(Lua_Primary_Side::Index(L, 1))->primary_transfer_mode);
	return 1;
}

static void update_line_redundancy(short line_index)
{
	line_data *line= get_line_data(line_index);
	side_data *clockwise_side= NULL, *counterclockwise_side= NULL;
	
	bool landscaped= false;
	bool transparent_texture= false;

	if (line->clockwise_polygon_side_index!=NONE)
	{
		clockwise_side= get_side_data(line->clockwise_polygon_side_index);
	}
	if (line->counterclockwise_polygon_side_index!=NONE)
	{
		counterclockwise_side= get_side_data(line->counterclockwise_polygon_side_index);
	}

	if ((clockwise_side&&clockwise_side->primary_transfer_mode==_xfer_landscape) ||
		(counterclockwise_side&&counterclockwise_side->primary_transfer_mode==_xfer_landscape))
	{
		landscaped= true;
	}
	
	if ((clockwise_side && clockwise_side->transparent_texture.texture!=UNONE) ||
		(counterclockwise_side && counterclockwise_side->transparent_texture.texture!=UNONE))
	{
		transparent_texture= true;
	}
	
	SET_LINE_LANDSCAPE_STATUS(line, landscaped);
	SET_LINE_HAS_TRANSPARENT_SIDE(line, transparent_texture);
}

static int Lua_Primary_Side_Set_Collection(lua_State *L)
{
	short side_index = Lua_Primary_Side::Index(L, 1);
	short collection_index = Lua_Collection::ToIndex(L, 2);

	side_data *side = get_side_data(side_index);
	side->primary_texture.texture = BUILD_DESCRIPTOR(collection_index, GET_DESCRIPTOR_SHAPE(side->primary_texture.texture));
	update_line_redundancy(side->line_index);
	return 0;
}

static int Lua_Primary_Side_Set_Light(lua_State *L)
{
	short light_index;
	if (lua_isnumber(L, 2))
	{
		light_index = static_cast<short>(lua_tonumber(L, 2));
		if (light_index < 0 || light_index >= MAXIMUM_LIGHTS_PER_MAP)
			return luaL_error(L, "light: invalid light index");
	}
	else
	{
		light_index = Lua_Light::Index(L, 2);
	}
	
	get_side_data(Lua_Polygon_Floor::Index(L, 1))->primary_lightsource_index = light_index;
	return 0;
}
static int Lua_Primary_Side_Set_Texture_Index(lua_State *L)
{
	side_data *side = get_side_data(Lua_Primary_Side::Index(L, 1));
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "texture_index: incorrect argument type");

	short shape_index = static_cast<short>(lua_tonumber(L, 2));
	if (shape_index < 0 || shape_index >= MAXIMUM_SHAPES_PER_COLLECTION)
		return luaL_error(L, "texture_index: invalid texture index");
	
	side->primary_texture.texture = BUILD_DESCRIPTOR(GET_DESCRIPTOR_COLLECTION(side->primary_texture.texture), shape_index);
	update_line_redundancy(side->line_index);
	return 0;
}

static int Lua_Primary_Side_Set_Texture_X(lua_State *L)
{
	side_data *side = get_side_data(Lua_Primary_Side::Index(L, 1));
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "texture_x: incorrect argument type");

	side->primary_texture.x0 = static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE);
	return 0;
}

static int Lua_Primary_Side_Set_Texture_Y(lua_State *L)
{
	side_data *side = get_side_data(Lua_Primary_Side::Index(L, 1));
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "texture_y: incorrect argument type");

	side->primary_texture.y0 = static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE);
	return 0;
}

static int Lua_Primary_Side_Set_Transfer_Mode(lua_State *L)
{
	side_data *side = get_side_data(Lua_Primary_Side::Index(L, 1));
	side->primary_transfer_mode = Lua_TransferMode::ToIndex(L, 2);
	return 0;
}
const luaL_Reg Lua_Primary_Side_Get[] = {
	{"collection", Lua_Primary_Side_Get_Collection},
	{"light", Lua_Primary_Side_Get_Light},
	{"texture_index", Lua_Primary_Side_Get_Texture_Index},
	{"texture_x", Lua_Primary_Side_Get_Texture_X},
	{"texture_y", Lua_Primary_Side_Get_Texture_Y},
	{"transfer_mode", Lua_Primary_Side_Get_Transfer_Mode},
	{0, 0}
};

const luaL_Reg Lua_Primary_Side_Set[] = {
	{"collection", Lua_Primary_Side_Set_Collection},
	{"light", Lua_Primary_Side_Set_Light},
	{"texture_index", Lua_Primary_Side_Set_Texture_Index},
	{"texture_x", Lua_Primary_Side_Set_Texture_X},
	{"texture_y", Lua_Primary_Side_Set_Texture_Y},
	{"transfer_mode", Lua_Primary_Side_Set_Transfer_Mode},
	{0, 0}
};

char Lua_Secondary_Side_Name[] = "secondary_side";
typedef L_Class<Lua_Secondary_Side_Name> Lua_Secondary_Side;

static int Lua_Secondary_Side_Get_Collection(lua_State *L)
{
	Lua_Collection::Push(L, GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(get_side_data(Lua_Secondary_Side::Index(L, 1))->secondary_texture.texture)));
	return 1;
}

static int Lua_Secondary_Side_Get_Light(lua_State *L)
{
	Lua_Light::Push(L, get_side_data(Lua_Secondary_Side::Index(L, 1))->secondary_lightsource_index);
	return 1;
}

static int Lua_Secondary_Side_Get_Texture_Index(lua_State *L)
{
	lua_pushnumber(L, GET_DESCRIPTOR_SHAPE(get_side_data(Lua_Secondary_Side::Index(L, 1))->secondary_texture.texture));
	return 1;
}

static int Lua_Secondary_Side_Get_Texture_X(lua_State *L)
{
	lua_pushnumber(L, (double) (get_side_data(Lua_Secondary_Side::Index(L, 1))->secondary_texture.x0) / WORLD_ONE);
	return 1;
}

static int Lua_Secondary_Side_Get_Texture_Y(lua_State *L)
{
	lua_pushnumber(L, (double) (get_side_data(Lua_Secondary_Side::Index(L, 1))->secondary_texture.y0) / WORLD_ONE);
	return 1;
}

static int Lua_Secondary_Side_Get_Transfer_Mode(lua_State *L)
{
	Lua_TransferMode::Push(L, get_side_data(Lua_Secondary_Side::Index(L, 1))->secondary_transfer_mode);
	return 1;
}

static int Lua_Secondary_Side_Set_Collection(lua_State *L)
{
	short side_index = Lua_Secondary_Side::Index(L, 1);
	short collection_index = Lua_Collection::ToIndex(L, 2);

	side_data *side = get_side_data(side_index);
	side->secondary_texture.texture = BUILD_DESCRIPTOR(collection_index, GET_DESCRIPTOR_SHAPE(side->secondary_texture.texture));
	update_line_redundancy(side->line_index);
	return 0;
}

static int Lua_Secondary_Side_Set_Light(lua_State *L)
{
	short light_index;
	if (lua_isnumber(L, 2))
	{
		light_index = static_cast<short>(lua_tonumber(L, 2));
		if (light_index < 0 || light_index >= MAXIMUM_LIGHTS_PER_MAP)
			return luaL_error(L, "light: invalid light index");
	}
	else
	{
		light_index = Lua_Light::Index(L, 2);
	}
	
	get_side_data(Lua_Polygon_Floor::Index(L, 1))->secondary_lightsource_index = light_index;
	return 0;
}
static int Lua_Secondary_Side_Set_Texture_Index(lua_State *L)
{
	side_data *side = get_side_data(Lua_Secondary_Side::Index(L, 1));
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "texture_index: incorrect argument type");

	short shape_index = static_cast<short>(lua_tonumber(L, 2));
	if (shape_index < 0 || shape_index >= MAXIMUM_SHAPES_PER_COLLECTION)
		return luaL_error(L, "texture_index: invalid texture index");
	
	side->secondary_texture.texture = BUILD_DESCRIPTOR(GET_DESCRIPTOR_COLLECTION(side->secondary_texture.texture), shape_index);
	update_line_redundancy(side->line_index);
	return 0;
}

static int Lua_Secondary_Side_Set_Texture_X(lua_State *L)
{
	side_data *side = get_side_data(Lua_Secondary_Side::Index(L, 1));
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "texture_x: incorrect argument type");

	side->secondary_texture.x0 = static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE);
	return 0;
}

static int Lua_Secondary_Side_Set_Texture_Y(lua_State *L)
{
	side_data *side = get_side_data(Lua_Secondary_Side::Index(L, 1));
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "texture_y: incorrect argument type");

	side->secondary_texture.y0 = static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE);
	return 0;
}

static int Lua_Secondary_Side_Set_Transfer_Mode(lua_State *L)
{
	side_data *side = get_side_data(Lua_Secondary_Side::Index(L, 1));
	side->secondary_transfer_mode = Lua_TransferMode::ToIndex(L, 2);
	return 0;
}
const luaL_Reg Lua_Secondary_Side_Get[] = {
	{"collection", Lua_Secondary_Side_Get_Collection},
	{"light", Lua_Secondary_Side_Get_Light},
	{"texture_index", Lua_Secondary_Side_Get_Texture_Index},
	{"texture_x", Lua_Secondary_Side_Get_Texture_X},
	{"texture_y", Lua_Secondary_Side_Get_Texture_Y},
	{"transfer_mode", Lua_Secondary_Side_Get_Transfer_Mode},
	{0, 0}
};

const luaL_Reg Lua_Secondary_Side_Set[] = {
	{"collection", Lua_Secondary_Side_Set_Collection},
	{"light", Lua_Secondary_Side_Set_Light},
	{"texture_index", Lua_Secondary_Side_Set_Texture_Index},
	{"texture_x", Lua_Secondary_Side_Set_Texture_X},
	{"texture_y", Lua_Secondary_Side_Set_Texture_Y},
	{"transfer_mode", Lua_Secondary_Side_Set_Transfer_Mode},
	{0, 0}
};

char Lua_Transparent_Side_Name[] = "transparent_side";
typedef L_Class<Lua_Transparent_Side_Name> Lua_Transparent_Side;

static int Lua_Transparent_Side_Get_Collection(lua_State *L)
{
	Lua_Collection::Push(L, GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(get_side_data(Lua_Transparent_Side::Index(L, 1))->transparent_texture.texture)));
	return 1;
}

static int Lua_Transparent_Side_Get_Empty(lua_State *L)
{
	lua_pushboolean(L, get_side_data(Lua_Transparent_Side::Index(L, 1))->transparent_texture.texture == UNONE);
	return 1;
}

static int Lua_Transparent_Side_Get_Light(lua_State *L)
{
	Lua_Light::Push(L, get_side_data(Lua_Transparent_Side::Index(L, 1))->transparent_lightsource_index);
	return 1;
}

static int Lua_Transparent_Side_Get_Texture_Index(lua_State *L)
{
	lua_pushnumber(L, GET_DESCRIPTOR_SHAPE(get_side_data(Lua_Transparent_Side::Index(L, 1))->transparent_texture.texture));
	return 1;
}

static int Lua_Transparent_Side_Get_Texture_X(lua_State *L)
{
	lua_pushnumber(L, (double) (get_side_data(Lua_Transparent_Side::Index(L, 1))->transparent_texture.x0) / WORLD_ONE);
	return 1;
}

static int Lua_Transparent_Side_Get_Texture_Y(lua_State *L)
{
	lua_pushnumber(L, (double) (get_side_data(Lua_Transparent_Side::Index(L, 1))->transparent_texture.y0) / WORLD_ONE);
	return 1;
}

static int Lua_Transparent_Side_Get_Transfer_Mode(lua_State *L)
{
	Lua_TransferMode::Push(L, get_side_data(Lua_Transparent_Side::Index(L, 1))->transparent_transfer_mode);
	return 1;
}

static int Lua_Transparent_Side_Set_Collection(lua_State *L)
{
	short side_index = Lua_Transparent_Side::Index(L, 1);
	short collection_index = Lua_Collection::ToIndex(L, 2);

	side_data *side = get_side_data(side_index);
	side->transparent_texture.texture = BUILD_DESCRIPTOR(collection_index, GET_DESCRIPTOR_SHAPE(side->transparent_texture.texture));
	update_line_redundancy(side->line_index);
	return 0;
}

static int Lua_Transparent_Side_Set_Empty(lua_State *L)
{
	short side_index = Lua_Transparent_Side::Index(L, 1);
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "empty: incorrect argument type");

	if (lua_toboolean(L, 2))
	{
		side_data *side = get_side_data(side_index);
		side->transparent_texture.texture = UNONE;
		update_line_redundancy(side->line_index);
	}
	return 0;
}

static int Lua_Transparent_Side_Set_Light(lua_State *L)
{
	short light_index;
	if (lua_isnumber(L, 2))
	{
		light_index = static_cast<short>(lua_tonumber(L, 2));
		if (light_index < 0 || light_index >= MAXIMUM_LIGHTS_PER_MAP)
			return luaL_error(L, "light: invalid light index");
	}
	else
	{
		light_index = Lua_Light::Index(L, 2);
	}
	
	get_side_data(Lua_Polygon_Floor::Index(L, 1))->transparent_lightsource_index = light_index;
	return 0;
}

static int Lua_Transparent_Side_Set_Texture_Index(lua_State *L)
{
	side_data *side = get_side_data(Lua_Transparent_Side::Index(L, 1));
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "texture_index: incorrect argument type");

	short shape_index = static_cast<short>(lua_tonumber(L, 2));
	if (shape_index < 0 || shape_index >= MAXIMUM_SHAPES_PER_COLLECTION)
		return luaL_error(L, "texture_index: invalid texture index");
	
	side->transparent_texture.texture = BUILD_DESCRIPTOR(GET_DESCRIPTOR_COLLECTION(side->transparent_texture.texture), shape_index);
	update_line_redundancy(side->line_index);
	return 0;
}

static int Lua_Transparent_Side_Set_Texture_X(lua_State *L)
{
	side_data *side = get_side_data(Lua_Transparent_Side::Index(L, 1));
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "texture_x: incorrect argument type");

	side->transparent_texture.x0 = static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE);
	return 0;
}

static int Lua_Transparent_Side_Set_Texture_Y(lua_State *L)
{
	side_data *side = get_side_data(Lua_Transparent_Side::Index(L, 1));
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "texture_y: incorrect argument type");

	side->transparent_texture.y0 = static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE);
	return 0;
}

static int Lua_Transparent_Side_Set_Transfer_Mode(lua_State *L)
{
	side_data *side = get_side_data(Lua_Transparent_Side::Index(L, 1));
	side->transparent_transfer_mode = Lua_TransferMode::ToIndex(L, 2);
	return 0;
}
const luaL_Reg Lua_Transparent_Side_Get[] = {
	{"collection", Lua_Transparent_Side_Get_Collection},
	{"empty", Lua_Transparent_Side_Get_Empty},
	{"light", Lua_Transparent_Side_Get_Light},
	{"texture_index", Lua_Transparent_Side_Get_Texture_Index},
	{"texture_x", Lua_Transparent_Side_Get_Texture_X},
	{"texture_y", Lua_Transparent_Side_Get_Texture_Y},
	{"transfer_mode", Lua_Transparent_Side_Get_Transfer_Mode},
	{0, 0}
};

const luaL_Reg Lua_Transparent_Side_Set[] = {
	{"collection", Lua_Transparent_Side_Set_Collection},
	{"empty", Lua_Transparent_Side_Set_Empty},
	{"light", Lua_Transparent_Side_Set_Light},
	{"texture_index", Lua_Transparent_Side_Set_Texture_Index},
	{"texture_x", Lua_Transparent_Side_Set_Texture_X},
	{"texture_y", Lua_Transparent_Side_Set_Texture_Y},
	{"transfer_mode", Lua_Transparent_Side_Set_Transfer_Mode},
	{0, 0}
};

char Lua_SideType_Name[] = "side_type";
char Lua_SideTypes_Name[] = "SideTypes";

char Lua_Side_Name[] = "side";

int Lua_Side_Play_Sound(lua_State *L)
{
	short sound_code = Lua_Sound::ToIndex(L, 2);
	_fixed pitch = FIXED_ONE;
	if (lua_gettop(L) == 3)
	{
		if (!lua_isnumber(L, 3))
			return luaL_error(L, "play_sound: incorrect argument type");
		pitch = static_cast<_fixed>(lua_tonumber(L, 3) * FIXED_ONE);
	}

	_play_side_sound(Lua_Side::Index(L, 1), sound_code, pitch);
	return 0;
}

int Lua_Side_Recalculate_Type(lua_State *L)
{
	recalculate_side_type(Lua_Side::Index(L, 1));
	return 0;
}

static int Lua_Side_Get_Ambient_Delta(lua_State* L)
{
	auto side = get_side_data(Lua_Side::Index(L, 1));
	lua_pushnumber(L, static_cast<double>(side->ambient_delta) / FIXED_ONE);
	return 1;
}

static int Lua_Side_Get_Control_Panel(lua_State *L)
{
	int16 side_index = Lua_Side::Index(L, 1);
	side_data *side = get_side_data(side_index);
	if (SIDE_IS_CONTROL_PANEL(side))
	{
		Lua_Side_ControlPanel::Push(L, side_index);
	}
	else
	{
		lua_pushnil(L);
	}

	return 1;
}

static int Lua_Side_Get_Line(lua_State *L)
{
	Lua_Line::Push(L, get_side_data(Lua_Side::Index(L, 1))->line_index);
	return 1;
}

static int Lua_Side_Get_Polygon(lua_State *L)
{
	Lua_Polygon::Push(L, get_side_data(Lua_Side::Index(L, 1))->polygon_index);
	return 1;
}

static int Lua_Side_Get_Primary(lua_State *L)
{
	Lua_Primary_Side::Push(L, Lua_Side::Index(L, 1));
	return 1;
}

static int Lua_Side_Get_Secondary(lua_State *L)
{
	Lua_Secondary_Side::Push(L, Lua_Side::Index(L, 1));
	return 1;
}

static int Lua_Side_Get_Transparent(lua_State *L)
{
	Lua_Transparent_Side::Push(L, Lua_Side::Index(L, 1));
	return 1;
}

static int Lua_Side_Get_Type(lua_State *L)
{
	Lua_SideType::Push(L, get_side_data(Lua_Side::Index(L, 1))->type);
	return 1;
}

const luaL_Reg Lua_Side_Get[] = {
	{"ambient_delta", Lua_Side_Get_Ambient_Delta},
	{"control_panel", Lua_Side_Get_Control_Panel},
	{"line", Lua_Side_Get_Line},
	{"play_sound", L_TableFunction<Lua_Side_Play_Sound>},
	{"polygon", Lua_Side_Get_Polygon},
	{"primary", Lua_Side_Get_Primary},
	{"recalculate_type", L_TableFunction<Lua_Side_Recalculate_Type>},
	{"secondary", Lua_Side_Get_Secondary},
	{"transparent", Lua_Side_Get_Transparent},
	{"type", Lua_Side_Get_Type},
	{0, 0}
};

static int Lua_Side_Set_Ambient_Delta(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "ambient_delta: incorrect argument type");

	auto side = get_side_data(Lua_Side::Index(L, 1));
	side->ambient_delta = static_cast<int32_t>(lua_tonumber(L, 2) * FIXED_ONE);
	return 1;
}

static int Lua_Side_Set_Control_Panel(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "control_panel: incorrect argument type");
	
	side_data *side = get_side_data(Lua_Side::Index(L, 1));

	if (lua_toboolean(L, 2) != (side->flags & _side_is_control_panel))
	{
		// new control panel, or deleting old; clear either way
		side->control_panel_type = NONE;
		side->flags &= ~(_control_panel_status | _side_is_control_panel | _side_is_repair_switch | _side_is_destructive_switch | _side_is_lighted_switch | _side_switch_can_be_destroyed | _side_switch_can_only_be_hit_by_projectiles);
		
		if (lua_toboolean(L, 2))
			side->flags |= _side_is_control_panel;
	}

	return 0;
}

const luaL_Reg Lua_Side_Set[] = {
	{"ambient_delta", Lua_Side_Set_Ambient_Delta},
	{"control_panel", Lua_Side_Set_Control_Panel},
	{0, 0}
};
	

static bool Lua_Side_Valid(int16 index)
{
	return index >= 0 && index < SideList.size();
}

char Lua_Sides_Name[] = "Sides";
static int16 Lua_Sides_Length() { return SideList.size(); }

// Sides.new(polygon, line)
int Lua_Sides_New(lua_State *L)
{
	short polygon_index;
	if (lua_isnumber(L, 1))
	{
		polygon_index = static_cast<short>(lua_tonumber(L, 1));
		if (!Lua_Polygon::Valid(polygon_index))
			return luaL_error(L, "new: invalid polygon index");
	}
	else if (Lua_Polygon::Is(L, 1))
		polygon_index = Lua_Polygon::Index(L, 1);
	else
		return luaL_error(L, "new: incorrect argument type");

	short line_index;
	if (lua_isnumber(L, 2))
	{
		line_index = static_cast<short>(lua_tonumber(L, 2));
		if (!Lua_Line::Valid(line_index))
			return luaL_error(L, "new: invalid line index");
	}
	else if (Lua_Line::Is(L, 2))
		line_index = Lua_Line::Index(L, 2);
	else
		return luaL_error(L, "new: incorrect argument type");

	// make sure we don't assert
	line_data *line = get_line_data(line_index);
	if (!(line->clockwise_polygon_owner == polygon_index || line->counterclockwise_polygon_owner == polygon_index)) 
		return luaL_error(L, "new: line does not belong to polygon");

	if ((line->clockwise_polygon_owner == polygon_index && line->clockwise_polygon_side_index != NONE) || (line->counterclockwise_polygon_owner == polygon_index && line->counterclockwise_polygon_side_index != NONE))
		return luaL_error(L, "new: side already exists");
	
	Lua_Side::Push(L, new_side(polygon_index, line_index));
	return 1;
}

const luaL_Reg Lua_Sides_Methods[] = {
	{"new", L_TableFunction<Lua_Sides_New>},
	{0, 0}
};

char Lua_LightFunction_Name[] = "light_function";
typedef L_Enum<Lua_LightFunction_Name> Lua_LightFunction;

char Lua_LightFunctions_Name[] = "LightFunctions";
typedef L_EnumContainer<Lua_LightFunctions_Name, Lua_LightFunction> Lua_LightFunctions;

char Lua_LightPreset_Name[] = "light_preset";
typedef L_Enum<Lua_LightPreset_Name> Lua_LightPreset;

char Lua_LightPresets_Name[] = "LightPresets";
typedef L_EnumContainer<Lua_LightPresets_Name, Lua_LightPreset> Lua_LightPresets;

char Lua_LightState_Name[] = "light_state";
typedef L_Enum<Lua_LightState_Name> Lua_LightState;

char Lua_LightStates_Name[] = "LightStates";
typedef L_EnumContainer<Lua_LightStates_Name, Lua_LightState> Lua_LightStates;

char Lua_Light_State_Name[] = "light_state_subtable";

class Lua_Light_State : public L_Class<Lua_Light_State_Name>
{
public:
	int16 m_light_index;
	static Lua_Light_State* Push(lua_State* L, int16 light_index, int16 index);
	static int16 LightIndex(lua_State* L, int index);
};

Lua_Light_State* Lua_Light_State::Push(lua_State* L, int16 light_index, int16 index)
{
	Lua_Light_State* t = 0;

	if (!Lua_Light_State::Valid(index) || !Lua_Light::Valid(light_index))
	{
		lua_pushnil(L);
		return 0;
	}

	t = NewInstance<Lua_Light_State>(L, index);
	t->m_light_index = light_index;

	return t;
}

int16 Lua_Light_State::LightIndex(lua_State* L, int index)
{
	Lua_Light_State* t = static_cast<Lua_Light_State*>(Instance(L, index));
	if (!t) luaL_typerror(L, index, Lua_Light_State_Name);
	return t->m_light_index;
}

static lighting_function_specification* get_light_function_spec(int light_index, int state)
{
	light_data* light = get_light_data(light_index);
	switch (state)
	{
	case _light_becoming_active:
		return &light->static_data.becoming_active;
	case _light_primary_active:
		return &light->static_data.primary_active;
	case _light_secondary_active:
		return &light->static_data.secondary_active;
	case _light_becoming_inactive:
		return &light->static_data.becoming_inactive;
	case _light_primary_inactive:
		return &light->static_data.primary_inactive;
	case _light_secondary_inactive:
		return &light->static_data.secondary_inactive;
	default:
		assert(false);
	}
	
	return 0;
}

static int Lua_Light_State_Get_Delta_Intensity(lua_State* L)
{
	int light_index = Lua_Light_State::LightIndex(L, 1);
	lighting_function_specification* spec = get_light_function_spec(light_index, Lua_Light_State::Index(L, 1));
	lua_pushnumber(L, static_cast<double>(spec->delta_intensity) / FIXED_ONE);
	return 1;
}

static int Lua_Light_State_Get_Delta_Period(lua_State* L)
{
	int light_index = Lua_Light_State::LightIndex(L, 1);
	lighting_function_specification* spec = get_light_function_spec(light_index, Lua_Light_State::Index(L, 1));
	lua_pushnumber(L, static_cast<double>(spec->delta_period));
	return 1;
}

static int Lua_Light_State_Get_Function(lua_State* L)
{
	int light_index = Lua_Light_State::LightIndex(L, 1);
	lighting_function_specification* spec = get_light_function_spec(light_index, Lua_Light_State::Index(L, 1));
	Lua_LightFunction::Push(L, spec->function);
	return 1;
}

static int Lua_Light_State_Get_Intensity(lua_State* L)
{
	int light_index = Lua_Light_State::LightIndex(L, 1);
	lighting_function_specification* spec = get_light_function_spec(light_index, Lua_Light_State::Index(L, 1));
	lua_pushnumber(L, static_cast<double>(spec->intensity) / FIXED_ONE);
	return 1;
}

static int Lua_Light_State_Get_Period(lua_State* L)
{
	int light_index = Lua_Light_State::LightIndex(L, 1);
	lighting_function_specification* spec = get_light_function_spec(light_index, Lua_Light_State::Index(L, 1));
	lua_pushnumber(L, static_cast<double>(spec->period));
	return 1;
}


const luaL_Reg Lua_Light_State_Get[] = {
	{"delta_intensity", Lua_Light_State_Get_Delta_Intensity},
	{"delta_period", Lua_Light_State_Get_Delta_Period},
	{"intensity", Lua_Light_State_Get_Intensity},
	{"light_function", Lua_Light_State_Get_Function},
	{"period", Lua_Light_State_Get_Period},
	{0, 0}
};

static int Lua_Light_State_Set_Delta_Intensity(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "delta_intensity: incorrect argument type");

	lighting_function_specification* spec = get_light_function_spec(Lua_Light_State::LightIndex(L, 1), Lua_Light_State::Index(L, 1));
	spec->delta_intensity = static_cast<int32>(lua_tonumber(L, 2) * FIXED_ONE);
	return 1;
}

static int Lua_Light_State_Set_Delta_Period(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "delta_period: incorrect argument type");
	
	int16 period = lua_tonumber(L, 2);
	if (period < 0)
		return luaL_error(L, "delta_period: must be nonnegative");

	lighting_function_specification* spec = get_light_function_spec(Lua_Light_State::LightIndex(L, 1), Lua_Light_State::Index(L, 1));
	spec->delta_period = period;
	return 1;
}

static int Lua_Light_State_Set_Function(lua_State* L)
{
	int16 function = Lua_LightFunction::ToIndex(L, 2);
	lighting_function_specification* spec = get_light_function_spec(Lua_Light_State::LightIndex(L, 1), Lua_Light_State::Index(L, 1));
	spec->function = function;
	return 1;
}

static int Lua_Light_State_Set_Intensity(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "intensity: incorrect argument type");

	lighting_function_specification* spec = get_light_function_spec(Lua_Light_State::LightIndex(L, 1), Lua_Light_State::Index(L, 1));
	spec->intensity = static_cast<int32>(lua_tonumber(L, 2) * FIXED_ONE);
	return 1;
}

static int Lua_Light_State_Set_Period(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "period: incorrect argument type");
	
	int16 period = lua_tonumber(L, 2);
	if (period < 0)
		return luaL_error(L, "period: must be nonnegative");

	lighting_function_specification* spec = get_light_function_spec(Lua_Light_State::LightIndex(L, 1), Lua_Light_State::Index(L, 1));
	spec->period = period;
	return 1;
}

const luaL_Reg Lua_Light_State_Set[] = {
	{"delta_intensity", Lua_Light_State_Set_Delta_Intensity},
	{"delta_period", Lua_Light_State_Set_Delta_Period},
	{"intensity", Lua_Light_State_Set_Intensity},
	{"light_function", Lua_Light_State_Set_Function},
	{"period", Lua_Light_State_Set_Period},
	{0, 0}
};

char Lua_Light_States_Name[] = "light_states";
typedef L_Class<Lua_Light_States_Name> Lua_Light_States;

static int Lua_Light_States_Get(lua_State* L)
{
	int light_index = Lua_Light_States::Index(L, 1);
	int state_index = Lua_LightState::ToIndex(L, 2);
	Lua_Light_State::Push(L, light_index, state_index);
	return 1;
}

static int Lua_Light_States_Length(lua_State* L)
{
	lua_pushnumber(L, static_cast<int16>(_light_secondary_inactive) + 1);
	return 1;
}

const luaL_Reg Lua_Light_States_Metatable[] = {
	{"__index", Lua_Light_States_Get},
	{"__len", Lua_Light_States_Length},
	{0, 0}
};

char Lua_Light_Name[] = "light";

static int Lua_Light_Get_Active(lua_State *L)
{
	lua_pushboolean(L, get_light_status(Lua_Light::Index(L, 1)));
	return 1;
}

static int Lua_Light_Get_Intensity(lua_State* L)
{
	lua_pushnumber(L, static_cast<double>(get_light_intensity(Lua_Light::Index(L, 1))) / FIXED_ONE);
	return 1;
}

static int Lua_Light_Get_Initial_Phase(lua_State* L)
{
	light_data* data = get_light_data(Lua_Light::Index(L, 1));
	lua_pushnumber(L, static_cast<double>(data->static_data.phase));
	return 1;
}

static int Lua_Light_Get_Initially_Active(lua_State* L)
{
	light_data* data = get_light_data(Lua_Light::Index(L, 1));
	lua_pushboolean(L, data->static_data.flags & _light_is_initially_active);
	return 1;
}

static int Lua_Light_Get_States(lua_State *L)
{
	Lua_Light_States::Push(L, Lua_Light::Index(L, 1));
	return 1;
}

static int Lua_Light_Get_Tag(lua_State* L)
{
	light_data* data = get_light_data(Lua_Light::Index(L, 1));
	Lua_Tag::Push(L, data->static_data.tag);
	return 1;
}

const luaL_Reg Lua_Light_Get[] = {
	{"active", Lua_Light_Get_Active},
	{"initial_phase", Lua_Light_Get_Initial_Phase},
	{"initially_active", Lua_Light_Get_Initially_Active},
	{"intensity", Lua_Light_Get_Intensity},
	{"states", Lua_Light_Get_States},
	{"tag", Lua_Light_Get_Tag},
	{0, 0}
};

static int Lua_Light_Set_Active(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "active: incorrect argument type");

	size_t light_index = Lua_Light::Index(L, 1);
	bool active = lua_toboolean(L, 2);
	
	set_light_status(light_index, active);
	assume_correct_switch_position(_panel_is_light_switch, static_cast<short>(light_index), active);
	return 0;
}

static int Lua_Light_Set_Initially_Active(lua_State* L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "initially_active: incorrect argument type");
	
	light_data* data = get_light_data(Lua_Light::Index(L, 1));
	bool active = lua_toboolean(L, 2);
	if (active) 
	{
		data->static_data.flags |= _light_is_initially_active;
	} 
	else
	{
		data->static_data.flags &= ~_light_is_initially_active;
	}

	return 0;
}

static int Lua_Light_Set_Phase(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "phase: incorrect argument type");

	light_data* data = get_light_data(Lua_Light::Index(L, 1));
	data->static_data.phase = static_cast<int16>(lua_tonumber(L, 2));
	return 0;
}

static int Lua_Light_Set_Tag(lua_State* L)
{
	int16 tag = NONE;
	if (lua_isnumber(L, 2))
	{
		tag = static_cast<int16>(lua_tonumber(L, 2));
		if (!Lua_Tag::Valid(tag))
			return luaL_error(L, "tag: invalid tag index");
	}
	else if (Lua_Tag::Is(L, 2))
	{
		tag = Lua_Tag::Index(L, 2);
	} 
	else return luaL_error(L, "tag: incorrect argument type");

	light_data* data = get_light_data(Lua_Light::Index(L, 1));
	data->static_data.tag = tag;
	return 0;
}

const luaL_Reg Lua_Light_Set[] = {
	{"active", Lua_Light_Set_Active},
	{"initial_phase", Lua_Light_Set_Phase},
	{"initially_active", Lua_Light_Set_Initially_Active},
	{"tag", Lua_Light_Set_Tag},
	{0, 0}
};

bool Lua_Light_Valid(int16 index)
{
	return index >= 0 && index < MAXIMUM_LIGHTS_PER_MAP;
}

char Lua_Lights_Name[] = "Lights";
static int16 Lua_Lights_Length() { return LightList.size(); }

int Lua_Lights_New(lua_State* L)
{
	if (LightList.size() == INT16_MAX)
		return 0;

	short type;
	if (lua_isnil(L, 1))
	{
		type = _normal_light;
	}
	else
	{
		type = Lua_LightPreset::ToIndex(L, 1);
	}
	
	LightList.resize(LightList.size() + 1);
	short index = new_light(get_defaults_for_light_type(type));
	Lua_Light::Push(L, index);
	return 1;
}

const luaL_Reg Lua_Lights_Methods[] = {
	{"new", L_TableFunction<Lua_Lights_New>},
	{0, 0}
};

char Lua_Tag_Name[] = "tag";

static int Lua_Tag_Get_Active(lua_State *L)
{
	int tag = Lua_Tag::Index(L, 1);
	bool changed = false;

	size_t light_index;
	light_data *light;

	for (light_index= 0, light= lights; light_index<MAXIMUM_LIGHTS_PER_MAP && !changed; ++light_index, ++light)
	{
		if (light->static_data.tag==tag)
		{
			if (get_light_status(light_index))
			{
				changed= true;
			}
		}
	}

	short platform_index;
	platform_data *platform;

	for (platform_index= 0, platform= platforms; platform_index<dynamic_world->platform_count && !changed; ++platform_index, ++platform)
	{
		if (platform->tag==tag)
		{
			if (PLATFORM_IS_ACTIVE(platform))
			{
				changed= true;
			}
		}
	}

	lua_pushboolean(L, changed);
	return 1;
}

static int Lua_Tag_Set_Active(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "active: incorrect argument type");

	int16 tag = Lua_Tag::Index(L, 1);
	bool active = lua_toboolean(L, 2);

	set_tagged_light_statuses(tag, active);
	try_and_change_tagged_platform_states(tag, active);
	assume_correct_switch_position(_panel_is_tag_switch, tag, active);

	return 0;
}

const luaL_Reg Lua_Tag_Get[] = {
	{"active", Lua_Tag_Get_Active},
	{0, 0}
};

const luaL_Reg Lua_Tag_Set[] = {
	{"active", Lua_Tag_Set_Active},
	{0, 0}
};

bool Lua_Tag_Valid(int16 index) { return index >= 0; }

char Lua_Tags_Name[] = "Tags";

char Lua_Terminal_Name[] = "terminal";

extern short number_of_terminal_texts();

static bool Lua_Terminal_Valid(int16 index) 
{
	return index >= 0 && index < number_of_terminal_texts();
}

char Lua_Terminals_Name[] = "Terminals";

static int16 Lua_Terminals_Length() {
	return number_of_terminal_texts();
}

char Lua_MediaType_Name[] = "media_type";
typedef L_Enum<Lua_MediaType_Name> Lua_MediaType;

char Lua_MediaTypes_Name[] = "MediaTypes";
typedef L_EnumContainer<Lua_MediaTypes_Name, Lua_MediaType> Lua_MediaTypes;

char Lua_Media_Name[] = "media";

const float AngleConvert = 360/float(FULL_CIRCLE);

static int Lua_Media_Get_Direction(lua_State* L)
{
	media_data* media = get_media_data(Lua_Media::Index(L, 1));
	lua_pushnumber(L, static_cast<double>(media->current_direction) * AngleConvert);
	return 1;
}

static int Lua_Media_Get_Height(lua_State* L)
{
	media_data* media = get_media_data(Lua_Media::Index(L, 1));
	lua_pushnumber(L, static_cast<double>(media->height) / WORLD_ONE);
	return 1;
}

static int Lua_Media_Get_High(lua_State* L)
{
	media_data* media = get_media_data(Lua_Media::Index(L, 1));
	lua_pushnumber(L, static_cast<double>(media->high) / WORLD_ONE);
	return 1;
}

static int Lua_Media_Get_Light(lua_State* L)
{
	media_data* media = get_media_data(Lua_Media::Index(L, 1));
	Lua_Light::Push(L, media->light_index);
	return 1;
}

static int Lua_Media_Get_Low(lua_State* L)
{
	media_data* media = get_media_data(Lua_Media::Index(L, 1));
	lua_pushnumber(L, static_cast<double>(media->low) / WORLD_ONE);
	return 1;
}

static int Lua_Media_Get_Speed(lua_State* L)
{
	media_data* media = get_media_data(Lua_Media::Index(L, 1));
	lua_pushnumber(L, static_cast<double>(media->current_magnitude) / WORLD_ONE);
	return 1;
}

static int Lua_Media_Get_Type(lua_State *L)
{
	media_data *media = get_media_data(Lua_Media::Index(L, 1));
	Lua_MediaType::Push(L, media->type);
	return 1;
}

const luaL_Reg Lua_Media_Get[] = {
	{"direction", Lua_Media_Get_Direction},
	{"height", Lua_Media_Get_Height},
	{"high", Lua_Media_Get_High},
	{"light", Lua_Media_Get_Light},
	{"low", Lua_Media_Get_Low},
	{"speed", Lua_Media_Get_Speed},
	{"type", Lua_Media_Get_Type},
	{0, 0}
};

extern void update_one_media(size_t, bool);

static int Lua_Media_Set_Direction(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "direction: incorrect argument type");
	
	media_data* media = get_media_data(Lua_Media::Index(L, 1));
	media->current_direction = static_cast<angle>(lua_tonumber(L, 2) / AngleConvert);
	return 0;
}

static int Lua_Media_Set_High(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "high: incorrect argument type");

	int media_index = Lua_Media::Index(L, 1);
	media_data* media = get_media_data(media_index);
	media->high = static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE);
	update_one_media(media_index, true);
	return 0;
}

static int Lua_Media_Set_Light(lua_State* L)
{
	int light_index;
	if (lua_isnumber(L, 2))
	{
		light_index = static_cast<int>(lua_tonumber(L, 2));
		if (!Lua_Light::Valid(light_index))
			return luaL_error(L, "light: invalid light index");
	}
	else if (Lua_Light::Is(L, 2))
	{
		light_index = Lua_Light::Index(L, 2);
	}
	else
	{
		return luaL_error(L, "light: incorrect argument type");
	}

	int media_index = Lua_Media::Index(L, 1);
	media_data* media = get_media_data(media_index);
	media->light_index = light_index;
	update_one_media(media_index, true);
	return 0;
}

static int Lua_Media_Set_Low(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "high: incorrect argument type");

	int media_index = Lua_Media::Index(L, 1);
	media_data* media = get_media_data(media_index);
	media->low = static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE);
	update_one_media(media_index, true);
	return 0;
}

static int Lua_Media_Set_Speed(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "speed: incorrect argument type");

	media_data* media = get_media_data(Lua_Media::Index(L, 1));
	media->current_magnitude = static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE);
	return 0;
}

static int Lua_Media_Set_Type(lua_State* L)
{
	int media_index = Lua_Media::Index(L, 1);
	media_data* media = get_media_data(media_index);
	media->type = Lua_MediaType::ToIndex(L, 2);
	update_one_media(media_index, true);
	return 0;
}

const luaL_Reg Lua_Media_Set[] = {
	{"direction", Lua_Media_Set_Direction},
	{"high", Lua_Media_Set_High},
	{"light", Lua_Media_Set_Light},
	{"low", Lua_Media_Set_Low},
	{"speed", Lua_Media_Set_Speed},
	{"type", Lua_Media_Set_Type},
	{0, 0}
};

static bool Lua_Media_Valid(int16 index)
{
	return index >= 0 && index< MAXIMUM_MEDIAS_PER_MAP;
}

char Lua_Medias_Name[] = "Media";
static int16 Lua_Medias_Length() { return MediaList.size(); }

int Lua_Medias_New(lua_State* L)
{
	if (MediaList.size() == INT16_MAX)
		return 0;

	struct media_data data{0};
	MediaList.resize(MediaList.size() + 1);
	short index = new_media(&data);
	Lua_Media::Push(L, index);
	return 1;
}

const luaL_Reg Lua_Medias_Methods[] = {
	{"new", L_TableFunction<Lua_Medias_New>},
	{0, 0}
};

char Lua_Annotation_Name[] = "annotation";
typedef L_Class<Lua_Annotation_Name> Lua_Annotation;

static int Lua_Annotation_Get_Polygon(lua_State *L)
{
	Lua_Polygon::Push(L, MapAnnotationList[Lua_Annotation::Index(L, 1)].polygon_index);
	return 1;
}

static int Lua_Annotation_Get_Text(lua_State *L)
{
	lua_pushstring(L, MapAnnotationList[Lua_Annotation::Index(L, 1)].text);
	return 1;
}

static int Lua_Annotation_Get_X(lua_State *L)
{
	lua_pushnumber(L, (double) MapAnnotationList[Lua_Annotation::Index(L, 1)].location.x / WORLD_ONE);
	return 1;
}

static int Lua_Annotation_Get_Y(lua_State *L)
{
	lua_pushnumber(L, (double) MapAnnotationList[Lua_Annotation::Index(L, 1)].location.y / WORLD_ONE);
	return 1;
}

const luaL_Reg Lua_Annotation_Get[] = {
	{"polygon", Lua_Annotation_Get_Polygon},
	{"text", Lua_Annotation_Get_Text},
	{"x", Lua_Annotation_Get_X},
	{"y", Lua_Annotation_Get_Y},
	{0, 0}
};

static int Lua_Annotation_Set_Polygon(lua_State *L)
{
	int polygon_index = NONE;
	if (lua_isnil(L, 2))
	{
		polygon_index = NONE;
	}
	else
	{
		polygon_index = Lua_Polygon::Index(L, 2);
	}

	MapAnnotationList[Lua_Annotation::Index(L, 1)].polygon_index = polygon_index;
	return 0;
}

static int Lua_Annotation_Set_Text(lua_State *L)
{
	if (!lua_isstring(L, 2))
		return luaL_error(L, "text: incorrect argument type");

	int annotation_index = Lua_Annotation::Index(L, 1);
	strncpy(MapAnnotationList[annotation_index].text, lua_tostring(L, 2), MAXIMUM_ANNOTATION_TEXT_LENGTH);
	MapAnnotationList[annotation_index].text[MAXIMUM_ANNOTATION_TEXT_LENGTH-1] = '\0';
	return 0;
}

static int Lua_Annotation_Set_X(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "x: incorrect argument type");
	
	MapAnnotationList[Lua_Annotation::Index(L, 1)].location.x = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	return 0;
}

static int Lua_Annotation_Set_Y(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "y: incorrect argument type");

	MapAnnotationList[Lua_Annotation::Index(L, 1)].location.y = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	return 0;
}

const luaL_Reg Lua_Annotation_Set[] = {
	{"polygon", Lua_Annotation_Set_Polygon},
	{"text", Lua_Annotation_Set_Text},
	{"x", Lua_Annotation_Set_X},
	{"y", Lua_Annotation_Set_Y},
	{0, 0}
};

bool Lua_Annotation_Valid(int16 index)
{
	return index >= 0 && index < MapAnnotationList.size();
}

char Lua_Annotations_Name[] = "Annotations";
typedef L_Container<Lua_Annotations_Name, Lua_Annotation> Lua_Annotations;

// Annotations.new(polygon, text, [x, y])
int Lua_Annotations_New(lua_State *L)
{
	if (dynamic_world->default_annotation_count == INT16_MAX)
		return luaL_error(L, "new: annotation limit reached");

	if (!lua_isstring(L, 2))
		return luaL_error(L, "new: incorrect argument type");

	map_annotation annotation;
	annotation.type = 0;

	if (lua_isnil(L, 1))
	{
		annotation.polygon_index = NONE;
	}
	else
	{
		annotation.polygon_index = Lua_Polygon::Index(L, 1);
	}

	int x = 0, y = 0;
	if (annotation.polygon_index != NONE)
	{
		polygon_data *polygon = get_polygon_data(annotation.polygon_index);
		x = polygon->center.x;
		y = polygon->center.y;
	}
		
	annotation.location.x = luaL_optint(L, 3, x);
	annotation.location.y = luaL_optint(L, 4, y);

	strncpy(annotation.text, lua_tostring(L, 2), MAXIMUM_ANNOTATION_TEXT_LENGTH);
	annotation.text[MAXIMUM_ANNOTATION_TEXT_LENGTH-1] = '\0';
	
	MapAnnotationList.push_back(annotation);
	dynamic_world->default_annotation_count++;
	Lua_Annotation::Push(L, MapAnnotationList.size() - 1);
	return 1;
}

const luaL_Reg Lua_Annotations_Methods[] = {
	{"new", L_TableFunction<Lua_Annotations_New>},
	{0, 0}
};

int16 Lua_Annotations_Length() {
	return MapAnnotationList.size();
}

char Lua_Fog_Color_Name[] = "fog_color";
typedef L_Class<Lua_Fog_Color_Name> Lua_Fog_Color;

static int Lua_Fog_Color_Get_R(lua_State *L)
{
	lua_pushnumber(L, (float) (OGL_GetFogData(Lua_Fog_Color::Index(L, 1))->Color.red) / 65535);
	return 1;
}

static int Lua_Fog_Color_Get_G(lua_State *L)
{
	lua_pushnumber(L, (float) (OGL_GetFogData(Lua_Fog_Color::Index(L, 1))->Color.green) / 65535);
	return 1;
}

static int Lua_Fog_Color_Get_B(lua_State *L)
{
	lua_pushnumber(L, (float) (OGL_GetFogData(Lua_Fog_Color::Index(L, 1))->Color.blue) / 65535);
	return 1;
}

static int Lua_Fog_Color_Set_R(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		luaL_error(L, "r: incorrect argument type");

	float color = static_cast<float>(lua_tonumber(L, 2));
	OGL_GetFogData(Lua_Fog_Color::Index(L, 1))->Color.red = PIN(int(65535 * color + 0.5), 0, 65535);
	return 0;
}

static int Lua_Fog_Color_Set_G(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		luaL_error(L, "g: incorrect argument type");

	float color = static_cast<float>(lua_tonumber(L, 2));
	OGL_GetFogData(Lua_Fog_Color::Index(L, 1))->Color.green = PIN(int(65535 * color + 0.5), 0, 65535);
	return 0;
}

static int Lua_Fog_Color_Set_B(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		luaL_error(L, "b: incorrect argument type");

	float color = static_cast<float>(lua_tonumber(L, 2));
	OGL_GetFogData(Lua_Fog_Color::Index(L, 1))->Color.blue = PIN(int(65535 * color + 0.5), 0, 65535);
	return 0;
}

const luaL_Reg Lua_Fog_Color_Get[] = {
	{"r", Lua_Fog_Color_Get_R},
	{"g", Lua_Fog_Color_Get_G},
	{"b", Lua_Fog_Color_Get_B},
	{0, 0}
};

const luaL_Reg Lua_Fog_Color_Set[] = {
	{"r", Lua_Fog_Color_Set_R},
	{"g", Lua_Fog_Color_Set_G},
	{"b", Lua_Fog_Color_Set_B},
	{0, 0}
};

char Lua_Fog_Name[] = "fog";
typedef L_Class<Lua_Fog_Name> Lua_Fog;

static int Lua_Fog_Get_Active(lua_State *L)
{
	lua_pushboolean(L, OGL_GetFogData(Lua_Fog::Index(L, 1))->IsPresent);
	return 1;
}

static int Lua_Fog_Get_Affects_Landscapes(lua_State *L)
{
	lua_pushboolean(L, OGL_GetFogData(Lua_Fog::Index(L, 1))->AffectsLandscapes);
	return 1;
}

static int Lua_Fog_Get_Color(lua_State *L)
{
	Lua_Fog_Color::Push(L, Lua_Fog::Index(L, 1));
	return 1;
}

static int Lua_Fog_Get_Depth(lua_State *L)
{
	lua_pushnumber(L, OGL_GetFogData(Lua_Fog::Index(L, 1))->Depth);
	return 1;
}

const luaL_Reg Lua_Fog_Get[] = {
	{"active", Lua_Fog_Get_Active},
	{"affects_landscapes", Lua_Fog_Get_Affects_Landscapes},
	{"color", Lua_Fog_Get_Color},
	{"depth", Lua_Fog_Get_Depth},
	{"present", Lua_Fog_Get_Active},
	{0, 0}
};

static int Lua_Fog_Set_Active(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "active: incorrect argument type");
	
	OGL_GetFogData(Lua_Fog::Index(L, 1))->IsPresent = static_cast<bool>(lua_toboolean(L, 2));
	return 0;
}

static int Lua_Fog_Set_Affects_Landscapes(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "affects_landscapes: incorrect argument type");
	
	OGL_GetFogData(Lua_Fog::Index(L, 1))->AffectsLandscapes = static_cast<bool>(lua_toboolean(L, 2));
	return 0;
}

static int Lua_Fog_Set_Depth(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "depth: incorrect argument type");

	OGL_GetFogData(Lua_Fog::Index(L, 1))->Depth = static_cast<float>(lua_tonumber(L, 2));
	return 0;
}

const luaL_Reg Lua_Fog_Set[] = {
	{"active", Lua_Fog_Set_Active},
	{"affects_landscapes", Lua_Fog_Set_Affects_Landscapes},
	{"depth", Lua_Fog_Set_Depth},
	{"present", Lua_Fog_Set_Active},
	{0, 0}
};


char Lua_Level_Stash_Name[] = "LevelStash";
typedef L_Class<Lua_Level_Stash_Name> Lua_Level_Stash;

extern std::unordered_map<std::string, std::string> lua_stash;

static int Lua_Level_Stash_Get(lua_State* L)
{
        if (!lua_isstring(L, 2))
                return luaL_error(L, "stash: incorrect argument type");
        
        auto it = lua_stash.find(lua_tostring(L, 2));
        if (it != lua_stash.end())
        {
                lua_pushlstring(L, it->second.data(), it->second.size());
        }
        else
        {
                lua_pushnil(L);
        }

        return 1;
}

static int Lua_Level_Stash_Set(lua_State* L)
{
        if (!lua_isstring(L, 3) && !lua_isnil(L, 3))
                return luaL_error(L, "stash: incorrect argument type");

        auto key = lua_tostring(L, 2);
        if (lua_isstring(L, 3))
        {
                size_t len;
                auto s = lua_tolstring(L, 3, &len);
                lua_stash[key] = std::string{s, len};
        }
        else
        {
                lua_stash.erase(key);
        }

        return 0;
}

const luaL_Reg Lua_Level_Stash_Metatable[] = {
        {"__index", Lua_Level_Stash_Get},
        {"__newindex", Lua_Level_Stash_Set},
        {0, 0}
};

char Lua_Level_Name[] = "Level";
typedef L_Class<Lua_Level_Name> Lua_Level;

char Lua_CompletionState_Name[] = "completion_state";
typedef L_Enum<Lua_CompletionState_Name> Lua_CompletionState;

char Lua_CompletionStates_Name[] = "CompletionStates";
typedef L_EnumContainer<Lua_CompletionStates_Name, Lua_CompletionState> Lua_CompletionStates;

int Lua_Level_Calculate_Completion_State(lua_State *L)
{
	Lua_CompletionState::Push(L, calculate_level_completion_state());
	return 1;
}

int Lua_Level_Get_Completed(lua_State* L)
{
	lua_pushboolean(L, get_game_state() == _change_level);
	return 1;
}

template<int16 flag>
static int Lua_Level_Get_Environment_Flag(lua_State *L)
{
	lua_pushboolean(L, static_world->environment_flags & flag);
	return 1;
}

template<int16 flag>
static int Lua_Level_Get_Mission_Flag(lua_State *L)
{
	lua_pushboolean(L, static_world->mission_flags & flag);
	return 1;
}

static int Lua_Level_Get_Fog(lua_State *L)
{
	Lua_Fog::Push(L, OGL_Fog_AboveLiquid);
	return 1;
}
	
static int Lua_Level_Get_Name(lua_State *L)
{
	lua_pushstring(L, static_world->level_name);
	return 1;
}

static int Lua_Level_Get_Index(lua_State *L)
{
	lua_pushinteger(L, dynamic_world->current_level_number);
	return 1;
}

static int Lua_Level_Get_Map_Checksum(lua_State *L)
{
#if !defined(DISABLE_NETWORKING)
	if (game_is_networked)
		lua_pushinteger(L, ((game_info *) NetGetGameData())->parent_checksum);
	else
#endif
		lua_pushinteger(L, get_current_map_checksum());
	return 1;
}

static int Lua_Level_Get_Stash(lua_State* L)
{
        Lua_Level_Stash::Push(L, 0);
        return 1;
}

static int Lua_Level_Get_Underwater_Fog(lua_State *L)
{
	Lua_Fog::Push(L, OGL_Fog_BelowLiquid);
	return 1;
}

const luaL_Reg Lua_Level_Get[] = {
	{"calculate_completion_state", L_TableFunction<Lua_Level_Calculate_Completion_State>},
	{"completed", Lua_Level_Get_Completed},
	{"extermination", Lua_Level_Get_Mission_Flag<_mission_extermination>},
	{"exploration", Lua_Level_Get_Mission_Flag<_mission_exploration>},
	{"fog", Lua_Level_Get_Fog},
	{"low_gravity", Lua_Level_Get_Environment_Flag<_environment_low_gravity>},
	{"magnetic", Lua_Level_Get_Environment_Flag<_environment_magnetic>},
	{"name", Lua_Level_Get_Name},
	{"index", Lua_Level_Get_Index},
	{"map_checksum", Lua_Level_Get_Map_Checksum},
	{"rebellion", Lua_Level_Get_Environment_Flag<_environment_rebellion>},
	{"retrieval", Lua_Level_Get_Mission_Flag<_mission_retrieval>},
	{"repair", Lua_Level_Get_Mission_Flag<_mission_repair>},
	{"rescue", Lua_Level_Get_Mission_Flag<_mission_rescue>},
        {"stash", Lua_Level_Get_Stash},
	{"underwater_fog", Lua_Level_Get_Underwater_Fog},
	{"vacuum", Lua_Level_Get_Environment_Flag<_environment_vacuum>},
	{0, 0}
};

char Lua_TransferMode_Name[] = "transfer_mode";
char Lua_TransferModes_Name[] = "TransferModes";

static void compatibility(lua_State *L);
#define NUMBER_OF_CONTROL_PANEL_DEFINITIONS 54

extern bool collection_loaded(short);

int Lua_Map_register(lua_State *L)
{
	Lua_AmbientSound::Register(L, 0, 0, 0, Lua_AmbientSound_Mnemonics);
	Lua_AmbientSound::Valid = Lua_AmbientSound::ValidRange(NUMBER_OF_AMBIENT_SOUND_DEFINITIONS);
	Lua_AmbientSounds::Register(L);
	Lua_AmbientSounds::Length = Lua_AmbientSounds::ConstantLength(NUMBER_OF_AMBIENT_SOUND_DEFINITIONS);

	Lua_Collection::Register(L, Lua_Collection_Get, 0, 0, Lua_Collection_Mnemonics);
	Lua_Collection::Valid = collection_loaded;
	Lua_Collections::Register(L);
	Lua_Collections::Length = Lua_Collections::ConstantLength(MAXIMUM_COLLECTIONS);

	Lua_CompletionState::Register(L, 0, 0, 0, Lua_CompletionState_Mnemonics);
	Lua_CompletionState::Valid = Lua_CompletionState::ValidRange(static_cast<int16>(_level_failed) + 1);
	Lua_CompletionStates::Register(L);
	Lua_CompletionStates::Length = Lua_CompletionStates::ConstantLength(static_cast<int16>(_level_failed) + 1);	

	Lua_ControlPanelClass::Register(L, 0, 0, 0, Lua_ControlPanelClass_Mnemonics);
	Lua_ControlPanelClass::Valid = Lua_ControlPanelClass::ValidRange(NUMBER_OF_CONTROL_PANELS);

	Lua_ControlPanelClasses::Register(L);
	Lua_ControlPanelClasses::Length = Lua_ControlPanelClasses::ConstantLength(NUMBER_OF_CONTROL_PANELS);

	Lua_ControlPanelType::Register(L, Lua_ControlPanelType_Get);
	Lua_ControlPanelType::Valid = Lua_ControlPanelType::ValidRange(NUMBER_OF_CONTROL_PANEL_DEFINITIONS);
	
	Lua_ControlPanelTypes::Register(L);
	Lua_ControlPanelTypes::Length = Lua_ControlPanelTypes::ConstantLength(NUMBER_OF_CONTROL_PANEL_DEFINITIONS);

	Lua_DamageType::Register(L, 0, 0, 0, Lua_DamageType_Mnemonics);
	Lua_DamageType::Valid = Lua_DamageType::ValidRange(NUMBER_OF_DAMAGE_TYPES);
	Lua_DamageTypes::Register(L);
	Lua_DamageTypes::Length = Lua_DamageTypes::ConstantLength(NUMBER_OF_DAMAGE_TYPES);

	Lua_Endpoint::Register(L, Lua_Endpoint_Get);
	Lua_Endpoint::Valid = Lua_Endpoint_Valid;

	Lua_Endpoints::Register(L);
	// CodeWarrior doesn't let me do this:
//	Lua_Endpoints::Length = boost::bind(&std::vector<endpoint_data>::size, &EndpointList);
	Lua_Endpoints::Length = Lua_Endpoints_Length;

	Lua_Line_Endpoints::Register(L, 0, 0, Lua_Line_Endpoints_Metatable);
	
	Lua_Line::Register(L, Lua_Line_Get, Lua_Line_Set);
	Lua_Line::Valid = Lua_Line_Valid;

	Lua_Lines::Register(L);
	Lua_Lines::Length = Lua_Lines_Length;

	Lua_Platform::Register(L, Lua_Platform_Get, Lua_Platform_Set);
	Lua_Platform::Valid = Lua_Platform_Valid;

	Lua_Platforms::Register(L);
	Lua_Platforms::Length = Lua_Platforms_Length;

	Lua_PlatformType::Register(L, 0, 0, 0, Lua_PlatformType_Mnemonics);
	Lua_PlatformType::Valid = Lua_PlatformType::ValidRange(NUMBER_OF_PLATFORM_TYPES);
	
	Lua_PlatformTypes::Register(L);
	Lua_PlatformTypes::Length = Lua_PlatformTypes::ConstantLength(NUMBER_OF_PLATFORM_TYPES);

	Lua_Polygon_Floor::Register(L, Lua_Polygon_Floor_Get, Lua_Polygon_Floor_Set);
	Lua_Polygon_Floor::Valid = Lua_Polygon_Valid;

	Lua_Polygon_Ceiling::Register(L, Lua_Polygon_Ceiling_Get, Lua_Polygon_Ceiling_Set);
	Lua_Polygon_Ceiling::Valid = Lua_Polygon_Valid;

	Lua_PolygonType::Register(L, 0, 0, 0, Lua_PolygonType_Mnemonics);
	Lua_PolygonType::Valid = Lua_PolygonType::ValidRange(static_cast<int16>(_polygon_is_superglue) + 1);
	
	Lua_PolygonTypes::Register(L);
	Lua_PolygonTypes::Length = Lua_PolygonTypes::ConstantLength(static_cast<int16>(_polygon_is_superglue) + 1);

	Lua_Adjacent_Polygons::Register(L, 0, 0, Lua_Adjacent_Polygons_Metatable);
	Lua_Polygon_Endpoints::Register(L, 0, 0, Lua_Polygon_Endpoints_Metatable);
	Lua_Polygon_Lines::Register(L, 0, 0, Lua_Polygon_Lines_Metatable);
	Lua_Polygon_Sides::Register(L, 0, 0, Lua_Polygon_Sides_Metatable);

	Lua_Polygon::Register(L, Lua_Polygon_Get, Lua_Polygon_Set);
	Lua_Polygon::Valid = Lua_Polygon_Valid;

	Lua_Polygons::Register(L);
	Lua_Polygons::Length = Lua_Polygons_Length;

	Lua_Side_ControlPanel::Register(L, Lua_Side_ControlPanel_Get, Lua_Side_ControlPanel_Set);

	Lua_Primary_Side::Register(L, Lua_Primary_Side_Get, Lua_Primary_Side_Set);
	Lua_Secondary_Side::Register(L, Lua_Secondary_Side_Get, Lua_Secondary_Side_Set);
	Lua_Transparent_Side::Register(L, Lua_Transparent_Side_Get, Lua_Transparent_Side_Set);

	Lua_Side::Register(L, Lua_Side_Get, Lua_Side_Set);
	Lua_Side::Valid = Lua_Side_Valid;

	Lua_Sides::Register(L, Lua_Sides_Methods);
	Lua_Sides::Length = Lua_Sides_Length;

	Lua_SideType::Register(L, 0, 0, 0, Lua_SideType_Mnemonics);
	Lua_SideType::Valid = Lua_SideType::ValidRange(static_cast<int16>(_split_side) + 1);
	Lua_SideTypes::Register(L);
	Lua_SideTypes::Length = Lua_SideTypes::ConstantLength(static_cast<int16>(_split_side) + 1);

	Lua_Light_State::Register(L, Lua_Light_State_Get, Lua_Light_State_Set);
	Lua_Light_State::Valid = Lua_Light_State::ValidRange(static_cast<int16>(_light_secondary_inactive) + 1);

	Lua_Light_States::Register(L, 0, 0, Lua_Light_States_Metatable);
	Lua_Light_States::Valid = Lua_Light_Valid;

	Lua_LightPreset::Register(L, 0, 0, 0, Lua_LightPreset_Mnemonics);
	Lua_LightPreset::Valid = Lua_LightPreset::ValidRange(NUMBER_OF_LIGHT_TYPES);

	Lua_LightPresets::Register(L);
	Lua_LightPresets::Length = Lua_LightPresets::ConstantLength(NUMBER_OF_LIGHT_TYPES);

	Lua_LightFunction::Register(L, 0, 0, 0, Lua_LightFunction_Mnenonics);
	Lua_LightFunction::Valid = Lua_LightFunction::ValidRange(NUMBER_OF_LIGHTING_FUNCTIONS);
	
	Lua_LightFunctions::Register(L);
	Lua_LightFunctions::Length = Lua_LightFunctions::ConstantLength(NUMBER_OF_LIGHTING_FUNCTIONS);

	Lua_LightState::Register(L, 0, 0, 0, Lua_LightState_Mnemonics);
	Lua_LightState::Valid = Lua_LightState::ValidRange(static_cast<int16>(_light_secondary_inactive) + 1);
	
	Lua_LightStates::Register(L);
	Lua_LightStates::Length = Lua_LightStates::ConstantLength(static_cast<int16>(_light_secondary_inactive) + 1);

	Lua_Light::Register(L, Lua_Light_Get, Lua_Light_Set);
	Lua_Light::Valid = Lua_Light_Valid;

	Lua_Lights::Register(L, Lua_Lights_Methods);
	Lua_Lights::Length = Lua_Lights_Length;
		
	Lua_Tag::Register(L, Lua_Tag_Get, Lua_Tag_Set);
	Lua_Tag::Valid = Lua_Tag_Valid;

	Lua_Tags::Register(L);
	Lua_Tags::Length = Lua_Tags::ConstantLength(INT16_MAX);
	
	Lua_Terminal::Register(L);
	Lua_Terminal::Valid = Lua_Terminal_Valid;
	
	Lua_Terminals::Register(L);
	Lua_Terminals::Length = Lua_Terminals_Length;

	Lua_TransferMode::Register(L, 0, 0, 0, Lua_TransferMode_Mnemonics);
	Lua_TransferMode::Valid = Lua_TransferMode::ValidRange(NUMBER_OF_TRANSFER_MODES);
	
	Lua_TransferModes::Register(L);
	Lua_TransferModes::Length = Lua_TransferModes::ConstantLength(NUMBER_OF_TRANSFER_MODES);

	Lua_MediaType::Register(L, 0, 0, 0, Lua_MediaType_Mnemonics);
	Lua_MediaType::Valid = Lua_MediaType::ValidRange(NUMBER_OF_MEDIA_TYPES);

	Lua_MediaTypes::Register(L);
	Lua_MediaTypes::Length = Lua_MediaTypes::ConstantLength(NUMBER_OF_MEDIA_TYPES);

	Lua_Media::Register(L, Lua_Media_Get, Lua_Media_Set);
	Lua_Media::Valid = Lua_Media_Valid;

	Lua_Medias::Register(L, Lua_Medias_Methods);
	Lua_Medias::Length = Lua_Medias_Length;

        Lua_Level_Stash::Register(L, 0, 0, Lua_Level_Stash_Metatable);

	Lua_Level::Register(L, Lua_Level_Get);

	Lua_Annotation::Register(L, Lua_Annotation_Get, Lua_Annotation_Set);
	Lua_Annotation::Valid = Lua_Annotation_Valid;

	Lua_Annotations::Register(L, Lua_Annotations_Methods);
	Lua_Annotations::Length = Lua_Annotations_Length;

	Lua_Fog::Register(L, Lua_Fog_Get, Lua_Fog_Set);

	Lua_Fog_Color::Register(L, Lua_Fog_Color_Get, Lua_Fog_Color_Set);

	// register one Level userdatum globally
	Lua_Level::Push(L, 0);
	lua_setglobal(L, Lua_Level_Name);

	compatibility(L);
	return 0;
}

static const char* compatibility_script = ""
	"function annotations() local i = 0 local n = # Annotations return function() if i < n then a = Annotations[i] i = i + 1 if a.polygon then p = a.polygon.index else p = -1 end return a.text, p, a.x, a.y else return nil end end end\n"
	"function get_all_fog_attributes() local attributes = { OGL_Fog_AboveLiquid = { Depth = Level.fog.depth, Color = { red = Level.fog.color.r, blue = Level.fog.color.b, green = Level.fog.color.g }, IsPresent = Level.fog.present, AffectsLandscapes = Level.fog.affects_landscapes }, OGL_Fog_BelowLiquid = { Depth = Level.underwater_fog.depth, Color = { red = Level.underwater_fog.color.r, green = Level.underwater_fog.color.g, blue = Level.underwater_fog.color.b, }, IsPresent = Level.underwater_fog.present, AffectsLandscapes = Level.underwater_fog.affects_landscapes } } return attributes end\n"
	"function get_fog_affects_landscapes() return Level.fog.affects_landscapes end\n"
	"function get_fog_color() return Level.fog.color.r, Level.fog.color.g, Level.fog.color.b end\n"
	"function get_fog_depth() return Level.fog.depth end\n"
	"function get_fog_present() return Level.fog.active end\n"
	"function get_level_name() return Level.name end\n"
	"function get_light_state(light) return Lights[light].active end\n"
	"function get_map_environment() return Level.vacuum, Level.magnetic, Level.rebellion, Level.low_gravity end\n"
	"function get_platform_ceiling_height(polygon) if Polygons[polygon].platform then return Polygons[polygon].platform.ceiling_height end end\n"
	"function get_platform_floor_height(polygon) if Polygons[polygon].platform then return Polygons[polygon].platform.floor_height end end\n"
	"function get_platform_index(polygon) if Polygons[polygon].platform then return Polygons[polygon].platform.index else return -1 end end\n"
	"function get_polygon_media(polygon) if Polygons[polygon].media then return Polygons[polygon].media.index else return -1 end end\n"
	"function get_platform_monster_control(polygon) if Polygons[polygon].platform then return Polygons[polygon].platform.monster_controllable end end\n"
	"function get_platform_movement(polygon) if Polygons[polygon].platform then return Polygons[polygon].platform.extending end end\n"
	"function get_platform_player_control(polygon) if Polygons[polygon].platform then return Polygons[polygon].platform.player_controllable end end\n"
	"function get_platform_speed(polygon) if Polygons[polygon].platform then return Polygons[polygon].platform.speed * 1024 end end\n"
	"function get_platform_state(polygon) if Polygons[polygon].platform then return Polygons[polygon].platform.active end end\n"
	"function get_polygon_ceiling_height(polygon) return Polygons[polygon].ceiling.height end\n"
	"function get_polygon_center(polygon) return Polygons[polygon].x * 1024, Polygons[polygon].y * 1024 end\n"
	"function get_polygon_floor_height(polygon) return Polygons[polygon].floor.height end\n"
	"function get_polygon_target(polygon) return Polygons[polygon].permutation end\n"
	"function get_polygon_type(polygon) return Polygons[polygon].type.index end\n"
	"function get_tag_state(tag) return Tags[tag].active end\n"
	"function get_terminal_text_number(poly, line) if Lines[line].ccw_polygon == Polygons[poly] then s = Lines[line].ccw_side elseif Lines[line].cw_polygon == Polygons[poly] then s = Lines[line].cw_side else return line end if s.control_panel and s.control_panel.type.class == 8 then return s.control_panel.permutation else return line end end\n"
	"function get_underwater_fog_affects_landscapes() return Level.underwater_fog.affects_landscapes end\n"
	"function get_underwater_fog_color() return Level.underwater_fog.color.r, Level.underwater_fog.color.g, Level.underwater_fog.color.b end\n"
	"function get_underwater_fog_depth() return Level.underwater_fog.depth end\n"
	"function get_underwater_fog_present() return Level.underwater_fog.active end\n"

	"function number_of_polygons() return # Polygons end\n"
	"function select_monster(type, poly) for m in Polygons[poly]:monsters() do if m.type == type and m.visible and (m.action < 6 or m.action > 9) then return m.index end end return nil end\n"
	"function set_all_fog_attributes(t) Level.fog.depth = t.OGL_Fog_AboveLiquid.Depth Level.fog.present = t.OGL_Fog_AboveLiquid.IsPresent Level.fog.affects_landscapes = t.OGL_Fog_AboveLiquid.AffectsLandscapes Level.fog.color.r = t.OGL_Fog_AboveLiquid.Color.red Level.fog.color.g = t.OGL_Fog_AboveLiquid.Color.green Level.fog.color.b = t.OGL_Fog_AboveLiquid.Color.blue Level.underwater_fog.depth = t.OGL_Fog_BelowLiquid.Depth Level.underwater_fog.present = t.OGL_Fog_BelowLiquid.IsPresent Level.underwater_fog.affects_landscapes = t.OGL_Fog_BelowLiquid.AffectsLandscapes Level.underwater_fog.color.r = t.OGL_Fog_BelowLiquid.Color.red Level.underwater_fog.color.g = t.OGL_Fog_BelowLiquid.Color.green Level.underwater_fog.color.b = t.OGL_Fog_BelowLiquid.Color.blue end\n"
	"function set_fog_affects_landscapes(affects_landscapes) Level.fog.affects_landscapes = affects_landscapes end\n"
	"function set_fog_color(r, g, b) Level.fog.color.r = r Level.fog.color.g = g Level.fog.color.b = b end\n"
	"function set_fog_depth(depth) Level.fog.depth = depth end\n"
	"function set_fog_present(present) Level.fog.active = present end\n"
	"function set_light_state(light, state) Lights[light].active = state end\n"
	"function set_platform_ceiling_height(polygon, height) if Polygons[polygon].platform then Polygons[polygon].platform.ceiling_height = height end end\n"
	"function set_platform_floor_height(polygon, height) if Polygons[polygon].platform then Polygons[polygon].platform.floor_height = height end end\n"
	"function set_platform_monster_control(polygon, control) if Polygons[polygon].platform then Polygons[polygon].platform.monster_controllable = control end end\n"
	"function set_platform_movement(polygon, movement) if Polygons[polygon].platform then Polygons[polygon].platform.extending = movement end end\n"
	"function set_platform_player_control(polygon, control) if Polygons[polygon].platform then Polygons[polygon].platform.player_controllable = control end end\n"
	"function set_platform_speed(polygon, speed) if Polygons[polygon].platform then Polygons[polygon].platform.speed = speed / 1024 end end\n"
	"function set_platform_state(polygon, state) if Polygons[polygon].platform then Polygons[polygon].platform.active = state end end\n"
	"function set_polygon_ceiling_height(polygon, height) Polygons[polygon].ceiling.height = height end\n"
	"function set_polygon_floor_height(polygon, height) Polygons[polygon].floor.height = height end\n"
	"function set_polygon_media(polygon, media) if media == -1 then Polygons[polygon].media = nil else Polygons[polygon].media = media end end\n"
	"function set_polygon_target(polygon, target) Polygons[polygon].permutation = target end\n"
	"function set_polygon_type(polygon, type) Polygons[polygon].type = type end\n"
	"function set_tag_state(tag, state) Tags[tag].active = state end\n"
	"function set_terminal_text_number(poly, line, id) if Lines[line].ccw_polygon == Polygons[poly] then s = Lines[line].ccw_side elseif Lines[line].cw_polygon == Polygons[poly] then s = Lines[line].cw_side else return end if s.control_panel and s.control_panel.type.class == 8 then s.control_panel.permutation = id end end\n"
	"function set_underwater_fog_affects_landscapes(affects_landscapes) Level.underwater_fog.affects_landscapes = affects_landscapes end\n"
	"function set_underwater_fog_color(r, g, b) Level.underwater_fog.color.r = r Level.underwater_fog.color.g = g Level.underwater_fog.color.b = b end\n"
	"function set_underwater_fog_depth(depth) Level.underwater_fog.depth = depth end\n"
	"function set_underwater_fog_present(present) Level.underwater_fog.active = present end\n"
	;

static void compatibility(lua_State *L)
{
	luaL_loadbuffer(L, compatibility_script, strlen(compatibility_script), "map_compatibility");
	lua_pcall(L, 0, 0, 0);
}

#endif

