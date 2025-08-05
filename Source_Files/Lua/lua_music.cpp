#include "lua_music.h"
#include "Music.h"

static std::vector<std::pair<uint32_t, uint32_t>> music_presets_indexes;
static std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> music_segment_indexes;

void Reset_Music_InternalIndexes()
{
	music_presets_indexes.clear();
	music_segment_indexes.clear();
}

static std::pair<uint32_t, uint32_t> Get_MusicPreset(uint32_t preset_index)
{
	return music_presets_indexes[preset_index];
}

static uint32_t Get_LuaMusicPreset(uint32_t music_index, uint32_t preset_index)
{
	auto target = std::make_pair(music_index, preset_index);
	auto result = std::find(music_presets_indexes.begin(), music_presets_indexes.end(), target);
	return std::distance(music_presets_indexes.begin(), result);
}

static uint32_t Add_MusicPreset(uint32_t music_index, uint32_t preset_index)
{
	music_presets_indexes.emplace_back(music_index, preset_index);
	return static_cast<uint32_t>(music_presets_indexes.size() - 1);
}

static std::tuple<uint32_t, uint32_t, uint32_t> Get_MusicSegment(uint32_t segment_index)
{
	return music_segment_indexes[segment_index];
}

static std::optional<uint32_t> Get_LuaMusicSegment(uint32_t music_index, uint32_t preset_index, uint32_t segment_index)
{
	auto target = std::make_tuple(music_index, preset_index, segment_index);
	auto result = std::find(music_segment_indexes.begin(), music_segment_indexes.end(), target);
	if (result != music_segment_indexes.end()) return std::distance(music_segment_indexes.begin(), result);
	return std::nullopt;
}

static uint32_t Add_MusicSegment(uint32_t music_index, uint32_t preset_index, uint32_t segment_index)
{
	music_segment_indexes.emplace_back(music_index, preset_index, segment_index);
	return static_cast<uint32_t>(music_segment_indexes.size() - 1);
}

static int Lua_MusicManager_Clear(lua_State* L)
{
	Music::instance()->ClearLevelPlaylist();
	return 0;
}

static int Lua_MusicManager_Fade(lua_State* L)
{
	int duration = lua_isnumber(L, 1) ? static_cast<int>(lua_tonumber(L, 1) * 1000) : 1000;
	auto fadeType = lua_gettop(L) >= 2 ? static_cast<Music::FadeType>(Lua_MusicFadeType::ToIndex(L, 2)) : Music::FadeType::Linear;
	Music::instance()->Fade(0, duration, fadeType);
	Music::instance()->ClearLevelPlaylist();
	return 0;
}

static int Lua_Music_Fade(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "fade: incorrect argument type");

	float limitVolume = static_cast<float>(lua_tonumber(L, 2));
	int duration = lua_isnumber(L, 3) ? static_cast<int>(lua_tonumber(L, 3) * 1000) : 1000;
	bool stopOnNoVolume = lua_isboolean(L, 4) ? static_cast<bool>(lua_toboolean(L, 4)) : true;
	auto fadeType = lua_gettop(L) >= 5 ? static_cast<Music::FadeType>(Lua_MusicFadeType::ToIndex(L, 5)) : Music::FadeType::Linear;

	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return luaL_error(L, "fade: index out of bounds");

	slot->Fade(limitVolume, duration, fadeType, stopOnNoVolume);
	return 0;
}

static int Lua_Music_Play(lua_State* L)
{
	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return luaL_error(L, "play: index out of bounds");

	uint32_t preset_index = 0, segment_index = 0;

	if (lua_gettop(L) >= 2)
	{
		auto [music_index, preset_idx, segment_idx] = Get_MusicSegment(static_cast<uint32_t>(Lua_MusicSegment::Index(L, 2)));

		if (music_index != index)
		{
			return luaL_error(L, "play: invalid operation");
		}

		preset_index = preset_idx;
		segment_index = segment_idx;
	}

	slot->Play(preset_index, segment_index);
	return 0;
}

static int Lua_Music_Stop(lua_State* L)
{
	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return luaL_error(L, "stop: index out of bounds");

	slot->Pause();
	return 0;
}

static int Lua_Music_Active_Get(lua_State* L)
{
	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return luaL_error(L, "active: index out of bounds");

	lua_pushboolean(L, slot->Playing());
	return 1;
}

static int Lua_Music_Volume_Get(lua_State* L)
{
	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return luaL_error(L, "volume: index out of bounds");

	lua_pushnumber(L, static_cast<double>(slot->GetParameters().volume));
	return 1;
}

static int Lua_Music_Volume_Set(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "volume: incorrect argument type");

	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return luaL_error(L, "volume: index out of bounds");

	slot->SetVolume(static_cast<float>(lua_tonumber(L, 2)));
	return 0;
}

static bool Lua_Music_Valid(int16 index)
{
	return index >= 0 && Music::instance()->GetSlot(index + Music::reserved_music_slots);
}

static int Lua_MusicManager_New(lua_State* L)
{
	bool load_with_file = lua_type(L, 1) == LUA_TSTRING;
	int volume_index = load_with_file ? 2 : 1;
	int loop_index = load_with_file ? 3 : 2;
	float volume = lua_isnumber(L, volume_index) ? static_cast<float>(lua_tonumber(L, volume_index)) : 1;
	bool loop = lua_isboolean(L, loop_index) ? static_cast<bool>(lua_toboolean(L, loop_index)) : true;

	FileSpecifier music_file;

	if (load_with_file)
	{
		std::string search_path = L_Get_Search_Path(L);

		if (search_path.size())
		{
			if (!music_file.SetNameWithPath(lua_tostring(L, 1), search_path))
				return luaL_error(L, "new: file not found");
		}
		else
		{
			if (!music_file.SetNameWithPath(lua_tostring(L, 1)))
				return luaL_error(L, "new: file not found");
		}
	}

	auto id = Music::instance()->Add({ volume, loop }, load_with_file ? &music_file : nullptr);
	if (!id.has_value()) return luaL_error(L, "new: invalid file format");

	if (load_with_file) //a track loaded with a file shouldn't become a segmented track but just in case that happens, we handle it
	{
		Add_MusicPreset(id.value(), 0);
		Add_MusicSegment(id.value(), 0, 0);
	}

	Lua_Music::Push(L, id.value() - Music::reserved_music_slots);
	return 1;
}

static int Lua_Music_Load_Track(lua_State* L)
{
	if (!lua_isstring(L, 2))
		return luaL_error(L, "load_track: invalid file specifier");

	std::string search_path = L_Get_Search_Path(L);

	FileSpecifier file;
	if (search_path.size())
	{
		if (!file.SetNameWithPath(lua_tostring(L, 2), search_path)) 
			return luaL_error(L, "load_track: file not found");
	}
	else
	{
		if (!file.SetNameWithPath(lua_tostring(L, 2))) 
			return luaL_error(L, "load_track: file not found");
	}

	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return luaL_error(L, "load_track: index out of bounds");

	auto track_index = slot->LoadTrack(&file);
	if (!track_index.has_value()) return luaL_error(L, "load_track: invalid file format");

	lua_pushnumber(L, track_index.value());
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
			if (!file.SetNameWithPath(lua_tostring(L, n), search_path))
				return luaL_error(L, "play: file not found");
		}
		else
		{
			if (!file.SetNameWithPath(lua_tostring(L, n)))
				return luaL_error(L, "play: file not found");
		}

		Music::instance()->PushBackLevelMusic(file);
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

static bool Lua_MusicPreset_Valid(int16 index)
{
	return index >= 0 && index < music_presets_indexes.size();
}

static int Lua_Music_Add_Preset(lua_State* L)
{
	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return luaL_error(L, "add_preset: index out of bounds");

	auto preset_index = slot->AddPreset();
	if (!preset_index.has_value()) return luaL_error(L, "add_preset: invalid operation");

	Lua_MusicPreset::Push(L, Add_MusicPreset(index, preset_index.value()));
	return 1;
}

static int Lua_Music_Set_Preset_Transition(lua_State* L)
{
	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return luaL_error(L, "request_transition: index out of bounds");

	auto [music_index, preset_index] = Get_MusicPreset(static_cast<uint32_t>(Lua_MusicPreset::Index(L, 2)));
	if (!slot->SetPresetTransition(preset_index))
		return luaL_error(L, "request_transition: invalid operation");

	return 0;
}

static int Lua_Music_Get_Current_Segment(lua_State* L)
{
	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return luaL_error(L, "current_segment: index out of bounds");

	auto indexes = slot->GetCurrentPresetSegmentIndex();

	if (!indexes.has_value()) lua_pushnil(L);
	else
	{
		auto preset_index = indexes.value().first;
		auto segment_index = indexes.value().second;
		auto lua_segment_index = Get_LuaMusicSegment(index, preset_index, segment_index);
		if (!lua_segment_index.has_value()) lua_pushnil(L);
		else Lua_MusicSegment::Push(L, lua_segment_index.value());
	}

	return 1;
}

static int Lua_MusicPreset_Add_Segment(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "add_segment: incorrect argument type");

	auto index = static_cast<uint32_t>(Lua_MusicPreset::Index(L, 1));
	auto [music_index, preset_index] = Get_MusicPreset(index);

	auto slot = Music::instance()->GetSlot(music_index);
	if (!slot) return luaL_error(L, "add_segment: index out of bounds");

	auto segment_index = slot->AddSegmentToPreset(preset_index, lua_tonumber(L, 2));
	if (!segment_index.has_value()) return luaL_error(L, "add_segment: index out of bounds");

	Lua_MusicSegment::Push(L, Add_MusicSegment(music_index, preset_index, segment_index.value()));
	return 1;
}

static bool Lua_MusicSegment_Valid(int16 index)
{
	return index >= 0 && index < music_segment_indexes.size();
}

static int Lua_MusicSegment_Map_Segment_Transition(lua_State* L)
{
	auto index = static_cast<uint32_t>(Lua_MusicSegment::Index(L, 1));
	auto [music_index, preset_index, segment_index] = Get_MusicSegment(index);

	auto next_index = static_cast<uint32_t>(Lua_MusicSegment::Index(L, 2));
	auto [next_music_index, next_preset_index, next_segment_index] = Get_MusicSegment(next_index);

	if (music_index != next_music_index)
		return luaL_error(L, "map_segment_transition: incompatible segment index");

	auto slot = Music::instance()->GetSlot(music_index);
	if (!slot) return luaL_error(L, "map_segment_transition: index out of bounds");

	if (!slot->SetNextSegment(preset_index, segment_index, next_preset_index, next_segment_index))
		return luaL_error(L, "map_segment_transition: invalid operation");

	return 0;
}

static int Lua_MusicSegment_Get_Preset(lua_State* L)
{
	auto index = static_cast<uint32_t>(Lua_MusicSegment::Index(L, 1));
	auto [music_index, preset_index, segment_index] = Get_MusicSegment(index);
	auto lua_preset_index = Get_LuaMusicPreset(music_index, preset_index);
	Lua_MusicPreset::Push(L, lua_preset_index);
	return 1;
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
	{"load_track", L_TableFunction<Lua_Music_Load_Track>},
	{"add_preset", L_TableFunction<Lua_Music_Add_Preset>},
	{"request_transition", L_TableFunction<Lua_Music_Set_Preset_Transition>},
	{"current_segment", Lua_Music_Get_Current_Segment},
	{0, 0}
};

const luaL_Reg Lua_Music_Set[] = {
	{"volume", Lua_Music_Volume_Set},
	{0, 0}
};

const luaL_Reg Lua_MusicPreset_Get[] = {
	{"add_segment", L_TableFunction<Lua_MusicPreset_Add_Segment>},
	{0, 0}
};

const luaL_Reg Lua_MusicSegment_Get[] = {
	{"map_segment_transition", L_TableFunction<Lua_MusicSegment_Map_Segment_Transition>},
	{"preset", Lua_MusicSegment_Get_Preset},
	{0, 0}
};

char Lua_Music_Name[] = "music";
char Lua_MusicManager_Name[] = "Music";
char Lua_MusicPresets_Name[] = "MusicPresets";
char Lua_MusicSegments_Name[] = "MusicSegments";
char Lua_MusicPreset_Name[] = "music_preset";
char Lua_MusicSegment_Name[] = "music_segment";
char Lua_MusicFadeTypes_Name[] = "MusicFadeTypes";
char Lua_MusicFadeType_Name[] = "music_fade_type";

int Lua_Music_register(lua_State* L)
{
	constexpr int fade_type_enum_length = 2;

	Lua_MusicFadeType::Register(L, 0, 0, 0, Lua_MusicFadeType_Mnemonics);
	Lua_MusicFadeType::Valid = Lua_MusicFadeTypes::ValidRange(fade_type_enum_length);

	Lua_MusicFadeTypes::Register(L);
	Lua_MusicFadeTypes::Length = Lua_MusicFadeTypes::ConstantLength(fade_type_enum_length);

	Lua_Music::Register(L, Lua_Music_Get, Lua_Music_Set);
	Lua_Music::Valid = Lua_Music_Valid;

	Lua_MusicPresets::Register(L);
	Lua_MusicPresets::Length = std::bind(&std::vector<std::pair<uint32_t, uint32_t>>::size, &music_presets_indexes);

	Lua_MusicPreset::Register(L, Lua_MusicPreset_Get, nullptr);
	Lua_MusicPreset::Valid = Lua_MusicPreset_Valid;

	Lua_MusicSegments::Register(L);
	Lua_MusicSegments::Length = std::bind(&std::vector<std::tuple<uint32_t, uint32_t, uint32_t>>::size, &music_segment_indexes);

	Lua_MusicSegment::Register(L, Lua_MusicSegment_Get, nullptr);
	Lua_MusicSegment::Valid = Lua_MusicSegment_Valid;

	Lua_MusicManager::Register(L, Lua_MusicManager_Methods);
	Lua_MusicManager::Length = Lua_MusicManager::ConstantLength(16); //whatever
	return 0;
}
