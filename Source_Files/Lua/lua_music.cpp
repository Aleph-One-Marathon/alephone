#include "lua_music.h"
#include "Music.h"

static std::vector<std::pair<uint32_t, uint32_t>> music_sequence_indexes;
static std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> music_segment_indexes;

void Reset_Music_InternalIndexes()
{
	music_sequence_indexes.clear();
	music_segment_indexes.clear();
}

static std::pair<uint32_t, uint32_t> Get_MusicSequence(uint32_t sequence_index)
{
	return music_sequence_indexes[sequence_index];
}

static uint32_t Add_MusicSequence(uint32_t music_index, uint32_t sequence_index)
{
	music_sequence_indexes.emplace_back(music_index, sequence_index);
	return static_cast<uint32_t>(music_sequence_indexes.size() - 1);
}

static std::tuple<uint32_t, uint32_t, uint32_t> Get_MusicSegment(uint32_t segment_index)
{
	return music_segment_indexes[segment_index];
}

static uint32_t Add_MusicSegment(uint32_t music_index, uint32_t sequence_index, uint32_t segment_index)
{
	music_segment_indexes.emplace_back(music_index, sequence_index, segment_index);
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
	auto fadeType = lua_gettop(L) >= 2 ? static_cast<MusicPlayer::FadeType>(Lua_MusicFadeType::ToIndex(L, 2)) : MusicPlayer::FadeType::Linear;
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
	auto fadeType = lua_gettop(L) >= 5 ? static_cast<MusicPlayer::FadeType>(Lua_MusicFadeType::ToIndex(L, 5)) : MusicPlayer::FadeType::Linear;

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

	uint32_t sequence_index = 0, segment_index = 0;

	if (lua_gettop(L) >= 2)
	{
		auto [music_index, sequence_idx, segment_idx] = Get_MusicSegment(static_cast<uint32_t>(Lua_MusicSegment::Index(L, 2)));

		if (music_index != index)
		{
			return luaL_error(L, "play: invalid operation");
		}

		sequence_index = sequence_idx;
		segment_index = segment_idx;
	}

	slot->Play(sequence_index, segment_index);
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

static int Lua_Music_AddTrack(lua_State* L)
{
	if (!lua_isstring(L, 2))
		return luaL_error(L, "add_track: invalid file specifier");

	std::string search_path = L_Get_Search_Path(L);

	FileSpecifier file;
	if (search_path.size())
	{
		if (!file.SetNameWithPath(lua_tostring(L, 2), search_path))
			return luaL_error(L, "add_track: file not found");
	}
	else
	{
		if (!file.SetNameWithPath(lua_tostring(L, 2)))
			return luaL_error(L, "add_track: file not found");
	}

	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return luaL_error(L, "add_track: index out of bounds");

	auto track_index = slot->AddTrack(&file);
	if (!track_index.has_value()) return luaL_error(L, "add_track: invalid file format");

	lua_pushnumber(L, track_index.value());
	return 1;
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
		Add_MusicSequence(id.value(), 0);
		Add_MusicSegment(id.value(), 0, 0);
	}

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

static int Lua_Get_MusicSequence(lua_State* L)
{
	Lua_MusicSequence::Push(L, Lua_Music::Index(L, 1));
	return 1;
}

static int Lua_MusicSequence_New(lua_State* L)
{
	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return luaL_error(L, "new: index out of bounds");

	auto sequence_index = slot->AddSequence();
	if (!sequence_index.has_value()) return luaL_error(L, "new: invalid operation");

	Lua_MusicSequence::Push(L, Add_MusicSequence(index, sequence_index.value()));
	return 1;
}

static int Lua_Music_Set_Sequence_Transition(lua_State* L)
{
	int index = Lua_Music::Index(L, 1) + Music::reserved_music_slots;
	auto slot = Music::instance()->GetSlot(index);
	if (!slot) return luaL_error(L, "request_sequence_transition: index out of bounds");

	auto [music_index, sequence_index] = Get_MusicSequence(static_cast<uint32_t>(Lua_MusicSequence::Index(L, 2)));

	if (music_index != index || !slot->SetSequenceTransition(sequence_index))
		return luaL_error(L, "request_sequence_transition: invalid operation");

	return 0;
}

static int Lua_MusicSegment_New(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "new: incorrect argument type");

	auto index = static_cast<uint32_t>(Lua_MusicSequence::Index(L, 1));
	auto [music_index, sequence_index] = Get_MusicSequence(index);

	auto slot = Music::instance()->GetSlot(music_index);
	if (!slot) return luaL_error(L, "new: index out of bounds");

	auto segment_index = slot->AddSegmentToSequence(sequence_index, static_cast<int>(lua_tonumber(L, 2)));
	if (!segment_index.has_value()) return luaL_error(L, "new: index out of bounds");

	Lua_MusicSegment::Push(L, Add_MusicSegment(music_index, sequence_index, segment_index.value()));
	return 1;
}

static int Lua_Get_MusicSegment(lua_State* L)
{
	Lua_MusicSegment::Push(L, Lua_MusicSequence::Index(L, 1));
	return 1;
}

static int Lua_MusicSegment_Add_Segment_Transition(lua_State* L)
{
	auto index = static_cast<uint32_t>(Lua_MusicSegment::Index(L, 1));
	auto [music_index, sequence_index, segment_index] = Get_MusicSegment(index);

	auto next_index = static_cast<uint32_t>(Lua_MusicSegment::Index(L, 2));
	auto [next_music_index, next_sequence_index, next_segment_index] = Get_MusicSegment(next_index);

	if (music_index != next_music_index)
		return luaL_error(L, "add_transition: incompatible segment index");

	auto slot = Music::instance()->GetSlot(music_index);
	if (!slot) return luaL_error(L, "add_transition: index out of bounds");

	auto fadeTypeOut = lua_gettop(L) >= 3 ? static_cast<MusicPlayer::FadeType>(Lua_MusicFadeType::ToIndex(L, 3)) : MusicPlayer::FadeType::None;
	auto fadeTypeOutSeconds = lua_isnumber(L, 4) ? static_cast<float>(lua_tonumber(L, 4)) : 0;
	auto fadeTypeIn = lua_gettop(L) >= 5 ? static_cast<MusicPlayer::FadeType>(Lua_MusicFadeType::ToIndex(L, 5)) : MusicPlayer::FadeType::None;
	auto fadeTypeInSeconds = lua_isnumber(L, 6) ? static_cast<float>(lua_tonumber(L, 6)) : 0;
	bool crossfade = lua_isboolean(L, 7) ? static_cast<bool>(lua_toboolean(L, 7)) : false;

	MusicPlayer::Segment::Edge edge(next_segment_index, { fadeTypeOut , fadeTypeOutSeconds }, { fadeTypeIn , fadeTypeInSeconds }, crossfade);

	if (!slot->SetSegmentEdge(sequence_index, segment_index, next_sequence_index, edge))
		return luaL_error(L, "add_transition: invalid operation");

	return 0;
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
	{"volume", Lua_Music_Volume_Get},
	{"active", Lua_Music_Active_Get},
	{"sequences", Lua_Get_MusicSequence},
	{0, 0}
};

const luaL_Reg Lua_Music_Get_Mutable[] = {
	{"fade", L_TableFunction<Lua_Music_Fade>},
	{"play", L_TableFunction<Lua_Music_Play>},
	{"stop", L_TableFunction<Lua_Music_Stop>},
	{"add_track", L_TableFunction<Lua_Music_AddTrack>},
	{"request_sequence_transition", L_TableFunction<Lua_Music_Set_Sequence_Transition>},
	{0, 0}
};

const luaL_Reg Lua_Music_Set[] = {
	{"volume", Lua_Music_Volume_Set},
	{0, 0}
};

const luaL_Reg Lua_MusicSequence_Get_Mutable[] = {
	{"new", L_TableFunction<Lua_MusicSequence_New>},
	{0, 0}
};

const luaL_Reg Lua_MusicSequence_Get[] = {
	{"segments", Lua_Get_MusicSegment},
	{0, 0}
};

const luaL_Reg Lua_MusicSegment_Get_Mutable[] = {
    {"new", L_TableFunction<Lua_MusicSegment_New>},
	{"add_transition", L_TableFunction<Lua_MusicSegment_Add_Segment_Transition>},
	{0, 0}
};

char Lua_Music_Name[] = "music";
char Lua_MusicManager_Name[] = "Music";
char Lua_MusicSequence_Name[] = "music_sequence";
char Lua_MusicSegment_Name[] = "music_segment";
char Lua_MusicFadeTypes_Name[] = "MusicFadeTypes";
char Lua_MusicFadeType_Name[] = "music_fade_type";

int Lua_Music_register(lua_State* L, const LuaMutabilityInterface& m)
{
	Lua_Music::Register(L, Lua_Music_Get);
	Lua_MusicSequence::Register(L, Lua_MusicSequence_Get);

	if (m.world_mutable() || m.music_mutable())
	{
		Lua_Music::RegisterAdditional(L, Lua_Music_Get_Mutable, Lua_Music_Set);
		Lua_MusicSequence::RegisterAdditional(L, Lua_MusicSequence_Get_Mutable);
		Lua_MusicSegment::Register(L, Lua_MusicSegment_Get_Mutable);
	}

	Lua_Music::Valid = Lua_Music_Valid;

	if (m.world_mutable() || m.music_mutable())
	{
		Lua_MusicManager::Register(L, Lua_MusicManager_Methods);
	}
	else
	{
		Lua_MusicManager::Register(L);
	}

	constexpr int fade_type_enum_length = 3;

	Lua_MusicFadeType::Register(L, 0, 0, 0, Lua_MusicFadeType_Mnemonics);
	Lua_MusicFadeType::Valid = Lua_MusicFadeTypes::ValidRange(fade_type_enum_length);

	Lua_MusicFadeTypes::Register(L);
	Lua_MusicFadeTypes::Length = Lua_MusicFadeTypes::ConstantLength(fade_type_enum_length);
	
	Lua_MusicManager::Length = Lua_MusicManager::ConstantLength(16); //whatever
	return 0;
}

