/*
LUA_EPHEMERA.CPP

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

	Implements Lua ephemera classes
*/

#include "ephemera.h"
#include "lua_ephemera.h"
#include "lua_map.h"

const luaL_Reg Lua_Ephemera_Get[] = {
	{0, 0}
};

const luaL_Reg Lua_Ephemera_Set[] = {
	{0, 0}
};

char Lua_Ephemera_Name[] = "Ephemera";
char Lua_Ephemeras_Name[] = "Ephemeras";

const float AngleConvert = 360/float(FULL_CIRCLE);

// x, y, z, polygon, collection, sequence, facing
static int Lua_Ephemeras_New(lua_State* L)
{
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) ||
		!lua_isnumber(L, 5) || !lua_isnumber(L, 6) || !lua_isnumber(L, 7))
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

	uint16_t collection = static_cast<uint16_t>(lua_tonumber(L, 5));
	uint16_t shape = static_cast<uint16_t>(lua_tonumber(L, 6));

	world_point3d origin;
	origin.x = static_cast<int>(lua_tonumber(L, 1) * WORLD_ONE);
	origin.y = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	origin.z = static_cast<int>(lua_tonumber(L, 3) * WORLD_ONE);

	angle facing = static_cast<int>(lua_tonumber(L, 7) / AngleConvert);

	int16_t ephemera_index = ::new_ephemera(origin, polygon_index, BUILD_DESCRIPTOR(collection, shape), facing);

	if (ephemera_index == NONE)
	{
		return 0;
	}

	Lua_Ephemera::Push(L, ephemera_index);
	return 1;
}

const luaL_Reg Lua_Ephemeras_Methods[] = {
	{"new", L_TableFunction<Lua_Ephemeras_New>},
	{0, 0}
};

static bool Lua_Ephemera_Valid(int32 index)
{
	if (index < 0 || index >= get_max_ephemera())
	{
		return false;
	}
	else
	{
		return SLOT_IS_USED(get_ephemera_data(index));
	}
}

int Lua_Ephemera_register(lua_State* L)
{
	Lua_Ephemera::Register(L, Lua_Ephemera_Get, Lua_Ephemera_Set);
	Lua_Ephemera::Valid = Lua_Ephemera_Valid;

	Lua_Ephemeras::Register(L, Lua_Ephemeras_Methods);

	return 0;
}
