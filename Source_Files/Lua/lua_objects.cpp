/*
LUA_OBJECTS.CPP

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

	Implements the Lua map objects classes
*/

#include "lua_objects.h"
#include "lua_map.h"
#include "lua_templates.h"

#include "effects.h"
#include "items.h"
#include "monsters.h"
#include "scenery.h"
#include "player.h"
#define DONT_REPEAT_DEFINITIONS
#include "item_definitions.h"
#include "scenery_definitions.h"

#include "SoundManager.h"

#include <boost/bind.hpp>

#ifdef HAVE_LUA

const float AngleConvert = 360/float(FULL_CIRCLE);

template<class T>
int lua_delete_object(lua_State *L)
{
	remove_map_object(T::Index(L, 1));
	return 0;
}

template<>
int lua_delete_object<Lua_Scenery>(lua_State *L)
{
	short object_index = Lua_Scenery::Index(L, 1);
	remove_map_object(object_index);
	deanimate_scenery(object_index);
	return 0;
}

template<class T>
int lua_play_object_sound(lua_State *L)
{
	short sound_code = Lua_Sound::ToIndex(L, 2);
	play_object_sound(T::Index(L, 1), sound_code);
	return 0;
}

template<class T>
static int get_object_facing(lua_State *L)
{
	object_data *object = get_object_data(T::Index(L, 1));
	lua_pushnumber(L, (double) object->facing * AngleConvert);
	return 1;
}

template<class T>
static int get_object_polygon(lua_State *L)
{
	object_data *object = get_object_data(T::Index(L, 1));
	Lua_Polygon::Push(L, object->polygon);
	return 1;
}

template<class T, class TT>
static int get_object_type(lua_State *L)
{
	object_data *object = get_object_data(T::Index(L, 1));
	TT::Push(L, object->permutation);
	return 1;
}

template<class T>
static int get_object_x(lua_State *L)
{
	object_data *object = get_object_data(T::Index(L, 1));
	lua_pushnumber(L, (double) object->location.x / WORLD_ONE);
	return 1;
}

template<class T>
static int get_object_y(lua_State *L)
{
	object_data *object = get_object_data(T::Index(L, 1));
	lua_pushnumber(L, (double) object->location.y / WORLD_ONE);
	return 1;
}

template<class T>
static int get_object_z(lua_State *L)
{
	object_data *object = get_object_data(T::Index(L, 1));
	lua_pushnumber(L, (double) object->location.z / WORLD_ONE);
	return 1;
}

template<class T>
static int set_object_facing(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "facing: incorrect argument type");

	object_data *object = get_object_data(T::Index(L, 1));
	object->facing = static_cast<int>(lua_tonumber(L, 2) / AngleConvert);
	return 0;
}

extern void add_object_to_polygon_object_list(short, short);

template<class T>
int lua_object_position(lua_State *L)
{
	if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		return luaL_error(L, "position: incorrect argument type");

	short polygon_index = 0;
	if (lua_isnumber(L, 5))
	{
		polygon_index = static_cast<int>(lua_tonumber(L, 5));
		if (!Lua_Polygon::Valid(polygon_index))
			return luaL_error(L, "position: invalid polygon index");
	}
	else if (Lua_Polygon::Is(L, 5))
	{
		polygon_index = Lua_Polygon::Index(L, 5);
	}
	else
		return luaL_error(L, "position: incorrect argument type");

	short object_index = T::Index(L, 1);
	object_data *object = get_object_data(object_index);
	object->location.x = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	object->location.y = static_cast<int>(lua_tonumber(L, 3) * WORLD_ONE);
	object->location.z = static_cast<int>(lua_tonumber(L, 4) * WORLD_ONE);

	if (polygon_index != object->polygon)
	{
		remove_object_from_polygon_object_list(object_index);
		add_object_to_polygon_object_list(object_index, polygon_index);
	}

	return 0;
}

char Lua_Effect_Name[] = "effect";

int Lua_Effect_Delete(lua_State* L)
{
	remove_effect(Lua_Effect::Index(L, 1));
	return 0;
}

static int Lua_Effect_Get_Facing(lua_State* L)
{
	effect_data* effect = get_effect_data(Lua_Effect::Index(L, 1));
	object_data* object = get_object_data(effect->object_index);
	lua_pushnumber(L, (double) object->facing * AngleConvert);
	return 1;
}

static int Lua_Effect_Get_Polygon(lua_State* L)
{
	effect_data* effect = get_effect_data(Lua_Effect::Index(L, 1));
	object_data* object = get_object_data(effect->object_index);
	Lua_Polygon::Push(L, object->polygon);
	return 1;
}

static int Lua_Effect_Get_Type(lua_State* L)
{
	effect_data* effect = get_effect_data(Lua_Effect::Index(L, 1));
	Lua_EffectType::Push(L, effect->type);
	return 1;
}

static int Lua_Effect_Get_X(lua_State* L)
{
	effect_data* effect = get_effect_data(Lua_Effect::Index(L, 1));
	object_data* object = get_object_data(effect->object_index);
	lua_pushnumber(L, (double) object->location.x / WORLD_ONE);
	return 1;
}

static int Lua_Effect_Get_Y(lua_State* L)
{
	effect_data* effect = get_effect_data(Lua_Effect::Index(L, 1));
	object_data* object = get_object_data(effect->object_index);
	lua_pushnumber(L, (double) object->location.y / WORLD_ONE);
	return 1;
}

static int Lua_Effect_Get_Z(lua_State* L)
{
	effect_data* effect = get_effect_data(Lua_Effect::Index(L, 1));
	object_data* object = get_object_data(effect->object_index);
	lua_pushnumber(L, (double) object->location.z / WORLD_ONE);
	return 1;
}

int Lua_Effect_Play_Sound(lua_State* L)
{
	effect_data* effect = get_effect_data(Lua_Effect::Index(L, 1));
	short sound_code = Lua_Sound::ToIndex(L, 2);
	play_object_sound(effect->object_index, sound_code);
	return 0;
}

int Lua_Effect_Position(lua_State* L)
{
	if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		return luaL_error(L, "position: incorrect argument type");

	short polygon_index = 0;
	if (lua_isnumber(L, 5))
	{
		polygon_index = static_cast<int>(lua_tonumber(L, 5));
		if (!Lua_Polygon::Valid(polygon_index))
			return luaL_error(L, "position: invalid polygon index");
	}
	else if (Lua_Polygon::Is(L, 5))
	{
		polygon_index = Lua_Polygon::Index(L, 5);
	}
	else
		return luaL_error(L, "position: incorrect argument type");

	effect_data* effect = get_effect_data(Lua_Effect::Index(L, 1));
	object_data *object = get_object_data(effect->object_index);
	object->location.x = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	object->location.y = static_cast<int>(lua_tonumber(L, 3) * WORLD_ONE);
	object->location.z = static_cast<int>(lua_tonumber(L, 4) * WORLD_ONE);

	if (polygon_index != object->polygon)
	{
		remove_object_from_polygon_object_list(effect->object_index);
		add_object_to_polygon_object_list(effect->object_index, polygon_index);
	}

	return 0;
	
}

const luaL_Reg Lua_Effect_Get[] = {
	{"delete", L_TableFunction<Lua_Effect_Delete>},
	{"facing", Lua_Effect_Get_Facing},
	{"play_sound", L_TableFunction<Lua_Effect_Play_Sound>},
	{"polygon", Lua_Effect_Get_Polygon},
	{"position", L_TableFunction<Lua_Effect_Position>},
	{"type", Lua_Effect_Get_Type},
	{"x", Lua_Effect_Get_X},
	{"y", Lua_Effect_Get_Y},
	{"z", Lua_Effect_Get_Z},
	{0, 0}
};

static int Lua_Effect_Set_Facing(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "facing: incorrect argument type");

	effect_data* effect = get_effect_data(Lua_Effect::Index(L, 1));
	object_data* object = get_object_data(effect->object_index);
	object->facing = static_cast<int>(lua_tonumber(L, 2) / AngleConvert);
	return 0;
}

const luaL_Reg Lua_Effect_Set[] = {
	{"facing", Lua_Effect_Set_Facing},
	{0, 0}
};

bool Lua_Effect_Valid(int32 index)
{
	if (index < 0 || index >= MAXIMUM_EFFECTS_PER_MAP)
		return false;

	effect_data *effect = GetMemberWithBounds(effects, index, MAXIMUM_EFFECTS_PER_MAP);
	return SLOT_IS_USED(effect);
}

char Lua_Effects_Name[] = "Effects";

// Effects.new(x, y, height, polygon, type)
int Lua_Effects_New(lua_State *L)
{
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		luaL_error(L, "new: incorrect argument type");
	
	int polygon_index = 0;
	if (lua_isnumber(L, 4))
	{
		polygon_index = static_cast<int>(lua_tonumber(L, 4));
		if (!Lua_Polygon::Valid(polygon_index))
			return luaL_error(L, "new: invalid polygon index");
	}
	else if (Lua_Polygon::Is(L, 4))
	{
		polygon_index = Lua_Polygon::Index(L, 4);
	}
	else
		return luaL_error(L, "new: incorrect argument type");
	
	short effect_type = Lua_EffectType::ToIndex(L, 5);
	
	world_point3d location;
	location.x = static_cast<world_distance>(lua_tonumber(L, 1) * WORLD_ONE);
	location.y = static_cast<world_distance>(lua_tonumber(L, 2) * WORLD_ONE);
	location.z = static_cast<world_distance>(lua_tonumber(L, 3) * WORLD_ONE);

	short effect_index = ::new_effect(&location, polygon_index, effect_type, 0);
	if (effect_index == NONE)
		return 0;

	Lua_Effect::Push(L, effect_index);
	return 1;
}

const luaL_Reg Lua_Effects_Methods[] = {
	{"new", L_TableFunction<Lua_Effects_New>},
	{0, 0}
};

char Lua_EffectType_Name[] = "effect_type";

char Lua_EffectTypes_Name[] = "EffectTypes";

char Lua_Item_Name[] = "item";

int Lua_Item_Delete(lua_State* L)
{
	auto object_index = Lua_Item::Index(L, 1);
	object_data* object = get_object_data(object_index);
	int16 item_type = object->permutation;

	// check if the item is teleporting in and remove associated effect
	if (OBJECT_IS_INVISIBLE(object))
	{
		for (auto i = 0; i < MAXIMUM_EFFECTS_PER_MAP; ++i)
		{
			auto effect = &effects[i];
			if (SLOT_IS_USED(effect) &&
				effect->type == _effect_teleport_object_in &&
				effect->data == object_index)
			{
				remove_effect(i);
				break;
			}
		}
	}

	remove_map_object(Lua_Item::Index(L, 1));
	if (L_Get_Proper_Item_Accounting(L))
	{
		object_was_just_destroyed(_object_is_item, item_type);
	}
	return 0;
}

const luaL_Reg Lua_Item_Get[] = {
	{"delete", L_TableFunction<Lua_Item_Delete>},
	{"facing", get_object_facing<Lua_Item>},
	{"play_sound", L_TableFunction<lua_play_object_sound<Lua_Item> >},
	{"polygon", get_object_polygon<Lua_Item>},
	{"position", L_TableFunction<lua_object_position<Lua_Item> >},
	{"type", get_object_type<Lua_Item, Lua_ItemType>},
	{"x", get_object_x<Lua_Item>},
	{"y", get_object_y<Lua_Item>},
	{"z", get_object_z<Lua_Item>},
	{0, 0}
};

const luaL_Reg Lua_Item_Set[] = {
	{"facing", set_object_facing<Lua_Item>},
	{0, 0}
};

char Lua_Items_Name[] = "Items";

// Items.new(x, y, height, polygon, type)
int Lua_Items_New(lua_State *L)
{
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		luaL_error(L, "new: incorrect argument type");

	int polygon_index = 0;
	if (lua_isnumber(L, 4))
	{
		polygon_index = static_cast<int>(lua_tonumber(L, 4));
		if (!Lua_Polygon::Valid(polygon_index))
			return luaL_error(L, "new: invalid polygon index");
	}
	else if (Lua_Polygon::Is(L, 4))
	{
		polygon_index = Lua_Polygon::Index(L, 4);
	}
	else
		return luaL_error(L, "new: incorrect argument type");

	short item_type = Lua_ItemType::ToIndex(L, 5);

	object_location location;
	location.p.x = static_cast<int>(lua_tonumber(L, 1) * WORLD_ONE);
	location.p.y = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	location.p.z = static_cast<int>(lua_tonumber(L, 3) * WORLD_ONE);

	location.polygon_index = polygon_index;
	location.yaw = 0;
	location.pitch = 0;
	location.flags = 0;

	short item_index = ::new_item(&location, item_type);
	if (item_index == NONE)
		return 0;

	Lua_Item::Push(L, item_index);
	return 1;
}

const luaL_Reg Lua_Items_Methods[] = {
	{"new", L_TableFunction<Lua_Items_New>},
	{0, 0}
};

	
bool Lua_Item_Valid(int32 index)
{
	if (index < 0 || index >= MAXIMUM_OBJECTS_PER_MAP)
		return false;

	object_data *object = GetMemberWithBounds(objects, index, MAXIMUM_OBJECTS_PER_MAP);
	return (SLOT_IS_USED(object) && GET_OBJECT_OWNER(object) == _object_is_item);
}

char Lua_ItemKind_Name[] = "item_kind";
typedef L_Enum<Lua_ItemKind_Name> Lua_ItemKind;

static bool Lua_ItemKind_Valid(int32 index)
{
	return index >= 0 && index <= NUMBER_OF_ITEM_TYPES;
}

char Lua_ItemKinds_Name[] = "ItemKinds";
typedef L_EnumContainer<Lua_ItemKinds_Name, Lua_ItemKind> Lua_ItemKinds;

static int Lua_ItemType_Get_Ball(lua_State *L)
{
	lua_pushboolean(L, (get_item_kind(Lua_ItemType::Index(L, 1)) == _ball));
	return 1;
}

static int Lua_ItemType_Get_Kind(lua_State* L)
{
	Lua_ItemKind::Push(L, get_item_kind(Lua_ItemType::Index(L, 1)));
	return 1;
}

static int Lua_ItemType_Get_Initial_Count(lua_State* L)
{
	lua_pushnumber(L, get_placement_info()[Lua_ItemType::Index(L, 1)].initial_count);
	return 1;
}

static int Lua_ItemType_Get_Maximum_Count(lua_State* L)
{
	lua_pushnumber(L, get_placement_info()[Lua_ItemType::Index(L, 1)].maximum_count);
	return 1;
}

static int Lua_ItemType_Get_Maximum_Inventory(lua_State* L)
{
	auto is_m1 = static_world->environment_flags & _environment_m1_weapons;
	auto difficulty_level = dynamic_world->game_information.difficulty_level;
	lua_pushnumber(L, get_item_definition_external(Lua_ItemType::Index(L, 1))->get_maximum_count_per_player(is_m1, difficulty_level));
	return 1;
}

static int Lua_ItemType_Get_Minimum_Count(lua_State* L)
{
	lua_pushnumber(L, get_placement_info()[Lua_ItemType::Index(L, 1)].minimum_count);
	return 1;
}

static int Lua_ItemType_Get_Random_Count(lua_State* L)
{
	lua_pushnumber(L, get_placement_info()[Lua_ItemType::Index(L, 1)].random_count);
	return 1;
}

static int Lua_ItemType_Get_Random_Chance(lua_State* L)
{
	lua_pushnumber(L, static_cast<double>(get_placement_info()[Lua_ItemType::Index(L, 1)].random_chance) / UINT16_MAX);
	return 1;
}

static int Lua_ItemType_Get_Random_Location(lua_State* L)
{
	lua_pushboolean(L, get_placement_info()[Lua_ItemType::Index(L, 1)].flags & _reappears_in_random_location);
	return 1;
}

static int Lua_ItemType_Set_Initial_Count(lua_State* L)
{
	if (lua_isnumber(L, 2))
	{
		get_placement_info()[Lua_ItemType::Index(L, 1)].initial_count = lua_tonumber(L, 2);
	}
	else
	{
		return luaL_error(L, "initial_count: incorrect argument type");
	}
	return 0;
}

static int Lua_ItemType_Set_Maximum_Count(lua_State* L)
{
	if (lua_isnumber(L, 2))
	{
		get_placement_info()[Lua_ItemType::Index(L, 1)].maximum_count = lua_tonumber(L, 2);
	}
	else
	{
		return luaL_error(L, "maximum_count: incorrect argument type");
	}
	return 0;
}

static int Lua_ItemType_Set_Maximum_Inventory(lua_State* L)
{
	if (lua_isnumber(L, 2))
	{
		auto definition = get_item_definition_external(Lua_ItemType::Index(L, 1));
		const auto difficulty_level = dynamic_world->game_information.difficulty_level;
		definition->extended_maximum_count[difficulty_level] = static_cast<int16_t>(lua_tonumber(L, 2));
		definition->has_extended_maximum_count[difficulty_level] = true;
	}
	else
	{
		return luaL_error(L, "maximum_inventory: incorrect argument type");
	}

	return 0;
}
 
static int Lua_ItemType_Set_Minimum_Count(lua_State* L)
{
	if (lua_isnumber(L, 2))
	{
		get_placement_info()[Lua_ItemType::Index(L, 1)].minimum_count = lua_tonumber(L, 2);
	}
	else
	{
		return luaL_error(L, "minimum_count: incorrect argument type");
	}
	return 0;
}

static int Lua_ItemType_Set_Random_Chance(lua_State* L)
{
	if (lua_isnumber(L, 2))
	{
		get_placement_info()[Lua_ItemType::Index(L, 1)].random_chance = static_cast<uint16>(lua_tonumber(L, 2) * UINT16_MAX + 0.5);
	}
	else
	{
		return luaL_error(L, "random_chance: incorrect argument type");
	}
	return 0;
}
 
static int Lua_ItemType_Set_Random_Count(lua_State* L)
{
	if (lua_isnumber(L, 2))
	{
		get_placement_info()[Lua_ItemType::Index(L, 1)].random_count = lua_tonumber(L, 2);
	}
	else
	{
		return luaL_error(L, "random_count: incorrect argument type");
	}
	return 0;
}

static int Lua_ItemType_Set_Random_Location(lua_State* L)
{
	if (lua_isboolean(L, 2))
	{
		if (lua_toboolean(L, 2))
		{
			get_placement_info()[Lua_ItemType::Index(L, 1)].flags |= _reappears_in_random_location;
		}
		else
		{
			get_placement_info()[Lua_ItemType::Index(L, 1)].flags &= ~_reappears_in_random_location;
		}
	}
	else
	{
		return luaL_error(L, "random_location: incorrect argument type");
	}
	return 0;
}


static bool Lua_ItemType_Valid(int32 index) { 
	return index >= 0 && index < NUMBER_OF_DEFINED_ITEMS;
}

char Lua_ItemType_Name[] = "item_type";
const luaL_Reg Lua_ItemType_Get[] = {
	{"ball", Lua_ItemType_Get_Ball},
	{"kind", Lua_ItemType_Get_Kind},
	{"initial_count", Lua_ItemType_Get_Initial_Count},
	{"maximum_count", Lua_ItemType_Get_Maximum_Count},
	{"maximum_inventory", Lua_ItemType_Get_Maximum_Inventory},
	{"minimum_count", Lua_ItemType_Get_Minimum_Count},
	{"random_chance", Lua_ItemType_Get_Random_Chance},
	{"random_location", Lua_ItemType_Get_Random_Location},
	{"total_available", Lua_ItemType_Get_Random_Count},
	{0, 0}
};

const luaL_Reg Lua_ItemType_Set[] = {
	{"initial_count", Lua_ItemType_Set_Initial_Count},
	{"maximum_count", Lua_ItemType_Set_Maximum_Count},
	{"maximum_inventory", Lua_ItemType_Set_Maximum_Inventory},
	{"minimum_count", Lua_ItemType_Set_Minimum_Count},
	{"random_chance", Lua_ItemType_Set_Random_Chance},
	{"random_location", Lua_ItemType_Set_Random_Location},
	{"total_available", Lua_ItemType_Set_Random_Count},
	{0, 0}
};

char Lua_ItemTypes_Name[] = "ItemTypes";

char Lua_SceneryType_Name[] = "scenery_type";
typedef L_Enum<Lua_SceneryType_Name> Lua_SceneryType;

static bool Lua_SceneryType_Valid(int32 index)
{
	return index >= 0 && index <= NUMBER_OF_SCENERY_DEFINITIONS;
}

char Lua_SceneryTypes_Name[] = "SceneryTypes";
typedef L_EnumContainer<Lua_SceneryTypes_Name, Lua_SceneryType> Lua_SceneryTypes;

char Lua_Scenery_Name[] = "scenery";

int Lua_Scenery_Damage(lua_State *L)
{
	damage_scenery(Lua_Scenery::Index(L, 1));
	return 0;
}

static int Lua_Scenery_Get_Damaged(lua_State *L)
{
	object_data *object = get_object_data(Lua_Scenery::Index(L, 1));

	// if it made it this far (past valid() check),
	// it's either scenery or normal
	lua_pushboolean(L, GET_OBJECT_OWNER(object) == _object_is_normal);
	return 1;
}

static int Lua_Scenery_Get_Solid(lua_State *L)
{
	object_data *object = get_object_data(Lua_Scenery::Index(L, 1));
	lua_pushboolean(L, OBJECT_IS_SOLID(object));
	return 1;
}

const luaL_Reg Lua_Scenery_Get[] = {
	{"damage", L_TableFunction<Lua_Scenery_Damage>},
	{"damaged", Lua_Scenery_Get_Damaged},
	{"delete", L_TableFunction<lua_delete_object<Lua_Scenery> >},
	{"facing", get_object_facing<Lua_Scenery>},
	{"play_sound", L_TableFunction<lua_play_object_sound<Lua_Scenery> >},
	{"polygon", get_object_polygon<Lua_Scenery>},
	{"position", L_TableFunction<lua_object_position<Lua_Scenery> >},
	{"solid", Lua_Scenery_Get_Solid},
	{"type", get_object_type<Lua_Scenery, Lua_SceneryType>},
	{"x", get_object_x<Lua_Scenery>},
	{"y", get_object_y<Lua_Scenery>},
	{"z", get_object_z<Lua_Scenery>},
	{0, 0}
};

static int Lua_Scenery_Set_Solid(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "solid: incorrect argument type");

	object_data *object = get_object_data(Lua_Scenery::Index(L, 1));
	SET_OBJECT_SOLIDITY(object, lua_toboolean(L, 2));
	return 0;
}

const luaL_Reg Lua_Scenery_Set[] = {
	{"facing", set_object_facing<Lua_Scenery>},
	{"solid", Lua_Scenery_Set_Solid},
	{0, 0}
};

char Lua_Sceneries_Name[] = "Scenery";

// Scenery.new(x, y, height, polygon, type)
int Lua_Sceneries_New(lua_State *L)
{
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		luaL_error(L, "new: incorrect argument type");

	int polygon_index = 0;
	if (lua_isnumber(L, 4))
	{
		polygon_index = static_cast<int>(lua_tonumber(L, 4));
		if (!Lua_Polygon::Valid(polygon_index))
			return luaL_error(L, "new: invalid polygon index");
	}
	else if (Lua_Polygon::Is(L, 4))
	{
		polygon_index = Lua_Polygon::Index(L, 4);
	}
	else
		return luaL_error(L, "new: incorrect argument type");

	short scenery_type = Lua_SceneryType::ToIndex(L, 5);

	object_location location;
	location.p.x = static_cast<int>(lua_tonumber(L, 1) * WORLD_ONE);
	location.p.y = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	location.p.z = static_cast<int>(lua_tonumber(L, 3) * WORLD_ONE);
	
	location.polygon_index = polygon_index;
	location.yaw = 0;
	location.pitch = 0;
	location.flags = 0;

	short scenery_index = ::new_scenery(&location, scenery_type);
	if (scenery_index == NONE)
		return 0;

	randomize_scenery_shape(scenery_index);

	Lua_Scenery::Push(L, scenery_index);
	return 1;
}

static bool Lua_Scenery_Valid(int32 index)
{
	if (index < 0 || index >= MAXIMUM_OBJECTS_PER_MAP)
		return false;

	object_data *object = GetMemberWithBounds(objects, index, MAXIMUM_OBJECTS_PER_MAP);
	if (SLOT_IS_USED(object))
	{
		if (GET_OBJECT_OWNER(object) == _object_is_scenery) 
			return true;
		else if (GET_OBJECT_OWNER(object) == _object_is_normal)
		{
			// check to make sure it's not a player's legs or torso
			for (int player_index = 0; player_index < dynamic_world->player_count; player_index++)
			{
				player_data *player = get_player_data(player_index);
				monster_data *monster = get_monster_data(player->monster_index);
				if (monster->object_index == index) 
					return false;
				else
				{
					object_data *object = get_object_data(monster->object_index);
					if (object->parasitic_object == index)
						return false;
				}
			}

			return true;
		}
	}

	return false;
}

const luaL_Reg Lua_Sceneries_Methods[] = {
	{"new", L_TableFunction<Lua_Sceneries_New>},
	{0, 0}
};

// these are not sound objects
char Lua_Sound_Name[] = "lua_sound";
char Lua_Sounds_Name[] = "Sounds";

int Lua_Sounds_Length()
{
	return INT16_MAX;
}

const luaL_Reg Lua_Sounds_Methods[] = {
	{0, 0}
};

static void compatibility(lua_State *L);

int Lua_Objects_register(lua_State *L)
{
	Lua_Effect::Register(L, Lua_Effect_Get, Lua_Effect_Set);
	Lua_Effect::Valid = Lua_Effect_Valid;

	Lua_Effects::Register(L, Lua_Effects_Methods);
	Lua_Effects::Length = boost::bind(get_dynamic_limit, (int) _dynamic_limit_effects);

	Lua_Item::Register(L, Lua_Item_Get, Lua_Item_Set);
	Lua_Item::Valid = Lua_Item_Valid;

	Lua_Items::Register(L, Lua_Items_Methods);
	Lua_Items::Length = boost::bind(get_dynamic_limit, (int) _dynamic_limit_objects);

	Lua_Scenery::Register(L, Lua_Scenery_Get, Lua_Scenery_Set);
	Lua_Scenery::Valid = Lua_Scenery_Valid;

	Lua_Sceneries::Register(L, Lua_Sceneries_Methods);
	Lua_Sceneries::Length = boost::bind(get_dynamic_limit, (int) _dynamic_limit_objects);

	Lua_EffectType::Register(L, 0, 0, 0, Lua_EffectType_Mnemonics);
	Lua_EffectType::Valid = Lua_EffectType::ValidRange(NUMBER_OF_EFFECT_TYPES);

	Lua_EffectTypes::Register(L);
	Lua_EffectTypes::Length = Lua_EffectTypes::ConstantLength(NUMBER_OF_EFFECT_TYPES);

	Lua_ItemKind::Register(L, 0, 0, 0, Lua_ItemKind_Mnemonics);
	Lua_ItemKind::Valid = Lua_ItemKind_Valid;

	Lua_ItemKinds::Register(L);
	Lua_ItemKinds::Length = Lua_ItemKinds::ConstantLength(NUMBER_OF_ITEM_TYPES);

	Lua_ItemType::Register(L, Lua_ItemType_Get, Lua_ItemType_Set, 0, Lua_ItemType_Mnemonics);
	Lua_ItemType::Valid = Lua_ItemType_Valid;

	Lua_ItemTypes::Register(L);
	Lua_ItemTypes::Length = Lua_ItemTypes::ConstantLength(NUMBER_OF_DEFINED_ITEMS);

	Lua_SceneryType::Register(L, 0, 0, 0, Lua_SceneryType_Mnemonics);
	Lua_SceneryType::Valid = Lua_SceneryType_Valid;

	Lua_SceneryTypes::Register(L);
	Lua_SceneryTypes::Length = Lua_SceneryTypes::ConstantLength(NUMBER_OF_SCENERY_DEFINITIONS);

	Lua_Sound::Register(L, 0, 0, 0, Lua_Sound_Mnemonics);

	Lua_Sounds::Register(L, Lua_Sounds_Methods);
	Lua_Sounds::Length = Lua_Sounds_Length;

	compatibility(L);
	return 0;
}

static const char *compatibility_script = ""
	"function delete_item(item) Items[item]:delete() end\n"
	"function get_item_polygon(item) return Items[item].polygon.index end\n"
	"function get_item_type(item) return Items[item].type.index end\n"
	"function item_index_valid(item) if Items[item] then return true else return false end end\n"
	"function new_item(type, polygon, height) if (height) then return Items.new(Polygons[polygon].x, Polygons[polygon].y, height / 1024, polygon, type).index else return Items.new(Polygons[polygon].x, Polygons[polygon].y, 0, polygon, type).index end end\n"
	;

static void compatibility(lua_State *L)
{
	luaL_loadbuffer(L, compatibility_script, strlen(compatibility_script), "items_compatibility");
	lua_pcall(L, 0, 0, 0);
}

#endif
