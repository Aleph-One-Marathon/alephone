/*
LUA_SAVED_OBJECTS.CPP

	Copyright (C) 2010 by Gregory Smith
 
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

	Implements the Lua saved map objects classes
*/

#include "lua_monsters.h"
#include "lua_objects.h"
#include "lua_player.h"
#include "lua_saved_objects.h"
#include "lua_templates.h"

#include "lua_map.h"

#include "SoundManagerEnums.h"

const float AngleConvert = 360/float(FULL_CIRCLE);

map_object* get_map_object(short index) {
	return saved_objects + index;
}

template<class T>
static int get_saved_object_facing(lua_State* L)
{
	map_object* object = get_map_object(T::Index(L, 1));
	lua_pushnumber(L, (double) object->facing * AngleConvert);
	return 1;
}

template<class T, int16 flag>
static int get_saved_object_flag(lua_State* L)
{
	map_object* object = get_map_object(T::Index(L, 1));
	lua_pushboolean(L, object->flags & flag);
	return 1;
}

template<class T>
static int get_saved_object_polygon(lua_State* L)
{
	map_object* object = get_map_object(T::Index(L, 1));
	Lua_Polygon::Push(L, object->polygon_index);
	return 1;
}

template<class T>
static int get_saved_object_x(lua_State* L)
{
	map_object* object = get_map_object(T::Index(L, 1));
	lua_pushnumber(L, (double) object->location.x / WORLD_ONE);
	return 1;
}

template<class T>
static int get_saved_object_y(lua_State* L)
{
	map_object* object = get_map_object(T::Index(L, 1));
	lua_pushnumber(L, (double) object->location.y / WORLD_ONE);
	return 1;
}

template<class T>
static int get_saved_object_z(lua_State* L)
{
	map_object* object = get_map_object(T::Index(L, 1));
	lua_pushnumber(L, (double) object->location.z / WORLD_ONE);
	return 1;
}

int saved_objects_length() {
	return dynamic_world->initial_objects_count;
}

template<short T>
bool saved_object_valid(short index)
{
	if (index < 0 || index >= dynamic_world->initial_objects_count) 
		return false;
	
	map_object* object = get_map_object(index);
	return object->type == T;
}

char Lua_Goal_Name[] = "goal";

static int Lua_Goal_Get_ID(lua_State* L)
{
	map_object* object = get_map_object(Lua_Goal::Index(L, 1));
	lua_pushnumber(L, object->index);
	return 1;
}

const luaL_Reg Lua_Goal_Get[] = {
	{"facing", get_saved_object_facing<Lua_Goal>},
	{"id", Lua_Goal_Get_ID},
	{"polygon", get_saved_object_polygon<Lua_Goal>},
	{"x", get_saved_object_x<Lua_Goal>},
	{"y", get_saved_object_y<Lua_Goal>},
	{"z", get_saved_object_z<Lua_Goal>},
	{0, 0}
};

char Lua_Goals_Name[] = "Goals";

char Lua_ItemStart_Name[] = "item_start";

static int Lua_ItemStart_Get_Type(lua_State* L)
{
	map_object* object = get_map_object(Lua_ItemStart::Index(L, 1));
	Lua_ItemType::Push(L, object->index);
	return 1;
}

const luaL_Reg Lua_ItemStart_Get[] = {
	{"facing", get_saved_object_facing<Lua_ItemStart>},
	{"from_ceiling", get_saved_object_flag<Lua_ItemStart, _map_object_hanging_from_ceiling>},
	{"invisible", get_saved_object_flag<Lua_ItemStart, _map_object_is_invisible>},
	{"polygon", get_saved_object_polygon<Lua_ItemStart>},
	{"type", Lua_ItemStart_Get_Type},
	{"x", get_saved_object_x<Lua_ItemStart>},
	{"y", get_saved_object_y<Lua_ItemStart>},
	{"z", get_saved_object_z<Lua_ItemStart>},
	{0, 0}
};

char Lua_ItemStarts_Name[] = "ItemStarts";

char Lua_MonsterStart_Name[] = "monster_start";

static int Lua_MonsterStart_Get_Type(lua_State* L)
{
	map_object* object = get_map_object(Lua_MonsterStart::Index(L, 1));
	Lua_MonsterType::Push(L, object->index);
	return 1;
}

const luaL_Reg Lua_MonsterStart_Get[] = {
	{"blind", get_saved_object_flag<Lua_MonsterStart, _map_object_is_blind>},
	{"deaf", get_saved_object_flag<Lua_MonsterStart, _map_object_is_deaf>},
	{"facing", get_saved_object_facing<Lua_MonsterStart>},
	{"from_ceiling", get_saved_object_flag<Lua_MonsterStart, _map_object_hanging_from_ceiling>},
	{"invisible", get_saved_object_flag<Lua_MonsterStart, _map_object_is_invisible>},
	{"polygon", get_saved_object_polygon<Lua_MonsterStart>},
	{"type", Lua_MonsterStart_Get_Type},
	{"x", get_saved_object_x<Lua_MonsterStart>},
	{"y", get_saved_object_y<Lua_MonsterStart>},
	{"z", get_saved_object_z<Lua_MonsterStart>},
	{0, 0}
};

char Lua_MonsterStarts_Name[] = "MonsterStarts";

char Lua_PlayerStart_Name[] = "player_start";

static int Lua_PlayerStart_Get_Team(lua_State* L)
{
	map_object* object = get_map_object(Lua_PlayerStart::Index(L, 1));
	Lua_PlayerColor::Push(L, object->index);
	return 1;
}

const luaL_Reg Lua_PlayerStart_Get[] = {
	{"facing", get_saved_object_facing<Lua_PlayerStart>},
	{"from_ceiling", get_saved_object_flag<Lua_PlayerStart, _map_object_hanging_from_ceiling>},
	{"polygon", get_saved_object_polygon<Lua_PlayerStart>},
	{"team", Lua_PlayerStart_Get_Team},
	{"x", get_saved_object_x<Lua_PlayerStart>},
	{"y", get_saved_object_y<Lua_PlayerStart>},
	{"z", get_saved_object_z<Lua_PlayerStart>},
	{0, 0}
};

char Lua_PlayerStarts_Name[] = "PlayerStarts";

char Lua_SoundObject_Name[] = "sound_object";

static int Lua_SoundObject_Get_Light(lua_State* L)
{
	map_object* object = get_map_object(Lua_SoundObject::Index(L, 1));
	if (object->facing < 0)
	{
		Lua_Light::Push(L, -object->facing);
	}
	else
	{
		lua_pushnil(L);
	}

	return 1;
}

static int Lua_SoundObject_Get_Type(lua_State* L)
{
	map_object* object = get_map_object(Lua_SoundObject::Index(L, 1));
	Lua_AmbientSound::Push(L, object->index);
	return 1;
}

static int Lua_SoundObject_Get_Volume(lua_State* L)
{
	map_object* object = get_map_object(Lua_SoundObject::Index(L, 1));
	if (object->facing >= 0) 
	{
		lua_pushnumber(L, (double) object->facing / MAXIMUM_SOUND_VOLUME);
	}
	else
	{
		lua_pushnil(L);
	}

	return 1;
}

const luaL_Reg Lua_SoundObject_Get[] = {
	{"from_ceiling", get_saved_object_flag<Lua_SoundObject, _map_object_hanging_from_ceiling>},
	{"light", Lua_SoundObject_Get_Light},
	{"on_platform", get_saved_object_flag<Lua_SoundObject, _map_object_is_platform_sound>},
	{"polygon", get_saved_object_polygon<Lua_SoundObject>},
	{"type", Lua_SoundObject_Get_Type},
	{"volume", Lua_SoundObject_Get_Volume},
	{"x", get_saved_object_x<Lua_SoundObject>},
	{"y", get_saved_object_y<Lua_SoundObject>},
	{"z", get_saved_object_z<Lua_SoundObject>},
	{0, 0}
};

char Lua_SoundObjects_Name[] = "SoundObjects";

int Lua_Saved_Objects_register(lua_State* L)
{
	Lua_Goal::Register(L, Lua_Goal_Get);
	Lua_Goal::Valid = saved_object_valid<_saved_goal>;

	Lua_Goals::Register(L);
	Lua_Goals::Length = saved_objects_length;

	Lua_ItemStart::Register(L, Lua_ItemStart_Get);
	Lua_ItemStart::Valid = saved_object_valid<_saved_item>;

	Lua_ItemStarts::Register(L);
	Lua_ItemStarts::Length = saved_objects_length;

	Lua_MonsterStart::Register(L, Lua_MonsterStart_Get);
	Lua_MonsterStart::Valid = saved_object_valid<_saved_monster>;

	Lua_MonsterStarts::Register(L);
	Lua_MonsterStarts::Length = saved_objects_length;

	Lua_PlayerStart::Register(L, Lua_PlayerStart_Get);
	Lua_PlayerStart::Valid = saved_object_valid<_saved_player>;
	
	Lua_PlayerStarts::Register(L);
	Lua_PlayerStarts::Length = saved_objects_length;

	Lua_SoundObject::Register(L, Lua_SoundObject_Get);
	Lua_SoundObject::Valid = saved_object_valid<_saved_sound_source>;

	Lua_SoundObjects::Register(L);
	Lua_SoundObjects::Length = saved_objects_length;
	
	return 0;
}
