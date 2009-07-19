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
using namespace std;
#include <stdlib.h>
#include <set>


#include "screen.h"
#include "tags.h"
#include "player.h"
#include "render.h"
#include "shell.h"
#include "Logging.h"
#include "lightsource.h"
#include "game_window.h"
#include "items.h"
#include "platforms.h"
#include "media.h"
#include "weapons.h"
#include "monsters.h"
#include "flood_map.h"
#include "vbl.h"
#include "fades.h"
#include "physics_models.h"
#include "Crosshairs.h"
#include "OGL_Setup.h"
#include "SoundManager.h"
#include "world.h"
#include "computer_interface.h"
#include "network.h"
#include "network_games.h"
#include "Random.h"
#include "Console.h"
#include "Music.h"
#include "ViewControl.h"
#include "preferences.h"
#include "BStream.h"

#include "lua_script.h"
#include "lua_map.h"
#include "lua_monsters.h"
#include "lua_objects.h"
#include "lua_player.h"
#include "lua_projectiles.h"
#include "lua_serialize.h"
#include "lua_hud_objects.h"

#include <boost/shared_ptr.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
namespace io = boost::iostreams;

#define DONT_REPEAT_DEFINITIONS
#include "item_definitions.h"
#include "monster_definitions.h"


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
//  {LUA_LOADLIBNAME, luaopen_package},
{LUA_TABLIBNAME, luaopen_table},
{LUA_IOLIBNAME, luaopen_io},
{LUA_OSLIBNAME, luaopen_os},
{LUA_STRLIBNAME, luaopen_string},
{LUA_MATHLIBNAME, luaopen_math},
{LUA_DBLIBNAME, luaopen_debug},
{NULL, NULL}
};

void* L_Persistent_Table_Key();


class LuaHUDState
{
public:
	LuaHUDState() : running_(false), num_scripts_(0) {
		state_.reset(lua_open(), lua_close);
	}

	virtual ~LuaHUDState() {
	}

public:

	bool Load(const char *buffer, size_t len);
	bool Loaded() { return num_scripts_ > 0; }
	bool Running() { return running_; }
	bool Run();
	void Stop() { running_ = false; }

	virtual void Initialize() {
		const luaL_Reg *lib = lualibs;
		for (; lib->func; lib++)
		{
			lua_pushcfunction(State(), lib->func);
			lua_pushstring(State(), lib->name);
			lua_call(State(), 1, 0);
		}


		lua_pushcfunction(State(), luaopen_io);
		lua_pushstring(State(), LUA_IOLIBNAME);
		lua_call(State(), 1, 0);
		
		RegisterFunctions();
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
};

LuaHUDState *hud_state = NULL;

bool LuaHUDState::GetTrigger(const char* trigger)
{
	if (!running_)
		return false;

	lua_pushstring(State(), "Triggers");
	lua_gettable(State(), LUA_GLOBALSINDEX);
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
}

void LuaHUDState::Draw()
{
	if (GetTrigger("draw"))
		CallTrigger();
}

void LuaHUDState::Resize()
{
	if (GetTrigger("resize"))
		CallTrigger();
}

void LuaHUDState::Cleanup()
{
	if (GetTrigger("cleanup"))
		CallTrigger();
}

void LuaHUDState::RegisterFunctions()
{
	Lua_HUDObjects_register(State());
}

bool LuaHUDState::Load(const char *buffer, size_t len)
{
	int status = luaL_loadbuffer(State(), buffer, len, "hud_script");
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


static bool LuaHUDRunning()
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


bool RunLuaHUDScript()
{
	if (hud_state)
		return hud_state->Run();
}

void LoadHUDLua()
{
	if (environment_preferences->use_hud_lua)
	{
		FileSpecifier fs (environment_preferences->hud_lua_file);
		OpenedFile script_file;
		if (fs.Open(script_file))
		{
			long script_length;
			script_file.GetLength(script_length);

			std::vector<char> script_buffer(script_length);
			if (script_file.Read(script_length, &script_buffer[0]))
			{
				LoadLuaHUDScript(&script_buffer[0], script_length);
			}
		}
	}
}

void CloseLuaHUDScript()
{
	delete hud_state;
	hud_state = NULL;
}


#endif /* HAVE_LUA */
