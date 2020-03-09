/*
LUA_HUD_SCRIPT.CPP
 
    Copyright (C) 2009 by Jeremiah Morris and the Aleph One developers

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

    Implements Lua HUD state and trigger callbacks
*/

// cseries defines HAVE_LUA on A1/SDL
#include "cseries.h"

#include "mouse.h"
#include "interface.h"

#ifdef HAVE_LUA
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}
#endif

#include <string>
#include <stdlib.h>
#include <set>


#include "Logging.h"
#include "preferences.h"
#include "Plugins.h"

#include "lua_hud_script.h"
#include "lua_hud_objects.h"

#include <boost/shared_ptr.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
namespace io = boost::iostreams;


bool use_lua_hud_crosshairs;

#ifndef HAVE_LUA

void L_Call_HUDInit() {}
void L_Call_HUDCleanup() {}
void L_Call_HUDDraw() {}
void L_Call_HUDResize() {}

#else /* HAVE_LUA */

// LP: used by several functions here
extern float AngleConvert;

// Steal all this stuff
extern bool MotionSensorActive;

extern struct view_data *world_view;
extern struct static_data *static_world;

static const luaL_Reg lualibs[] = {
{"", luaopen_base},
{LUA_TABLIBNAME, luaopen_table},
{LUA_STRLIBNAME, luaopen_string},
{LUA_BITLIBNAME, luaopen_bit32},
{LUA_MATHLIBNAME, luaopen_math},
{LUA_DBLIBNAME, luaopen_debug},
{LUA_IOLIBNAME, luaopen_io},
{NULL, NULL}
};

void* L_Persistent_Table_Key();


class LuaHUDState
{
public:
	LuaHUDState() : running_(false), inited_(false), num_scripts_(0) {
		state_.reset(luaL_newstate(), lua_close);
	}

	virtual ~LuaHUDState() {
	}

public:

	bool Load(const char *buffer, size_t len);
	bool Loaded() { return num_scripts_ > 0; }
	bool Running() { return running_; }
	bool Run();
	void Stop() { running_ = false; }
	void MarkCollections(std::set<short>& collections);

	virtual void Initialize() {
		const luaL_Reg *lib = lualibs;
		for (; lib->func; lib++)
		{
			luaL_requiref(State(), lib->name, lib->func, 1);
			lua_pop(State(), 1);
		}
		
		RegisterFunctions();
	}

	void SetSearchPath(const std::string& path) {
		L_Set_Search_Path(State(), path);
	}

protected:
	bool GetTrigger(const char *trigger);
	void CallTrigger(int numArgs = 0);

	virtual void RegisterFunctions();

	boost::shared_ptr<lua_State> state_;
	lua_State* State() { return state_.get(); }

public:
	// triggers
	void Init();
	void Draw();
	void Resize();
	void Cleanup();

private:
	bool running_;
	int num_scripts_;
    bool inited_;
};

LuaHUDState *hud_state = NULL;

bool LuaHUDState::GetTrigger(const char* trigger)
{
	if (!running_)
		return false;

	lua_getglobal(State(), "Triggers");
	if (!lua_istable(State(), -1))
	{
		lua_pop(State(), 1);
		return false;
	}

	lua_pushstring(State(), trigger);
	lua_gettable(State(), -2);
	if (!lua_isfunction(State(), -1))
	{
		lua_pop(State(), 2);
		return false;
	}

	lua_remove(State(), -2);
	return true;
}

void LuaHUDState::CallTrigger(int numArgs)
{
	if (lua_pcall(State(), numArgs, 0, 0) == LUA_ERRRUN)
		L_Error(lua_tostring(State(), -1));
}

void LuaHUDState::Init()
{
	if (GetTrigger("init"))
		CallTrigger();
    inited_ = true;
}

void LuaHUDState::Draw()
{
    if (!inited_)
        return;
	if (GetTrigger("draw"))
		CallTrigger();
}

void LuaHUDState::Resize()
{
    if (!inited_)
        return;
	if (GetTrigger("resize"))
		CallTrigger();
}

void LuaHUDState::Cleanup()
{
    if (!inited_)
        return;
	if (GetTrigger("cleanup"))
		CallTrigger();
    inited_ = false;
}

void LuaHUDState::RegisterFunctions()
{
	Lua_HUDObjects_register(State());
}

bool LuaHUDState::Load(const char *buffer, size_t len)
{
	int status = luaL_loadbufferx(State(), buffer, len, "HUD Lua", "t");
	if (status == LUA_ERRRUN)
		logWarning("Lua loading failed: error running script.");
	if (status == LUA_ERRFILE)
		logWarning("Lua loading failed: error loading file.");
	if (status == LUA_ERRSYNTAX) {
		logWarning("Lua loading failed: syntax error.");
		logWarning(lua_tostring(State(), -1));
	}
	if (status == LUA_ERRMEM)
		logWarning("Lua loading failed: error allocating memory.");
	if (status == LUA_ERRERR)
		logWarning("Lua loading failed: unknown error.");

	num_scripts_ += ((status == 0) ? 1 : 0);
	return (status == 0);
}


bool LuaHUDState::Run()
{
	if (!Loaded()) return false;

	int result = 0;
	// Reverse the functions we're calling
	for (int i = 0; i < num_scripts_ - 1; ++i)
		lua_insert(State(), -(num_scripts_ - i));

	// Call 'em
	for (int i = 0; i < num_scripts_; ++i)
		result = result || lua_pcall(State(), 0, LUA_MULTRET, 0);
	
	if (result == 0) running_ = true;
	return (result == 0);
}

void LuaHUDState::MarkCollections(std::set<short>& collections)
{
	if (!running_)
		return;
    
	lua_getglobal(State(), "CollectionsUsed");
	
	if (lua_istable(State(), -1))
	{
		int i = 1;
		lua_pushnumber(State(), i++);
		lua_gettable(State(), -2);
		while (lua_isnumber(State(), -1))
		{
			short collection_index = static_cast<short>(lua_tonumber(State(), -1));
			if (collection_index >= 0 && collection_index < NUMBER_OF_COLLECTIONS)
			{
				mark_collection_for_loading(collection_index);
				collections.insert(collection_index);
			}
			lua_pop(State(), 1);
			lua_pushnumber(State(), i++);
			lua_gettable(State(), -2);
		}
        
		lua_pop(State(), 2);
	}
	else if (lua_isnumber(State(), -1))
	{
		short collection_index = static_cast<short>(lua_tonumber(State(), -1));
		if (collection_index >= 0 && collection_index < NUMBER_OF_COLLECTIONS)
		{
			mark_collection_for_loading(collection_index);
			collections.insert(collection_index);
		}
        
		lua_pop(State(), 1);
	}
	else
	{
		lua_pop(State(), 1);
	}
}


bool LuaHUDRunning()
{
	return (hud_state && hud_state->Running());
}

void L_Call_HUDInit()
{
	if (hud_state)
		hud_state->Init();
}

void L_Call_HUDCleanup()
{
	if (hud_state)
		hud_state->Cleanup();
}

void L_Call_HUDDraw()
{
	if (hud_state)
		hud_state->Draw();
}

void L_Call_HUDResize()
{
	if (hud_state)
		hud_state->Resize();
}


bool LoadLuaHUDScript(const char *buffer, size_t len)
{
	if (!hud_state)
	{
		hud_state = new LuaHUDState();
		hud_state->Initialize();
	}
	return hud_state->Load(buffer, len);
}

void SetLuaHUDScriptSearchPath(const std::string& directory)
{
	hud_state->SetSearchPath(directory);
}

bool RunLuaHUDScript()
{
	use_lua_hud_crosshairs = false;
	return (hud_state && hud_state->Run());
}

void LoadHUDLua()
{
	std::string file;
	std::string directory;

	const Plugin* hud_lua_plugin = Plugins::instance()->find_hud_lua();
	if (hud_lua_plugin)
	{
		file = hud_lua_plugin->hud_lua;
		directory = hud_lua_plugin->directory.GetPath();
	}

	if (file.size())
	{
		FileSpecifier fs (file.c_str());
		if (directory.size())
		{
			fs.SetNameWithPath(file.c_str(), directory);
		}

		OpenedFile script_file;
		if (fs.Open(script_file))
		{
			int32 script_length;
			script_file.GetLength(script_length);

			std::vector<char> script_buffer(script_length);
			if (script_file.Read(script_length, &script_buffer[0]))
			{
				LoadLuaHUDScript(&script_buffer[0], script_length);
				if (directory.size()) 
				{
					SetLuaHUDScriptSearchPath(directory);
				}
			}
		}
	}
}

void CloseLuaHUDScript()
{
	delete hud_state;
	hud_state = NULL;
}

void MarkLuaHUDCollections(bool loading)
{
	static std::set<short> collections;
	if (loading)
	{
		collections.clear();
        if (hud_state)
            hud_state->MarkCollections(collections);
	}
	else
	{
		for (std::set<short>::iterator it = collections.begin(); it != collections.end(); it++)
		{
			mark_collection_for_unloading(*it);
		}
	}
}



#endif /* HAVE_LUA */
