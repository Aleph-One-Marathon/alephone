/*
LUA_OBJECTS.CPP

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

char Lua_Effect_Name[] = "effect";

const luaL_reg Lua_Effect_Get[] = {
	{"delete", L_TableFunction<lua_delete_object<Lua_Effect> >},
	{"facing", get_object_facing<Lua_Effect>},
	{"play_sound", L_TableFunction<lua_play_object_sound<Lua_Effect> >},
	{"polygon", get_object_polygon<Lua_Effect>},
	{"type", get_object_type<Lua_Effect, Lua_EffectType>},
	{"x", get_object_x<Lua_Effect>},
	{"y", get_object_y<Lua_Effect>},
	{"z", get_object_z<Lua_Effect>},
	{0, 0}
};

bool Lua_Effect_Valid(int32 index)
{
	if (index < 0 || index >= MAXIMUM_OBJECTS_PER_MAP)
		return false;

	object_data *object = GetMemberWithBounds(objects, index, MAXIMUM_OBJECTS_PER_MAP);
	return (SLOT_IS_USED(object) && GET_OBJECT_OWNER(object) == _object_is_effect);
}

char Lua_Effects_Name[] = "Effects";

// Effects.new(x, y, height, polygon, type)
static int Lua_Effects_New(lua_State *L)
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

const luaL_reg Lua_Effects_Methods[] = {
	{"new", Lua_Effects_New},
	{0, 0}
};

char Lua_EffectType_Name[] = "effect_type";

char Lua_EffectTypes_Name[] = "EffectTypes";

char Lua_Item_Name[] = "item";

const luaL_reg Lua_Item_Get[] = {
	{"delete", L_TableFunction<lua_delete_object<Lua_Item> >},
	{"facing", get_object_facing<Lua_Item>},
	{"play_sound", L_TableFunction<lua_play_object_sound<Lua_Item> >},
	{"polygon", get_object_polygon<Lua_Item>},
	{"type", get_object_type<Lua_Item, Lua_ItemType>},
	{"x", get_object_x<Lua_Item>},
	{"y", get_object_y<Lua_Item>},
	{"z", get_object_z<Lua_Item>},
	{0, 0}
};

const luaL_reg Lua_Item_Set[] = {
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

const luaL_reg Lua_Items_Methods[] = {
	{"new", Lua_Items_New},
	{0, 0}
};

	
bool Lua_Item_Valid(int32 index)
{
	if (index < 0 || index >= MAXIMUM_OBJECTS_PER_MAP)
		return false;

	object_data *object = GetMemberWithBounds(objects, index, MAXIMUM_OBJECTS_PER_MAP);
	return (SLOT_IS_USED(object) && GET_OBJECT_OWNER(object) == _object_is_item);
}

static int Lua_ItemType_Get_Ball(lua_State *L)
{
	lua_pushboolean(L, (get_item_kind(Lua_ItemType::Index(L, 1)) == _ball));
	return 1;
}

static bool Lua_ItemType_Valid(int32 index) { 
	return index >= 0 && index < NUMBER_OF_DEFINED_ITEMS;
}

char Lua_ItemType_Name[] = "item_type";
const luaL_reg Lua_ItemType_Get[] = {
	{"ball", Lua_ItemType_Get_Ball},
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

const luaL_reg Lua_Scenery_Get[] = {
	{"damage", L_TableFunction<Lua_Scenery_Damage>},
	{"damaged", Lua_Scenery_Get_Damaged},
	{"delete", L_TableFunction<lua_delete_object<Lua_Scenery> >},
	{"facing", get_object_facing<Lua_Scenery>},
	{"play_sound", L_TableFunction<lua_play_object_sound<Lua_Scenery> >},
	{"polygon", get_object_polygon<Lua_Scenery>},
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

const luaL_reg Lua_Scenery_Set[] = {
	{"facing", set_object_facing<Lua_Scenery>},
	{"solid", Lua_Scenery_Set_Solid},
	{0, 0}
};

char Lua_Sceneries_Name[] = "Scenery";

// Scenery.new(x, y, height, polygon, type)
static int Lua_Sceneries_New(lua_State *L)
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
	if (scenery_index == 0)
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

const luaL_reg Lua_Sceneries_Methods[] = {
	{"new", Lua_Sceneries_New},
	{0, 0}
};

// these are not sound objects
char Lua_Sound_Name[] = "lua_sound";
char Lua_Sounds_Name[] = "Sounds";

int Lua_Sounds_Length()
{
	return SoundManager::instance()->NumberOfSoundDefinitions();
}

// Sounds.new(path)
static int Lua_Sounds_New(lua_State *L) {
  int top = lua_gettop(L);
  for(int n = 1; n <= top; ++n)
    if(!lua_isstring(L, n))
      luaL_error(L, "new: incorrect argument type");
  int index = SoundManager::instance()->NewCustomSoundDefinition();
  if(index < 0)
    lua_pushnil(L);
  else {
    int slots = 0;
    for(int n = 1; n <= top; ++n) {
      if(SoundManager::instance()->AddCustomSoundSlot(index, lua_tostring(L, n)))
	if(++slots >= 5 /*MAXIMUM_PERMUTATIONS_PER_SOUND*/) break;
    }
    Lua_Sound::Push(L, index);
  }
  return 1;
}

const luaL_reg Lua_Sounds_Methods[] = {
	{"new", Lua_Sounds_New},
	{0, 0}
};

static void compatibility(lua_State *L);

int Lua_Objects_register(lua_State *L)
{
	Lua_Effect::Register(L, Lua_Effect_Get);
	Lua_Effect::Valid = Lua_Effect_Valid;

	Lua_Effects::Register(L, Lua_Effects_Methods);
	Lua_Effects::Length = boost::bind(get_dynamic_limit, (int) _dynamic_limit_objects);

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

	Lua_ItemType::Register(L, Lua_ItemType_Get, 0, 0, Lua_ItemType_Mnemonics);
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
