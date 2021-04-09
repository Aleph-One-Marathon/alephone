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

#include "lua_monsters.h"
#include "lua_map.h"
#include "lua_objects.h"
#include "lua_player.h"
#include "lua_templates.h"

#include "flood_map.h"
#include "monsters.h"
#include "player.h"

#include <boost/bind.hpp>

#define DONT_REPEAT_DEFINITIONS
#include "monster_definitions.h"

#ifdef HAVE_LUA

const float AngleConvert = 360/float(FULL_CIRCLE);

static inline bool powerOfTwo(int32 x)
{
	return !((x-1) & x);
}

char Lua_MonsterClass_Name[] = "monster_class";
typedef L_Enum<Lua_MonsterClass_Name, int32> Lua_MonsterClass;

bool Lua_MonsterClass_Valid(int32 index)
{
	return index >= 1 && index <= _class_yeti && powerOfTwo(index);
}

char Lua_MonsterClasses_Name[] = "MonsterClasses";
typedef L_EnumContainer<Lua_MonsterClasses_Name, Lua_MonsterClass> Lua_MonsterClasses;

template<>
int L_Container<Lua_MonsterClasses_Name, Lua_MonsterClass>::_iterator(lua_State *L)
{
	int32 index = static_cast<int32>(lua_tonumber(L, lua_upvalueindex(1)));
	while (index < Length())
	{
		if (Lua_MonsterClass::Valid(1 << index))
		{
			Lua_MonsterClass::Push(L, 1 << index);
			lua_pushnumber(L, ++index);
			lua_replace(L, lua_upvalueindex(1));
			return 1;
		}
		else
		{
			++index;
		}
	}

	lua_pushnil(L);
	return 1;
}

extern object_frequency_definition* monster_placement_info;

char Lua_MonsterType_Enemies_Name[] = "monster_type_enemies";
typedef L_Class<Lua_MonsterType_Enemies_Name> Lua_MonsterType_Enemies;

static int Lua_MonsterType_Enemies_Get(lua_State *L)
{
	int monster_type = Lua_MonsterType_Enemies::Index(L, 1);
	int enemy_class = Lua_MonsterClass::ToIndex(L, 2);
	
	monster_definition *definition = get_monster_definition_external(monster_type);
	lua_pushboolean(L, definition->enemies & enemy_class);
	return 1;
}

static int Lua_MonsterType_Enemies_Set(lua_State *L)
{
	if (!lua_isboolean(L, 3))
		return luaL_error(L, "enemies: incorrect argument type");

	int monster_type = Lua_MonsterType_Enemies::Index(L, 1);
	int enemy_class = Lua_MonsterClass::ToIndex(L, 2);
	bool enemy = lua_toboolean(L, 3);
	monster_definition *definition = get_monster_definition_external(monster_type);
	if (enemy)
	{
		definition->enemies = definition->enemies | enemy_class;
	}
	else
	{
		definition->enemies = definition->enemies & ~(enemy_class);
	}

	return 0;
}

const luaL_Reg Lua_MonsterType_Enemies_Metatable[] = {
	{"__index", Lua_MonsterType_Enemies_Get},
	{"__newindex", Lua_MonsterType_Enemies_Set},
	{0, 0}
};

char Lua_MonsterType_Friends_Name[] = "monster_type_friends";
typedef L_Class<Lua_MonsterType_Friends_Name> Lua_MonsterType_Friends;

static int Lua_MonsterType_Friends_Get(lua_State *L)
{
	int monster_type = Lua_MonsterType_Friends::Index(L, 1);
	int friend_class = Lua_MonsterClass::ToIndex(L, 2);
	
	monster_definition *definition = get_monster_definition_external(monster_type);
	lua_pushboolean(L, definition->friends & friend_class);
	return 1;
}

static int Lua_MonsterType_Friends_Set(lua_State *L)
{
	if (!lua_isboolean(L, 3))
		return luaL_error(L, "enemies: incorrect argument type");

	int monster_type = Lua_MonsterType_Friends::Index(L, 1);
	int friend_class = Lua_MonsterClass::ToIndex(L, 2);
	bool friendly = lua_toboolean(L, 3);
	monster_definition *definition = get_monster_definition_external(monster_type);
	if (friendly)
	{
		definition->friends = definition->friends | friend_class;
	}
	else
	{
		definition->friends = definition->friends & ~(friend_class);
	}

	return 0;
}

const luaL_Reg Lua_MonsterType_Friends_Metatable[] = {
	{"__index", Lua_MonsterType_Friends_Get},
	{"__newindex", Lua_MonsterType_Friends_Set},
	{0, 0}
};

char Lua_MonsterType_Immunities_Name[] = "monster_type_immunities";
typedef L_Class<Lua_MonsterType_Immunities_Name> Lua_MonsterType_Immunities;

static int Lua_MonsterType_Immunities_Get(lua_State *L)
{
	int monster_type = Lua_MonsterType_Immunities::Index(L, 1);
	int damage_type = Lua_DamageType::ToIndex(L, 2);

	monster_definition *definition = get_monster_definition_external(monster_type);
	lua_pushboolean(L, definition->immunities & (1 << damage_type));
	return 1;
}

static int Lua_MonsterType_Immunities_Set(lua_State *L)
{
	if (!lua_isboolean(L, 3))
		luaL_error(L, "immunities: incorrect argument type");

	int monster_type = Lua_MonsterType_Immunities::Index(L, 1);
	int damage_type = Lua_DamageType::ToIndex(L, 2);
	bool immune = lua_toboolean(L, 3);
	
	monster_definition *definition = get_monster_definition_external(monster_type);
	if (immune)
	{
		definition->immunities |= (1 << damage_type);
	}
	else
	{
		definition->immunities &= ~(1 << damage_type);
	}
	
	return 0;
}

const luaL_Reg Lua_MonsterType_Immunities_Metatable[] = {
	{"__index", Lua_MonsterType_Immunities_Get},
	{"__newindex", Lua_MonsterType_Immunities_Set},
	{0, 0}
};

char Lua_MonsterType_Weaknesses_Name[] = "monster_type_weaknesses";
typedef L_Class<Lua_MonsterType_Weaknesses_Name> Lua_MonsterType_Weaknesses;

int Lua_MonsterType_Weaknesses_Get(lua_State *L)
{
	int monster_type = Lua_MonsterType_Weaknesses::Index(L, 1);
	int damage_type = Lua_DamageType::ToIndex(L, 2);

	monster_definition *definition = get_monster_definition_external(monster_type);
	lua_pushboolean(L, definition->weaknesses & (1 << damage_type));
	return 1;
}

int Lua_MonsterType_Weaknesses_Set(lua_State *L)
{
	if (!lua_isboolean(L, 3))
		luaL_error(L, "immunities: incorrect argument type");

	int monster_type = Lua_MonsterType_Weaknesses::Index(L, 1);
	int damage_type = Lua_DamageType::ToIndex(L, 2);
	bool weakness = lua_toboolean(L, 3);
	
	monster_definition *definition = get_monster_definition_external(monster_type);
	if (weakness)
	{
		definition->weaknesses |= (1 << damage_type);
	}
	else
	{
		definition->weaknesses &= ~(1 << damage_type);
	}
	
	return 0;
}

const luaL_Reg Lua_MonsterType_Weaknesses_Metatable[] = {
	{"__index", Lua_MonsterType_Weaknesses_Get},
	{"__newindex", Lua_MonsterType_Weaknesses_Set},
	{0, 0}
};

char Lua_MonsterType_Name[] = "monster_type";

static bool Lua_MonsterType_Valid(int16 index)
{
	return index >= 0 && index < NUMBER_OF_MONSTER_TYPES;
}


static int Lua_MonsterType_Get_Class(lua_State *L) {
	monster_definition *definition = get_monster_definition_external(Lua_MonsterType::Index(L, 1));
	Lua_MonsterClass::Push(L, definition->_class);
	return 1;
}

static int Lua_MonsterType_Get_Collection(lua_State* L)
{
	auto definition = get_monster_definition_external(Lua_MonsterType::Index(L, 1));
	Lua_Collection::Push(L, GET_COLLECTION(definition->collection));
	return 1;
}

static int Lua_MonsterType_Get_Clut_Index(lua_State* L)
{
	auto definition = get_monster_definition_external(Lua_MonsterType::Index(L, 1));
	lua_pushnumber(L, GET_COLLECTION_CLUT(definition->collection));
	return 1;
}

static int Lua_MonsterType_Get_Enemies(lua_State *L) {
	Lua_MonsterType_Enemies::Push(L, Lua_MonsterType::Index(L, 1));
	return 1;
}

template<uint32 flag>
static int Lua_MonsterType_Get_Flag(lua_State* L)
{
	monster_definition* definition = get_monster_definition_external(Lua_MonsterType::Index(L, 1));
	lua_pushboolean(L, definition->flags & flag);
	return 1;
}


static int Lua_MonsterType_Get_Height(lua_State *L) {
	monster_definition *definition = get_monster_definition_external(Lua_MonsterType::Index(L, 1));
	lua_pushnumber(L, (double) definition->height / WORLD_ONE);
	return 1;
}

static int Lua_MonsterType_Get_Friends(lua_State *L) {
	Lua_MonsterType_Friends::Push(L, Lua_MonsterType::Index(L, 1));
	return 1;
}

static int Lua_MonsterType_Get_Immunities(lua_State *L) {
	Lua_MonsterType_Immunities::Push(L, Lua_MonsterType::Index(L, 1));
	return 1;
}

static int Lua_MonsterType_Get_Impact_Effect(lua_State *L) {
	monster_definition *definition = get_monster_definition_external(Lua_MonsterType::Index(L, 1));
	Lua_EffectType::Push(L, definition->impact_effect);
	return 1;
}

static int Lua_MonsterType_Get_Initial_Count(lua_State* L)
{
	lua_pushnumber(L, monster_placement_info[Lua_MonsterType::Index(L, 1)].initial_count);
	return 1;
}

static int Lua_MonsterType_Get_Maximum_Count(lua_State* L)
{
	lua_pushnumber(L, monster_placement_info[Lua_MonsterType::Index(L, 1)].maximum_count);
	return 1;
}


static int Lua_MonsterType_Get_Melee_Impact_Effect(lua_State *L) {
	monster_definition *definition = get_monster_definition_external(Lua_MonsterType::Index(L, 1));
	Lua_EffectType::Push(L, definition->melee_impact_effect);
	return 1;
}

static int Lua_MonsterType_Get_Minimum_Count(lua_State* L)
{
	lua_pushnumber(L, monster_placement_info[Lua_MonsterType::Index(L, 1)].minimum_count);
	return 1;
}

static int Lua_MonsterType_Get_Radius(lua_State *L) {
	monster_definition *definition = get_monster_definition_external(Lua_MonsterType::Index(L, 1));
	lua_pushnumber(L, (double) definition->radius / WORLD_ONE);
	return 1;
}

static int Lua_MonsterType_Get_Random_Chance(lua_State* L)
{
	lua_pushnumber(L, (double) monster_placement_info[Lua_MonsterType::Index(L, 1)].random_chance / UINT16_MAX);
	return 1;
}

static int Lua_MonsterType_Get_Random_Count(lua_State* L)
{
	lua_pushnumber(L, monster_placement_info[Lua_MonsterType::Index(L, 1)].random_count);
	return 1;
}

static int Lua_MonsterType_Get_Random_Location(lua_State* L)
{
	lua_pushboolean(L, monster_placement_info[Lua_MonsterType::Index(L, 1)].flags & _reappears_in_random_location);
	return 1;
}

static int Lua_MonsterType_Get_Weaknesses(lua_State *L) {
	Lua_MonsterType_Weaknesses::Push(L, Lua_MonsterType::Index(L, 1));
	return 1;
}

static int Lua_MonsterType_Get_Item(lua_State *L) {
	monster_definition *definition = get_monster_definition_external(Lua_MonsterType::Index(L, 1));
	Lua_ItemType::Push(L, definition->carrying_item_type);
	return 1;
}

static int Lua_MonsterType_Set_Class(lua_State *L) {
	monster_definition *definition = get_monster_definition_external(Lua_MonsterType::Index(L, 1));
	definition->_class = static_cast<int32>(Lua_MonsterClass::ToIndex(L, 2));
	return 0;
}

template<uint32 flag>
static int Lua_MonsterType_Set_Flag(lua_State* L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "monster flag: incorrect argument type");

	monster_definition* definition = get_monster_definition_external(Lua_MonsterType::Index(L, 1));
	if (lua_toboolean(L, 2)) 
	{
		definition->flags |= flag;
	}
	else
	{
		definition->flags &= ~flag;
	}

	return 0;
}

static int Lua_MonsterType_Set_Item(lua_State *L) {
	int item_type = 0;
	if (lua_isnumber(L, 2))
	{
		item_type = static_cast<int>(lua_tonumber(L, 2));
	}
	else if (lua_isnil(L, 2))
	{
		item_type = NONE;
	}
	else
		return luaL_error(L, "item: incorrect argument type");
	
	monster_definition *definition = get_monster_definition_external(Lua_MonsterType::ToIndex(L, 1));

	definition->carrying_item_type = item_type;
	return 0;
}

static int Lua_MonsterType_Set_Initial_Count(lua_State* L)
{
	if (lua_isnumber(L, 2))
	{
		monster_placement_info[Lua_MonsterType::Index(L, 1)].initial_count = lua_tonumber(L, 2);
	}
	else
	{
		return luaL_error(L, "initial_count: incorrect argument type");
	}
	return 0;
}

static int Lua_MonsterType_Set_Maximum_Count(lua_State* L)
{
	if (lua_isnumber(L, 2))
	{
		monster_placement_info[Lua_MonsterType::Index(L, 1)].maximum_count = lua_tonumber(L, 2);
	}
	else
	{
		return luaL_error(L, "maximum_count: incorrect argument type");
	}
	return 0;
}

static int Lua_MonsterType_Set_Minimum_Count(lua_State* L)
{
	if (lua_isnumber(L, 2))
	{
		monster_placement_info[Lua_MonsterType::Index(L, 1)].minimum_count = lua_tonumber(L, 2);
	}
	else
	{
		return luaL_error(L, "minimum_count: incorrect argument type");
	}
	return 0;
}

static int Lua_MonsterType_Set_Random_Chance(lua_State* L)
{
	if (lua_isnumber(L, 2))
	{
		monster_placement_info[Lua_MonsterType::Index(L, 1)].random_chance = static_cast<uint16>(lua_tonumber(L, 2) * UINT16_MAX + 0.5);
	}
	else
	{
		return luaL_error(L, "random_chance: incorrect argument type");
	}
	return 0;
}

static int Lua_MonsterType_Set_Random_Count(lua_State* L)
{
	if (lua_isnumber(L, 2))
	{
		monster_placement_info[Lua_MonsterType::Index(L, 1)].random_count = lua_tonumber(L, 2);
	}
	else
	{
		return luaL_error(L, "random_count: incorrect argument type");
	}
	return 0;
}

static int Lua_MonsterType_Set_Random_Location(lua_State* L)
{
	if (lua_isboolean(L, 2))
	{
		if (lua_toboolean(L, 2))
		{
			monster_placement_info[Lua_MonsterType::Index(L, 1)].flags |= _reappears_in_random_location;
		}
		else
		{
			monster_placement_info[Lua_MonsterType::Index(L, 1)].flags &= ~_reappears_in_random_location;
		}
	}
	else
	{
		return luaL_error(L, "random_location: incorrect argument type");
	}
	return 0;
}

const luaL_Reg Lua_MonsterType_Get[] = {
	{"attacks_immediately", Lua_MonsterType_Get_Flag<_monster_attacks_immediately>},
	{"cannot_be_dropped", Lua_MonsterType_Get_Flag<_monster_cannot_be_dropped>},
	{"class", Lua_MonsterType_Get_Class},
	{"collection", Lua_MonsterType_Get_Collection},
	{"clut_index", Lua_MonsterType_Get_Clut_Index},
	{"enemies", Lua_MonsterType_Get_Enemies},
	{"friends", Lua_MonsterType_Get_Friends},
	{"height", Lua_MonsterType_Get_Height},
	{"immunities", Lua_MonsterType_Get_Immunities},
	{"impact_effect", Lua_MonsterType_Get_Impact_Effect},
	{"initial_count", Lua_MonsterType_Get_Initial_Count},
	{"major", Lua_MonsterType_Get_Flag<_monster_major>},
	{"maximum_count", Lua_MonsterType_Get_Maximum_Count},
	{"melee_impact_effect", Lua_MonsterType_Get_Melee_Impact_Effect},
	{"minimum_count", Lua_MonsterType_Get_Minimum_Count},
	{"minor", Lua_MonsterType_Get_Flag<_monster_minor>},
	{"item", Lua_MonsterType_Get_Item},
	{"radius", Lua_MonsterType_Get_Radius},
	{"random_chance", Lua_MonsterType_Get_Random_Chance},
	{"random_location", Lua_MonsterType_Get_Random_Location},
	{"total_available", Lua_MonsterType_Get_Random_Count},
	{"weaknesses", Lua_MonsterType_Get_Weaknesses},
	{"waits_with_clear_shot", Lua_MonsterType_Get_Flag<_monster_waits_with_clear_shot>},
	{0, 0}
};

const luaL_Reg Lua_MonsterType_Set[] = {
	{"attacks_immediately", Lua_MonsterType_Set_Flag<_monster_attacks_immediately>},
	{"cannot_be_dropped", Lua_MonsterType_Set_Flag<_monster_cannot_be_dropped>},
	{"class", Lua_MonsterType_Set_Class},
	{"initial_count", Lua_MonsterType_Set_Initial_Count},
	{"item", Lua_MonsterType_Set_Item},
	{"major", Lua_MonsterType_Set_Flag<_monster_major>},
	{"maximum_count", Lua_MonsterType_Set_Maximum_Count},
	{"minimum_count", Lua_MonsterType_Set_Minimum_Count},
	{"minor", Lua_MonsterType_Set_Flag<_monster_minor>},
	{"random_chance", Lua_MonsterType_Set_Random_Chance},
	{"random_location", Lua_MonsterType_Set_Random_Location},
	{"total_available", Lua_MonsterType_Set_Random_Count},
	{"waits_with_clear_shot", Lua_MonsterType_Set_Flag<_monster_waits_with_clear_shot>},
	{0, 0}
};

char Lua_MonsterMode_Name[] = "monster_mode";
typedef L_Enum<Lua_MonsterMode_Name> Lua_MonsterMode;

char Lua_MonsterModes_Name[] = "MonsterModes";
typedef L_EnumContainer<Lua_MonsterModes_Name, Lua_MonsterMode> Lua_MonsterModes;

char Lua_MonsterAction_Name[] = "monster_action";

char Lua_MonsterActions_Name[] = "MonsterActions";
typedef L_EnumContainer<Lua_MonsterActions_Name, Lua_MonsterAction> Lua_MonsterActions;

char Lua_MonsterTypes_Name[] = "MonsterTypes";
typedef L_EnumContainer<Lua_MonsterTypes_Name, Lua_MonsterType> Lua_MonsterTypes;

char Lua_Monster_Name[] = "monster";

int Lua_Monster_Accelerate(lua_State *L)
{
	if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		return luaL_error(L, "accelerate: incorrect argument type");

	short monster_index = Lua_Monster::Index(L, 1);
	monster_data *monster = get_monster_data(monster_index);
	(void)monster;
	double direction = static_cast<double>(lua_tonumber(L, 2));
	double velocity = static_cast<double>(lua_tonumber(L, 3));
	double vertical_velocity = static_cast<double>(lua_tonumber(L, 4));
	
	accelerate_monster(monster_index, static_cast<int>(vertical_velocity * WORLD_ONE), static_cast<int>(direction/AngleConvert), static_cast<int>(velocity * WORLD_ONE));
	return 0;
}

int Lua_Monster_Attack(lua_State *L)
{
	short target = 0;
	if (lua_isnumber(L, 2))
		target = static_cast<short>(lua_tonumber(L, 2));
	else if (Lua_Monster::Is(L, 2))
		target = Lua_Monster::Index(L, 2);
	else
		return luaL_error(L, "attack: incorrect argument type");
	
	change_monster_target(Lua_Monster::Index(L, 1), target);
	return 0;
}

int Lua_Monster_Play_Sound(lua_State *L)
{
	short sound_index = Lua_Sound::ToIndex(L, 2);
	monster_data *monster = get_monster_data(Lua_Monster::Index(L, 1));
	play_object_sound(monster->object_index, sound_index);
	return 0;
}

int Lua_Monster_Damage(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "damage: incorrect argument type");
	
	int damage_amount = static_cast<int>(lua_tonumber(L, 2));
	int damage_type = NONE;
	if (lua_gettop(L) == 3)
	{
		damage_type = Lua_DamageType::ToIndex(L, 3);
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
	
	int monster_index = Lua_Monster::Index(L, 1);
	monster_data *monster = get_monster_data(monster_index);
	damage_monster(monster_index, NONE, NONE, &(monster->sound_location), &damage, NONE);
	return 0;
}

int Lua_Monster_Delete(lua_State* L)
{
	auto monster_index = Lua_Monster::Index(L, 1);
	auto monster = get_monster_data(monster_index);
	if (MONSTER_IS_PLAYER(monster))
		return luaL_error(L, "delete: monster is player");

	monster->action = _monster_is_dying_soft; // to prevent aggressors from
											  // relocking, per monsters.cpp
	monster_died(monster_index);
	auto object = get_object_data(monster->object_index);
	remove_map_object(monster->object_index);

	/* recover original type and notify the object stuff a monster died */
	if (monster->flags&_monster_was_promoted) monster->type-= 1;
	if (monster->flags&_monster_was_demoted) monster->type+= 1;
	object_was_just_destroyed(_object_is_monster, monster->type);
	
	L_Invalidate_Monster(monster_index);
	MARK_SLOT_AS_FREE(monster);

	return 0;
}

int Lua_Monster_Valid(int16 index)
{
	if (index < 0 || index >= MAXIMUM_MONSTERS_PER_MAP)
		return false;

	monster_data *monster = GetMemberWithBounds(monsters, index, MAXIMUM_MONSTERS_PER_MAP);
	return (SLOT_IS_USED(monster));
}

struct monster_pathfinding_data
{
	struct monster_definition *definition;
	struct monster_data *monster;

	bool cross_zone_boundaries;
};

extern void advance_monster_path(short monster_index);
extern int32 monster_pathfinding_cost_function(short source_polygon_index, short line_index, short destination_polygon_index, void *data);
extern void set_monster_action(short monster_index, short action);
extern void set_monster_mode(short monster_index, short new_mode, short target_index);

int Lua_Monster_Move_By_Path(lua_State *L)
{
	int monster_index = Lua_Monster::Index(L, 1);
	int polygon_index = 0;
	if (lua_isnumber(L, 2))
	{
		polygon_index = static_cast<int>(lua_tonumber(L, 2));
		if (!Lua_Polygon::Valid(polygon_index))
			return luaL_error(L, "move_by_path: invalid polygon index");
	}
	else if (Lua_Polygon::Is(L, 2))
	{
		polygon_index = Lua_Polygon::Index(L, 2);
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

int Lua_Monster_Position(lua_State *L)
{
	if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		return luaL_error(L, "position: incorrect argument type");

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

	monster_data *monster = get_monster_data(Lua_Monster::Index(L, 1));
	object_data *object = get_object_data(monster->object_index);
	object->location.x = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	object->location.y = static_cast<int>(lua_tonumber(L, 3) * WORLD_ONE);
	object->location.z = static_cast<int>(lua_tonumber(L, 4) * WORLD_ONE);
	
	if (polygon_index != object->polygon)
	{
		remove_object_from_polygon_object_list(monster->object_index);
		add_object_to_polygon_object_list(monster->object_index, polygon_index);
	}
	return 0;
}
		

static int Lua_Monster_Get_Action(lua_State *L)
{
	monster_data *monster = get_monster_data(Lua_Monster::Index(L, 1));
	Lua_MonsterAction::Push(L, monster->action);
	return 1;
}

static int Lua_Monster_Get_Active(lua_State *L)
{
	monster_data *monster = get_monster_data(Lua_Monster::Index(L, 1));
	if (MONSTER_IS_PLAYER(monster))
		return luaL_error(L, "active: monster is a player");
	
	lua_pushboolean(L, MONSTER_IS_ACTIVE(monster));
	return 1;
}

static int Lua_Monster_Get_External_Velocity(lua_State *L)
{
	monster_data* monster = get_monster_data(Lua_Monster::Index(L, 1));
	lua_pushnumber(L, (double) monster->external_velocity / WORLD_ONE);
	return 1;
}

static int Lua_Monster_Get_Facing(lua_State *L)
{
	monster_data *monster = get_monster_data(Lua_Monster::Index(L, 1));
	object_data *object = get_object_data(monster->object_index);
	lua_pushnumber(L, (double) object->facing * AngleConvert);
	return 1;
}

template<uint32 flag>
static int Lua_Monster_Get_Flag(lua_State* L)
{
	auto monster = get_monster_data(Lua_Monster::Index(L, 1));
	lua_pushboolean(L, monster->flags & flag);
	return 1;
}

static int Lua_Monster_Get_Mode(lua_State *L)
{
	monster_data *monster = get_monster_data(Lua_Monster::Index(L, 1));
	Lua_MonsterMode::Push(L, monster->mode);
	return 1;
}

static int Lua_Monster_Get_Player(lua_State *L)
{
	int monster_index = Lua_Monster::Index(L, 1);
	monster_data *monster = get_monster_data(monster_index);
	if (MONSTER_IS_PLAYER(monster))
		Lua_Player::Push(L, monster_index_to_player_index(monster_index));
	else
		lua_pushnil(L);
	
	return 1;
}

static int Lua_Monster_Get_Polygon(lua_State *L)
{
	monster_data *monster = get_monster_data(Lua_Monster::Index(L, 1));
	object_data *object = get_object_data(monster->object_index);
	Lua_Polygon::Push(L, object->polygon);
	return 1;
}

static int Lua_Monster_Get_Type(lua_State *L)
{
	monster_data *monster = get_monster_data(Lua_Monster::Index(L, 1));
	Lua_MonsterType::Push(L, monster->type);
	return 1;
}

static int Lua_Monster_Get_Valid(lua_State *L)
{
	lua_pushboolean(L, Lua_Monster::Valid(Lua_Monster::Index(L, 1)));
	return 1;
}

static int Lua_Monster_Get_Vertical_Velocity(lua_State *L)
{
	monster_data* monster = get_monster_data(Lua_Monster::Index(L, 1));
	lua_pushnumber(L, (double) monster->vertical_velocity / WORLD_ONE);
	return 1;
}

static int Lua_Monster_Get_Visible(lua_State *L)
{
	monster_data *monster = get_monster_data(Lua_Monster::Index(L, 1));
	object_data *object = get_object_data(monster->object_index);
	lua_pushboolean(L, OBJECT_IS_VISIBLE(object));
	return 1;
}

static int Lua_Monster_Get_Vitality(lua_State *L)
{
	monster_data *monster = get_monster_data(Lua_Monster::Index(L, 1));
	lua_pushnumber(L, monster->vitality);
	return 1;
}

static int Lua_Monster_Get_X(lua_State *L)
{
	monster_data *monster = get_monster_data(Lua_Monster::Index(L, 1));
	object_data *object = get_object_data(monster->object_index);
	lua_pushnumber(L, (double) object->location.x / WORLD_ONE);
	return 1;
}

static int Lua_Monster_Get_Y(lua_State *L)
{
	monster_data *monster = get_monster_data(Lua_Monster::Index(L, 1));
	object_data *object = get_object_data(monster->object_index);
	lua_pushnumber(L, (double) object->location.y / WORLD_ONE);
	return 1;
}

static int Lua_Monster_Get_Z(lua_State *L)
{
	monster_data *monster = get_monster_data(Lua_Monster::Index(L, 1));
	object_data *object = get_object_data(monster->object_index);
	lua_pushnumber(L, (double) object->location.z / WORLD_ONE);
	return 1;
}

static int Lua_Monster_Set_Active(lua_State *L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "active: incorrect argument type");	
	
	bool activate = lua_toboolean(L, 2);
	int monster_index = Lua_Monster::Index(L, 1);
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

static int Lua_Monster_Set_External_Velocity(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "external_velocity: incorrect argument type");

	monster_data* monster = get_monster_data(Lua_Monster::Index(L, 1));
	monster->external_velocity = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	return 0;
}

static int Lua_Monster_Set_Facing(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "facing: incorrect argument type");

	monster_data *monster = get_monster_data(Lua_Monster::Index(L, 1));
	object_data *object = get_object_data(monster->object_index);
	object->facing = static_cast<int>(lua_tonumber(L, 2) / AngleConvert);
	return 0;
}

template<uint32 flag, bool before_activation>
static int Lua_Monster_Set_Flag(lua_State* L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "monster flag: incorrect argument type");

	auto monster = get_monster_data(Lua_Monster::Index(L, 1));
	if (before_activation && MONSTER_IS_ACTIVE(monster))
		return luaL_error(L, "monster flag: monster already active");

	if (lua_toboolean(L, 2))
	{
		monster->flags |= flag;
	}
	else
	{
		monster->flags &= ~flag;
	}

	return 0;
}

static int Lua_Monster_Set_Vertical_Velocity(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "vertical_velocity: incorrect argument type");

	monster_data* monster = get_monster_data(Lua_Monster::Index(L, 1));
	monster->vertical_velocity = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	return 0;
}
	
static int Lua_Monster_Set_Visible(lua_State *L) {
	int monster_index = Lua_Monster::Index(L, 1);
	monster_data *monster = get_monster_data(monster_index);
	object_data *object = get_object_data(monster->object_index);
	int invisible = !lua_toboolean(L, 2);
	if(monster->action == _monster_is_teleporting_out) return 0;
	if(MONSTER_IS_ACTIVE(monster) || monster->vitality >= 0)
		return luaL_error(L, "visible: monster already activated");

	// No real possibility of messing stuff up here.
	SET_OBJECT_INVISIBILITY(object, invisible);
	return 0;
}

static int Lua_Monster_Set_Vitality(lua_State *L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "vitality: incorrect argument type");

	monster_data *monster = get_monster_data(Lua_Monster::Index(L, 1));
	monster->vitality = static_cast<int>(lua_tonumber(L, 2));
	return 0;
}

const luaL_Reg Lua_Monster_Get[] = {
	{"accelerate", L_TableFunction<Lua_Monster_Accelerate>},
	{"action", Lua_Monster_Get_Action},
	{"active", Lua_Monster_Get_Active},
	{"attack", L_TableFunction<Lua_Monster_Attack>},
	{"blind", Lua_Monster_Get_Flag<_monster_is_blind>},
	{"damage", L_TableFunction<Lua_Monster_Damage>},
	{"deaf", Lua_Monster_Get_Flag<_monster_is_deaf>},
	{"delete", L_TableFunction<Lua_Monster_Delete>},
	{"external_velocity", Lua_Monster_Get_External_Velocity},
	{"facing", Lua_Monster_Get_Facing},
	{"life", Lua_Monster_Get_Vitality},
	{"mode", Lua_Monster_Get_Mode},
	{"move_by_path", L_TableFunction<Lua_Monster_Move_By_Path>},
	{"player", Lua_Monster_Get_Player},
	{"play_sound", L_TableFunction<Lua_Monster_Play_Sound>},
	{"polygon", Lua_Monster_Get_Polygon},
	{"position", L_TableFunction<Lua_Monster_Position>},
	{"teleports_out", Lua_Monster_Get_Flag<_monster_teleports_out_when_deactivated>},
	{"type", Lua_Monster_Get_Type},
	{"valid", Lua_Monster_Get_Valid},
	{"vertical_velocity", Lua_Monster_Get_Vertical_Velocity},
	{"visible", Lua_Monster_Get_Visible},
	{"vitality", Lua_Monster_Get_Vitality},
	{"x", Lua_Monster_Get_X},
	{"y", Lua_Monster_Get_Y},
	{"yaw", Lua_Monster_Get_Facing},
	{"z", Lua_Monster_Get_Z},
	{0, 0}
};

const luaL_Reg Lua_Monster_Set[] = {
	{"active", Lua_Monster_Set_Active},
	{"blind", Lua_Monster_Set_Flag<_monster_is_blind, true>},
	{"deaf", Lua_Monster_Set_Flag<_monster_is_deaf, true>},
	{"external_velocity", Lua_Monster_Set_External_Velocity},
	{"facing", Lua_Monster_Set_Facing},
	{"life", Lua_Monster_Set_Vitality},
	{"teleports_out", Lua_Monster_Set_Flag<_monster_teleports_out_when_deactivated, false>},
	{"vertical_velocity", Lua_Monster_Set_Vertical_Velocity},
	{"visible", Lua_Monster_Set_Visible},
	{"vitality", Lua_Monster_Set_Vitality},
	{"yaw", Lua_Monster_Set_Facing},
	{0, 0}
};

char Lua_Monsters_Name[] = "Monsters";

// Monsters.new(x, y, height, polygon, type)
int Lua_Monsters_New(lua_State *L)
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

	short monster_type = Lua_MonsterType::ToIndex(L, 5);
	
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

	Lua_Monster::Push(L, monster_index);
	return 1;
}

const luaL_Reg Lua_Monsters_Methods[] = {
	{"new", L_TableFunction<Lua_Monsters_New>},
	{0, 0}
};

static void compatibility(lua_State *L);

int Lua_Monsters_register(lua_State *L)
{
	Lua_MonsterClass::Register(L, 0, 0, 0, Lua_MonsterClass_Mnemonics);
	Lua_MonsterClass::Valid = Lua_MonsterClass_Valid;

	Lua_MonsterClasses::Register(L);
	Lua_MonsterClasses::Length = Lua_MonsterClasses::ConstantLength(_class_yeti_bit + 1);


	Lua_MonsterType_Enemies::Register(L, 0, 0, Lua_MonsterType_Enemies_Metatable);
	Lua_MonsterType_Friends::Register(L, 0, 0, Lua_MonsterType_Friends_Metatable);
	Lua_MonsterType_Immunities::Register(L, 0, 0, Lua_MonsterType_Immunities_Metatable);
	Lua_MonsterType_Weaknesses::Register(L, 0, 0, Lua_MonsterType_Weaknesses_Metatable);

	Lua_MonsterMode::Register(L, 0, 0, 0, Lua_MonsterMode_Mnemonics);
	Lua_MonsterMode::Valid = Lua_MonsterMode::ValidRange(NUMBER_OF_MONSTER_MODES);
	Lua_MonsterModes::Register(L);
	Lua_MonsterModes::Length = Lua_MonsterModes::ConstantLength(NUMBER_OF_MONSTER_MODES);
	
	Lua_MonsterAction::Register(L, 0, 0, 0, Lua_MonsterAction_Mnemonics);
	Lua_MonsterAction::Valid = Lua_MonsterAction::ValidRange(NUMBER_OF_MONSTER_ACTIONS);
	Lua_MonsterActions::Register(L);
	Lua_MonsterActions::Length = Lua_MonsterActions::ConstantLength(NUMBER_OF_MONSTER_ACTIONS);

	Lua_MonsterType::Register(L, Lua_MonsterType_Get, Lua_MonsterType_Set, 0, Lua_MonsterType_Mnemonics);
	Lua_MonsterType::Valid = Lua_MonsterType_Valid;
	
	Lua_MonsterTypes::Register(L);
	Lua_MonsterTypes::Length = Lua_MonsterTypes::ConstantLength(NUMBER_OF_MONSTER_TYPES);

	Lua_Monster::Register(L, Lua_Monster_Get, Lua_Monster_Set);
	Lua_Monster::Valid = Lua_Monster_Valid;

	Lua_Monsters::Register(L, Lua_Monsters_Methods);
	Lua_Monsters::Length = boost::bind(get_dynamic_limit, (int) _dynamic_limit_monsters);

	compatibility(L);
	return 0;
}

static const char *compatibility_script = ""
// there are some conversions to and from internal units, because old
// monster API was wrong
	"function activate_monster(monster) Monsters[monster].active = true end\n"
	"function attack_monster(agressor, target) Monsters[aggressor]:attack(target) end\n"
	"function damage_monster(monster, damage, type) if type then Monsters[monster]:damage(damage, type) else Monsters[monster]:damage(damage) end end\n"
	"function deactivate_monster(monster) Monsters[monster].active = false end\n"
	"function get_monster_action(monster) if Monsters[monster].action then return Monsters[monster].action.index else return -1 end end\n"
	"function get_monster_enemy(monster_type, enemy_type) return MonsterTypes[monster_type].enemies[enemy_type] end\n"
	"function get_monster_friend(monster_type, friend_type) return MonsterTypes[monster_type].friends[friend_type] end\n"
	"function get_monster_facing(monster) return Monsters[monster].facing * 512 / 360 end\n"
	"function get_monster_immunity(monster, damage_type) return MonsterTypes[monster].immunities[damage_type] end\n"
	"function get_monster_item(monster) if MonsterTypes[monster].item then return MonsterTypes[monster].item.index else return -1 end end\n"
	"function get_monster_mode(monster) if Monsters[monster].mode then return Monsters[monster].mode.index else return -1 end end\n"
	"function get_monster_polygon(monster) return Monsters[monster].polygon.index end\n"
	"function get_monster_position(monster) return Monsters[monster].x * 1024, Monsters[monster].y * 1024, Monsters[monster].z * 1024 end\n"
	"function get_monster_type(monster) return Monsters[monster].type.index end\n"
	"function get_monster_type_class(monster) return MonsterTypes[monster].class.index end\n"
	"function get_monster_visible(monster) return Monsters[monster].visible end\n"
	"function get_monster_vitality(monster) return Monsters[monster].vitality end\n"
	"function get_monster_weakness(monster, damage) return MonsterTypes[monster].weaknesses[damage] end\n"
	"function monster_index_valid(monster) if Monsters[monster] then return true else return false end end\n"
	"function move_monster(monster, polygon) Monsters[monster]:move_by_path(polygon) end\n"
	"function new_monster(type, poly, facing, height, x, y) if (x and y) then m = Monsters.new(x, y, height / 1024, poly, type) elseif (height) then m = Monsters.new(Polygons[poly].x, Polygons[poly].y, height / 1024, poly, type) else m = Monsters.new(Polygons[poly].x, Polygons[poly].y, 0, poly, type) end if m then return m.index else return -1 end end\n"
	"function set_monster_enemy(monster_type, enemy_type, hostile) MonsterTypes[monster_type].enemies[enemy_type] = hostile end\n"
	"function set_monster_friend(monster_type, friend_type, friendly) MonsterTypes[monster_type].friends[friend_type] = friendly end\n"
	"function set_monster_immunity(monster, damage, immune) MonsterTypes[monster].immunities[damage] = immune end\n"
	"function set_monster_item(monster, item) if item == -1 then MonsterTypes[monster].item = nil else MonsterTypes[monster].item = item end end\n"
	"function set_monster_position(monster, polygon, x, y, z) Monsters[monster]:position(x, y, z, polygon) end\n"
	"function set_monster_type_class(monster, class) MonsterTypes[monster].class = class end\n"
	"function set_monster_vitality(monster, vitality) Monsters[monster].vitality = vitality end\n"
	"function set_monster_weakness(monster, damage, weak) MonsterTypes[monster].weaknesses[damage] = weak end\n"
	;

static void compatibility(lua_State *L)
{
	luaL_loadbuffer(L, compatibility_script, strlen(compatibility_script), "monsters_compatibility");
	lua_pcall(L, 0, 0, 0);
}



#endif
