#ifndef __LUA_OBJECTS_H
#define __LUA_OBJECTS_H

/*
LUA_OBJECTS.H

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

	Implements Lua map objects classes
*/

#include "cseries.h"

#ifdef HAVE_LUA
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "items.h"
#include "map.h"

struct Lua_Items {
	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg methods[];
	static int length() { return MAXIMUM_OBJECTS_PER_MAP; }
	static bool valid(int);

	static int new_item(lua_State *L);
};

struct Lua_Item {
	short index;
	static bool valid(int index) { return Lua_Items::valid(index); }

	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];

	static int delete_item(lua_State *L);

	static int get_polygon(lua_State *L);
	static int get_type(lua_State *L);
	static int get_x(lua_State *L);
	static int get_y(lua_State *L);
	static int get_z(lua_State *L);
};

struct Lua_ItemTypes {
	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg methods[];
	static int length() { return NUMBER_OF_DEFINED_ITEMS; }
	static bool valid(int index) { return index >= 0 && index < NUMBER_OF_DEFINED_ITEMS; }
};

struct Lua_ItemType {
	short index;
	static bool valid(int index) { return Lua_ItemTypes::valid(index); }
	
	static const char *name;
	static const luaL_reg metatable[];
	static const luaL_reg index_table[];
	static const luaL_reg newindex_table[];

	static int get_ball(lua_State *L);
};

int Lua_Objects_register(lua_State *L);

#endif

#endif
