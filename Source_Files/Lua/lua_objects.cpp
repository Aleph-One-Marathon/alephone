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

#include "items.h"
#include "scenery.h"
#define DONT_REPEAT_DEFINITIONS
#include "scenery_definitions.h"

#ifdef HAVE_LUA

template<class T>
int lua_delete_object(lua_State *L)
{
	remove_map_object(L_Index<T>(L, 1));
	return 0;
}

template<>
int lua_delete_object<Lua_Scenery>(lua_State *L)
{
	short object_index = L_Index<Lua_Scenery>(L, 1);
	remove_map_object(object_index);
	deanimate_scenery(object_index);
	return 0;
}

template<class T>
static int get_object_polygon(lua_State *L)
{
	object_data *object = get_object_data(L_Index<T>(L, 1));
	L_Push<Lua_Polygon>(L, object->polygon);
	return 1;
}

template<class T, class TT>
static int get_object_type(lua_State *L)
{
	object_data *object = get_object_data(L_Index<T>(L, 1));
	L_Push<TT>(L, object->permutation);
	return 1;
}

template<class T>
static int get_object_x(lua_State *L)
{
	object_data *object = get_object_data(L_Index<T>(L, 1));
	lua_pushnumber(L, (double) object->location.x / WORLD_ONE);
	return 1;
}

template<class T>
static int get_object_y(lua_State *L)
{
	object_data *object = get_object_data(L_Index<T>(L, 1));
	lua_pushnumber(L, (double) object->location.y / WORLD_ONE);
	return 1;
}

template<class T>
static int get_object_z(lua_State *L)
{
	object_data *object = get_object_data(L_Index<T>(L, 1));
	lua_pushnumber(L, (double) object->location.z / WORLD_ONE);
	return 1;
}

const char *Lua_Item::name = "item";

const luaL_reg Lua_Item::index_table[] = {
	{"delete", L_TableFunction<lua_delete_object<Lua_Item> >},
	{"index", L_TableIndex<Lua_Item>},
	{"polygon", get_object_polygon<Lua_Item>},
	{"type", get_object_type<Lua_Item, Lua_ItemType>},
	{"x", get_object_x<Lua_Item>},
	{"y", get_object_y<Lua_Item>},
	{"z", get_object_z<Lua_Item>},
	{0, 0}
};

const luaL_reg Lua_Item::newindex_table[] = {
	{0, 0}
};

const luaL_reg Lua_Item::metatable[] = {
	{"__index", L_TableGet<Lua_Item>},
	{"__newindex", L_TableSet<Lua_Item>},
	{0, 0}
};

const char *Lua_Items::name = "Items";

const luaL_reg Lua_Items::methods[] = {
	{"new", Lua_Items::new_item},
	{0, 0}
};

const luaL_reg Lua_Items::metatable[] = {
	{"__index", L_GlobalIndex<Lua_Items, Lua_Item>},
	{"__newindex", L_GlobalNewindex<Lua_Items>},
	{"__call", L_GlobalCall<Lua_Items, Lua_Item>},
	{0, 0}
};

// Items.new(x, y, height, polygon, type)
int Lua_Items::new_item(lua_State *L)
{
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		luaL_error(L, "new: incorrect argument type");

	int polygon_index = 0;
	if (lua_isnumber(L, 4))
	{
		polygon_index = static_cast<int>(lua_tonumber(L, 4));
		if (!Lua_Polygons::valid(polygon_index))
			return luaL_error(L, "new: invalid polygon index");
	}
	else if (L_Is<Lua_Polygon>(L, 4))
	{
		polygon_index = L_Index<Lua_Polygon>(L, 4);
	}
	else
		return luaL_error(L, "new: incorrect argument type");

	short item_type = L_ToIndex<Lua_ItemType>(L, 5);

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

	L_Push<Lua_Item>(L, item_index);
	return 1;
}
	
bool Lua_Items::valid(int index)
{
	if (index < 0 || index >= MAXIMUM_OBJECTS_PER_MAP)
		return false;

	object_data *object = GetMemberWithBounds(objects, index, MAXIMUM_OBJECTS_PER_MAP);
	return (SLOT_IS_USED(object) && GET_OBJECT_OWNER(object) == _object_is_item);
}

int Lua_ItemType::get_ball(lua_State *L)
{
	lua_pushboolean(L, (get_item_kind(L_Index<Lua_ItemType>(L, 1)) == _ball));
	return 1;
}

const char *Lua_ItemType::name = "item_type";
const luaL_reg Lua_ItemType::index_table[] = {
	{"ball", Lua_ItemType::get_ball},
	{"index", L_TableIndex<Lua_ItemType>},
	{0, 0}
};

const luaL_reg Lua_ItemType::newindex_table[] = {
	{0, 0}
};

const luaL_reg Lua_ItemType::metatable[] = {
	{"__eq", L_Equals<Lua_ItemType>},
	{"__index", L_TableGet<Lua_ItemType>},
	{"__newindex", L_TableSet<Lua_ItemType>},
	{0, 0}
};

const char *Lua_ItemTypes::name = "ItemTypes";
const luaL_reg Lua_ItemTypes::metatable[] = {
	{"__index", L_GlobalIndex<Lua_ItemTypes, Lua_ItemType>},
	{"__newindex", L_GlobalNewindex<Lua_ItemTypes>},
	{"__call", L_GlobalCall<Lua_ItemTypes, Lua_ItemType>},
	{0, 0}
};

const luaL_reg Lua_ItemTypes::methods[] = {
	{0, 0}
};

struct Lua_SceneryTypes {
	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg methods[];
	static int length() { return NUMBER_OF_SCENERY_DEFINITIONS; }
	static bool valid(int index) { return index >= 0 && index < NUMBER_OF_SCENERY_DEFINITIONS; }
};

struct Lua_SceneryType {
	short index;
	static bool valid(int index) { return Lua_SceneryTypes::valid(index); }
	
	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];
};

const char *Lua_SceneryType::name = "scenery_type";
const luaL_reg Lua_SceneryType::index_table[] = {
	{"index", L_TableIndex<Lua_SceneryType>},
	{0, 0}
};

const luaL_reg Lua_SceneryType::newindex_table[] = {
	{0, 0}
};

const luaL_reg Lua_SceneryType::metatable[] = {
	{"__eq", L_Equals<Lua_SceneryType>},
	{"__index", L_TableGet<Lua_SceneryType>},
	{"__newindex", L_TableSet<Lua_SceneryType>},
	{0, 0}
};

const char *Lua_SceneryTypes::name = "SceneryTypes";
const luaL_reg Lua_SceneryTypes::metatable[] = {
	{"__index", L_GlobalIndex<Lua_SceneryTypes, Lua_SceneryType>},
	{"__newindex", L_GlobalNewindex<Lua_SceneryTypes>},
	{"__call", L_GlobalCall<Lua_SceneryTypes, Lua_SceneryType>},
	{0, 0}
};

const luaL_reg Lua_SceneryTypes::methods[] = {
	{0, 0}
};

const char *Lua_Scenery::name = "scenery";

int Lua_Scenery::damage(lua_State *L)
{
	damage_scenery(L_Index<Lua_Scenery>(L, 1));
	return 0;
}

int Lua_Scenery::get_solid(lua_State *L)
{
	object_data *object = get_object_data(L_Index<Lua_Scenery>(L, 1));
	lua_pushboolean(L, OBJECT_IS_SOLID(object));
	return 1;
}

const luaL_reg Lua_Scenery::index_table[] = {
	{"damage", L_TableFunction<Lua_Scenery::damage>},
	{"delete", L_TableFunction<lua_delete_object<Lua_Scenery> >},
	{"index", L_TableIndex<Lua_Scenery>},
	{"polygon", get_object_polygon<Lua_Scenery>},
	{"solid", Lua_Scenery::get_solid},
	{"type", get_object_type<Lua_Scenery, Lua_SceneryType>},
	{"x", get_object_x<Lua_Scenery>},
	{"y", get_object_y<Lua_Scenery>},
	{"z", get_object_z<Lua_Scenery>},
	{0, 0}
};

int Lua_Scenery::set_solid(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "solid: incorrect argument type");

	object_data *object = get_object_data(L_Index<Lua_Scenery>(L, 1));
	SET_OBJECT_SOLIDITY(object, lua_toboolean(L, 2));
	return 0;
}

const luaL_reg Lua_Scenery::newindex_table[] = {
	{"solid", Lua_Scenery::set_solid},
	{0, 0}
};

const luaL_reg Lua_Scenery::metatable[] = {
	{"__index", L_TableGet<Lua_Scenery>},
	{"__newindex", L_TableSet<Lua_Scenery>},
	{0, 0}
};

const char *Lua_Sceneries::name = "Scenery";

// Scenery.new(x, y, height, polygon, type)
int Lua_Sceneries::new_scenery(lua_State *L)
{
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		luaL_error(L, "new: incorrect argument type");

	int polygon_index = 0;
	if (lua_isnumber(L, 4))
	{
		polygon_index = static_cast<int>(lua_tonumber(L, 4));
		if (!Lua_Polygons::valid(polygon_index))
			return luaL_error(L, "new: invalid polygon index");
	}
	else if (L_Is<Lua_Polygon>(L, 4))
	{
		polygon_index = L_Index<Lua_Polygon>(L, 4);
	}
	else
		return luaL_error(L, "new: incorrect argument type");

	short scenery_type = L_ToIndex<Lua_SceneryType>(L, 5);

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

	L_Push<Lua_Scenery>(L, scenery_index);
	return 1;
}

bool Lua_Sceneries::valid(int index)
{
	if (index < 0 || index >= MAXIMUM_OBJECTS_PER_MAP)
		return false;

	object_data *object = GetMemberWithBounds(objects, index, MAXIMUM_OBJECTS_PER_MAP);
	return (SLOT_IS_USED(object) && GET_OBJECT_OWNER(object) == _object_is_scenery);
}

const luaL_reg Lua_Sceneries::methods[] = {
	{"new", Lua_Sceneries::new_scenery},
	{0, 0}
};

const luaL_reg Lua_Sceneries::metatable[] = {
	{"__index", L_GlobalIndex<Lua_Sceneries, Lua_Scenery>},
	{"__newindex", L_GlobalNewindex<Lua_Sceneries>},
	{"__call", L_GlobalCall<Lua_Sceneries, Lua_Scenery>},
	{0, 0}
};

static void compatibility(lua_State *L);

int Lua_Objects_register(lua_State *L)
{
	L_Register<Lua_Item>(L);
	L_GlobalRegister<Lua_Items>(L);
	L_Register<Lua_ItemType>(L);
	L_GlobalRegister<Lua_ItemTypes>(L);
	L_Register<Lua_Scenery>(L);
	L_GlobalRegister<Lua_Sceneries>(L);
	L_Register<Lua_SceneryType>(L);
	L_GlobalRegister<Lua_SceneryTypes>(L);

	compatibility(L);
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
