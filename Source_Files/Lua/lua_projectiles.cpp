/*
LUA_MONSTERS.CPP

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

	Implements the Lua monster classes
*/

#include "lua_map.h"
#include "lua_monsters.h"
#include "lua_objects.h"
#include "lua_player.h"
#include "lua_projectiles.h"
#include "lua_templates.h"

#include "dynamic_limits.h"
#include "map.h"
#include "monsters.h"
#include "player.h"
#include "projectiles.h"

#include <boost/bind.hpp>

#define DONT_REPEAT_DEFINITIONS
#include "projectile_definitions.h"

#ifdef HAVE_LUA

const float AngleConvert = 360/float(FULL_CIRCLE);

char Lua_Projectile_Name[] = "projectile";

int Lua_Projectile_Delete(lua_State* L)
{
	remove_projectile(Lua_Projectile::Index(L, 1));
	return 0;
}

static int Lua_Projectile_Get_Damage_Scale(lua_State *L)
{
	projectile_data *projectile = get_projectile_data(Lua_Projectile::Index(L, 1));
	lua_pushnumber(L, (double) projectile->damage_scale / FIXED_ONE);
	return 1;
}

static int Lua_Projectile_Get_Elevation(lua_State *L)
{
	projectile_data *projectile = get_projectile_data(Lua_Projectile::Index(L, 1));
	double elevation = projectile->elevation * AngleConvert;
	if (elevation > 180) elevation -= 360.0;
	lua_pushnumber(L, elevation);
	return 1;
}

static int Lua_Projectile_Get_Facing(lua_State *L)
{
	projectile_data *projectile = get_projectile_data(Lua_Projectile::Index(L, 1));
	object_data *object = get_object_data(projectile->object_index);
	lua_pushnumber(L, (double) object->facing * AngleConvert);
	return 1;
}

static int Lua_Projectile_Get_Gravity(lua_State *L)
{
	projectile_data *projectile = get_projectile_data(Lua_Projectile::Index(L, 1));
	lua_pushnumber(L, (double) projectile->gravity / WORLD_ONE);
	return 1;
}

static int Lua_Projectile_Get_Owner(lua_State *L)
{
	projectile_data *projectile = get_projectile_data(Lua_Projectile::Index(L, 1));
	Lua_Monster::Push(L, projectile->owner_index);
	return 1;
}

static int Lua_Projectile_Get_Polygon(lua_State *L)
{
	projectile_data *projectile = get_projectile_data(Lua_Projectile::Index(L, 1));
	object_data *object = get_object_data(projectile->object_index);
	Lua_Polygon::Push(L, object->polygon);
	return 1;
}

static int Lua_Projectile_Get_Target(lua_State *L)
{
	projectile_data *projectile = get_projectile_data(Lua_Projectile::Index(L, 1));
	Lua_Monster::Push(L, projectile->target_index);
	return 1;
}

static int Lua_Projectile_Get_Type(lua_State *L)
{
	projectile_data *projectile = get_projectile_data(Lua_Projectile::Index(L, 1));
	Lua_ProjectileType::Push(L, projectile->type);
	return 1;
}

static int Lua_Projectile_Get_X(lua_State *L)
{
	projectile_data *projectile = get_projectile_data(Lua_Projectile::Index(L, 1));
	object_data *object = get_object_data(projectile->object_index);
	lua_pushnumber(L, (double) object->location.x / WORLD_ONE);
	return 1;
}

static int Lua_Projectile_Get_Y(lua_State *L)
{
	projectile_data *projectile = get_projectile_data(Lua_Projectile::Index(L, 1));
	object_data *object = get_object_data(projectile->object_index);
	lua_pushnumber(L, (double) object->location.y / WORLD_ONE);
	return 1;
}

static int Lua_Projectile_Get_Z(lua_State *L)
{
	projectile_data *projectile = get_projectile_data(Lua_Projectile::Index(L, 1));
	object_data *object = get_object_data(projectile->object_index);
	lua_pushnumber(L, (double) object->location.z / WORLD_ONE);
	return 1;
}

static int Lua_Projectile_Set_Damage_Scale(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "damage_scale: incorrect argument type");

	projectile_data *projectile = get_projectile_data(Lua_Projectile::Index(L, 1));
	projectile->damage_scale = static_cast<int>(lua_tonumber(L, 2) * FIXED_ONE);
	return 0;
}

static int Lua_Projectile_Set_Elevation(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "elevation: incorrect argument type");

	projectile_data *projectile = get_projectile_data(Lua_Projectile::Index(L, 1));
	double elevation = lua_tonumber(L, 2);
	if (elevation < 0.0) elevation += 360.0;
	projectile->elevation = static_cast<int>(elevation / AngleConvert);
	return 0;
}

static int Lua_Projectile_Set_Facing(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "facing: incorrect argument type");

	projectile_data* projectile = get_projectile_data(Lua_Projectile::Index(L, 1));
	object_data* object = get_object_data(projectile->object_index);
	object->facing = static_cast<int>(lua_tonumber(L, 2) / AngleConvert);
	return 0;
}

static int Lua_Projectile_Set_Gravity(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "dz: incorrect argument type");

	projectile_data *projectile = get_projectile_data(Lua_Projectile::Index(L, 1));
	projectile->gravity = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	return 0;
}

static int Lua_Projectile_Set_Owner(lua_State *L)
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
	else if (Lua_Monster::Is(L, 2))
	{
		monster_index = Lua_Monster::Index(L, 2);
	}
	else if (Lua_Player::Is(L, 2))
	{
		player_data *player = get_player_data(Lua_Player::Index(L, 2));
		monster_index = player->monster_index;
	}
	else
	{
		return luaL_error(L, "owner: incorrect argument type");
	}

	projectile_data *projectile = get_projectile_data(Lua_Projectile::Index(L, 1));
	projectile->owner_index = monster_index;
	return 0;
}

extern void add_object_to_polygon_object_list(short object_index, short polygon_index);

static int Lua_Projectile_Set_Target(lua_State *L)
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
	else if (Lua_Monster::Is(L, 2))
	{
		monster_index = Lua_Monster::Index(L, 2);
	}
	else if (Lua_Player::Is(L, 2))
	{
		player_data *player = get_player_data(Lua_Player::Index(L, 2));
		monster_index = player->monster_index;
	}
	else
	{
		return luaL_error(L, "owner: incorrect argument type");
	}

	projectile_data *projectile = get_projectile_data(Lua_Projectile::Index(L, 1));
	projectile->target_index = monster_index;
	return 0;
}

int Lua_Projectile_Play_Sound(lua_State *L)
{
	short sound_code = Lua_Sound::ToIndex(L, 2);
	projectile_data *projectile = get_projectile_data(Lua_Projectile::Index(L, 1));
	play_object_sound(projectile->object_index, sound_code);
	return 0;
}

int Lua_Projectile_Position(lua_State *L)
{
	if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
	{
		return luaL_error(L, "position: incorrect argument type");
	}
	
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

	projectile_data *projectile = get_projectile_data(Lua_Projectile::Index(L, 1));
	object_data *object = get_object_data(projectile->object_index);
	object->location.x = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	object->location.y = static_cast<int>(lua_tonumber(L, 3) * WORLD_ONE);
	object->location.z = static_cast<int>(lua_tonumber(L, 4) * WORLD_ONE);

	if (polygon_index != object->polygon)
	{
		remove_object_from_polygon_object_list(projectile->object_index);
		add_object_to_polygon_object_list(projectile->object_index, polygon_index);
	}
	return 0;
}

const luaL_Reg Lua_Projectile_Get[] = {
	{"damage_scale", Lua_Projectile_Get_Damage_Scale},
	{"delete", L_TableFunction<Lua_Projectile_Delete>},
	{"dz", Lua_Projectile_Get_Gravity},
	{"elevation", Lua_Projectile_Get_Elevation},
	{"facing", Lua_Projectile_Get_Facing},
	{"play_sound", L_TableFunction<Lua_Projectile_Play_Sound>},
	{"position", L_TableFunction<Lua_Projectile_Position>},
	{"owner", Lua_Projectile_Get_Owner},
	{"pitch", Lua_Projectile_Get_Elevation},
	{"polygon", Lua_Projectile_Get_Polygon},
	{"target", Lua_Projectile_Get_Target},
	{"type", Lua_Projectile_Get_Type},
	{"x", Lua_Projectile_Get_X},
	{"y", Lua_Projectile_Get_Y},
	{"yaw", Lua_Projectile_Get_Facing},
	{"z", Lua_Projectile_Get_Z},
	{0, 0}
};

const luaL_Reg Lua_Projectile_Set[] = {
	{"damage_scale", Lua_Projectile_Set_Damage_Scale},
	{"dz", Lua_Projectile_Set_Gravity},
	{"elevation", Lua_Projectile_Set_Elevation},
	{"facing", Lua_Projectile_Set_Facing},
	{"owner", Lua_Projectile_Set_Owner},
	{"pitch", Lua_Projectile_Set_Elevation},
	{"target", Lua_Projectile_Set_Target},
	{"yaw", Lua_Projectile_Set_Facing},
	{0, 0}
};

char Lua_Projectiles_Name[] = "Projectiles";

// Projectiles.new(x, y, z, polygon, type)
int Lua_Projectiles_New_Projectile(lua_State *L)
{
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
	{
		return luaL_error(L, "new: incorrect argument type");
	}

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

	world_point3d origin;
	origin.x = static_cast<int>(lua_tonumber(L, 1) * WORLD_ONE);
	origin.y = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	origin.z = static_cast<int>(lua_tonumber(L, 3) * WORLD_ONE);

	world_point3d vector;
	vector.x = WORLD_ONE;
	vector.y = 0;
	vector.z = 0;

	short type = Lua_ProjectileType::ToIndex(L, 5);

	short projectile_index = ::new_projectile(&origin, polygon_index, &vector, 0, type, NONE, NONE, NONE, FIXED_ONE);
	Lua_Projectile::Push(L, projectile_index);
	return 1;
}

const luaL_Reg Lua_Projectiles_Methods[] = {
	{"new", L_TableFunction<Lua_Projectiles_New_Projectile>},
	{0, 0}
};

bool Lua_Projectile_Valid(int32 index)
{
	if (index < 0 || index >= MAXIMUM_PROJECTILES_PER_MAP)
		return false;

	projectile_data *projectile = GetMemberWithBounds(projectiles, index ,MAXIMUM_PROJECTILES_PER_MAP);
	return (SLOT_IS_USED(projectile));
}

extern projectile_definition *get_projectile_definition(short type);

char Lua_ProjectileTypeDamage_Name[] = "projectile_type_damage";
typedef L_Class<Lua_ProjectileTypeDamage_Name> Lua_ProjectileTypeDamage;

static int Lua_ProjectileTypeDamage_Get_Type(lua_State *L)
{
	projectile_definition *definition = get_projectile_definition(Lua_ProjectileTypeDamage::Index(L, 1));
	Lua_DamageType::Push(L, definition->damage.type);
	return 1;
}

const luaL_Reg Lua_ProjectileTypeDamage_Get[] = {
	{"type", Lua_ProjectileTypeDamage_Get_Type},
	{0, 0}
};

char Lua_ProjectileType_Name[] = "projectile_type";

static int Lua_ProjectileType_Get_Damage(lua_State *L)
{
	Lua_ProjectileTypeDamage::Push(L, Lua_ProjectileType::Index(L, 1));
	return 1;
}

const luaL_Reg Lua_ProjectileType_Get[] = {
	{"damage", Lua_ProjectileType_Get_Damage},
	{0, 0}
};

char Lua_ProjectileTypes_Name[] = "ProjectileTypes";

static bool Lua_ProjectileType_Valid(int32 index)
{
	return (index >= 0 && index < NUMBER_OF_PROJECTILE_TYPES);
}

static void compatibility(lua_State *L);

int Lua_Projectiles_register(lua_State *L)
{
	Lua_Projectile::Register(L, Lua_Projectile_Get, Lua_Projectile_Set);
	Lua_Projectile::Valid = Lua_Projectile_Valid;

	Lua_Projectiles::Register(L, Lua_Projectiles_Methods);
	Lua_Projectiles::Length = boost::bind(get_dynamic_limit, (int) _dynamic_limit_projectiles);

	Lua_ProjectileType::Register(L, Lua_ProjectileType_Get, 0, 0, Lua_ProjectileType_Mnemonics);
	Lua_ProjectileType::Valid = Lua_ProjectileType_Valid;

	Lua_ProjectileTypeDamage::Register(L, Lua_ProjectileTypeDamage_Get);
	
	Lua_ProjectileTypes::Register(L);
	Lua_ProjectileTypes::Length = Lua_ProjectileTypes::ConstantLength(NUMBER_OF_PROJECTILE_TYPES);

	compatibility(L);
	return 0;
}

const char *compatibility_script = ""
	"function get_projectile_angle(index) local elevation = Projectiles[index].elevation if elevation < 0.0 then elevation = elevation + 360 end return Projectiles[index].facing, elevation end\n"
	"function get_projectile_damage_type(index) return Projectiles[index].type.damage.type.index end\n"
	"function get_projectile_owner(index) if Projectiles[index].owner then return Projectiles[index].owner.index end end\n"
	"function get_projectile_position(index) return Projectiles[index].polygon.index, Projectiles[index].x, Projectiles[index].y, Projectiles[index].z end\n"
	"function get_projectile_target(index) if Projectiles[index].target then return Projectiles[index].target.index end end\n"
	"function get_projectile_type(index) return Projectiles[index].type.index end\n"
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
