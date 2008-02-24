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

#include <boost/function.hpp>

/* There are two sets of templates here: the first is for objects like player,
   and the second are for the global accessors that get them, like Players */

/* Templates: classes should resemble this one:
struct Lua_Example {
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
T* L_PushNew(lua_State *L)
{
	T* t = static_cast<T*>(lua_newuserdata(L, sizeof(T)));
	luaL_getmetatable(L, T::name);
	lua_setmetatable(L, -2);
	return t;
}

template<class T>
T* L_Push(lua_State *L, int index)
{
	T* t = 0;

	if (!T::valid(index))
	{
		lua_pushnil(L);
		return 0;
	}

	// look it up in the index table
	lua_pushlightuserdata(L, (void *) &T::metatable);
	lua_gettable(L, LUA_REGISTRYINDEX);

	lua_pushnumber(L, index);
	lua_gettable(L, -2);

	if (lua_istable(L, -1))
	{
		// remove the index table
		lua_remove(L, -2);

		lua_pushnumber(L, 0);
		lua_gettable(L, -2);
		lua_remove(L, -2);
		
		T* t = static_cast<T*>(lua_touserdata(L, -1));
	}
	else if (lua_isnil(L, -1))
	{
		// get rid of the nil
		lua_pop(L, 1);
		
		lua_newtable(L);

		t = L_PushNew<T>(L);
		t->index = index;
		
		lua_pushvalue(L, -1);
		lua_insert(L, -4);
		lua_pushnumber(L, 0);
		lua_insert(L, -2);
		lua_settable(L, -3);

		lua_pushnumber(L, index); 
		lua_insert(L, -2);
		lua_settable(L, -3);

		lua_pop(L, 1);
	}

	return t;
}

// assumes argument is the correct userdatum, does not perform validation
// use L_ToIndex<> if you want validation, conversion from number/string
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

template<class T>
int L_ToIndex(lua_State *L, int index)
{
	if (L_Is<T>(L, index))
		return L_Index<T>(L, index);
	else if (lua_isnumber(L, index))
	{
		int to_index = static_cast<int>(lua_tonumber(L, index));
		if (!T::valid(to_index))
		{
			string error = string(T::name) + ": invalid index";
			return luaL_error(L, error.c_str());
		}
		else
			return to_index;
	}
	else
	{
		string error = string(T::name) +  ": incorrect argument type";
		return luaL_error(L, error.c_str());
	}
}

/* the functions below are to be bound in Lua--that's why they only take 1 arg
   and return int
 */

template<class T>
int L_Equals(lua_State *L)
{
	lua_pushboolean(L, L_ToIndex<T>(L, 1) == L_ToIndex<T>(L, 2));
	return 1;
}

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

		if (!T::valid(L_Index<T>(L, 1)) && strcmp(lua_tostring(L, 2), "valid") != 0)
			luaL_error(L, "invalid object");
		
		// pop the get table
		lua_pushlightuserdata(L, (void *)(&T::index_table));
		lua_gettable(L, LUA_REGISTRYINDEX);
		
		// get the function from that table
		lua_pushvalue(L, 2);
		lua_gettable(L, -2);
		lua_remove(L, -2);

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
			lua_pop(L, 1);
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

	lua_pop(L, 1);

	return 0;
};

template<class T>
void L_Register(lua_State *L)
{
	// create the metatable itself
	luaL_newmetatable(L, T::name);
	luaL_openlib(L, 0, T::metatable, 0);
	lua_pop(L, 1);

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

	// register a table for instances
	lua_pushlightuserdata(L, (void *) (&T::metatable));
	lua_newtable(L);
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

/* Templates for Globals: classes should resemble this one:
struct Lua_Example {
	static const char *name;
	static const luaL_reg metatable[];
	static int length();
	static bool valid(int index); // whether to return while iterating
};
*/

template<class G, class T>
int L_GlobalIterator(lua_State *L)
{
	int index = static_cast<int>(lua_tonumber(L, lua_upvalueindex(1)));
	while (index < G::length())
	{
		if (G::valid(index))
		{
			L_Push<T>(L, index);
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

template<class G, class T>
int L_GlobalCall(lua_State *L)
{
	lua_pushnumber(L, 0);
	lua_pushcclosure(L, L_GlobalIterator<G, T>, 1);
	return 1;
}

template<class G, class T>
int L_GlobalIndex(lua_State *L)
{
	if (lua_isnumber(L, 2))
	{
		int index = static_cast<int>(lua_tonumber(L, 2));
		if (!T::valid(index))
		{
			lua_pushnil(L);
		}
		else
		{
			L_Push<T>(L, index);
		}
	}
	else
	{
		// get the function from methods

		lua_pushlightuserdata(L, (void *)(&G::methods));
		lua_gettable(L, LUA_REGISTRYINDEX);

		lua_pushvalue(L, 2);
		lua_gettable(L, -2);
	}

	return 1;
}

template<class G>
int L_GlobalLength(lua_State *L)
{
	lua_pushnumber(L, G::length());
	return 1;
}

template<class G>
int L_GlobalNewindex(lua_State *L)
{
	std::string error = std::string(G::name) + " is read-only";
	luaL_error(L, error.c_str());
}

template<class G>
void L_GlobalRegister(lua_State *L)
{
	lua_newuserdata(L, 0);
	lua_pushvalue(L, -1);
	luaL_newmetatable(L, G::name);
	luaL_openlib(L, 0, G::metatable, 0);
	lua_setmetatable(L, -2);
	lua_setglobal(L, G::name);
	lua_pop(L, 1);

	lua_pushlightuserdata(L, (void *) (&G::methods));
	lua_newtable(L);
	luaL_openlib(L, 0, G::methods, 0);
	lua_settable(L, LUA_REGISTRYINDEX);
}

template<char *name>
class L_Class {
public:

	short m_index;

	static void Register(lua_State *L, const luaL_reg get[] = 0, const luaL_reg set[] = 0, const luaL_reg metatable[] = 0);
	static L_Class *Push(lua_State *L, int32 index);
	static int32 Index(lua_State *L, int index);
	static bool Is(lua_State *L, int index);
	static boost::function<bool (int32)> Valid;
private:
	// C functions for Lua
	static int _index(lua_State *L);
	static int _get(lua_State *L);
	static int _set(lua_State *L);
	static int _is(lua_State *L);
};

static bool always_valid(int32) { return true; }

template<char *name>
boost::function<bool (int32)> L_Class<name>::Valid = always_valid;

template<char *name>
void L_Class<name>::Register(lua_State *L, const luaL_reg get[], const luaL_reg set[], const luaL_reg metatable[])
{
	// create the metatable itself
	luaL_newmetatable(L, name);

	// register metatable get
	lua_pushstring(L, "__index");
	lua_pushcfunction(L, _get);
	lua_settable(L, -3);

	// register metatable set
	lua_pushstring(L, "__newindex");
	lua_pushcfunction(L, _set);
	lua_settable(L, -3);

	if (metatable)
		luaL_openlib(L, 0, metatable, 0);
	
	lua_pop(L, 1);
	
	// register get methods
	lua_pushlightuserdata(L, (void *) (&name[1]));
	lua_newtable(L);

	// always want index
	lua_pushstring(L, "index");
	lua_pushcfunction(L, _index);
	lua_settable(L, -3);

	if (get)
		luaL_openlib(L, 0, get, 0);
	lua_settable(L, LUA_REGISTRYINDEX);

	// register set methods
	lua_pushlightuserdata(L, (void *) (&name[2]));
	lua_newtable(L);

	if (set)
		luaL_openlib(L, 0, set, 0);
	lua_settable(L, LUA_REGISTRYINDEX);
		
	// register a table for instances
	lua_pushlightuserdata(L, (void *) (&name[3]));
	lua_newtable(L);
	lua_settable(L, LUA_REGISTRYINDEX);

	// register is_
	lua_pushcfunction(L, _is);
	std::string is_name = "is_" + std::string(name);
	lua_setglobal(L, is_name.c_str());
}

template<char *name>
L_Class<name> *L_Class<name>::Push(lua_State *L, int32 index)
{
	L_Class<name>* t = 0;

	if (!Valid(index))
	{
		lua_pushnil(L);
		return 0;
	}

	// look it up in the index table
	lua_pushlightuserdata(L, (void *) (&name[3]));
	lua_gettable(L, LUA_REGISTRYINDEX);

	lua_pushnumber(L, index);
	lua_gettable(L, -2);

	if (lua_istable(L, -1))
	{
		// remove the index table
		lua_remove(L, -2);
		
		lua_pushnumber(L, 0);
		lua_gettable(L, -2);
		lua_remove(L, -2);

		t = static_cast<L_Class<name> *>(lua_touserdata(L, -1));
	}
	else if (lua_isnil(L, -1))
	{
		lua_pop(L, 1);

		lua_newtable(L);
		
		
		t = static_cast<L_Class<name> *>(lua_newuserdata(L, sizeof(L_Class<name>)));
		luaL_getmetatable(L, name);
		lua_setmetatable(L, -2);
		t->m_index = index;

		lua_pushvalue(L, -1);
		lua_insert(L, -4);
		lua_pushnumber(L, 0);
		lua_insert(L, -2);
		lua_settable(L, -3);

		lua_pushnumber(L, index);
		lua_insert(L, -2);
		lua_settable(L, -3);

		lua_pop(L, 1);
	}

	return t;
}

template<char *name>
int32 L_Class<name>::Index(lua_State *L, int index)
{
	L_Class<name> *t = static_cast<L_Class<name> *>(lua_touserdata(L, index));
	if (!t) luaL_typerror(L, index, name);
	return t->m_index;
}

template<char *name>
bool L_Class<name>::Is(lua_State *L, int index)
{
	L_Class<name>* t = static_cast<L_Class<name>*>(lua_touserdata(L, index));
	if (!t) return false;

	if (lua_getmetatable(L, index))
	{
		lua_getfield(L, LUA_REGISTRYINDEX, name);
		if (lua_rawequal(L, -1, -2))
		{
			lua_pop(L, 2);
			return true;
		}
		else
		{
			lua_pop(L, 2);
			return false;
		}
	}

	return false;
}

template<char *name>
int L_Class<name>::_index(lua_State *L)
{
	lua_pushnumber(L, Index(L, 1));
	return 1;
}

template<char *name>
int L_Class<name>::_is(lua_State *L)
{
	lua_pushboolean(L, Is(L, 1));
	return 1;
}

template<char *name>
int L_Class<name>::_get(lua_State *L)
{
	if (lua_isstring(L, 2))
	{
		luaL_checktype(L, 1, LUA_TUSERDATA);
		luaL_checkudata(L, 1, name);
		if (!Valid(Index(L, 1)) && strcmp(lua_tostring(L, 2), "valid") != 0)
			luaL_error(L, "invalid object");

		// pop the get table
		lua_pushlightuserdata(L, (void *) (&name[1]));
		lua_gettable(L, LUA_REGISTRYINDEX);

		// get the function from that table
		lua_pushvalue(L, 2);
		lua_gettable(L, -2);
		lua_remove(L, -2);
		
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
			lua_pop(L, 1);
			lua_pushnil(L);
		}
	}
	else
	{
		lua_pushnil(L);
	}

	return 1;
}

template<char *name>
int L_Class<name>::_set(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	luaL_checkudata(L, 1, name);
	
	// pop the set table
	lua_pushlightuserdata(L, (void *)(&name[2]));
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

	lua_pop(L, 1);

	return 0;
}

// enum classes define equality with numbers
template<char *name>
class L_Enum : public L_Class<name>
{
public:
	static void Register(lua_State *L, const luaL_reg get[] = 0, const luaL_reg set[] = 0, const luaL_reg metatable[] = 0);
	static int32 ToIndex(lua_State *L, int index);
private:
	static int _equals(lua_State *L);
};

template<char *name>
void L_Enum<name>::Register(lua_State *L, const luaL_reg get[], const luaL_reg set[], const luaL_reg metatable[])
{
	L_Class<name>::Register(L, get, set, 0);

	luaL_getmetatable(L, name);
	lua_pushstring(L, "__eq");
	lua_pushcfunction(L, _equals);
	lua_settable(L, -3);

	if (metatable)
		luaL_openlib(L, 0, metatable, 0);

	lua_pop(L, 1);
}

template<char *name>
int32 L_Enum<name>::ToIndex(lua_State *L, int index)
{
	if (L_Class<name>::Is(L, index))
		return L_Class<name>::Index(L, index);
	else if (lua_isnumber(L, index))
	{
		int to_index = static_cast<int>(lua_tonumber(L, index));
		if (!L_Class<name>::Valid(to_index))
		{
			string error = string(name) + ": invalid index";
			return luaL_error(L, error.c_str());
		}
		else
			return to_index;
	}
	else
	{
		string error = string(name) + ": incorrect argument type";
		return luaL_error(L, error.c_str());
	}
}

template<char *name>
int L_Enum<name>::_equals(lua_State *L)
{
	lua_pushboolean(L, ToIndex(L, 1) == ToIndex(L, 2));
	return 1;
}

template<char *name, class T>
class L_Container {
public:
	static void Register(lua_State *L, const luaL_reg methods[] = 0, const luaL_reg metatable[] = 0);
	static boost::function<int (void)> Length;
	template<int length> static int ConstantLength() { return length; }
private:
	static int _get(lua_State *);
	static int _set(lua_State *);
	static int _call(lua_State *);
	static int _iterator(lua_State *);
	static int _length(lua_State *);
};

template<char *name, class T>
boost::function<int (void)> L_Container<name, T>::Length = ConstantLength<1>;

template<char *name, class T>
void L_Container<name, T>::Register(lua_State *L, const luaL_reg methods[], const luaL_reg metatable[])
{
	lua_newuserdata(L, 0);
	lua_pushvalue(L, -1);
	luaL_newmetatable(L, name);
	
	lua_pushstring(L, "__index");
	lua_pushcfunction(L, _get);
	lua_settable(L, -3);
	lua_pushstring(L, "__newindex");
	lua_pushcfunction(L, _set);
	lua_settable(L, -3);
	lua_pushstring(L, "__call");
	lua_pushcfunction(L, _call);
	lua_settable(L, -3);
	if (metatable)
		luaL_openlib(L, 0, metatable, 0);
	lua_setmetatable(L, -2);
	lua_setglobal(L, name);
	lua_pop(L, 1);

	lua_pushlightuserdata(L, (void *) (&name[1]));
	lua_newtable(L);
	if (methods)
		luaL_openlib(L, 0, methods, 0);
	lua_settable(L, LUA_REGISTRYINDEX);
}

template<char *name, class T>
int L_Container<name, T>::_get(lua_State *L)
{
	if (lua_isnumber(L, 2))
	{
		int32 index = static_cast<int32>(lua_tonumber(L, 2));
		if (!T::Valid(index))
		{
			lua_pushnil(L);
		}
		else
		{
			T::Push(L, index);
		}
	}
	else
	{
		// get the function from methods
		lua_pushlightuserdata(L, (void *) (&name[1]));
		lua_gettable(L, LUA_REGISTRYINDEX);

		lua_pushvalue(L, 2);
		lua_gettable(L, -2);
	}

	return 1;
}

template<char *name, class T>
int L_Container<name, T>::_set(lua_State *L)
{
	std::string error = std::string(name) + " is read-only";
	luaL_error(L, error.c_str());
}

template<char *name, class T>
int L_Container<name, T>::_iterator(lua_State *L)
{
	int32 index = static_cast<int32>(lua_tonumber(L, lua_upvalueindex(1)));
	while (index < Length())
	{
		if (T::Valid(index))
		{
			T::Push(L, index);
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

template<char *name, class T>
int L_Container<name, T>::_call(lua_State *L)
{
	lua_pushnumber(L, 0);
	lua_pushcclosure(L, _iterator, 1);
	return 1;
}

// enum containers will be able to look up by strings
template<char *name, class T>
class L_EnumContainer : public L_Container<name, T>
{

};

#endif

#endif
