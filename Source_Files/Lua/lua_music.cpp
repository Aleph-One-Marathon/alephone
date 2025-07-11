#include "lua_music.h"
#include "Music.h"

static int Lua_MusicManager_Clear(lua_State* L)
{
	Music::instance()->ClearLevelMusic();
	return 0;
}

static int Lua_MusicManager_Fade(lua_State* L)
{
	int duration = lua_isnumber(L, 1) ? static_cast<int>(lua_tonumber(L, 1) * 1000) : 1000;
	Music::instance()->Fade(0, duration);
	Music::instance()->ClearLevelMusic();
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
	Music::instance()->Fade(limitVolume, duration, stopOnNoVolume, index);
	return 0;
}

static int Lua_Music_Play(lua_State* L)
{
	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return 0;

	auto preset_index = lua_isnumber(L, 2) ? static_cast<uint32_t>(lua_tonumber(L, 2)) : 0;
	auto segment_index = lua_isnumber(L, 3) ? static_cast<uint32_t>(lua_tonumber(L, 3)) : 0;

	if (!slot->IsSegmentIndexValid(preset_index, segment_index))
		return luaL_error(L, "preset_index or segment_index: out of bounds value");

	slot->Play(preset_index, segment_index);
	return 0;
}

static int Lua_Music_Stop(lua_State* L)
{
	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	Music::instance()->Pause(index);
	return 0;
}

static int Lua_Music_Active_Get(lua_State* L)
{
	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	lua_pushboolean(L, Music::instance()->Playing(index));
	return 1;
}

static int Lua_Music_Volume_Get(lua_State* L)
{
	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return 0;

	lua_pushnumber(L, static_cast<double>(slot->GetVolume()));
	return 1;
}

static int Lua_Music_Volume_Set(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "volume: incorrect argument type");

	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return 0;

	slot->SetVolume(static_cast<float>(lua_tonumber(L, 2)));
	return 0;
}

static bool Lua_Music_Valid(int16 index)
{
	return Music::instance()->GetSlot(index + Music::reserved_music_slots);
}

static int Lua_MusicManager_New(lua_State* L)
{
	bool load_with_file = lua_type(L, 1) == LUA_TSTRING;
	int volume_index = load_with_file ? 2 : 1;
	int loop_index = load_with_file ? 3 : 2;
	float volume = lua_isnumber(L, volume_index) ? static_cast<float>(lua_tonumber(L, volume_index)) : 1;
	bool loop = lua_isboolean(L, loop_index) ? static_cast<bool>(lua_toboolean(L, loop_index)) : true;
	FileSpecifier* music_file = nullptr;

	if (load_with_file)
	{
		std::string search_path = L_Get_Search_Path(L);

		FileSpecifier file;
		if (search_path.size())
		{
			if (!file.SetNameWithPath(lua_tostring(L, 1), search_path)) return 0;
		}
		else
		{
			if (!file.SetNameWithPath(lua_tostring(L, 1))) return 0;
		}

		music_file = &file;
	}

	auto id = Music::instance()->Add({ volume, loop }, music_file);
	if (!id.has_value() || id.value() < Music::reserved_music_slots) return 0;

	Lua_Music::Push(L, id.value() - Music::reserved_music_slots);
	return 1;
}

static int Lua_DynamicMusic_Load_Track(lua_State* L)
{
	if (!lua_isstring(L, 2))
		return luaL_error(L, "play: invalid file specifier");

	std::string search_path = L_Get_Search_Path(L);

	FileSpecifier file;
	if (search_path.size())
	{
		if (!file.SetNameWithPath(lua_tostring(L, 2), search_path)) return 0;
	}
	else
	{
		if (!file.SetNameWithPath(lua_tostring(L, 2))) return 0;
	}

	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return 0;

	auto segment_index = slot->LoadTrack(&file);
	if (!segment_index.has_value()) return 0;

	lua_pushnumber(L, segment_index.value());
	return 1;
}

static int Lua_DynamicMusic_Add_Preset(lua_State* L)
{
	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return 0;

	auto preset_index = slot->AddPreset();
	if (!preset_index.has_value()) return 0;

	lua_pushnumber(L, preset_index.value());
	return 1;
}

static int Lua_DynamicMusic_Add_Segment(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "preset_index: incorrect argument type");

	if (!lua_isnumber(L, 3))
		return luaL_error(L, "segment_index: incorrect argument type");

	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return 0;

	auto segment_index = slot->AddSegmentToPreset(lua_tonumber(L, 2), lua_tonumber(L, 3));
	if (!segment_index.has_value()) return 0;

	lua_pushnumber(L, segment_index.value());
	return 1;
}

static int Lua_DynamicMusic_Set_Next_Segment(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "preset_index: incorrect argument type");

	if (!lua_isnumber(L, 3))
		return luaL_error(L, "segment_index: incorrect argument type");

	if (!lua_isnumber(L, 4))
		return luaL_error(L, "transition_preset_index: incorrect argument type");

	if (!lua_isnumber(L, 5))
		return luaL_error(L, "transition_segment_index: incorrect argument type");

	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return 0;

	slot->SetNextSegment(lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));
	return 0;
}

static int Lua_DynamicMusic_Set_Preset_Transition(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "preset_index: incorrect argument type");

	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return 0;

	slot->SetPresetTransition(lua_tonumber(L, 2));
	return 0;
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
	Music::instance()->ClearLevelMusic();
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
	{"load_track", L_TableFunction<Lua_DynamicMusic_Load_Track>},
	{"add_preset", L_TableFunction<Lua_DynamicMusic_Add_Preset>},
	{"add_segment", L_TableFunction<Lua_DynamicMusic_Add_Segment>},
	{"set_next_segment", L_TableFunction<Lua_DynamicMusic_Set_Next_Segment>},
	{"request_transition", L_TableFunction<Lua_DynamicMusic_Set_Preset_Transition>},
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

