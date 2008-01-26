/*
LUA_MONSTERS.CPP

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

	Implements the Lua monster classes
*/

#include "lua_map.h"
#include "lua_monsters.h"
#include "lua_player.h"
#include "lua_projectiles.h"
#include "lua_templates.h"

#include "monsters.h"
#include "player.h"

#ifdef HAVE_LUA


const float AngleConvert = 360/float(FULL_CIRCLE);

const char *Lua_Projectile::name = "projectile";

int Lua_Projectile::get_damage_scale(lua_State *L)
{
	projectile_data *projectile = get_projectile_data(L_Index<Lua_Projectile>(L, 1));
	lua_pushnumber(L, (double) projectile->damage_scale / FIXED_ONE);
	return 1;
}

int Lua_Projectile::get_elevation(lua_State *L)
{
	projectile_data *projectile = get_projectile_data(L_Index<Lua_Projectile>(L, 1));
	double elevation = projectile->elevation * AngleConvert;
	if (elevation > 180) elevation -= 360.0;
	lua_pushnumber(L, elevation);
	return 1;
}

int Lua_Projectile::get_facing(lua_State *L)
{
	projectile_data *projectile = get_projectile_data(L_Index<Lua_Projectile>(L, 1));
	object_data *object = get_object_data(projectile->object_index);
	lua_pushnumber(L, (double) object->facing * AngleConvert);
	return 1;
}

int Lua_Projectile::get_gravity(lua_State *L)
{
	projectile_data *projectile = get_projectile_data(L_Index<Lua_Projectile>(L, 1));
	lua_pushnumber(L, (double) projectile->gravity / WORLD_ONE);
	return 1;
}

int Lua_Projectile::get_owner(lua_State *L)
{
	projectile_data *projectile = get_projectile_data(L_Index<Lua_Projectile>(L, 1));
	L_Push<Lua_Monster>(L, projectile->owner_index);
	return 1;
}

int Lua_Projectile::get_polygon(lua_State *L)
{
	projectile_data *projectile = get_projectile_data(L_Index<Lua_Projectile>(L, 1));
	object_data *object = get_object_data(projectile->object_index);
	L_Push<Lua_Polygon>(L, object->polygon);
	return 1;
}

int Lua_Projectile::get_target(lua_State *L)
{
	projectile_data *projectile = get_projectile_data(L_Index<Lua_Projectile>(L, 1));
	L_Push<Lua_Monster>(L, projectile->target_index);
	return 1;
}

int Lua_Projectile::get_type(lua_State *L)
{
	projectile_data *projectile = get_projectile_data(L_Index<Lua_Projectile>(L, 1));
	lua_pushnumber(L, projectile->type);
	return 1;
}

int Lua_Projectile::get_x(lua_State *L)
{
	projectile_data *projectile = get_projectile_data(L_Index<Lua_Projectile>(L, 1));
	object_data *object = get_object_data(projectile->object_index);
	lua_pushnumber(L, (double) object->location.x / WORLD_ONE);
	return 1;
}

int Lua_Projectile::get_y(lua_State *L)
{
	projectile_data *projectile = get_projectile_data(L_Index<Lua_Projectile>(L, 1));
	object_data *object = get_object_data(projectile->object_index);
	lua_pushnumber(L, (double) object->location.y / WORLD_ONE);
	return 1;
}

int Lua_Projectile::get_z(lua_State *L)
{
	projectile_data *projectile = get_projectile_data(L_Index<Lua_Projectile>(L, 1));
	object_data *object = get_object_data(projectile->object_index);
	lua_pushnumber(L, (double) object->location.z / WORLD_ONE);
	return 1;
}

int Lua_Projectile::set_damage_scale(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "damage_scale: incorrect argument type");

	projectile_data *projectile = get_projectile_data(L_Index<Lua_Projectile>(L, 1));
	projectile->damage_scale = static_cast<int>(lua_tonumber(L, 2) * FIXED_ONE);
	return 0;
}

int Lua_Projectile::set_elevation(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "elevation: incorrect argument type");

	projectile_data *projectile = get_projectile_data(L_Index<Lua_Projectile>(L, 1));
	double elevation = lua_tonumber(L, 2);
	if (elevation < 0.0) elevation += 360.0;
	projectile->elevation = static_cast<int>(elevation / AngleConvert);
	return 0;
}

int Lua_Projectile::set_facing(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "facing: incorrect argument type");

	projectile_data* projectile = get_projectile_data(L_Index<Lua_Projectile>(L, 1));
	object_data* object = get_object_data(projectile->object_index);
	object->facing = static_cast<int>(lua_tonumber(L, 2) / AngleConvert);
	return 0;
}

int Lua_Projectile::set_gravity(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "dz: incorrect argument type");

	projectile_data *projectile = get_projectile_data(L_Index<Lua_Projectile>(L, 1));
	projectile->gravity = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	return 0;
}

int Lua_Projectile::set_owner(lua_State *L)
{
	short monster_index = 0;
	if (lua_isnil(L, 2))
	{
		monster_index = NONE;
	}
	else if (lua_isnumber(L, 2))
	{
		monster_index = static_cast<int>(lua_tonumber(L, 2));
	}
	else if (L_Is<Lua_Monster>(L, 2))
	{
		monster_index = L_Index<Lua_Monster>(L, 2);
	}
	else if (L_Is<Lua_Player>(L, 2))
	{
		player_data *player = get_player_data(L_Index<Lua_Player>(L, 2));
		monster_index = player->monster_index;
	}
	else
	{
		return luaL_error(L, "owner: incorrect argument type");
	}

	projectile_data *projectile = get_projectile_data(L_Index<Lua_Projectile>(L, 1));
	projectile->owner_index = monster_index;
	return 0;
}

extern void add_object_to_polygon_object_list(short object_index, short polygon_index);

int Lua_Projectile::set_target(lua_State *L)
{
	short monster_index = 0;
	if (lua_isnil(L, 2))
	{
		monster_index = NONE;
	}
	else if (lua_isnumber(L, 2))
	{
		monster_index = static_cast<int>(lua_tonumber(L, 2));
	}
	else if (L_Is<Lua_Monster>(L, 2))
	{
		monster_index = L_Index<Lua_Monster>(L, 2);
	}
	else if (L_Is<Lua_Player>(L, 2))
	{
		player_data *player = get_player_data(L_Index<Lua_Player>(L, 2));
		monster_index = player->monster_index;
	}
	else
	{
		return luaL_error(L, "owner: incorrect argument type");
	}

	projectile_data *projectile = get_projectile_data(L_Index<Lua_Projectile>(L, 1));
	projectile->target_index = monster_index;
	return 0;
}

int Lua_Projectile::position(lua_State *L)
{
	if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
	{
		return luaL_error(L, "position: incorrect argument type");
	}
	
	short polygon_index = 0;
	if (lua_isnumber(L, 5))
	{
		polygon_index = static_cast<int>(lua_tonumber(L, 5));
		if (!Lua_Polygons::valid(polygon_index))
			return luaL_error(L, "position: invalid polygon index");
	}
	else if (L_Is<Lua_Polygon>(L, 5))
	{
		polygon_index = L_Index<Lua_Polygon>(L, 5);
	}
	else
		return luaL_error(L, "position: incorrect argument type");

	projectile_data *projectile = get_projectile_data(L_Index<Lua_Projectile>(L, 1));
	object_data *object = get_object_data(projectile->object_index);
	object->location.x = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	object->location.y = static_cast<int>(lua_tonumber(L, 3) * WORLD_ONE);
	object->location.z = static_cast<int>(lua_tonumber(L, 4) * WORLD_ONE);

	if (polygon_index != object->polygon)
	{
		remove_object_from_polygon_object_list(projectile->object_index);
		add_object_to_polygon_object_list(projectile->object_index, polygon_index);
	}
}

const luaL_reg Lua_Projectile::index_table[] = {
	{"damage_scale", get_damage_scale},
	{"dz", get_gravity},
	{"elevation", get_elevation},
	{"facing", get_facing},
	{"index", L_TableIndex<Lua_Projectile>},
	{"position", L_TableFunction<Lua_Projectile::position>},
	{"owner", get_owner},
	{"pitch", get_elevation},
	{"polygon", get_polygon},
	{"type", get_type},
	{"x", get_x},
	{"y", get_y},
	{"yaw", get_facing},
	{"z", get_z},
	{0, 0}
};

const luaL_reg Lua_Projectile::newindex_table[] = {
	{"damage_scale", set_damage_scale},
	{"dz", set_gravity},
	{"elevation", set_elevation},
	{"facing", set_facing},
	{"owner", set_owner},
	{"pitch", set_elevation},
	{"yaw", set_facing},
	{0, 0}
};

const luaL_reg Lua_Projectile::metatable[] = {
	{"__index", L_TableGet<Lua_Projectile>},
	{"__newindex", L_TableSet<Lua_Projectile>},
	{ 0, 0}
};

const char *Lua_Projectiles::name = "Projectiles";

// Projectiles.new(x, y, z, polygon, type)
int Lua_Projectiles::new_projectile(lua_State *L)
{
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 5))
	{
		return luaL_error(L, "new: incorrect argument type");
	}

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

	world_point3d origin;
	origin.x = static_cast<int>(lua_tonumber(L, 1) * WORLD_ONE);
	origin.y = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	origin.z = static_cast<int>(lua_tonumber(L, 3) * WORLD_ONE);

	world_point3d vector;
	vector.x = WORLD_ONE;
	vector.y = 0;
	vector.z = 0;

	short type = static_cast<int>(lua_tonumber(L, 5));
	if (type < 0 || type >= NUMBER_OF_PROJECTILE_TYPES)
		return luaL_error(L, "new: invalid projectile type");


	short projectile_index = ::new_projectile(&origin, polygon_index, &vector, 0, type, NONE, NONE, NONE, FIXED_ONE);
	L_Push<Lua_Projectile>(L, projectile_index);
	return 1;
}

const luaL_reg Lua_Projectiles::methods[] = {
	{"new", Lua_Projectiles::new_projectile},
	{0, 0}
};

const luaL_reg Lua_Projectiles::metatable[] = {
	{"__index", L_GlobalIndex<Lua_Projectiles, Lua_Projectile>},
	{"__newindex", L_GlobalNewindex<Lua_Projectiles>},
	{"__call", L_GlobalCall<Lua_Projectiles, Lua_Projectile>},
	{0, 0}
};

bool Lua_Projectiles::valid(int index)
{
	if (index < 0 || index >= MAXIMUM_PROJECTILES_PER_MAP)
		return false;

	projectile_data *projectile = GetMemberWithBounds(projectiles, index ,MAXIMUM_PROJECTILES_PER_MAP);
	return (SLOT_IS_USED(projectile));
}

static void compatibility(lua_State *L);

int Lua_Projectiles_register(lua_State *L)
{
	L_Register<Lua_Projectile>(L);
	L_GlobalRegister<Lua_Projectiles>(L);

	compatibility(L);
}

const char *compatibility_script = ""
	"function get_projectile_angle(index) local elevation = Projectiles[index].elevation if elevation < 0.0 then elevation = elevation + 360 end return Projectiles[index].facing, elevation end\n"
	"function get_projectile_owner(index) if Projectiles[index].owner then return Projectiles[index].owner.index end end\n"
	"function get_projectile_position(index) return Projectiles[index].polygon.index, Projectiles[index].x, Projectiles[index].y, Projectiles[index].z end\n"
	"function get_projectile_target(index) if Projectiles[index].target then return Projectiles[index].target.index end end\n"
	"function get_projectile_type(index) return Projectiles[index].type end\n"
	"function projectile_index_valid(index) if Projectiles[index] then return true else return false end end\n"
	"function set_projectile_angle(index, yaw, pitch) Projectiles[index].facing = yaw Projectiles[index].elevation = pitch end\n"
	"function set_projectile_owner(index, owner) Projectiles[index].owner = owner end\n"
	"function set_projectile_position(index, polygon, x, y, z) Projectiles[index]:position(x, y, z, polygon) end\n"
	"function set_projectile_target(index, target) Projectiles[index].target = target end\n"
	;

static void compatibility(lua_State *L)
{
	luaL_loadbuffer(L, compatibility_script, strlen(compatibility_script), "projectiles_compatibility");
	lua_pcall(L, 0, 0, 0);
}
#endif
