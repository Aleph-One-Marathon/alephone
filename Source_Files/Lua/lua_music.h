#ifndef __LUA_MUSIC_H
#define __LUA_MUSIC_H


#include "cseries.h"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "lua_templates.h"

extern char Lua_Music_Name[]; //music
typedef L_Class<Lua_Music_Name> Lua_Music;

extern char Lua_MusicManager_Name[]; //Music
typedef L_Container<Lua_MusicManager_Name, Lua_Music> Lua_MusicManager;

int Lua_Music_register(lua_State* L);

#endif
