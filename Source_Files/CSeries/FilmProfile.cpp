#include "FilmProfile.h"

#include "Plugins.h"

static FilmProfile alephone1_7 = {
	true,  // keyframe_fix
	false, // damage_aggressor_last_in_tag
	true,  // swipe_nearby_items_fix
	true,  // initial_monster_fix
	true,  // long_distance_physics
	true,  // animate_items
	true,  // inexplicable_pin_change
	false, // increased_dynamic_limits_1_0
	true,  // increased_dynamic_limits_1_1
	true,  // line_is_obstructed_fix
	false, // a1_smg
	true,  // infinity_smg
	true,  // use_vertical_kick_threshold
	true,  // infinity_tag_fix
	true,  // adjacent_polygons_always_intersect
	true,  // early_object_initialization
	true,  // fix_sliding_on_platforms
	true,  // prevent_dead_projectile_owners
	true,  // validate_random_ranged_attack
	true,  // allow_short_kamikaze
	true,  // ketchup_fix
	false, // lua_increments_rng
	true,  // destroy_players_ball_fix
	true,  // calculate_terminal_lines_correctly
	true,  // key_frame_zero_shrapnel_fix
	true,  // count_dead_dropped_items_correctly
	true,  // m1_low_gravity_projectiles
	true,  // m1_buggy_repair_goal
	false, // find_action_key_target_has_side_effects
	true,  // m1_object_unused
	true,  // m1_platform_flood
	true,  // m1_teleport_without_delay
	true,  // better_terminal_word_wrap
	true,  // lua_monster_killed_trigger_fix
	true,  // chip_insertion_ignores_tag_state
	true,  // page_up_past_full_width_term_pict
	true,  // fix_destroy_scenery_random_frame
	true,  // m1_reload_sound
	true,  // m1_landscape_effects
	true,  // m1_bce_pickup
};

static FilmProfile alephone1_4 = {
	true,  // keyframe_fix
	false, // damage_aggressor_last_in_tag
	true,  // swipe_nearby_items_fix
	true,  // initial_monster_fix
	true,  // long_distance_physics
	true,  // animate_items
	true,  // inexplicable_pin_change
	false, // increased_dynamic_limits_1_0
	true,  // increased_dynamic_limits_1_1
	true,  // line_is_obstructed_fix
	false, // a1_smg
	true,  // infinity_smg
	true,  // use_vertical_kick_threshold
	true,  // infinity_tag_fix
	true,  // adjacent_polygons_always_intersect
	true,  // early_object_initialization
	true,  // fix_sliding_on_platforms
	true,  // prevent_dead_projectile_owners
	true,  // validate_random_ranged_attack
	true,  // allow_short_kamikaze
	true,  // ketchup_fix
	false, // lua_increments_rng
	true,  // destroy_players_ball_fix
	true,  // calculate_terminal_lines_correctly
	true,  // key_frame_zero_shrapnel_fix
	true,  // count_dead_dropped_items_correctly
	true,  // m1_low_gravity_projectiles
	true,  // m1_buggy_repair_goal
	false, // find_action_key_target_has_side_effects
	true,  // m1_object_unused
	true,  // m1_platform_flood
	true,  // m1_teleport_without_delay
	true,  // better_terminal_word_wrap
	true,  // lua_monster_killed_trigger_fix
	false, // chip_insertion_ignores_tag_state
	false, // page_up_past_full_width_term_pict
	false, // fix_destroy_scenery_random_frame
	false, // m1_reload_sound
	false, // m1_landscape_effects
	false, // m1_bce_pickup
};


static FilmProfile alephone1_3 = {
	true, // keyframe_fix
	false, // damage_aggressor_last_in_tag
	true, // swipe_nearby_items_fix
	true, // initial_monster_fix
	true, // long_distance_physics
	true, // animate_items
	true, // inexplicable_pin_change
	false, // increased_dynamic_limits_1_0
	true, // increased_dynamic_limits_1_1
	true, // line_is_obstructed_fix
	false, // a1_smg
	true, // infinity_smg
	true, // use_vertical_kick_threshold
	true, // infinity_tag_fix
	true, // adjacent_polygons_always_intersect
	true, // early_object_initialization
	true, // fix_sliding_on_platforms
	true, // prevent_dead_projectile_owners
	true, // validate_random_ranged_attack
	true, // allow_short_kamikaze
	true, // ketchup_fix
	false, // lua_increments_rng
	true, // destroy_players_ball_fix
	true, // calculate_terminal_lines_correctly
	true, // key_frame_zero_shrapnel_fix
	true, // count_dead_dropped_items_correctly
	true, // m1_low_gravity_projectiles
	true, // m1_buggy_repair_goal
	false, // find_action_key_target_has_side_effects
	true,  // m1_object_unused
	true, // m1_platform_flood
	true, // m1_teleport_without_delay
	false, // better_terminal_word_wrap
	false, // lua_monster_killed_trigger_fix
	false, // chip_insertion_ignores_tag_state
	false, // page_up_past_full_width_term_pict
	false, // fix_destroy_scenery_random_frame
	false, // m1_reload_sound
	false, // m1_landscape_effects
	false, // m1_bce_pickup
};

static FilmProfile alephone1_2 = {
	true, // keyframe_fix
	false, // damage_aggressor_last_in_tag
	true, // swipe_nearby_items_fix
	true, // initial_monster_fix
	true, // long_distance_physics
	true, // animate_items
	true, // inexplicable_pin_change
	false, // increased_dynamic_limits_1_0
	true, // increased_dynamic_limits_1_1
	true, // line_is_obstructed_fix
	false, // a1_smg
	true, // infinity_smg
	true, // use_vertical_kick_threshold
	true, // infinity_tag_fix
	true, // adjacent_polygons_always_intersect
	true, // early_object_initialization
	true, // fix_sliding_on_platforms
	true, // prevent_dead_projectile_owners
	true, // validate_random_ranged_attack
	true, // allow_short_kamikaze
	true, // ketchup_fix
	false, // lua_increments_rng
	true, // destroy_players_ball_fix
	true, // calculate_terminal_lines_correctly
	true, // key_frame_zero_shrapnel_fix
	true, // count_dead_dropped_items_correctly
	true, // m1_low_gravity_projectiles
	true, // m1_buggy_repair_goal
	false, // find_action_key_target_has_side_effects
	false, // m1_object_unused
	false, // m1_platform_flood
	false, // m1_teleport_without_delay
	false, // better_terminal_word_wrap
	false, // lua_monster_killed_trigger_fix
	false, // chip_insertion_ignores_tag_state
	false, // page_up_past_full_width_term_pict
	false, // fix_destroy_scenery_random_frame
	false, // m1_reload_sound
	false, // m1_landscape_effects
	false, // m1_bce_pickup
};

static FilmProfile alephone1_1 = {
	true, // keyframe_fix
	false, // damage_aggressor_last_in_tag
	true, // swipe_nearby_items_fix
	true, // initial_monster_fix
	true, // long_distance_physics
	true, // animate_items
	true, // inexplicable_pin_change
	false, // increased_dynamic_limits_1_0
	true, // increased_dynamic_limits_1_1
	true, // line_is_obstructed_fix
	false, // a1_smg
	true, // infinity_smg
	true, // use_vertical_kick_threshold
	true, // infinity_tag_fix
	true, // adjacent_polygons_always_intersect
	true, // early_object_initialization
	true, // fix_sliding_on_platforms
	true, // prevent_dead_projectile_owners
	true, // validate_random_ranged_attack
	true, // allow_short_kamikaze
	true, // ketchup_fix
	false, // lua_increments_rng
	true, // destroy_players_ball_fix
	true, // calculate_terminal_lines_correctly
	true, // key_frame_zero_shrapnel_fix
	true, // count_dead_dropped_items_correctly
	false, // m1_low_gravity_projectiles
	false, // m1_buggy_repair_goal
	true, // find_action_key_target_has_side_effects
	false, // m1_object_unused
	false, // m1_platform_flood
	false, // m1_teleport_without_delay
	false, // better_terminal_word_wrap
	false, // lua_monster_killed_trigger_fix
	false, // chip_insertion_ignores_tag_state
	false, // page_up_past_full_width_term_pict
	false, // fix_destroy_scenery_random_frame
	false, // m1_reload_sound
	false, // m1_landscape_effects
	false, // m1_bce_pickup
};

static FilmProfile alephone1_0 = {
	true, // keyframe_fix
	true, // damage_aggressor_last_in_tag
	true, // swipe_nearby_items_fix
	true, // initial_monster_fix
	true, // long_distance_physics
	true, // animate_items
	true, // inexplicable_pin_change
	true, // increased_dynamic_limits_1_0
	false, // increased_dynamic_limits_1_1
	false, // line_is_obstructed_fix
	true, // a1_smg
	false, // infinity_smg
	false, // use_vertical_kick_threshold
	false, // infinity_tag_fix
	false, // adjacent_polygons_always_intersect
	true, // early_object_initialization
	false, // fix_sliding_on_platforms
	false, // prevent_dead_projectile_owners
	false, // validate_random_ranged_attack
	false, // allow_short_kamikaze
	false, // ketchup_fix
	true, // lua_increments_rng
	false, // destroy_players_ball_fix
	false, // calculate_terminal_lines_correctly
	false, // key_frame_zero_shrapnel_fix
	false, // count_dead_dropped_items_correctly
	false, // m1_low_gravity_projectiles
	false, // m1_buggy_repair_goal
	true, // find_action_key_target_has_side_effects
	false, // m1_object_unused
	false, // m1_platform_flood
	false, // m1_teleport_without_delay
	false, // better_terminal_word_wrap
	false, // lua_monster_killed_trigger_fix
	false, // chip_insertion_ignores_tag_state
	false, // page_up_past_full_width_term_pict
	false, // fix_destroy_scenery_random_frame
	false, // m1_reload_sound
	false, // m1_landscape_effects
	false, // m1_bce_pickup
};

static FilmProfile marathon2 = {
	false, // keyframe_fix
	false, // damage_aggressor_last_in_tag
	false, // swipe_nearby_items_fix
	false, // initial_monster_fix
	false, // long_distance_physics
	false, // animate_items
	false, // inexplicable_pin_change
	false, // increased_dynamic_limits_1_0
	false, // increased_dynamic_limits_1_1
	false, // line_is_obstructed
	false, // a1_smg
	false, // infinity_smg
	true, // use_vertical_kick_threshold
	false, // infinity_tag_fix
	false, // adjacent_polygons_always_intersect
	false, // early_object_initialization
	false, // fix_sliding_on_platforms
	false, // prevent_dead_projectile_owners
	false, // validate_random_ranged_attack
	false, // allow_short_kamikaze
	false, // ketchup_fix
	false, // lua_increments_rng
	false, // destroy_players_ball_fix
	false, // calculate_terminal_lines_correctly
	false, // key_frame_zero_shrapnel_fix
	false, // count_dead_dropped_items_correctly
	false, // m1_low_gravity_projectiles
	false, // m1_buggy_repair_goal
	false, // find_action_key_target_has_side_effects
	false, // m1_object_unused
	false, // m1_platform_flood
	false, // m1_teleport_without_delay
	true,  // better_terminal_word_wrap
	false, // lua_monster_killed_trigger_fix
	false, // chip_insertion_ignores_tag_state
	false, // page_up_past_full_width_term_pict
	false, // fix_destroy_scenery_random_frame
	false, // m1_reload_sound
	false, // m1_landscape_effects
	false, // m1_bce_pickup
};

static FilmProfile marathon_infinity = {
	false, // keyframe_fix
	false, // damage_aggressor_last_in_tag
	false, // swipe_nearby_items_fix
	true, // initial_monster_fix
	false, // long_distance_physics
	false, // animate_items
	false, // inexplicable_pin_change
	false, // increased_dynamic_limits_1_0
	false, // increased_dynamic_limits_1_1
	true, // line_is_obstructed_fix
	false, // a1_smg
	true, // infinity_smg
	true, // use_vertical_kick_threshold
	true, // infinity_tag_fix
	true, // adjacent_polygons_always_intersect
	false, // early_object_initialization
	false, // fix_sliding_on_platforms
	false, // prevent_dead_projectile_owners
	false, // validate_random_ranged_attack
	false, // allow_short_kamikaze
	false, // ketchup_fix
	false, // lua_increments_rng
	false, // destroy_players_ball_fix
	false, // calculate_terminal_lines_correctly
	false, // key_frame_zero_shrapnel_fix
	false, // count_dead_dropped_items_correctly
	false, // m1_low_gravity_projectiles
	false, // m1_buggy_repair_goal
	false, // find_action_key_target_has_side_effects
	false, // m1_object_unused
	false, // m1_platform_flood
	false, // m1_teleport_without_delay
	true,  // better_terminal_word_wrap
	false, // lua_monster_killed_trigger_fix
	false, // chip_insertion_ignores_tag_state
	false, // page_up_past_full_width_term_pict
	false, // fix_destroy_scenery_random_frame
	false, // m1_reload_sound
	false, // m1_landscape_effects
	false, // m1_bce_pickup
};

FilmProfile film_profile = alephone1_7;

extern void LoadBaseMMLScripts(bool load_menu_mml_only);
extern void ResetAllMMLValues();

void load_film_profile(FilmProfileType type, bool reload_mml)
{
	switch (type)
	{
	case FILM_PROFILE_DEFAULT:
		film_profile = alephone1_7;
		break;
	case FILM_PROFILE_MARATHON_2:
		film_profile = marathon2;
		break;
	case FILM_PROFILE_MARATHON_INFINITY:
		film_profile = marathon_infinity;
		break;
	case FILM_PROFILE_ALEPH_ONE_1_0:
		film_profile = alephone1_0;
		break;
	case FILM_PROFILE_ALEPH_ONE_1_1:
		film_profile = alephone1_1;
		break;
	case FILM_PROFILE_ALEPH_ONE_1_2:
		film_profile = alephone1_2;
		break;
	case FILM_PROFILE_ALEPH_ONE_1_3:
		film_profile = alephone1_3;
		break;
	case FILM_PROFILE_ALEPH_ONE_1_4:
		film_profile = alephone1_4;
		break;
	}

	if (reload_mml)
	{
		ResetAllMMLValues();
		LoadBaseMMLScripts(false);
		Plugins::instance()->load_mml(false);
	}
}

