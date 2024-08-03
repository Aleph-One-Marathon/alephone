#include "achievements.h"

#include "crc.h"
#include "extensions.h"
#include "Logging.h"
#include "map.h"
#include "preferences.h"

#ifdef HAVE_STEAM
#include "steamshim_child.h"
#endif

Achievements* Achievements::instance() {
	static Achievements* instance_ = nullptr;
	if (!instance_)
	{
		instance_ = new Achievements();
	}

	return instance_;
}

std::string Achievements::get_lua()
{
	std::string lua;

#ifdef HAVE_STEAM
	if (get_game_controller() == _single_player)
	{
		auto map_checksum = get_current_map_checksum();
		auto physics_checksum = get_physics_file_checksum();
		
		static constexpr uint32_t m1_map_checksum = 0x03C9;
		static constexpr uint32_t m1_physics_checksum = 0x5BC77666;

		static constexpr uint32_t m2_map_checksum = 0x2d71ccb4;
		static constexpr uint32_t m2_win95_map_checksum = 0x5e0ba590;
		static constexpr uint32_t m2_physics_checksum = 0x91d72dab;

		static constexpr uint32_t inf_map_checksum = 0xA80E94B1;
		
		switch (map_checksum) {
			case m1_map_checksum:
				if (physics_checksum == m1_physics_checksum)
				{
					lua =
						#include "m1_achievements.lua"
				;
				}
				else
				{
					set_disabled_reason("Achievements disabled (third party physics)");
				}
				break;
			case m2_map_checksum:
			case m2_win95_map_checksum:
				if (physics_checksum == m2_physics_checksum)
				{
					lua =
						#include "m2_achievements.lua"
					;
				}
				else
				{
					set_disabled_reason("Achievements disabled (third party physics)");					
				}
				break;
			case inf_map_checksum:
				lua =
					#include "inf_achievements.lua"
			;
				break;
		}

		if (lua.size() == 0)
		{
			logNote("achievements: invalidating due to checksum mismatch (map: 0x%x 0x%x, phy 0x%x 0x%x)", map_checksum, m1_map_checksum, physics_checksum, m1_physics_checksum);	
		}
	}
#endif

	return lua;
}

void Achievements::set(const std::string& key)
{
	logNote("achievement: posting %s", key.c_str());
#ifdef HAVE_STEAM
	STEAMSHIM_setAchievement(key.c_str(), 1);
	STEAMSHIM_storeStats();
#endif
}
