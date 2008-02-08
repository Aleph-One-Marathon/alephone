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
#include "lua_templates.h"
#include "map.h"
#include "platforms.h"

#ifdef HAVE_LUA

const char *Lua_DamageTypes::name = "DamageTypes";

const luaL_reg Lua_DamageTypes::metatable[] = {
	{"__call", L_GlobalCall<Lua_DamageTypes, Lua_DamageType>},
	{"__index", L_GlobalIndex<Lua_DamageTypes, Lua_DamageType>},
	{"__newindex}", L_GlobalNewindex<Lua_DamageTypes>},
	{0, 0}
};

const luaL_reg Lua_DamageTypes::methods[] = {
	{0, 0}
};

const char *Lua_DamageType::name = "damage_type";

const luaL_reg Lua_DamageType::metatable[] = {
	{"__index", L_TableGet<Lua_DamageType>},
	{"__newindex", L_TableSet<Lua_DamageType>},
	{0, 0}
};

const luaL_reg Lua_DamageType::index_table[] = {
	{"index", L_TableIndex<Lua_DamageType>},
	{0, 0}
};

const luaL_reg Lua_DamageType::newindex_table[] = {
	{0, 0}
};

int Lua_Platform::get_polygon(lua_State *L)
{
	L_Push<Lua_Polygon>(L, get_platform_data(L_Index<Lua_Platform>(L, 1))->polygon_index);
	return 1;
}

const char *Lua_Platform::name = "platform";

const luaL_reg Lua_Platform::index_table[] = {
	{"index", L_TableIndex<Lua_Platform>},
	{"polygon", Lua_Platform::get_polygon},
	{0, 0}
};

const luaL_reg Lua_Platform::newindex_table[] = {
	{0, 0}
};

const luaL_reg Lua_Platform::metatable[] = {
	{"__index", L_TableGet<Lua_Platform>},
	{"__newindex", L_TableSet<Lua_Platform>},
	{0, 0}
};

struct Lua_Polygon_Floor {
	short index;
	static const char *name;
	static bool valid(int index) { return Lua_Polygons::valid(index); }

	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];

	static int get_height(lua_State *L);
	static int set_height(lua_State *L);
};

const char* Lua_Polygon_Floor::name = "polygon_floor";

const luaL_reg Lua_Polygon_Floor::index_table[] = {
	{"height", Lua_Polygon_Floor::get_height},
	{"z", Lua_Polygon_Floor::get_height},
	{0, 0}
};

const luaL_reg Lua_Polygon_Floor::newindex_table[] = {
	{"height", Lua_Polygon_Floor::set_height},
	{"z", Lua_Polygon_Floor::set_height},
	{0, 0}
};

const luaL_reg Lua_Polygon_Floor::metatable[] = {
	{"__index", L_TableGet<Lua_Polygon_Floor>},
	{"__newindex", L_TableSet<Lua_Polygon_Floor>},
	{0, 0}
};

int Lua_Polygon_Floor::get_height(lua_State *L)
{
	lua_pushnumber(L, (double) (get_polygon_data(L_Index<Lua_Polygon_Floor>(L, 1))->floor_height) / WORLD_ONE);
	return 1;
}

int Lua_Polygon_Floor::set_height(lua_State *L)
{
	if (!lua_isnumber(L, 2))
	{
		luaL_error(L, "height: incorrect argument type");
	}

	struct polygon_data *polygon = get_polygon_data(L_Index<Lua_Polygon_Floor>(L, 1));
	polygon->floor_height = static_cast<world_distance>(lua_tonumber(L,2)*WORLD_ONE);
	for (short i = 0; i < polygon->vertex_count; ++i)
	{
		recalculate_redundant_endpoint_data(polygon->endpoint_indexes[i]);
		recalculate_redundant_line_data(polygon->line_indexes[i]);
	}
	return 0;
}

struct Lua_Polygon_Ceiling {
	short index;
	static const char *name;
	static bool valid(int index) { return Lua_Polygons::valid(index); }

	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];

	static int get_height(lua_State *L);
	static int set_height(lua_State *L);
};

const char* Lua_Polygon_Ceiling::name = "polygon_ceiling";

const luaL_reg Lua_Polygon_Ceiling::index_table[] = {
	{"height", Lua_Polygon_Ceiling::get_height},
	{"z", Lua_Polygon_Ceiling::get_height},
	{0, 0}
};

const luaL_reg Lua_Polygon_Ceiling::newindex_table[] = {
	{"height", Lua_Polygon_Ceiling::set_height},
	{"z", Lua_Polygon_Ceiling::set_height},
	{0, 0}
};

const luaL_reg Lua_Polygon_Ceiling::metatable[] = {
	{"__index", L_TableGet<Lua_Polygon_Ceiling>},
	{"__newindex", L_TableSet<Lua_Polygon_Ceiling>},
	{0, 0}
};

int Lua_Polygon_Ceiling::get_height(lua_State *L)
{
	lua_pushnumber(L, (double) (get_polygon_data(L_Index<Lua_Polygon_Ceiling>(L, 1))->ceiling_height) / WORLD_ONE);
	return 1;
}

int Lua_Polygon_Ceiling::set_height(lua_State *L)
{
	if (!lua_isnumber(L, 2))
	{
		luaL_error(L, "height: incorrect argument type");
	}

	struct polygon_data *polygon = get_polygon_data(L_Index<Lua_Polygon_Ceiling>(L, 1));
	polygon->ceiling_height = static_cast<world_distance>(lua_tonumber(L,2)*WORLD_ONE);
	for (short i = 0; i < polygon->vertex_count; ++i)
	{
		recalculate_redundant_endpoint_data(polygon->endpoint_indexes[i]);
		recalculate_redundant_line_data(polygon->line_indexes[i]);
	}
	return 0;
}

int Lua_Polygon::get_ceiling(lua_State *L)
{
	L_Push<Lua_Polygon_Ceiling>(L, L_Index<Lua_Polygon>(L, 1));
	return 1;
}

int Lua_Polygon::get_floor(lua_State *L)
{
	L_Push<Lua_Polygon_Floor>(L, L_Index<Lua_Polygon>(L, 1));
	return 1;
}

int Lua_Polygon::get_type(lua_State *L)
{
	lua_pushnumber(L, get_polygon_data(L_Index<Lua_Polygon>(L, 1))->type);
	return 1;
}

int Lua_Polygon::get_x(lua_State *L)
{
	lua_pushnumber(L, (double) get_polygon_data(L_Index<Lua_Polygon>(L, 1))->center.x / WORLD_ONE);
	return 1;
}

int Lua_Polygon::get_y(lua_State *L)
{
	lua_pushnumber(L, (double) get_polygon_data(L_Index<Lua_Polygon>(L, 1))->center.y / WORLD_ONE);
	return 1;
}

int Lua_Polygon::get_z(lua_State *L)
{
	lua_pushnumber(L, (double) get_polygon_data(L_Index<Lua_Polygon>(L, 1))->floor_height / WORLD_ONE);
	return 1;
}

int Lua_Polygon::set_type(lua_State *L)
{
	if (!lua_isnumber(L, 2))
	{
		luaL_error(L, "type: incorrect argument type");
	}

	int type = static_cast<int>(lua_tonumber(L, 2));
	get_polygon_data(L_Index<Lua_Polygon>(L, 1))->type = type;

	return 0;
}

const char *Lua_Polygon::name = "polygon";

const luaL_reg Lua_Polygon::index_table[] = {
	{"ceiling", Lua_Polygon::get_ceiling},
	{"floor", Lua_Polygon::get_floor},
	{"index", L_TableIndex<Lua_Polygon>},
	{"type", Lua_Polygon::get_type},
	{"x", Lua_Polygon::get_x},
	{"y", Lua_Polygon::get_y},
	{"z", Lua_Polygon::get_z},
	{0, 0}
};

const luaL_reg Lua_Polygon::newindex_table[] = {
	{"type", Lua_Polygon::set_type},
	{0, 0}
};

const luaL_reg Lua_Polygon::metatable[] = {
	{"__index", L_TableGet<Lua_Polygon>},
	{"__newindex", L_TableSet<Lua_Polygon>},
	{0, 0}
};

static int compatibility(lua_State *L);

const char *Lua_Polygons::name = "Polygons";
const luaL_reg Lua_Polygons::methods[] = {
	{0, 0}
};

const luaL_reg Lua_Polygons::metatable[] = {
	{"__index", L_GlobalIndex<Lua_Polygons, Lua_Polygon>},
	{"__newindex", L_GlobalNewindex<Lua_Polygons>},
	{"__len", L_GlobalLength<Lua_Polygons>},
	{"__call", L_GlobalCall<Lua_Polygons, Lua_Polygon>},
	{0, 0}
};

int Lua_Map_register(lua_State *L)
{
	L_Register<Lua_DamageType>(L);
	L_GlobalRegister<Lua_DamageTypes>(L);
	L_Register<Lua_Platform>(L);
	L_Register<Lua_Polygon_Floor>(L);
	L_Register<Lua_Polygon_Ceiling>(L);
	L_Register<Lua_Polygon>(L);
	L_GlobalRegister<Lua_Polygons>(L);

	compatibility(L);
}

static const char* compatibility_script = ""
	"function get_polygon_ceiling_height(polygon) return Polygons[polygon].ceiling.height end\n"
	"function get_polygon_center(polygon) return Polygons[polygon].x * 1024, Polygons[polygon].y * 1024 end\n"
	"function get_polygon_floor_height(polygon) return Polygons[polygon].floor.height end\n"
	"function get_polygon_type(polygon) return Polygons[polygon].type end\n"
	"function number_of_polygons() return # Polygons end\n"
	"function set_polygon_ceiling_height(polygon, height) Polygons[polygon].ceiling.height = height end\n"
	"function set_polygon_floor_height(polygon, height) Polygons[polygon].floor.height = height end\n"
	"function set_polygon_type(polygon, type) Polygons[polygon].type = type end\n"
	;

static int compatibility(lua_State *L)
{
	luaL_loadbuffer(L, compatibility_script, strlen(compatibility_script), "map_compatibility");
	lua_pcall(L, 0, 0, 0);
}

#endif

