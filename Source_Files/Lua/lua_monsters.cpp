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

#include "monsters.h"
#include "player.h"

#ifdef HAVE_LUA

const char *Lua_Monster::name = "monster";

int Lua_Monster::valid(lua_State *L)
{
	lua_pushboolean(L, valid(L_Index<Lua_Monster>(L, 1)));
	return 1;
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

int Lua_Monster::get_vitality(lua_State *L)
{
	monster_data *monster = get_monster_data(L_Index<Lua_Monster>(L, 1));
	lua_pushnumber(L, monster->vitality);
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
	{"index", L_TableIndex<Lua_Monster>},
	{"life", Lua_Monster::get_vitality},
	{"mode", Lua_Monster::get_mode},
	{"player", Lua_Monster::get_player},
	{"polygon", Lua_Monster::get_polygon},
	{"type", Lua_Monster::get_type},
	{"valid", Lua_Monster::valid},
	{"vitality", Lua_Monster::get_vitality},
	{0, 0}
};

const luaL_reg Lua_Monster::newindex_table[] = {
	{"active", Lua_Monster::set_active},
	{"life", Lua_Monster::set_vitality},
	{"vitality", Lua_Monster::set_vitality},
	{0, 0}
};

const luaL_reg Lua_Monster::metatable[] = {
	{"__index", L_TableGet<Lua_Monster>},
	{"__newindex", L_TableSet<Lua_Monster>},
	{0, 0}
};

const char *Lua_Monsters::name = "Monsters";

const luaL_reg Lua_Monsters::methods[] = {
	{0, 0}
};

const luaL_reg Lua_Monsters::metatable[] = {
	{"__index", L_GlobalIndex<Lua_Monsters, Lua_Monster>},
	{"__newindex", L_GlobalNewindex<Lua_Monsters>},
	{"__call", L_GlobalCall<Lua_Monsters, Lua_Monster>},
	{0, 0}
};

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
	"function activate_monster(monster) Monsters[monster].active = true end\n"
	"function deactivate_monster(monster) Monsters[monster].active = false end\n"
	"function get_monster_action(monster) return Monsters[monster].action end\n"
	"function get_monster_mode(monster) return Monsters[monster].mode end\n"
	"function get_monster_polygon(monster) return Monsters[monster].polygon.index end\n"
	"function get_monster_type(monster) return Monsters[monster].type end\n"
	"function get_monster_vitality(monster) return Monsters[monster].vitality end\n"
	"function monster_index_valid(monster) if Monsters[monster] then return true else return false end end\n"
	"function set_monster_vitality(monster, vitality) Monsters[monster].vitality = vitality end\n"
	;

static int compatibility(lua_State *L)
{
	luaL_loadbuffer(L, compatibility_script, strlen(compatibility_script), "monsters_compatibility");
	lua_pcall(L, 0, 0, 0);
}



#endif
