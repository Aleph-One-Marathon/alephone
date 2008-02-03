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

#ifdef HAVE_LUA

const char *Lua_Item::name = "item";

int Lua_Item::delete_item(lua_State *L)
{
	remove_map_object(L_Index<Lua_Item>(L, 1));
	return 0;
}

int Lua_Item::get_polygon(lua_State *L)
{
	object_data *object = get_object_data(L_Index<Lua_Item>(L, 1));
	L_Push<Lua_Polygon>(L, object->polygon);
	return 1;
}

int Lua_Item::get_type(lua_State *L)
{
	object_data *object = get_object_data(L_Index<Lua_Item>(L, 1));
	L_Push<Lua_ItemType>(L, object->permutation);
	return 1;
}

int Lua_Item::get_x(lua_State *L)
{
	object_data *object = get_object_data(L_Index<Lua_Item>(L, 1));
	lua_pushnumber(L, (double) object->location.x / WORLD_ONE);
	return 1;
}

int Lua_Item::get_y(lua_State *L)
{
	object_data *object = get_object_data(L_Index<Lua_Item>(L, 1));
	lua_pushnumber(L, (double) object->location.y / WORLD_ONE);
	return 1;
}

int Lua_Item::get_z(lua_State *L)
{
	object_data *object = get_object_data(L_Index<Lua_Item>(L, 1));
	lua_pushnumber(L, (double) object->location.z / WORLD_ONE);
	return 1;
}

const luaL_reg Lua_Item::index_table[] = {
	{"delete", L_TableFunction<Lua_Item::delete_item>},
	{"index", L_TableIndex<Lua_Item>},
	{"polygon", Lua_Item::get_polygon},
	{"type", Lua_Item::get_type},
	{"x", Lua_Item::get_x},
	{"y", Lua_Item::get_y},
	{"z", Lua_Item::get_z},
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

static void compatibility(lua_State *L);

int Lua_Objects_register(lua_State *L)
{
	L_Register<Lua_Item>(L);
	L_GlobalRegister<Lua_Items>(L);
	L_Register<Lua_ItemType>(L);
	L_GlobalRegister<Lua_ItemTypes>(L);

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
