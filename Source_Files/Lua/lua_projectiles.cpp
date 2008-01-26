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

#ifdef HAVE_LUA

#include "lua_monsters.h"
#include "lua_player.h"
#include "lua_projectiles.h"
#include "lua_templates.h"

#include "monsters.h"
#include "player.h"

const char *Lua_Projectile::name = "projectile";

int Lua_Projectile::get_owner(lua_State *L)
{
	projectile_data *projectile = get_projectile_data(L_Index<Lua_Projectile>(L, 1));
	L_Push<Lua_Monster>(L, projectile->owner_index);
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
	

const luaL_reg Lua_Projectile::index_table[] = {
	{"index", L_TableIndex<Lua_Projectile>},
	{"owner", get_owner},
	{"type", get_type},
	{0, 0}
};

const luaL_reg Lua_Projectile::newindex_table[] = {
	{"owner", set_owner},
	{0, 0}
};

const luaL_reg Lua_Projectile::metatable[] = {
	{"__index", L_TableGet<Lua_Projectile>},
	{"__newindex", L_TableSet<Lua_Projectile>},
	{ 0, 0}
};

const char *Lua_Projectiles::name = "Projectiles";

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
	"function get_projectile_owner(index) if Projectiles[index].owner then return Projectiles[index].owner.index end end\n"
	"function get_projectile_target(index) if Projectiles[index].target then return Projectiles[index].target.index end end\n"
	"function get_projectile_type(index) return Projectiles[index].type end\n"
	"function projectile_index_valid(index) if Projectiles[index] then return true else return false end end\n"
	"function set_projectile_owner(index, owner) Projectiles[index].owner = owner end\n"
	"function set_projectile_target(index, target) Projectiles[index].target = target end\n"
	;

static void compatibility(lua_State *L)
{
	luaL_loadbuffer(L, compatibility_script, strlen(compatibility_script), "projectiles_compatibility");
	lua_pcall(L, 0, 0, 0);
}
#endif
