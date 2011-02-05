#include "FilmProfile.h"

#include "Plugins.h"

static FilmProfile alephone1_0 = {
	true, // keyframe_fix
	true, // damage_aggressor_last_in_tag
	true, // swipe_nearby_items_fix
	true, // initial_monster_fix
	true, // long_distance_physics
	true, // animate_items
	true, // inexplicable_pin_change
	true, // increased_dynamic_limits
};
static FilmProfile marathon2 = {
	false, // keyframe_fix
	false, // damage_aggressor_last_in_tag
	false, // swipe_nearby_items_fix
	false, // initial_monster_fix
	false, // long_distance_physics
	false, // animate_items
	false, // inexplicable_pin_change
	false, // increased_dynamic_limits
};

FilmProfile film_profile = alephone1_0;

extern void LoadBaseMMLScripts();
extern void ResetAllMMLValues();

void load_film_profile(FilmProfileType type, bool reload_mml)
{
	switch (type)
	{
	case FILM_PROFILE_DEFAULT:
		film_profile = alephone1_0;
		break;
	case FILM_PROFILE_MARATHON_2:
		film_profile = marathon2;
		break;
	}

	if (reload_mml)
	{
		ResetAllMMLValues();
		LoadBaseMMLScripts();
		Plugins::instance()->load_mml();
	}
}

