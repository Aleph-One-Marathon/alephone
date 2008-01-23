#ifndef __LUA_TEMPLATES_H
#define __LUA_TEMPLATES_H

/*
LUA_TEMPLATES.H

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

	Templates to help create the Lua/C interface
*/

#include "cseries.h"

#ifdef HAVE_LUA
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

/* Templates: classes should resemble this one:
class Lua_Example {
	int16 index;
	static const char *name;
	static const luaL_reg metatable[];
};
*/

template<class T>
T* L_To(lua_State *L, int index)
{
	T* t = static_cast<T*>(lua_touserdata(L, index));
	if (!t) luaL_typerror(L, index, T::name);
	return t;
}

template<class T>
T* L_Check(lua_State *L, int index)
{
	luaL_checktype(L, index, LUA_TUSERDATA);
	T* t = static_cast<T*>(luaL_checkudata(L, index, T::name));
	if (!t) luaL_typerror(L, index, T::name);
	return t;
}

template<class T>
T* L_Push(lua_State *L)
{
	T* t = static_cast<T*>(lua_newuserdata(L, sizeof(T)));
	luaL_getmetatable(L, T::name);
	lua_setmetatable(L, -2);

	return t;
}

template<class T>
T* L_Push(lua_State *L, int16 index)
{
	T* t = L_Push<T>(L);
	t->index = index;

	return t;
}

template<class T>
int L_Index(lua_State *L, int index)
{
	T* t = L_To<T>(L, index);
	return t->index;
}

template<class T>
bool L_Is(lua_State *L, int index)
{
	T* t = static_cast<T*>(lua_touserdata(L, index));
	if (!t) return false;

	if (lua_getmetatable(L, index))
	{
		lua_getfield(L, LUA_REGISTRYINDEX, T::name);
		if (lua_rawequal(L, -1, -2))
		{
			lua_pop(L, 2);
			return true;
		}
	}

	return false;
}

/* the functions below are to be bound in Lua--that's why they only take 1 arg
   and return int
 */

template<class T>
int L_TableIndex(lua_State *L)
{
	lua_pushnumber(L, L_Index<T>(L, 1));
	return 1;
}

template<class T>
int L_TableIs(lua_State *L)
{
	lua_pushboolean(L, L_Is<T>(L, 1));
	return 1;
}

template<class T>
int L_TableEqual(lua_State *L)
{
	if (!L_Is<T>(L, 1) || !L_Is<T>(L, 2)) 
	{
		lua_pushboolean(L, false);
	} 
	else
	{
		lua_pushboolean(L, (L_Index<T>(L, 1) == L_Index<T>(L, 2)));
	}

	return 1;
}

/* For these to work, add these fields:
   static const luaL_reg index_table[];
   static const luaL_reg newindex_table[];
*/


template<class T>
int L_TableGet(lua_State *L)
{
	if (lua_isstring(L, 2))
	{
		luaL_checktype(L, 1, LUA_TUSERDATA);
		luaL_checkudata(L, 1, T::name);

		// pop the get table
		lua_pushlightuserdata(L, (void *)(&T::index_table));
		lua_gettable(L, LUA_REGISTRYINDEX);
		
		// get the function from that table
		lua_pushvalue(L, 2);
		lua_gettable(L, -2);

		if (lua_isfunction(L, -1))
		{
			// execute the function with table as our argument
			lua_pushvalue(L, 1);
			if (lua_pcall(L, 1, 1, 0) == LUA_ERRRUN)
			{
				// report the error as being on this line
				luaL_where(L, 1);
				lua_pushvalue(L, -2);
				lua_concat(L, 2);
				lua_error(L);
			}
		}
		else
		{
			lua_pushnil(L);
		}
	}
	else
	{
		lua_pushnil(L);
	}
	
	return 1;
}

template<class T>
int L_TableSet(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	luaL_checkudata(L, 1, T::name);
	
	// pop the set table
	lua_pushlightuserdata(L, (void *)(&T::newindex_table));
	lua_gettable(L, LUA_REGISTRYINDEX);
	
	// get the function from that table
	lua_pushvalue(L, 2);
	lua_gettable(L, -2);
	
	if (lua_isnil(L, -1))
	{
		luaL_error(L, "no such index");
	}
	
	// execute the function with table, value as our arguments
	lua_pushvalue(L, 1);
	lua_pushvalue(L, 3);
	if (lua_pcall(L, 2, 0, 0) == LUA_ERRRUN)
	{
		// report the error as being on this line
		luaL_where(L, 1);
		lua_pushvalue(L, -2);
		lua_concat(L, 2);
		lua_error(L);
	}

	return 0;
};

template<class T>
void L_Register(lua_State *L)
{
	// create the metatable itself
	luaL_newmetatable(L, T::name);
	luaL_openlib(L, 0, T::metatable, 0);

	// register get methods
	lua_pushlightuserdata(L, (void *) (&T::index_table));
	lua_newtable(L);
	luaL_openlib(L, 0, T::index_table, 0);
	lua_settable(L, LUA_REGISTRYINDEX);

	// register set methods
	lua_pushlightuserdata(L, (void *) (&T::newindex_table));
	lua_newtable(L);
	luaL_openlib(L, 0, T::newindex_table, 0);
	lua_settable(L, LUA_REGISTRYINDEX);

	// register is_
	lua_pushcfunction(L, L_TableIs<T>);
	std::string is_name = "is_" + std::string(T::name);
	lua_setglobal(L, is_name.c_str());
}

// pushes a function that returns the parameterized function
template<lua_CFunction f>
int L_TableFunction(lua_State *L)
{
	lua_pushcfunction(L, f);
	return 1;
}

#endif

#endif
