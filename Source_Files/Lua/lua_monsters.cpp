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
#include "lua_templates.h"

#ifdef HAVE_LUA

const char *Lua_Monster::name = "monster";

int Lua_Monster::valid(lua_State *L)
{
	lua_pushboolean(L, valid(L_Index<Lua_Monster>(L, 1)));
	return 1;
}

const luaL_reg Lua_Monster::index_table[] = {
	{"index", L_TableIndex<Lua_Monster>},
	{"valid", Lua_Monster::valid},
	{0, 0}
};

const luaL_reg Lua_Monster::newindex_table[] = {
	{0, 0}
};

const luaL_reg Lua_Monster::metatable[] = {
	{"__index", L_TableGet<Lua_Monster>},
	{"__newindex", L_TableSet<Lua_Monster>},
	{0, 0}
};

const char *Lua_Monsters::name = "Monsters";

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

int Lua_Monsters_register(lua_State *L)
{
	L_Register<Lua_Monster>(L);
	L_GlobalRegister<Lua_Monsters>(L);
}


#endif
