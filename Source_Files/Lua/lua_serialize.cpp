/*
LUA_SERIALIZE.CPP

	Copyright (C) 2009 by Gregory Smith
 
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

	Serializes Lua objects
	Based on Pluto, but far less clever
*/

#include "lua_serialize.h"

#include "BStream.h"

const static int16 SAVED_REFERENCE_PSEUDOTYPE = -2;

static bool valid_key(int type)
{
	return (type == LUA_TNUMBER ||
		type == LUA_TBOOLEAN ||
		type == LUA_TSTRING ||
		type == LUA_TTABLE);
}

static void save(lua_State *L, BOStreamBE& s, uint32& counter)
{
	// if the object has already been written, write a reference to it

	lua_pushvalue(L, -1);
	lua_rawget(L, 1);
	if (!lua_isnil(L, -1))
	{
		s << SAVED_REFERENCE_PSEUDOTYPE
		  << static_cast<uint32>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		return;
	}
	lua_pop(L, 1);

	s << static_cast<int16>(lua_type(L, -1));
	switch (lua_type(L, -1))
	{
		case LUA_TNIL:
			break;
		case LUA_TNUMBER:
			{
				s << static_cast<double>(lua_tonumber(L, -1));
			}
			break;
		case LUA_TBOOLEAN:
			s << static_cast<uint8>(lua_toboolean(L, -1) ? 1 : 0);
			break;
		case LUA_TSTRING: 
			{
				s << static_cast<uint32>(lua_strlen(L, -1));
				s.write(lua_tostring(L, -1), lua_strlen(L, -1));
			}
			break;
		case LUA_TTABLE:
			{
				// add to the reference table
				lua_pushvalue(L, -1);
				lua_pushnumber(L, static_cast<double>(++counter));
				lua_rawset(L, 1);

				// write the reference
				s << counter;

				// write all k/v pairs
				lua_pushnil(L);
				while (lua_next(L, -2)) 
				{
					if (valid_key(lua_type(L, -2))) {
						// another key
						lua_pushvalue(L, -2);
						
						save(L, s, counter);
						lua_pop(L, 1);
						
						save(L, s, counter);
						lua_pop(L, 1);
					} else {
						lua_pop(L, 1);
					}
				}

				lua_pushnil(L);
				save(L, s, counter);
				lua_pop(L, 1);
			}
			break;
		default:
			// we silently ignore other types
			break;
	}
}

bool lua_save(lua_State *L, std::streambuf* sb)
{
	lua_assert(lua_gettop(L) == 1);

	// create a references table
	lua_newtable(L);

	// put it at the bottom of the stack
	lua_insert(L, 1);
	
	uint32 counter = 0;
	BOStreamBE s(sb);
	try 
	{
		save(L, s, counter);
	}
	catch (const basic_bstream::failure& e)
	{
		lua_settop(L, 0);
		return false;
	}

	// remove the reference table
	lua_remove(L, 1);
	return true;
}

static void restore(lua_State *L, BIStreamBE& s)
{
	int16 type;
	s >> type;

	switch (type) 
	{
		case LUA_TNIL:
			lua_pushnil(L);
			break;
		case LUA_TBOOLEAN:
			uint8 b;
			s >> b;			
			lua_pushboolean(L, b == 1);
			break;
		case LUA_TNUMBER:
			{
				double d;
				s >> d;
				lua_pushnumber(L, static_cast<lua_Number>(d));
			}
			break;
		case LUA_TSTRING:
			{
				uint32 length;
				s >> length;
				std::vector<char> v(length);
				s.read(&v[0], v.size());
				lua_pushlstring(L, &v[0], v.size());
			}
			break;
		case LUA_TTABLE:
			{
				uint32 index;
				s >> index;

				// add to the reference table
				lua_newtable(L);
				lua_pushnumber(L, static_cast<double>(index));
				lua_pushvalue(L, -2);
				lua_rawset(L, 1);

				restore(L, s);
				while (!lua_isnil(L, -1))
				{
					restore(L, s); // value
					lua_rawset(L, -3);
					restore(L, s); // next key
				}
				lua_pop(L, 1);

			}
			break;
		case SAVED_REFERENCE_PSEUDOTYPE:
			{
				uint32 index;
				s >> index;
				lua_pushnumber(L, static_cast<lua_Number>(index));
				lua_rawget(L, 1);
			}
			break;
		default:
			lua_pushnil(L);
			break;
	}
}

bool lua_restore(lua_State *L, std::streambuf* sb)
{
	// create a reference table
	lua_newtable(L);

	BIStreamBE s(sb);
	try {
		restore(L, s);
	}
	catch (const basic_bstream::failure& e)
	{
		lua_settop(L, 0);
		return false;
	}
	
	// remove the reference table
	lua_remove(L, -2);
	return true;
}
