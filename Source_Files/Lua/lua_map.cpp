/*
LUA_MAP.CPP

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

#include "lua_map.h"
#include "lua_monsters.h"
#include "lua_templates.h"
#include "lightsource.h"
#include "map.h"
#include "media.h"
#include "platforms.h"
#include "OGL_Setup.h"

#include <boost/bind.hpp>

#ifdef HAVE_LUA

char Lua_ControlPanelClass_Name[] = "control_panel_class";
char Lua_ControlPanelClasses_Name[] = "ControlPanelClasses";

char Lua_ControlPanelType_Name[] = "control_panel_type";

extern short get_panel_class(short panel_type);

static int Lua_ControlPanelType_Get_Class(lua_State *L)
{
	Lua_ControlPanelClass::Push(L, get_panel_class(Lua_ControlPanelType::Index(L, 1)));
	return 1;
}

const luaL_reg Lua_ControlPanelType_Get[] = {
	{"class", Lua_ControlPanelType_Get_Class},
	{0, 0}
};

char Lua_ControlPanelTypes_Name[] = "ControlPanelTypes";

char Lua_DamageType_Name[] = "damage_type";
char Lua_DamageTypes_Name[] = "DamageTypes";

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

const luaL_reg Lua_Line_Get[] = {
	{"cw_polygon", Lua_Line_Get_Clockwise_Polygon},
	{"ccw_polygon", Lua_Line_Get_Counterclockwise_Polygon},
	{"cw_side", Lua_Line_Get_Clockwise_Side},
	{"ccw_side", Lua_Line_Get_Counterclockwise_Side},
	{"clockwise_polygon", Lua_Line_Get_Clockwise_Polygon},
	{"clockwise_side", Lua_Line_Get_Clockwise_Side},
	{"counterclockwise_polygon", Lua_Line_Get_Counterclockwise_Polygon},
	{"counterclockwise_side", Lua_Line_Get_Counterclockwise_Side},
	{0, 0}
};

static bool Lua_Line_Valid(int16 index)
{
	return index >= 0 && index < LineList.size();
}

char Lua_Lines_Name[] = "Lines";

char Lua_Platform_Name[] = "platform";
bool Lua_Platform_Valid(int16 index)
{
	return index >= 0 && index < dynamic_world->platform_count;
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

static int Lua_Platform_Get_Monster_Controllable(lua_State *L)
{
	platform_data *platform = get_platform_data(Lua_Platform::Index(L, 1));
	lua_pushboolean(L, PLATFORM_IS_MONSTER_CONTROLLABLE(platform));
	return 1;
}

static int Lua_Platform_Get_Player_Controllable(lua_State *L)
{
	platform_data *platform = get_platform_data(Lua_Platform::Index(L, 1));
	lua_pushboolean(L, PLATFORM_IS_PLAYER_CONTROLLABLE(platform));
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

static int Lua_Platform_Set_Active(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "active: incorrect argument type");

	short platform_index = Lua_Platform::Index(L, 1);
	try_and_change_platform_state(platform_index, lua_toboolean(L, 2));
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

static int Lua_Platform_Set_Monster_Controllable(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "monster_controllable: incorrect argument type");
	
	platform_data *platform = get_platform_data(Lua_Platform::Index(L, 1));
	SET_PLATFORM_IS_MONSTER_CONTROLLABLE(platform, lua_toboolean(L, 2));
	return 0;
}

static int Lua_Platform_Set_Player_Controllable(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "monster_controllable: incorrect argument type");
	
	platform_data *platform = get_platform_data(Lua_Platform::Index(L, 1));
	SET_PLATFORM_IS_PLAYER_CONTROLLABLE(platform, lua_toboolean(L, 2));
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
	

const luaL_reg Lua_Platform_Get[] = {
	{"active", Lua_Platform_Get_Active},
	{"ceiling_height", Lua_Platform_Get_Ceiling_Height},
	{"contracting", Lua_Platform_Get_Contracting},
	{"extending", Lua_Platform_Get_Extending},
	{"floor_height", Lua_Platform_Get_Floor_Height},
	{"monster_controllable", Lua_Platform_Get_Monster_Controllable},
	{"player_controllable", Lua_Platform_Get_Player_Controllable},
	{"polygon", Lua_Platform_Get_Polygon},
	{"speed", Lua_Platform_Get_Speed},
	{0, 0}
};

const luaL_reg Lua_Platform_Set[] = {
	{"active", Lua_Platform_Set_Active},
	{"ceiling_height", Lua_Platform_Set_Ceiling_Height},
	{"contracting", Lua_Platform_Set_Contracting},
	{"extending", Lua_Platform_Set_Extending},
	{"floor_height", Lua_Platform_Set_Floor_Height},
	{"monster_controllable", Lua_Platform_Set_Monster_Controllable},
	{"player_controllable", Lua_Platform_Set_Player_Controllable},
	{"speed", Lua_Platform_Set_Speed},
	{0, 0}
};

char Lua_Platforms_Name[] = "Platforms";
int16 Lua_Platforms_Length() {
	return dynamic_world->platform_count;
}

char Lua_Polygon_Floor_Name[] = "polygon_floor";

static int Lua_Polygon_Floor_Get_Height(lua_State *L)
{
	lua_pushnumber(L, (double) (get_polygon_data(Lua_Polygon_Floor::Index(L, 1))->floor_height) / WORLD_ONE);
	return 1;
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

const luaL_reg Lua_Polygon_Floor_Get[] = {
	{"height", Lua_Polygon_Floor_Get_Height},
	{"z", Lua_Polygon_Floor_Get_Height},
	{0, 0}
};

const luaL_reg Lua_Polygon_Floor_Set[] = {
	{"height", Lua_Polygon_Floor_Set_Height},
	{"z", Lua_Polygon_Floor_Set_Height},
	{0, 0}
};

char Lua_Polygon_Ceiling_Name[] = "polygon_ceiling";

static int Lua_Polygon_Ceiling_Get_Height(lua_State *L)
{
	lua_pushnumber(L, (double) (get_polygon_data(Lua_Polygon_Ceiling::Index(L, 1))->ceiling_height) / WORLD_ONE);
	return 1;
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

const luaL_reg Lua_Polygon_Ceiling_Get[] = {
	{"height", Lua_Polygon_Ceiling_Get_Height},
	{"z", Lua_Polygon_Ceiling_Get_Height},
	{0, 0}
};

const luaL_reg Lua_Polygon_Ceiling_Set[] = {
	{"height", Lua_Polygon_Ceiling_Set_Height},
	{"z", Lua_Polygon_Ceiling_Set_Height},
	{0, 0}
};

char Lua_Polygon_Name[] = "polygon";

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

static int Lua_Polygon_Get_Ceiling(lua_State *L)
{
	Lua_Polygon_Ceiling::Push(L, Lua_Polygon::Index(L, 1));
	return 1;
}

static int Lua_Polygon_Get_Floor(lua_State *L)
{
	Lua_Polygon_Floor::Push(L, Lua_Polygon::Index(L, 1));
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

static int Lua_Polygon_Get_Type(lua_State *L)
{
	lua_pushnumber(L, get_polygon_data(Lua_Polygon::Index(L, 1))->type);
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
	if (!lua_isnumber(L, 2))
	{
		luaL_error(L, "type: incorrect argument type");
	}

	int type = static_cast<int>(lua_tonumber(L, 2));
	get_polygon_data(Lua_Polygon::Index(L, 1))->type = type;

	return 0;
}

static bool Lua_Polygon_Valid(int16 index)
{
	return index >= 0 && index < dynamic_world->polygon_count;
}

const luaL_reg Lua_Polygon_Get[] = {
	{"ceiling", Lua_Polygon_Get_Ceiling},
	{"floor", Lua_Polygon_Get_Floor},
	{"media", Lua_Polygon_Get_Media},
	{"monsters", L_TableFunction<Lua_Polygon_Monsters>},
	{"permutation", Lua_Polygon_Get_Permutation},
	{"platform", Lua_Polygon_Get_Platform},
	{"type", Lua_Polygon_Get_Type},
	{"x", Lua_Polygon_Get_X},
	{"y", Lua_Polygon_Get_Y},
	{"z", Lua_Polygon_Get_Z},
	{0, 0}
};

const luaL_reg Lua_Polygon_Set[] = {
	{"media", Lua_Polygon_Set_Media},
	{"permutation", Lua_Polygon_Set_Permutation},
	{"type", Lua_Polygon_Set_Type},
	{0, 0}
};

char Lua_Polygons_Name[] = "Polygons";

int16 Lua_Polygons_Length() {
	return dynamic_world->polygon_count;
}

char Lua_Side_ControlPanel_Name[] = "side_control_panel";
typedef L_Class<Lua_Side_ControlPanel_Name> Lua_Side_ControlPanel;

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

const luaL_reg Lua_Side_ControlPanel_Get[] = {
	{"type", Lua_Side_ControlPanel_Get_Type},
	{"permutation", Lua_Side_ControlPanel_Get_Permutation},
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

const luaL_reg Lua_Side_ControlPanel_Set[] = {
	{"permutation", Lua_Side_ControlPanel_Set_Permutation},
	{0, 0}
};

char Lua_Side_Name[] = "side";

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

const luaL_reg Lua_Side_Get[] = {
	{"control_panel", Lua_Side_Get_Control_Panel},
	{"line", Lua_Side_Get_Line},
	{"polygon", Lua_Side_Get_Polygon},
	{0, 0}
};

static bool Lua_Side_Valid(int16 index)
{
	return index >= 0 && index < SideList.size();
}

char Lua_Sides_Name[] = "Sides";

char Lua_Light_Name[] = "light";

static int Lua_Light_Get_Active(lua_State *L)
{
	lua_pushboolean(L, get_light_status(Lua_Light::Index(L, 1)));
	return 1;
}

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

const luaL_reg Lua_Light_Get[] = {
	{"active", Lua_Light_Get_Active},
	{0, 0}
};

const luaL_reg Lua_Light_Set[] = {
	{"active", Lua_Light_Set_Active},
	{0, 0}
};

bool Lua_Light_Valid(int16 index)
{
	return index >= 0 && index < MAXIMUM_LIGHTS_PER_MAP;
}

char Lua_Lights_Name[] = "Lights";

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

const luaL_reg Lua_Tag_Get[] = {
	{"active", Lua_Tag_Get_Active},
	{0, 0}
};

const luaL_reg Lua_Tag_Set[] = {
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

static int Lua_Media_Get_Type(lua_State *L)
{
	media_data *media = get_media_data(Lua_Media::Index(L, 1));
	Lua_MediaType::Push(L, media->type);
	return 1;
}

const luaL_reg Lua_Media_Get[] = {
	{"type", Lua_Media_Get_Type},
	{0, 0}
};

static bool Lua_Media_Valid(int16 index)
{
	return index >= 0 && index< MAXIMUM_MEDIAS_PER_MAP;
}

char Lua_Medias_Name[] = "Media";

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

const luaL_reg Lua_Annotation_Get[] = {
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

const luaL_reg Lua_Annotation_Set[] = {
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
static int Lua_Annotations_New(lua_State *L)
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

const luaL_reg Lua_Annotations_Methods[] = {
	{"new", Lua_Annotations_New},
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

const luaL_reg Lua_Fog_Color_Get[] = {
	{"r", Lua_Fog_Color_Get_R},
	{"g", Lua_Fog_Color_Get_G},
	{"b", Lua_Fog_Color_Get_B},
	{0, 0}
};

const luaL_reg Lua_Fog_Color_Set[] = {
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

const luaL_reg Lua_Fog_Get[] = {
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

const luaL_reg Lua_Fog_Set[] = {
	{"active", Lua_Fog_Set_Active},
	{"affects_landscapes", Lua_Fog_Set_Affects_Landscapes},
	{"depth", Lua_Fog_Set_Depth},
	{"present", Lua_Fog_Set_Active},
	{0, 0}
};

char Lua_Level_Name[] = "Level";
typedef L_Class<Lua_Level_Name> Lua_Level;

template<int16 flag>
static int Lua_Level_Get_Environment_Flag(lua_State *L)
{
	lua_pushboolean(L, static_world->environment_flags & flag);
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

static int Lua_Level_Get_Underwater_Fog(lua_State *L)
{
	Lua_Fog::Push(L, OGL_Fog_BelowLiquid);
	return 1;
}

const luaL_reg Lua_Level_Get[] = {
	{"fog", Lua_Level_Get_Fog},
	{"low_gravity", Lua_Level_Get_Environment_Flag<_environment_low_gravity>},
	{"magnetic", Lua_Level_Get_Environment_Flag<_environment_magnetic>},
	{"name", Lua_Level_Get_Name},
	{"rebellion", Lua_Level_Get_Environment_Flag<_environment_rebellion>},
	{"underwater_fog", Lua_Level_Get_Underwater_Fog},
	{"vacuum", Lua_Level_Get_Environment_Flag<_environment_vacuum>},
	{0, 0}
};

static int compatibility(lua_State *L);
#define NUMBER_OF_CONTROL_PANEL_DEFINITIONS 54

int Lua_Map_register(lua_State *L)
{
	Lua_ControlPanelClass::Register(L);
	Lua_ControlPanelClass::Valid = Lua_ControlPanelClass::ValidRange<NUMBER_OF_CONTROL_PANELS>;

	Lua_ControlPanelClasses::Register(L);
	Lua_ControlPanelClasses::Length = Lua_ControlPanelClasses::ConstantLength<NUMBER_OF_CONTROL_PANELS>;

	Lua_ControlPanelType::Register(L, Lua_ControlPanelType_Get);
	Lua_ControlPanelType::Valid = Lua_ControlPanelType::ValidRange<NUMBER_OF_CONTROL_PANEL_DEFINITIONS>;
	
	Lua_ControlPanelTypes::Register(L);
	Lua_ControlPanelTypes::Length = Lua_ControlPanelTypes::ConstantLength<NUMBER_OF_CONTROL_PANEL_DEFINITIONS>;

	Lua_DamageType::Register(L);
	Lua_DamageType::Valid = Lua_DamageType::ValidRange<NUMBER_OF_DAMAGE_TYPES>;
	Lua_DamageTypes::Register(L);
	Lua_DamageTypes::Length = Lua_DamageTypes::ConstantLength<NUMBER_OF_DAMAGE_TYPES>;
	
	Lua_Line::Register(L, Lua_Line_Get);
	Lua_Line::Valid = Lua_Line_Valid;

	Lua_Lines::Register(L);
	Lua_Lines::Length = boost::bind(&std::vector<line_data>::size, &LineList);

	Lua_Platform::Register(L, Lua_Platform_Get, Lua_Platform_Set);
	Lua_Platform::Valid = Lua_Platform_Valid;

	Lua_Platforms::Register(L);
	Lua_Platforms::Length = Lua_Platforms_Length;

	Lua_Polygon_Floor::Register(L, Lua_Polygon_Floor_Get, Lua_Polygon_Floor_Set);
	Lua_Polygon_Floor::Valid = Lua_Polygon_Valid;

	Lua_Polygon_Ceiling::Register(L, Lua_Polygon_Ceiling_Get, Lua_Polygon_Ceiling_Set);
	Lua_Polygon_Ceiling::Valid = Lua_Polygon_Valid;

	Lua_Polygon::Register(L, Lua_Polygon_Get, Lua_Polygon_Set);
	Lua_Polygon::Valid = Lua_Polygon_Valid;

	Lua_Polygons::Register(L);
	Lua_Polygons::Length = Lua_Polygons_Length;

	Lua_Side_ControlPanel::Register(L, Lua_Side_ControlPanel_Get, Lua_Side_ControlPanel_Set);

	Lua_Side::Register(L, Lua_Side_Get);
	Lua_Side::Valid = Lua_Side_Valid;

	Lua_Sides::Register(L);
	Lua_Sides::Length = boost::bind(&std::vector<side_data>::size, &SideList);

	Lua_Light::Register(L, Lua_Light_Get, Lua_Light_Set);
	Lua_Light::Valid = Lua_Light_Valid;

	Lua_Lights::Register(L);
	Lua_Lights::Length = boost::bind(&std::vector<light_data>::size, &LightList);
		
	Lua_Tag::Register(L, Lua_Tag_Get, Lua_Tag_Set);
	Lua_Tag::Valid = Lua_Tag_Valid;

	Lua_Tags::Register(L);
	Lua_Tags::Length = Lua_Tags::ConstantLength<INT16_MAX>;
	
	Lua_Terminal::Register(L);
	Lua_Terminal::Valid = Lua_Terminal_Valid;
	
	Lua_Terminals::Register(L);
	Lua_Terminals::Length = Lua_Terminals_Length;

	Lua_MediaType::Register(L);
	Lua_MediaType::Valid = Lua_MediaType::ValidRange<NUMBER_OF_MEDIA_TYPES>;

	Lua_MediaTypes::Register(L);
	Lua_MediaTypes::Length = Lua_MediaTypes::ConstantLength<NUMBER_OF_MEDIA_TYPES>;

	Lua_Media::Register(L, Lua_Media_Get);
	Lua_Media::Valid = Lua_Media_Valid;

	Lua_Medias::Register(L);
	Lua_Medias::Length = boost::bind(&std::vector<media_data>::size, &MediaList);

	Lua_Level::Register(L, Lua_Level_Get);

	Lua_Annotation::Register(L, Lua_Annotation_Get, Lua_Annotation_Set);
	Lua_Annotation::Valid = Lua_Annotation_Valid;

	Lua_Annotations::Register(L, Lua_Annotations_Methods);
	Lua_Annotations::Length = boost::bind(&std::vector<map_annotation>::size, &MapAnnotationList);

	Lua_Fog::Register(L, Lua_Fog_Get, Lua_Fog_Set);

	Lua_Fog_Color::Register(L, Lua_Fog_Color_Get, Lua_Fog_Color_Set);

	// register one Level userdatum globally
	Lua_Level::Push(L, 0);
	lua_setglobal(L, Lua_Level_Name);

	compatibility(L);
}

static const char* compatibility_script = ""
	"function annotations() local i = 0 local n = # Annotations return function() if i < n then a = Annotations[i] i = i + 1 if a.polygon then p = a.polygon.index else p = -1 end return a.text, p, a.x, a.y else return nil end end end\n"
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
	"function get_polygon_type(polygon) return Polygons[polygon].type end\n"
	"function get_tag_state(tag) return Tags[tag].active end\n"
	"function get_terminal_text_number(poly, line) if Lines[line].ccw_polygon == Polygons[poly] then s = Lines[line].ccw_side elseif Lines[line].cw_polygon == Polygons[poly] then s = Lines[line].cw_side else return line end if s.control_panel and s.control_panel.type.class == 8 then return s.control_panel.permutation else return line end end\n"
	"function get_underwater_fog_affects_landscapes() return Level.underwater_fog.affects_landscapes end\n"
	"function get_underwater_fog_color() return Level.underwater_fog.color.r, Level.underwater_fog.color.g, Level.underwater_fog.color.b end\n"
	"function get_underwater_fog_depth() return Level.underwater_fog.depth end\n"
	"function get_underwater_fog_present() return Level.underwater_fog.active end\n"

	"function number_of_polygons() return # Polygons end\n"
	"function select_monster(type, poly) for m in Polygons[poly]:monsters() do if m.type == type and m.visible and (m.action < 6 or m.action > 9) then return m.index end end return nil end\n"
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

static int compatibility(lua_State *L)
{
	luaL_loadbuffer(L, compatibility_script, strlen(compatibility_script), "map_compatibility");
	lua_pcall(L, 0, 0, 0);
}

#endif

