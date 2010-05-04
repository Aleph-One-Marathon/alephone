/*
LUA_SAVED_OBJECTS.CPP

	Copyright (C) 2010 by Gregory Smith
 
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

	Implements the Lua saved map objects classes
*/

#include "lua_saved_objects.h"
#include "lua_templates.h"

#include "lua_map.h"

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

const luaL_reg Lua_Goal_Get[] = {
	{"facing", get_saved_object_facing<Lua_Goal>},
	{"id", Lua_Goal_Get_ID},
	{"polygon", get_saved_object_polygon<Lua_Goal>},
	{"x", get_saved_object_x<Lua_Goal>},
	{"y", get_saved_object_y<Lua_Goal>},
	{"z", get_saved_object_z<Lua_Goal>},
	{0, 0}
};

char Lua_Goals_Name[] = "Goals";

int Lua_Saved_Objects_register(lua_State* L)
{
	Lua_Goal::Register(L, Lua_Goal_Get);
	Lua_Goal::Valid = saved_object_valid<_saved_goal>;

	Lua_Goals::Register(L);
	Lua_Goals::Length = saved_objects_length;
}
