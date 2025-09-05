#include "lua_music.h"
#include "Music.h"

static int Lua_MusicManager_Clear(lua_State* L)
{
	Music::instance()->ClearLevelPlaylist();
	return 0;
}

static int Lua_MusicManager_Fade(lua_State* L)
{
	int duration = lua_isnumber(L, 1) ? static_cast<int>(lua_tonumber(L, 1) * 1000) : 1000;
	Music::instance()->Fade(0, duration, MusicPlayer::FadeType::Linear);
	Music::instance()->ClearLevelPlaylist();
	return 0;
}

static int Lua_Music_Fade(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "volume: incorrect argument type");

	float limitVolume = static_cast<float>(lua_tonumber(L, 2));
	int duration = lua_isnumber(L, 3) ? static_cast<int>(lua_tonumber(L, 3) * 1000) : 1000;
	bool stopOnNoVolume = lua_isboolean(L, 4) ? static_cast<bool>(lua_toboolean(L, 4)) : true;

	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return luaL_error(L, "index: out of bounds");

	slot->Fade(limitVolume, duration, MusicPlayer::FadeType::Linear, stopOnNoVolume);
	return 0;
}

static int Lua_Music_Play(lua_State* L)
{
	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return luaL_error(L, "index: out of bounds");

	slot->Play();
	return 0;
}

static int Lua_Music_Stop(lua_State* L)
{
	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return luaL_error(L, "index: out of bounds");

	slot->Pause();
	return 0;
}

static int Lua_Music_Active_Get(lua_State* L)
{
	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return luaL_error(L, "index: out of bounds");

	lua_pushboolean(L, slot->Playing());
	return 1;
}

static int Lua_Music_Volume_Get(lua_State* L)
{
	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return luaL_error(L, "index: out of bounds");

	lua_pushnumber(L, static_cast<double>(slot->GetParameters().volume));
	return 1;
}

static int Lua_Music_Volume_Set(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "volume: incorrect argument type");

	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return luaL_error(L, "index: out of bounds");

	slot->SetVolume(static_cast<float>(lua_tonumber(L, 2)));
	return 0;
}

static bool Lua_Music_Valid(int16 index)
{
	return index >= 0 && Music::instance()->GetSlot(index + Music::reserved_music_slots);
}

static int Lua_MusicManager_New(lua_State* L)
{
	if (!lua_isstring(L, 1))
		return luaL_error(L, "track: invalid file specifier");

	float volume = lua_isnumber(L, 2) ? static_cast<float>(lua_tonumber(L, 2)) : 1.f;
	bool loop = lua_isboolean(L, 3) ? static_cast<bool>(lua_toboolean(L, 3)) : true;

	std::string search_path = L_Get_Search_Path(L);

	FileSpecifier file;
	if (search_path.size())
	{
		if (!file.SetNameWithPath(lua_tostring(L, 1), search_path)) 
			return luaL_error(L, "track: file not found");
	}
	else
	{
		if (!file.SetNameWithPath(lua_tostring(L, 1))) 
			return luaL_error(L, "track: file not found");
	}

	auto id = Music::instance()->Add({ volume, loop }, &file);
	if (!id.has_value() || id.value() < Music::reserved_music_slots) 
		return luaL_error(L, "track: error loading file");

	Lua_Music::Push(L, id.value() - Music::reserved_music_slots);
	return 1;
}

static int Lua_MusicManager_Play(lua_State* L)
{
	for (int n = 1; n <= lua_gettop(L); n++)
	{
		if (!lua_isstring(L, n))
			return luaL_error(L, "play: invalid file specifier");

		std::string search_path = L_Get_Search_Path(L);

		FileSpecifier file;
		if (search_path.size())
		{
			if (file.SetNameWithPath(lua_tostring(L, n), search_path))
				Music::instance()->PushBackLevelMusic(file);
		}
		else
		{
			if (file.SetNameWithPath(lua_tostring(L, n)))
				Music::instance()->PushBackLevelMusic(file);
		}
	}

	return 0;
}

static int Lua_MusicManager_Stop(lua_State* L)
{
	Music::instance()->ClearLevelPlaylist();
	Music::instance()->StopLevelMusic();

	return 0;
}

static int Lua_MusicManager_Valid(lua_State* L) {
	int top = lua_gettop(L);
	for (int n = 1; n <= top; n++) {
		if (!lua_isstring(L, n))
			return luaL_error(L, "valid: invalid file specifier");
		FileSpecifier path;

		bool found;
		auto search_path = L_Get_Search_Path(L);
		if (search_path.size())
		{
			found = path.SetNameWithPath(lua_tostring(L, n), search_path);
		}
		else
		{
			found = path.SetNameWithPath(lua_tostring(L, n));
		}

		lua_pushboolean(L, found && StreamDecoder::Get(path));
	}
	return top;
}

const luaL_Reg Lua_MusicManager_Methods[] = {
	{"new", L_TableFunction<Lua_MusicManager_New>},
	{"clear", L_TableFunction<Lua_MusicManager_Clear>},
	{"fade", L_TableFunction<Lua_MusicManager_Fade>},
	{"play", L_TableFunction<Lua_MusicManager_Play>},
	{"stop", L_TableFunction<Lua_MusicManager_Stop>},
	{"valid", L_TableFunction<Lua_MusicManager_Valid>},
	{0, 0}
};

const luaL_Reg Lua_Music_Get[] = {
	{"fade", L_TableFunction<Lua_Music_Fade>},
	{"volume", Lua_Music_Volume_Get},
	{"active", Lua_Music_Active_Get},
	{"play", L_TableFunction<Lua_Music_Play>},
	{"stop", L_TableFunction<Lua_Music_Stop>},
	{0, 0}
};

const luaL_Reg Lua_Music_Set[] = {
	{"volume", Lua_Music_Volume_Set},
	{0, 0}
};

char Lua_Music_Name[] = "music";
char Lua_MusicManager_Name[] = "Music";

int Lua_Music_register(lua_State* L)
{
	Lua_Music::Register(L, Lua_Music_Get, Lua_Music_Set);
	Lua_Music::Valid = Lua_Music_Valid;

	Lua_MusicManager::Register(L, Lua_MusicManager_Methods);
	Lua_MusicManager::Length = Lua_MusicManager::ConstantLength(16); //whatever
	return 0;
}

