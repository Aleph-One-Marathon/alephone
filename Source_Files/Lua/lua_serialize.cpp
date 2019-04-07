/*
LUA_SERIALIZE.CPP

	Copyright (C) 2009 by Gregory Smith
 
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

	Serializes Lua objects
	Based on Pluto, but far less clever
*/

#include "lua_serialize.h"
#include "Logging.h"

#include "BStream.h"

const static int SAVED_REFERENCE_PSEUDOTYPE = -2;
const uint16 kVersion = 1;

static bool valid_key(int type)
{
	return (type == LUA_TNUMBER ||
		type == LUA_TBOOLEAN ||
		type == LUA_TSTRING ||
		type == LUA_TTABLE ||
		type == LUA_TUSERDATA);
}

static void save(lua_State *L, BOStreamBE& s, uint32& counter)
{
	// if the object has already been written, write a reference to it

	lua_pushvalue(L, -1);
	lua_rawget(L, 1);
	if (!lua_isnil(L, -1))
	{
		s << static_cast<int8>(SAVED_REFERENCE_PSEUDOTYPE)
		  << static_cast<uint32>(lua_tonumber(L, -1));
		lua_pop(L, 1);
		return;
	}
	lua_pop(L, 1);

	s << static_cast<int8>(lua_type(L, -1));
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
				s << static_cast<uint32>(lua_rawlen(L, -1));
				s.write(lua_tostring(L, -1), lua_rawlen(L, -1));
			}
			break;
		case LUA_TTABLE:
			{
				// add to the reference table
				lua_pushvalue(L, -1);
				lua_pushnumber(L, static_cast<lua_Number>(++counter));
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
		case LUA_TUSERDATA:
			{
				// add to the reference table
				lua_pushvalue(L, -1);
				lua_pushnumber(L, static_cast<lua_Number>(++counter));
				lua_rawset(L, 1);

				// write the reference
				s << counter;

				// assume that this is one of our userdata
				lua_getmetatable(L, -1);
				lua_gettable(L, LUA_REGISTRYINDEX);

				s << static_cast<uint8>(lua_rawlen(L, -1));
				s.write(lua_tostring(L, -1), lua_rawlen(L, -1));
				lua_pop(L, 1);

				lua_getfield(L, -1, "index");
				
				s << static_cast<uint32>(lua_tonumber(L, -1));
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
		s << kVersion;
		save(L, s, counter);
	}
	catch (const basic_bstream::failure& e)
	{
		logWarning("failed to save Lua data; %s", e.what());
		lua_settop(L, 0);
		return false;
	}

	// remove the reference table
	lua_remove(L, 1);
	return true;
}

static int restore(lua_State *L, BIStreamBE& s)
{
	int8 type;
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
				uint32 reference;
				s >> reference;

				// add to the reference table
				lua_newtable(L);
				lua_pushnumber(L, static_cast<lua_Number>(reference));
				lua_pushvalue(L, -2);
				lua_rawset(L, 1);

				int key_type = restore(L, s);
				while (key_type != LUA_TNIL)
				{
					restore(L, s); // value
					if (lua_isnil(L, -2)) 
					{
						// maybe an invalid userdata?
						lua_pop(L, 2);
					} 
					else
					{
						lua_rawset(L, -3);
					}
					key_type = restore(L, s); // next key
				}
				lua_pop(L, 1);
			}
			break;
		case LUA_TUSERDATA:
			{
				uint32 reference;
				s >> reference;
				
				uint8 length;
				s >> length;
				std::vector<char> v(length);
				s.read(&v[0], v.size());
				lua_pushlstring(L, &v[0], v.size());

				uint32 index;
				s >> index;
				
				// get the metatable
				lua_gettable(L, LUA_REGISTRYINDEX);
				// get the accessor we added
				lua_getfield(L, -1, "__new");
				if (lua_isfunction(L, -1))
				{
					lua_pushnumber(L, static_cast<lua_Number>(index));
					lua_call(L, 1, 1);
				}

				lua_remove(L, -2);
				
				// add to the reference table
				lua_pushnumber(L, static_cast<lua_Number>(reference));
				lua_pushvalue(L, -2);
				lua_rawset(L, 1);
				
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

	return type;
}

bool lua_restore(lua_State *L, std::streambuf* sb)
{
	// create a reference table
	lua_newtable(L);

        // put it at the bottom of the stack
        lua_insert(L, 1);

	BIStreamBE s(sb);
	try {
		int16 version;
		s >> version;
		if (version > kVersion)
		{
			logWarning("failed to restore Lua data; saved data is newer version");
			return false;
		}

		restore(L, s);
	}
	catch (const basic_bstream::failure& e)
	{
		logWarning("failed to restore Lua data; %s", e.what());
		lua_settop(L, 0);
		return false;
	}
	
	// remove the reference table
	lua_remove(L, 1);
	return true;
}
