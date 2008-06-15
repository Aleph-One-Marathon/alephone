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
#include "lua_script.h"
#include "lua_mnemonics.h" // for lang_def and mnemonics
#include <sstream>

// pushes a function that returns the parameterized function
template<lua_CFunction f>
int L_TableFunction(lua_State *L)
{
	lua_pushcfunction(L, f);
	return 1;
}

template<char *name, typename index_t = int16>
class L_Class {
public:

	index_t m_index;
	typedef index_t index_type;

	static void Register(lua_State *L, const luaL_reg get[] = 0, const luaL_reg set[] = 0, const luaL_reg metatable[] = 0);
	static L_Class *Push(lua_State *L, index_t index);
	static index_t Index(lua_State *L, int index);
	static bool Is(lua_State *L, int index);
	static void Invalidate(lua_State *L, index_t index);
	static boost::function<bool (index_t)> Valid;
	struct ValidRange
	{
		ValidRange(int32 max_index) : m_max(max_index) {}
		bool operator() (int32 index)
		{
			return (index >= 0 && index < m_max);
		}

		int32 m_max;
	};

	// ghs: codewarrior chokes on this:
	//	template<index_t max_index> static bool ValidRange(index_t index) { return index >= 0 && index < max_index; }
private:
	// C functions for Lua
	static int _index(lua_State *L);
	static int _get(lua_State *L);
	static int _set(lua_State *L);
	static int _is(lua_State *L);
	static int _tostring(lua_State *L);
};

struct always_valid
{
	bool operator()(int32 x) { return true; }
};

template<char *name, typename index_t>
boost::function<bool (index_t)> L_Class<name, index_t>::Valid = always_valid();

template<char *name, typename index_t>
void L_Class<name, index_t>::Register(lua_State *L, const luaL_reg get[], const luaL_reg set[], const luaL_reg metatable[])
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

	// register metatable tostring
	lua_pushstring(L, "__tostring");
	lua_pushcfunction(L, _tostring);
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

template<char *name, typename index_t>
L_Class<name, index_t> *L_Class<name, index_t>::Push(lua_State *L, index_t index)
{
	L_Class<name, index_t>* t = 0;

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

		t = static_cast<L_Class<name, index_t> *>(lua_touserdata(L, -1));
	}
	else if (lua_isnil(L, -1))
	{
		lua_pop(L, 1);

		lua_newtable(L);
		
		
		t = static_cast<L_Class<name, index_t> *>(lua_newuserdata(L, sizeof(L_Class<name, index_t>)));
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

template<char *name, typename index_t>
index_t L_Class<name, index_t>::Index(lua_State *L, int index)
{
	L_Class<name, index_t> *t = static_cast<L_Class<name, index_t> *>(lua_touserdata(L, index));
	if (!t) luaL_typerror(L, index, name);
	return t->m_index;
}

template<char *name, typename index_t>
bool L_Class<name, index_t>::Is(lua_State *L, int index)
{
	L_Class<name, index_t>* t = static_cast<L_Class<name, index_t>*>(lua_touserdata(L, index));
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

template<char *name, typename index_t>
void L_Class<name, index_t>::Invalidate(lua_State *L, index_t index)
{
	// remove it from the index table
	lua_pushlightuserdata(L, (void *) (&name[3]));
	lua_gettable(L, LUA_REGISTRYINDEX);

	lua_pushnumber(L, index);
	lua_pushnil(L);
	lua_settable(L, -3);
	lua_pop(L, 1);
}

template<char *name, typename index_t>
int L_Class<name, index_t>::_index(lua_State *L)
{
	lua_pushnumber(L, Index(L, 1));
	return 1;
}

template<char *name, typename index_t>
int L_Class<name, index_t>::_is(lua_State *L)
{
	lua_pushboolean(L, Is(L, 1));
	return 1;
}

template<char *name, typename index_t>
int L_Class<name, index_t>::_get(lua_State *L)
{
	if (lua_isstring(L, 2))
	{
		luaL_checktype(L, 1, LUA_TUSERDATA);
		luaL_checkudata(L, 1, name);
		if (!Valid(Index(L, 1)) && strcmp(lua_tostring(L, 2), "valid") != 0)
			luaL_error(L, "invalid object");

		if (lua_tostring(L, 2)[0] == '_')
		{
			// look it up in the custom table
			lua_pushlightuserdata(L, (void *) (&name[3]));
			lua_gettable(L, LUA_REGISTRYINDEX);

			lua_pushnumber(L, Index(L, 1));
			lua_gettable(L, -2);

			if (lua_istable(L, -1))
			{
				// remove the index table
				lua_remove(L, -2);
				lua_pushvalue(L, -2);
				lua_gettable(L, -2);
				lua_remove(L, -2);
			}
			else
			{
				lua_pop(L, 2);
				lua_pushnil(L);
			}
		}
		else
		{
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
	}
	else
	{
		lua_pushnil(L);
	}

	return 1;
}

template<char *name, typename index_t>
int L_Class<name, index_t>::_set(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	luaL_checkudata(L, 1, name);

	if (lua_isstring(L, 2) && lua_tostring(L, 2)[0] == '_')
	{
		// get the index table
		lua_pushlightuserdata(L, (void *)(&name[3]));
		lua_gettable(L, LUA_REGISTRYINDEX);

		lua_pushnumber(L, Index(L, 1));
		lua_gettable(L, -2);

		if (lua_istable(L, -1))
		{
			lua_pushvalue(L, 2);
			lua_pushvalue(L, 3);
			lua_settable(L, -3);
			lua_pop(L, 1);
		}
	}
	else
	{
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
	}
		
	return 0;
}

template<char *name, typename index_t>
int L_Class<name, index_t>::_tostring(lua_State *L)
{
	std::ostringstream s;
	s << name << " " << Index(L, 1);
	lua_pushstring(L, s.str().c_str());
	return 1;
}

// enum classes define equality with numbers
template<char *name, typename index_t = int16>
class L_Enum : public L_Class<name, index_t>
{
public:
	static void Register(lua_State *L, const luaL_reg get[] = 0, const luaL_reg set[] = 0, const luaL_reg metatable[] = 0, const lang_def mnemonics[] = 0);
	static index_t ToIndex(lua_State *L, int index);
	static void PushMnemonicTable(lua_State *L);
protected:
	static bool _lookup(lua_State *L, int index, index_t& to);
	static int _equals(lua_State *L);
private:
	static int _get_mnemonic(lua_State *L);
	static int _set_mnemonic(lua_State *L);
};

// the justification for this specialization is long
template<char *name, typename index_t = int16>
class L_LazyEnum : public L_Enum<name, index_t>
{
public:
	static index_t ToIndex(lua_State *L, int index) {
		index_t to;
		if(lua_isnil(L, index)) return -1;
		else if(_lookup(L, index, to)) return to;
		else {
			std::string error;
			if(lua_isnumber(L, index) || lua_isstring(L, index))
				error = std::string(name) + ": invalid index";
			else
				error = std::string(name) + ": incorrect argument type";
			return luaL_error(L, error.c_str());
		}
	}
};

template<char *name, typename index_t>
void L_Enum<name, index_t>::Register(lua_State *L, const luaL_reg get[], const luaL_reg set[], const luaL_reg metatable[], const lang_def mnemonics[])
{
	L_Class<name, index_t>::Register(L, get, set, 0);

	lua_pushlightuserdata(L, (void *) (&name[1]));
	lua_gettable(L, LUA_REGISTRYINDEX);
	lua_pushstring(L, "mnemonic");
	lua_pushcfunction(L, _get_mnemonic);
	lua_settable(L, -3);
	lua_pop(L, 1);

	lua_pushlightuserdata(L, (void *) (&name[2]));
	lua_gettable(L, LUA_REGISTRYINDEX);
	lua_pushstring(L, "mnemonic");
	lua_pushcfunction(L, _set_mnemonic);
	lua_settable(L, -3);
	lua_pop(L, 1);

	luaL_getmetatable(L, name);
	lua_pushstring(L, "__eq");
	lua_pushcfunction(L, _equals);
	lua_settable(L, -3);

	lua_pushstring(L, "__tostring");
	lua_pushcfunction(L, _get_mnemonic);
	lua_settable(L, -3);

	if (metatable)
		luaL_openlib(L, 0, metatable, 0);

	lua_pop(L, 1);

	if (mnemonics)
	{
		lua_pushlightuserdata(L, (void *) (&name[4]));
		lua_newtable(L);
		
		const lang_def *mnemonic = mnemonics;
		while (mnemonic->name)
		{
			lua_pushstring(L, mnemonic->name);
			lua_pushnumber(L, mnemonic->value);
			lua_settable(L, -3);

			lua_pushnumber(L, mnemonic->value);
			lua_pushstring(L, mnemonic->name);
			lua_settable(L, -3);
			
			mnemonic++;
		}

		lua_settable(L, LUA_REGISTRYINDEX);
	}
}

template<char *name, typename index_t>
void L_Enum<name, index_t>::PushMnemonicTable(lua_State *L)
{
	lua_pushlightuserdata(L, (void *) (&name[4]));
	lua_gettable(L, LUA_REGISTRYINDEX);
}

template<char *name, typename index_t>
bool L_Enum<name, index_t>::_lookup(lua_State *L, int index, index_t& to)
{
	if (L_Class<name, index_t>::Is(L, index))
	{
		to = L_Class<name, index_t>::Index(L, index);
		return true;
	}
	else if (lua_isnumber(L, index))
	{
		to = static_cast<index_t>(lua_tonumber(L, index));
		return L_Class<name, index_t>::Valid(to);
	}
	else if (lua_isstring(L, index))
	{
		// look for mnemonic
		PushMnemonicTable(L);

		if (lua_istable(L, -1))
		{
			lua_pushvalue(L, index);
			lua_gettable(L, -2);
			if (lua_isnumber(L, -1))
			{
				to = static_cast<index_t>(lua_tonumber(L, -1));
				lua_pop(L, 2);
				return (L_Class<name, index_t>::Valid(to));
			}
			else
			{
				lua_pop(L, 2);
				return false;
			}
		}
		else
		{
			lua_pop(L, 1);
			return false;
		}
	}
	else
	{
		return false;
	}
}

template<char *name, typename index_t>
index_t L_Enum<name, index_t>::ToIndex(lua_State *L, int index)
{
	index_t to;
	if (_lookup(L, index, to))
	{
		return to;
	}
	else
	{
		std::string error;
		if (lua_isnumber(L, index) || lua_isstring(L, index))
		{
			error = std::string(name) + ": invalid index";
		}
		else
		{
			error = std::string(name) + ": incorrect argument type";
		}
		return luaL_error(L, error.c_str());
	}
}

template<char *name, typename index_t>
int L_Enum<name, index_t>::_equals(lua_State *L)
{
	index_t a, b;
	lua_pushboolean(L, _lookup(L, 1, a) && _lookup(L, 2, b) && (a == b));
	return 1;
}

template<char *name, typename index_t>
int L_Enum<name, index_t>::_get_mnemonic(lua_State *L)
{
	PushMnemonicTable(L);
	lua_pushnumber(L, ToIndex(L, 1));
	lua_gettable(L, -2);
	if (lua_isstring(L, -1))
	{
		lua_remove(L, -2);
		return 1;
	}
	else
	{
		lua_pop(L, 2);
		return 0;
	}
}

template<char *name, typename index_t>
int L_Enum<name, index_t>::_set_mnemonic(lua_State *L)
{
	if (!lua_isstring(L, 2))
		return luaL_error(L, "mnemonic: incorrect argument type");

	index_t index = ToIndex(L, 1);
	const char *new_mnemonic = lua_tostring(L, 2);

	PushMnemonicTable(L);
	
	// look up the old mnemonic
	lua_pushnumber(L, index);
	lua_gettable(L, -2);
	if (lua_isstring(L, -1))
	{
		// if it exists, remove the string key
		lua_pushnil(L);
		lua_settable(L, -3);
		
	}
	else
	{
		lua_pop(L, 1);
	}
	
	// update string key
	lua_pushstring(L, new_mnemonic);
	lua_pushnumber(L, index);
	lua_settable(L, -3);

	// update index key
	lua_pushnumber(L, index);
	lua_pushstring(L, new_mnemonic);
	lua_settable(L, -3);

	lua_pop(L, 1);
	return 0;
}

template<char *name, class T>
class L_Container {
public:
	static void Register(lua_State *L, const luaL_reg methods[] = 0, const luaL_reg metatable[] = 0);
	static boost::function<typename T::index_type (void)> Length;
	struct ConstantLength
	{
		ConstantLength(int32 length) : m_length(length) {}
		int32 operator() (void) { return m_length; }
		int32 m_length;
	};
private:
	static int _get(lua_State *);
	static int _set(lua_State *);
	static int _call(lua_State *);
	static int _iterator(lua_State *);
	static int _length(lua_State *);
};

template<char *name, class T>
boost::function<typename T::index_type (void)> L_Container<name, T>::Length = ConstantLength(1);

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
	lua_pushstring(L, "__len");
	lua_pushcfunction(L, _length);
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
	return luaL_error(L, error.c_str());
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

template<char *name, class T>
int L_Container<name, T>::_length(lua_State *L)
{
	lua_pushnumber(L, Length());
	return 1;
}

// enum containers will be able to look up by strings
template<char *name, class T>
class L_EnumContainer : public L_Container<name, T>
{
public:
	static void Register(lua_State *L, const luaL_reg methods[] = 0, const luaL_reg metatable[] = 0);
private:
	static int _get(lua_State *);
};

template <char *name, class T>
void L_EnumContainer<name, T>::Register(lua_State *L, const luaL_reg methods[], const luaL_reg metatable[])
{
	L_Container<name, T>::Register(L, methods, metatable);
	
	luaL_getmetatable(L, name);
	lua_pushstring(L, "__index");
	lua_pushcfunction(L, _get);
	lua_settable(L, -3);
	lua_pop(L, 1);
}

template <char *name, class T>
int L_EnumContainer<name, T>::_get(lua_State *L)
{
	if (lua_isnumber(L, 2))
	{
		int32 index = static_cast<int32>(lua_tonumber(L, 2));
		if (!T::Valid(index))
		{
			lua_pushnil(L);
			return 1;
		}
		else
		{
			T::Push(L, index);
			return 1;
		}
	}
	else if (lua_isstring(L, 2))
	{
		// try mnemonics
		T::PushMnemonicTable(L);

		if (lua_istable(L, -1))
		{
			lua_pushvalue(L, 2);
			lua_gettable(L, -2);
			if (lua_isnumber(L, -1))
			{
				int32 index = static_cast<int32>(lua_tonumber(L, -1));
				lua_pop(L, 2);
				T::Push(L, index);
				return 1;
			}
			else
			{
				// should not happen
				lua_pop(L, 2);
			}
		} 
		else
		{
			lua_pop(L, 1);
		}

	}

	// get the function from methods
	lua_pushlightuserdata(L, (void *) (&name[1]));
	lua_gettable(L, LUA_REGISTRYINDEX);
	
	lua_pushvalue(L, 2);
	lua_gettable(L, -2);
	
	return 1;
}

#endif

#endif
