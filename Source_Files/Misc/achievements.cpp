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
	if (!game_is_networked)
	{
		auto map_checksum = get_current_map_checksum();
		auto physics_checksum = get_physics_file_checksum();
		
		static constexpr uint32_t m1_map_checksum = 0x03C9;
		static constexpr uint32_t m1_physics_checksum = 0x5BC77666;
		
		if (map_checksum == m1_map_checksum &&
			physics_checksum == m1_physics_checksum)
		{
			lua = 
				#include "m1_achievements.lua"
		;
		}
		else
		{
			if (physics_checksum != m1_physics_checksum)
			{
				set_disabled_reason("Achievements disabled (third party physics)");
			}
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
