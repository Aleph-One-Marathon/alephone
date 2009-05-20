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

#include <SDL/SDL_endian.h>

const static int SAVED_REFERENCE_PSEUDOTYPE = -2;

static bool valid_key(int type)
{
	return (type == LUA_TNUMBER ||
		type == LUA_TBOOLEAN ||
		type == LUA_TSTRING ||
		type == LUA_TTABLE);
}

static void save(lua_State *L, SDL_RWops *rw, uint32& counter)
{
	// if the object has already been written, write a reference to it

	lua_pushvalue(L, -1);
	lua_rawget(L, 1);
	if (!lua_isnil(L, -1))
	{
		uint32 ref = static_cast<uint32>(lua_tonumber(L, -1));
		SDL_WriteBE16(rw, SAVED_REFERENCE_PSEUDOTYPE);
		SDL_WriteBE32(rw, ref);
		lua_pop(L, 1);
		return;
	}
	lua_pop(L, 1);

	SDL_WriteBE16(rw, lua_type(L, -1));
	switch (lua_type(L, -1))
	{
		case LUA_TNIL:
			break;
		case LUA_TNUMBER:
			{
				double d = lua_tonumber(L, -1);
				SDL_WriteBE64(rw, *reinterpret_cast<Uint64*>(&d));
			}
			break;
		case LUA_TBOOLEAN:
			SDL_WriteBE16(rw, lua_toboolean(L, -1) ? 1 : 0);
			break;
		case LUA_TSTRING: 
			{
				SDL_WriteBE32(rw, static_cast<uint32>(lua_strlen(L, -1)));
				SDL_RWwrite(rw, lua_tostring(L, -1), lua_strlen(L, -1), 1);
			}
			break;
		case LUA_TTABLE:
			{
				// add to the reference table
				lua_pushvalue(L, -1);
				lua_pushnumber(L, static_cast<double>(++counter));
				lua_rawset(L, 1);

				// write the reference
				SDL_WriteBE32(rw, counter);

				// write all k/v pairs
				lua_pushnil(L);
				while (lua_next(L, -2)) 
				{
					if (valid_key(lua_type(L, -2))) {
						// another key
						lua_pushvalue(L, -2);
						
						save(L, rw, counter);
						lua_pop(L, 1);
						
						save(L, rw, counter);
						lua_pop(L, 1);
					} else {
						lua_pop(L, 1);
					}
				}

				lua_pushnil(L);
				save(L, rw, counter);
				lua_pop(L, 1);
			}
			break;
		default:
			// we silently ignore other types
			break;
	}
}

void lua_save(lua_State *L, SDL_RWops *rw)
{
	lua_assert(lua_gettop(L) == 1);

	// create a references table
	lua_newtable(L);

	// put it at the bottom of the stack
	lua_insert(L, 1);
	
	uint32 counter = 0;
	save(L, rw, counter);

	// remove the reference table
	lua_remove(L, 1);
}

static void restore(lua_State *L, SDL_RWops *rw)
{
	int type = static_cast<int16>(SDL_ReadBE16(rw));

	switch (type) 
	{
		case LUA_TNIL:
			lua_pushnil(L);
			break;
		case LUA_TBOOLEAN:
			lua_pushboolean(L, SDL_ReadBE16(rw) == 1);
			break;
		case LUA_TNUMBER:
			{
				Uint64 i = SDL_ReadBE64(rw);
				lua_pushnumber(L, *reinterpret_cast<double*>(&i));
			}
			break;
		case LUA_TSTRING:
			{
				uint32 length = SDL_ReadBE32(rw);
				std::vector<char> buffer(length);
				SDL_RWread(rw, &buffer[0], length, 1);
				lua_pushlstring(L, &buffer[0], length);
			}
			break;
		case LUA_TTABLE:
			{
				uint32 index = SDL_ReadBE32(rw);
				
				// add to the reference table
				lua_newtable(L);
				lua_pushnumber(L, static_cast<double>(index));
				lua_pushvalue(L, -2);
				lua_rawset(L, 1);

				restore(L, rw);
				while (!lua_isnil(L, -1))
				{
					restore(L, rw); // value
					lua_rawset(L, -3);
					restore(L, rw); // next key
				}
				lua_pop(L, 1);

			}
			break;
		case SAVED_REFERENCE_PSEUDOTYPE:
			{
				uint32 index = SDL_ReadBE32(rw);
				lua_pushnumber(L, static_cast<double>(index));
				lua_rawget(L, 1);
			}
			break;
		default:
			lua_pushnil(L);
			break;
	}
}

void lua_restore(lua_State *L, SDL_RWops *rw)
{
	// create a reference table
	lua_newtable(L);

	restore(L, rw);
	
	// remove the reference table
	lua_remove(L, -2);
	
}
