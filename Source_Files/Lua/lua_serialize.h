#ifndef __LUA_SERIALIZE_H
#define __LUA_SERIALIZE_H

/*
LUA_SERIALIZE.H

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

	Serializes Lua objects
*/

#include "cseries.h"

#include <streambuf>

#ifdef HAVE_LUA
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

// saves object on top of the stack to s
bool lua_save(lua_State *L, std::streambuf* sb);

// restores object in s to top of the stack
bool lua_restore(lua_State *L, std::streambuf* sb);

#endif

#endif
