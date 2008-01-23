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

#ifdef HAVE_LUA

struct Lua_Polygon {
	short index;
	static const char *name;

	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];

	static int get_ceiling_height(lua_State *L);
	static int get_floor_height(lua_State *L);
	static int get_type(lua_State *L);
	static int set_ceiling_height(lua_State *L);
	static int set_floor_height(lua_State *L);
	static int set_type(lua_State *L);
};

int Lua_Polygon::get_ceiling_height(lua_State *L)
{
	lua_pushnumber(L, (double) (get_polygon_data(L_Index<Lua_Polygon>(L, 1))->ceiling_height) / WORLD_ONE);
	return 1;
}

int Lua_Polygon::get_floor_height(lua_State *L)
{
	lua_pushnumber(L, (double) (get_polygon_data(L_Index<Lua_Polygon>(L, 1))->floor_height) / WORLD_ONE);
	return 1;
}

int Lua_Polygon::get_type(lua_State *L)
{
	lua_pushnumber(L, get_polygon_data(L_Index<Lua_Polygon>(L, 1))->type);
	return 1;
}

int Lua_Polygon::set_ceiling_height(lua_State *L)
{
	if (!lua_isnumber(L, 2))
	{
		luaL_error(L, "polygon.ceiling_height: incorrect argument type");
	}

	struct polygon_data *polygon = get_polygon_data(L_Index<Lua_Polygon>(L, 1));
	polygon->ceiling_height = static_cast<world_distance>(lua_tonumber(L,2)*WORLD_ONE);
	for (short i = 0; i < polygon->vertex_count; ++i)
	{
		recalculate_redundant_endpoint_data(polygon->endpoint_indexes[i]);
		recalculate_redundant_line_data(polygon->line_indexes[i]);
	}
	return 0;
}

int Lua_Polygon::set_floor_height(lua_State *L)
{
	if (!lua_isnumber(L, 2))
	{
		luaL_error(L, "polygon.floor_height: incorrect argument type");
	}

	struct polygon_data *polygon = get_polygon_data(L_Index<Lua_Polygon>(L, 1));
	polygon->floor_height = static_cast<world_distance>(lua_tonumber(L,2)*WORLD_ONE);
	for (short i = 0; i < polygon->vertex_count; ++i)
	{
		recalculate_redundant_endpoint_data(polygon->endpoint_indexes[i]);
		recalculate_redundant_line_data(polygon->line_indexes[i]);
	}
	return 0;
}

int Lua_Polygon::set_type(lua_State *L)
{
	if (!lua_isnumber(L, 2))
	{
		luaL_error(L, "polygon.type: incorrect argument type");
	}

	int type = static_cast<int>(lua_tonumber(L, 2));
	get_polygon_data(L_Index<Lua_Polygon>(L, 1))->type = type;

	return 0;
}

const char *Lua_Polygon::name = "polygon";

const luaL_reg Lua_Polygon::index_table[] = {
	{"ceiling_height", Lua_Polygon::get_ceiling_height},
	{"floor_height", Lua_Polygon::get_floor_height},
	{"index", L_TableIndex<Lua_Polygon>},
	{"type", Lua_Polygon::get_type},
	{0, 0}
};

const luaL_reg Lua_Polygon::newindex_table[] = {
	{"ceiling_height", Lua_Polygon::set_ceiling_height},
	{"floor_height", Lua_Polygon::set_floor_height},
	{"type", Lua_Polygon::set_type},
	{0, 0}
};

const luaL_reg Lua_Polygon::metatable[] = {
	{"__index", L_TableGet<Lua_Polygon>},
	{"__newindex", L_TableSet<Lua_Polygon>},
	{0, 0}
};

struct Lua_Polygons {
	static const char *name;
	static const luaL_reg metatable[];
	static int length() { return dynamic_world->polygon_count; }
	static bool valid(int) { return true; }
};

static int compatibility(lua_State *L);

const char *Lua_Polygons::name = "Polygons";
const luaL_reg Lua_Polygons::metatable[] = {
	{"__index", L_GlobalIndex<Lua_Polygons, Lua_Polygon>},
	{"__newindex", L_GlobalNewindex<Lua_Polygons>},
	{"__len", L_GlobalLength<Lua_Polygons>},
	{"__call", L_GlobalCall<Lua_Polygons, Lua_Polygon>},
	{0, 0}
};

int Lua_Map_register(lua_State *L)
{
	L_Register<Lua_Polygon>(L);
	L_GlobalRegister<Lua_Polygons>(L);

	compatibility(L);
}

static const char* compatibility_script = ""
	"function get_polygon_ceiling_height(polygon) return Polygons[polygon].ceiling_height end\n"
	"function get_polygon_floor_height(polygon) return Polygons[polygon].floor_height end\n"
	"function get_polygon_type(polygon) return Polygons[polygon].type end\n"
	"function set_polygon_ceiling_height(polygon, height) Polygons[polygon].ceiling_height = height end\n"
	"function set_polygon_floor_height(polygon, height) Polygons[polygon].floor_height = height end\n"
	"function set_polygon_type(polygon, type) Polygons[polygon].type = type end\n"
	;

static int compatibility(lua_State *L)
{
	luaL_loadbuffer(L, compatibility_script, strlen(compatibility_script), "map_compatibility");
	lua_pcall(L, 0, 0, 0);
}

#endif

