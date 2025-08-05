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

extern char Lua_MusicPreset_Name[];
typedef L_Class<Lua_MusicPreset_Name> Lua_MusicPreset;

extern char Lua_MusicPresets_Name[];
typedef L_Container<Lua_MusicPresets_Name, Lua_MusicPreset> Lua_MusicPresets;

extern char Lua_MusicSegment_Name[];
typedef L_Class<Lua_MusicSegment_Name> Lua_MusicSegment;

extern char Lua_MusicSegments_Name[];
typedef L_Container<Lua_MusicSegments_Name, Lua_MusicSegment> Lua_MusicSegments;

extern char Lua_MusicManager_Name[]; //Music
typedef L_Container<Lua_MusicManager_Name, Lua_Music> Lua_MusicManager;

extern char Lua_MusicFadeType_Name[];
typedef L_Enum<Lua_MusicFadeType_Name> Lua_MusicFadeType;

extern char Lua_MusicFadeTypes_Name[];
typedef L_EnumContainer<Lua_MusicFadeTypes_Name, Lua_MusicFadeType> Lua_MusicFadeTypes;

int Lua_Music_register(lua_State* L);
void Reset_Music_InternalIndexes();

#endif
