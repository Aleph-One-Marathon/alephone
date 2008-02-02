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

#include "lua_monsters.h"
#include "lua_map.h"
#include "lua_player.h"
#include "lua_templates.h"

#include "flood_map.h"
#include "monsters.h"
#include "player.h"

#define DONT_REPEAT_DEFINITIONS
#include "monster_definitions.h"

#ifdef HAVE_LUA

const float AngleConvert = 360/float(FULL_CIRCLE);

const char *Lua_Monster::name = "monster";

int Lua_Monster::valid(lua_State *L)
{
	lua_pushboolean(L, valid(L_Index<Lua_Monster>(L, 1)));
	return 1;
}

int Lua_Monster::attack(lua_State *L)
{
	short target = 0;
	if (lua_isnumber(L, 2))
	{
		target = static_cast<int>(lua_tonumber(L, 2));
		if (!Lua_Monsters::valid(target))
			return luaL_error(L, "attack: invalid monster index");
	}
	else if (L_Is<Lua_Monster>(L, 2))
	{
		target = L_Index<Lua_Monster>(L, 2);
	}
	else
		return luaL_error(L, "attack: incorrect argument type");
	
	change_monster_target(L_Index<Lua_Monster>(L, 1), target);
	return 0;
}

int Lua_Monster::damage(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "damage: incorrect argument type");
	
	int damage_amount = static_cast<int>(lua_tonumber(L, 2));
	int damage_type = NONE;
	if (lua_gettop(L) == 3)
	{
		if (lua_isnumber(L, 3))
			damage_type = static_cast<int>(lua_tonumber(L, 3));
		else
			return luaL_error(L, "damage: incorrect argument type");
	}

	damage_definition damage;
	if (damage_type != NONE)
		damage.type = damage_type;
	else
		damage.type = _damage_fist;

	damage.base = damage_amount;
	damage.random = 0;
	damage.flags = 0;
	damage.scale = FIXED_ONE;
	
	int monster_index = L_Index<Lua_Monster>(L, 1);
	monster_data *monster = get_monster_data(monster_index);
	damage_monster(monster_index, NONE, NONE, &(monster->sound_location), &damage, NONE);
	return 0;
}

struct monster_pathfinding_data
{
	struct monster_definition *definition;
	struct monster_data *monster;

	bool cross_zone_boundaries;
};

extern void advance_monster_path(short monster_index);
extern long monster_pathfinding_cost_function(short source_polygon_index, short line_index, short destination_polygon_index, void *data);
extern void set_monster_action(short monster_index, short action);
extern void set_monster_mode(short monster_index, short new_mode, short target_index);

int Lua_Monster::move_by_path(lua_State *L)
{
	int monster_index = L_Index<Lua_Monster>(L, 1);
	int polygon_index = 0;
	if (lua_isnumber(L, 2))
	{
		polygon_index = static_cast<int>(lua_tonumber(L, 2));
		if (!Lua_Polygons::valid(polygon_index))
			return luaL_error(L, "move_by_path: invalid polygon index");
	}
	else if (L_Is<Lua_Polygon>(L, 2))
	{
		polygon_index = L_Index<Lua_Polygon>(L, 2);
	}
	else
		return luaL_error(L, "move_by_path: incorrect argument type");

	monster_data *monster = get_monster_data(monster_index);
	if (MONSTER_IS_PLAYER(monster))
		return luaL_error(L, "move_by_path: monster is player");

	monster_definition *definition = get_monster_definition_external(monster->type);
	object_data *object = get_object_data(monster->object_index);
	monster_pathfinding_data path;
	world_point2d destination;

	if (!MONSTER_IS_ACTIVE(monster))
		activate_monster(monster_index);

	if (monster->path != NONE)
	{
		delete_path(monster->path);
		monster->path = NONE;
	}

	SET_MONSTER_NEEDS_PATH_STATUS(monster, false);
	path.definition = definition;
	path.monster = monster;
	path.cross_zone_boundaries = true;

	destination = get_polygon_data(polygon_index)->center;
	
	monster->path = new_path((world_point2d *) &object->location, object->polygon, &destination, polygon_index, 3 * definition->radius, monster_pathfinding_cost_function, &path);
	if (monster->path == NONE)
	{
		if (monster->action != _monster_is_being_hit || MONSTER_IS_DYING(monster))
		{
			set_monster_action(monster_index, _monster_is_stationary);
		}
		set_monster_mode(monster_index, _monster_unlocked, NONE);
		return 0;
	}

	advance_monster_path(monster_index);
	return 0;
}

extern void add_object_to_polygon_object_list(short object_index, short polygon_index);

int Lua_Monster::position(lua_State *L)
{
	if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		return luaL_error(L, "position: incorrect argument type");

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

	monster_data *monster = get_monster_data(L_Index<Lua_Monster>(L, 1));
	object_data *object = get_object_data(monster->object_index);
	object->location.x = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	object->location.y = static_cast<int>(lua_tonumber(L, 3) * WORLD_ONE);
	object->location.z = static_cast<int>(lua_tonumber(L, 4) * WORLD_ONE);
	
	if (polygon_index != object->polygon)
	{
		remove_object_from_polygon_object_list(monster->object_index);
		add_object_to_polygon_object_list(monster->object_index, polygon_index);
	}
}
		

int Lua_Monster::get_action(lua_State *L)
{
	monster_data *monster = get_monster_data(L_Index<Lua_Monster>(L, 1));
	lua_pushnumber(L, monster->action);
	return 1;
}

int Lua_Monster::get_active(lua_State *L)
{
	monster_data *monster = get_monster_data(L_Index<Lua_Monster>(L, 1));
	if (MONSTER_IS_PLAYER(monster))
		return luaL_error(L, "active: monster is a player");
	
	lua_pushboolean(L, MONSTER_IS_ACTIVE(monster));
	return 1;
}

int Lua_Monster::get_facing(lua_State *L)
{
	monster_data *monster = get_monster_data(L_Index<Lua_Monster>(L, 1));
	object_data *object = get_object_data(monster->object_index);
	lua_pushnumber(L, (double) object->facing * AngleConvert);
	return 1;
}

int Lua_Monster::get_mode(lua_State *L)
{
	monster_data *monster = get_monster_data(L_Index<Lua_Monster>(L, 1));
	lua_pushnumber(L, monster->mode);
	return 1;
}

int Lua_Monster::get_player(lua_State *L)
{
	int monster_index = L_Index<Lua_Monster>(L, 1);
	monster_data *monster = get_monster_data(monster_index);
	if (MONSTER_IS_PLAYER(monster))
		L_Push<Lua_Player>(L, monster_index_to_player_index(monster_index));
	else
		lua_pushnil(L);
	
	return 1;
}

int Lua_Monster::get_polygon(lua_State *L)
{
	monster_data *monster = get_monster_data(L_Index<Lua_Monster>(L, 1));
	object_data *object = get_object_data(monster->object_index);
	L_Push<Lua_Polygon>(L, object->polygon);
	return 1;
}

int Lua_Monster::get_type(lua_State *L)
{
	monster_data *monster = get_monster_data(L_Index<Lua_Monster>(L, 1));
	lua_pushnumber(L, monster->type);
	return 1;
}

int Lua_Monster::get_visible(lua_State *L)
{
	monster_data *monster = get_monster_data(L_Index<Lua_Monster>(L, 1));
	object_data *object = get_object_data(monster->object_index);
	lua_pushboolean(L, !OBJECT_IS_INVISIBLE(object));
	return 1;
}

int Lua_Monster::get_vitality(lua_State *L)
{
	monster_data *monster = get_monster_data(L_Index<Lua_Monster>(L, 1));
	lua_pushnumber(L, monster->vitality);
	return 1;
}

int Lua_Monster::get_x(lua_State *L)
{
	monster_data *monster = get_monster_data(L_Index<Lua_Monster>(L, 1));
	object_data *object = get_object_data(monster->object_index);
	lua_pushnumber(L, (double) object->location.x / WORLD_ONE);
	return 1;
}

int Lua_Monster::get_y(lua_State *L)
{
	monster_data *monster = get_monster_data(L_Index<Lua_Monster>(L, 1));
	object_data *object = get_object_data(monster->object_index);
	lua_pushnumber(L, (double) object->location.y / WORLD_ONE);
	return 1;
}

int Lua_Monster::get_z(lua_State *L)
{
	monster_data *monster = get_monster_data(L_Index<Lua_Monster>(L, 1));
	object_data *object = get_object_data(monster->object_index);
	lua_pushnumber(L, (double) object->location.z / WORLD_ONE);
	return 1;
}

int Lua_Monster::set_active(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "active: incorrect argument type");	
	
	bool activate = lua_toboolean(L, 2);
	int monster_index = L_Index<Lua_Monster>(L, 1);
	monster_data *monster = get_monster_data(monster_index);
	if (MONSTER_IS_PLAYER(monster))
		return luaL_error(L, "active: monster is a player");
	if (activate)
	{
		if (!MONSTER_IS_ACTIVE(monster))
			activate_monster(monster_index);
	}
	else
	{
		if (MONSTER_IS_ACTIVE(monster))
			deactivate_monster(monster_index);
	}
	return 0;
}

int Lua_Monster::set_facing(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "facing: incorrect argument type");

	monster_data *monster = get_monster_data(L_Index<Lua_Monster>(L, 1));
	object_data *object = get_object_data(monster->object_index);
	object->facing = static_cast<int>(lua_tonumber(L, 2) / AngleConvert);
	return 0;
}
	
	

int Lua_Monster::set_vitality(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "vitality: incorrect argument type");

	monster_data *monster = get_monster_data(L_Index<Lua_Monster>(L, 1));
	monster->vitality = static_cast<int>(lua_tonumber(L, 2));
	return 0;
}

const luaL_reg Lua_Monster::index_table[] = {
	{"action", Lua_Monster::get_action},
	{"active", Lua_Monster::get_active},
	{"attack", L_TableFunction<Lua_Monster::attack>},
	{"damage", L_TableFunction<Lua_Monster::damage>},
	{"index", L_TableIndex<Lua_Monster>},
	{"facing", Lua_Monster::get_facing},
	{"life", Lua_Monster::get_vitality},
	{"mode", Lua_Monster::get_mode},
	{"move_by_path", L_TableFunction<Lua_Monster::move_by_path>},
	{"player", Lua_Monster::get_player},
	{"polygon", Lua_Monster::get_polygon},
	{"position", L_TableFunction<Lua_Monster::position>},
	{"type", Lua_Monster::get_type},
	{"valid", Lua_Monster::valid},
	{"visible", Lua_Monster::get_visible},
	{"vitality", Lua_Monster::get_vitality},
	{"x", Lua_Monster::get_x},
	{"y", Lua_Monster::get_y},
	{"yaw", Lua_Monster::get_facing},
	{"z", Lua_Monster::get_z},
	{0, 0}
};

const luaL_reg Lua_Monster::newindex_table[] = {
	{"active", Lua_Monster::set_active},
	{"facing", Lua_Monster::set_facing},
	{"life", Lua_Monster::set_vitality},
	{"vitality", Lua_Monster::set_vitality},
	{"yaw", Lua_Monster::set_facing},
	{0, 0}
};

const luaL_reg Lua_Monster::metatable[] = {
	{"__index", L_TableGet<Lua_Monster>},
	{"__newindex", L_TableSet<Lua_Monster>},
	{0, 0}
};

const char *Lua_Monsters::name = "Monsters";

const luaL_reg Lua_Monsters::methods[] = {
	{"new", Lua_Monsters::new_monster},
	{0, 0}
};

const luaL_reg Lua_Monsters::metatable[] = {
	{"__index", L_GlobalIndex<Lua_Monsters, Lua_Monster>},
	{"__newindex", L_GlobalNewindex<Lua_Monsters>},
	{"__call", L_GlobalCall<Lua_Monsters, Lua_Monster>},
	{0, 0}
};

// Monsters.new(x, y, height, polygon, type)
int Lua_Monsters::new_monster(lua_State *L)
{
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 5))
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

	short monster_type = static_cast<int>(lua_tonumber(L, 5));
	if (monster_type < 0 || monster_type > NUMBER_OF_MONSTER_TYPES)
		return luaL_error(L, "new: invalid monster type");
	
	object_location location;
	location.p.x = static_cast<int>(lua_tonumber(L, 1) * WORLD_ONE);
	location.p.y = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	location.p.z = static_cast<int>(lua_tonumber(L, 3) * WORLD_ONE);
	
	location.polygon_index = polygon_index;
	location.yaw = 0;
	location.pitch = 0;
	location.flags = 0;

	short monster_index = ::new_monster(&location, monster_type);
	if (monster_index == NONE)
		return 0;

	L_Push<Lua_Monster>(L, monster_index);
	return 1;
}

bool Lua_Monsters::valid(int index)
{
	if (index < 0 || index >= MAXIMUM_MONSTERS_PER_MAP)
		return false;

	monster_data *monster = GetMemberWithBounds(monsters, index, MAXIMUM_MONSTERS_PER_MAP);
	return (SLOT_IS_USED(monster));
}

static int compatibility(lua_State *L);

int Lua_Monsters_register(lua_State *L)
{
	L_Register<Lua_Monster>(L);
	L_GlobalRegister<Lua_Monsters>(L);

	compatibility(L);
}

static const char *compatibility_script = ""
// there are some conversions to and from internal units, because old
// monster API was wrong
	"function activate_monster(monster) Monsters[monster].active = true end\n"
	"function attack_monster(agressor, target) Monsters[aggressor]:attack(target) end\n"
	"function damage_monster(monster, damage, type) if type then Monsters[monster]:damage(damage, type) else Monsters[monster]:damage(damage) end end\n"
	"function deactivate_monster(monster) Monsters[monster].active = false end\n"
	"function get_monster_action(monster) return Monsters[monster].action end\n"
	"function get_monster_facing(monster) return Monsters[monster].facing * 512 / 360 end\n"
	"function get_monster_mode(monster) return Monsters[monster].mode end\n"
	"function get_monster_polygon(monster) return Monsters[monster].polygon.index end\n"
	"function get_monster_position(monster) return Monsters[monster].x * 1024, Monsters[monster].y * 1024, Monsters[monster].z * 1024 end\n"
	"function get_monster_type(monster) return Monsters[monster].type end\n"
	"function get_monster_visible(monster) return Monsters[monster].visible end\n"
	"function get_monster_vitality(monster) return Monsters[monster].vitality end\n"
	"function monster_index_valid(monster) if Monsters[monster] then return true else return false end end\n"
	"function move_monster(monster, polygon) Monsters[monster]:move_by_path(polygon) end\n"
	"function new_monster(type, poly, facing, height, x, y) if (x and y) then m = Monsters.new(x, y, height / 1024, poly, type) elseif (height) then m = Monsters.new(Polygons[poly].x, Polygons[poly].y, height / 1024, poly, type) else m = Monsters.new(Polygons[poly].x, Polygons[poly].y, 0, poly, type) end if m then return m.index else return -1 end end\n"	
	"function set_monster_position(monster, polygon, x, y, z) Monsters[monster]:position(x, y, z, polygon) end\n"
	"function set_monster_vitality(monster, vitality) Monsters[monster].vitality = vitality end\n"
	;

static int compatibility(lua_State *L)
{
	luaL_loadbuffer(L, compatibility_script, strlen(compatibility_script), "monsters_compatibility");
	lua_pcall(L, 0, 0, 0);
}



#endif
