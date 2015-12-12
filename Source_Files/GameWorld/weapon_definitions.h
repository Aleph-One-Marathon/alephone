#ifndef __WEAPON_DEFINITIONS_H
#define __WEAPON_DEFINITIONS_H

/*
	WEAPON_DEFINITIONS2.H

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
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

	Saturday, May 13, 1995 8:28:14 PM

	Sunday, May 14, 1995 3:11:52 AM- converted the piece of shit...

4 Feb 2000 (Loren Petrich):
	Added SMG stuff
*/

#include "items.h"
#include "map.h"
#include "projectiles.h"
#include "SoundManagerEnums.h"
#include "weapons.h"

/* TEMPORARY!!! */
enum {
	_projectile_ball_dropped= 1000
};

/* ---------- constants */
#define NORMAL_WEAPON_DZ (20)

/* This is the amount of ammo that charging weapons use at one time.. */
#define CHARGING_WEAPON_AMMO_COUNT 4

enum /* weapon classes */
{
	_melee_class, /* normal weapon, no ammunition, both triggers do the same thing */
	_normal_class, /* normal weapon, one ammunition type, both triggers do the same thing */
	_dual_function_class, /* normal weapon, one ammunition type, trigger does something different */
	_twofisted_pistol_class, /* two can be held at once (differnet triggers), same ammunition */
	_multipurpose_class /* two weapons in one (assault rifle, grenade launcher), two different
		ammunition types with two separate triggers; secondary ammunition is discrete (i.e., it
		is never loaded explicitly but appears in the weapon) */
};

enum /* weapon flags */
{
	_no_flags= 0x0,
	_weapon_is_automatic= 0x01,
	_weapon_disappears_after_use= 0x02,
	_weapon_plays_instant_shell_casing_sound= 0x04,
	_weapon_overloads= 0x08,
	_weapon_has_random_ammo_on_pickup= 0x10,
	_powerup_is_temporary= 0x20,
	_weapon_reloads_in_one_hand= 0x40,
	_weapon_fires_out_of_phase= 0x80,
	_weapon_fires_under_media= 0x100,
	_weapon_triggers_share_ammo= 0x200,
	_weapon_secondary_has_angular_flipping= 0x400,
	
	// definitions for Marathon compatibility
	_weapon_disappears_after_use_m1 = 0x04,
	_weapon_is_marathon_1 = 0x1000,
	_weapon_flutters_while_firing = 0x2000,
};

enum {
	_weapon_in_hand_collection= 1,
	_fist_idle= 0,
	_fist_punching,
	_pistol_idle,
	_pistol_firing,
	_pistol_reloading,
	_shotgun_idle,
	_shotgun_firing,
	_shotgun_reloading,
	_assault_rifle_idle,
	_assault_rifle_firing,
	_assault_rifle_reloading,
	_fusion_idle,
	_fusion_firing,
	_missile_launcher_idle,
	_missile_launcher_firing,
	_flamethrower_idle,
	_flamethrower_transit,
	_flamethrower_firing,
	_assault_rifle_shell_casing,
	_pistol_shell_casing,
	_fusion_charged,
	_alien_weapon_idle,
	_alien_weapon_firing,
	// LP additions:
	_smg_idle,
	_smg_firing,
	_smg_reloading,
	_smg_shell_casing
};

/* ---------- shell casings */

enum // shell casing types
{
	_shell_casing_assault_rifle,
	_shell_casing_pistol,
	_shell_casing_pistol_left,
	_shell_casing_pistol_right,
	// LP additions:
	_shell_casing_smg,
	
	NUMBER_OF_SHELL_CASING_TYPES
};

struct shell_casing_definition
{
	int16 collection, shape;
	
	_fixed x0, y0;
	_fixed vx0, vy0;
	_fixed dvx, dvy;
};

static struct shell_casing_definition shell_casing_definitions[NUMBER_OF_SHELL_CASING_TYPES]=
{
	{ // _shell_casing_assault_rifle,
		_collection_weapons_in_hand, 19, /* collection, shape */
		
		FIXED_ONE/2 + FIXED_ONE/6, FIXED_ONE/8, /* x0, y0 */
		FIXED_ONE/8, FIXED_ONE/32, /* vx0, vy0 */
		0, -FIXED_ONE/256, /* dvx, dvy */
	},
	
	{ // _shell_casing_pistol_center
		_collection_weapons_in_hand, 18, /* collection, shape */
		
		FIXED_ONE/2 + FIXED_ONE/8, FIXED_ONE/4, /* x0, y0 */
		FIXED_ONE/16, FIXED_ONE/32, /* vx0, vy0 */
		0, -FIXED_ONE/400, /* dvx, dvy */
	},
	
	{ // _shell_casing_pistol_left
		_collection_weapons_in_hand, 18, /* collection, shape */
		
		FIXED_ONE/2 - FIXED_ONE/4, FIXED_ONE/4, /* x0, y0 */
		- FIXED_ONE/16, FIXED_ONE/32, /* vx0, vy0 */
		0, -FIXED_ONE/400, /* dvx, dvy */
	},
	
	{ // _shell_casing_pistol_right
		_collection_weapons_in_hand, 18, /* collection, shape */
		
		FIXED_ONE/2 + FIXED_ONE/4, FIXED_ONE/4, /* x0, y0 */
		FIXED_ONE/16, FIXED_ONE/32, /* vx0, vy0 */
		0, -FIXED_ONE/400, /* dvx, dvy */
	},
	
	// LP addition: clone of assault-rifle casing
	{ // _shell_casing_smg,
		_collection_weapons_in_hand, 26, /* collection, shape */
		
		FIXED_ONE/2 + FIXED_ONE/6, FIXED_ONE/8, /* x0, y0 */
		FIXED_ONE/8, FIXED_ONE/32, /* vx0, vy0 */
		0, -FIXED_ONE/256, /* dvx, dvy */
	},
};

/* ---------- structures */

struct trigger_definition {
	int16 rounds_per_magazine;
	int16 ammunition_type;
	int16 ticks_per_round;
	int16 recovery_ticks;
	int16 charging_ticks;
	world_distance recoil_magnitude;
	int16 firing_sound;
	int16 click_sound;
	int16 charging_sound;
	int16 shell_casing_sound;
	int16 reloading_sound;
	int16 charged_sound;
	int16 projectile_type;
	int16 theta_error;
	int16 dx, dz;
	int16 shell_casing_type;
	int16 burst_count;
    int16 sound_activation_range; /* for Marathon compatibility */
};

struct weapon_definition {
	int16 item_type;
	int16 powerup_type;
	int16 weapon_class;
	int16 flags;

	_fixed firing_light_intensity;
	int16 firing_intensity_decay_ticks;

	/* weapon will come up to FIXED_ONE when fired; idle_height±bob_amplitude should be in
		the range [0,FIXED_ONE] */
	_fixed idle_height, bob_amplitude, kick_height, reload_height;
	_fixed idle_width, horizontal_amplitude;

	/* each weapon has three basic animations: idle, firing and reloading.  sounds and frames
		are pulled from the shape collection.  for automatic weapons the firing animation loops
		until the trigger is released or the gun is empty and the gun begins rising as soon as
		the trigger is depressed and is not lowered until the firing animation stops.  for single
		shot weapons the animation loops once; the weapon is raised and lowered as soon as the
		firing animation terminates */
	int16 collection;
	int16 idle_shape, firing_shape, reloading_shape;
	int16 unused;
	int16 charging_shape, charged_shape;

	/* How long does it take to ready the weapon? */
	/* load_rounds_tick is the point which you actually load them. */
	int16 ready_ticks, await_reload_ticks, loading_ticks, finish_loading_ticks, powerup_ticks;

	struct trigger_definition weapons_by_trigger[NUMBER_OF_TRIGGERS];
};

/* ------------------------ globals */

static int16 weapon_ordering_array[]= {
	_weapon_fist,
	_weapon_pistol,
	_weapon_plasma_pistol,
	_weapon_shotgun,
	_weapon_assault_rifle,
	// LP addition:
	_weapon_smg,
	_weapon_flamethrower,
	_weapon_missile_launcher,
	_weapon_alien_shotgun,
	_weapon_ball
};

//#define NUMBER_OF_WEAPONS static_cast<int>(sizeof(weapon_definitions)/sizeof(struct weapon_definition))
#define NUMBER_OF_WEAPONS 10

static struct weapon_definition weapon_definitions[NUMBER_OF_WEAPONS]; 
const struct weapon_definition original_weapon_definitions[NUMBER_OF_WEAPONS]=
{
	/* Fist*/
	{
		/* item type, powerup type, item class, item flags */
		_i_knife, NONE, _melee_class, _weapon_fires_under_media,

		FIXED_ONE_HALF, 0, /* firing intensity, firing decay */

		/* idle height, bob amplitude, kick height, reload height */
		FIXED_ONE+FIXED_ONE/15, FIXED_ONE/15, FIXED_ONE/16, 0,
		
		/* horizontal positioning.. */
		FIXED_ONE_HALF, 0,

		/* collection, idle, firing, reloading shapes; shell casing, charging, charged */
		_weapon_in_hand_collection,
		_fist_idle, _fist_punching, _fist_idle,
		NONE,
		NONE, NONE,

		/* ready/await/load/finish/powerup rounds ticks */
		TICKS_PER_SECOND/4, 0, 0, 0, 0,

		{
			{
				/* rounds per magazine */
				1, 
	
				/* Ammunition type */
				NONE, 
				
				/* Ticks per round, recovery ticks, charging ticks */
				NONE, TICKS_PER_SECOND/3, 0,
				
				/* recoil magnitude */
				0,
				
				/* firing, click, charging, shell casing sound, reloading sound */
				NONE, NONE, NONE, NONE, NONE, NONE,
				
				/* projectile type */
				_projectile_fist,
				
				/* theta error */
				0,
				
				/* dx, dz */
				0, 0,
				
				/* shell casing type */
				NONE, 

				/* burst count */
				0
			},
			{
				/* rounds per magazine */
				1, 
	
				/* Ammunition type */
				NONE, 
				
				/* Ticks per round, recovery ticks, charging ticks */
				NONE, TICKS_PER_SECOND/3, 0,
				
				/* recoil magnitude */
				0,
				
				/* firing, click, charging, shell casing, reload sound */
				NONE, NONE, NONE, NONE, NONE, NONE,
				
				/* projectile type */
				_projectile_fist,
				
				/* theta error */
				0,
				
				/* dx, dz */
				0, 0,
				
				/* shell casing type */
				NONE, 

				/* burst count */
				0
			}
		}
	},
	
	/* Magnum .45 "mega class"- dual fisted */
	{
		/* item type, powerup type, item class, item flags */
		_i_magnum, NONE, _twofisted_pistol_class, _weapon_fires_out_of_phase,

		3*FIXED_ONE/4, TICKS_PER_SECOND/8, /* firing intensity, firing decay */

		/* idle height, bob amplitude, kick height, reload height */
		FIXED_ONE+FIXED_ONE/15, FIXED_ONE/25, FIXED_ONE/8, FIXED_ONE,
		
		/* horizontal positioning.. */
		FIXED_ONE_HALF, 0,

		/* collection, idle, firing, reloading shapes; shell casing, charging, charged */
		_weapon_in_hand_collection,
		_pistol_idle, _pistol_firing, _pistol_reloading,
		NONE,
		NONE, NONE,

		/* ready/await/load/finish/powerup rounds ticks */
		TICKS_PER_SECOND/3, 5, 5, 5, 0, // was NONE

		{
			{
				/* rounds per magazine */
				8, 
	
				/* Ammunition type */
				_i_magnum_magazine, 
				
				/* Ticks per round, recovery ticks, charging ticks */
				NONE, TICKS_PER_SECOND/3, 0,
				
				/* recoil magnitude */
				10,
				
				/* firing, click, charging, shell casing, reload sound */
				_snd_magnum_firing, _snd_empty_gun, NONE, NONE, _snd_magnum_reloading, NONE,
				
				/* projectile type */
				_projectile_pistol_bullet,
				
				/* theta error */
				1,
				
				/* dx, dz */
				(WORLD_ONE_FOURTH/6), -NORMAL_WEAPON_DZ,		/* Primary */
				
				/* shell casing type */
				_shell_casing_pistol, 

				/* burst count */
				0
			},
			
			/* left weapon (for consistency)... */
			{
				/* rounds per magazine */
				8, 
	
				/* Ammunition type */
				_i_magnum_magazine, 
				
				/* Ticks per round, recovery ticks, charging ticks */
				NONE, TICKS_PER_SECOND/3, 0,
				
				/* recoil magnitude */
				10,
				
				/* firing, click, charging, shell casing, reload sound */
				_snd_magnum_firing, _snd_empty_gun, NONE, NONE, _snd_magnum_reloading, NONE,
				
				/* projectile type */
				_projectile_pistol_bullet,
				
				/* theta error */
				1,
				
				/* dx, dz */
				-(WORLD_ONE_FOURTH/6), -NORMAL_WEAPON_DZ,		/* Primary */
				
				/* shell casing type */
				_shell_casing_pistol,

				/* burst count */
				0
			}
		}
	},

	/* Fusion Pistol */
	{
		/* item type, powerup type, item class, item flags */
		_i_plasma_pistol, NONE, _dual_function_class, _weapon_overloads | _weapon_fires_under_media | _weapon_triggers_share_ammo,

		3*FIXED_ONE/4, TICKS_PER_SECOND/3, /* firing intensity, firing decay */

		/* idle height, bob amplitude, kick height, reload height */
		FIXED_ONE, FIXED_ONE/25, FIXED_ONE/8, FIXED_ONE,
		
		/* horizontal positioning.. */
		FIXED_ONE_HALF, 0,

		/* collection, idle, firing, reloading shapes; shell casing, charging, charged */
		_weapon_in_hand_collection,
		_fusion_idle, _fusion_firing, NONE,
		NONE,
		NONE, _fusion_charged,

		/* ready/await/load/finish/powerup rounds ticks */
		TICKS_PER_SECOND/3, TICKS_PER_SECOND/2, TICKS_PER_SECOND/2, TICKS_PER_SECOND/2, 0,

		{
			{
				/* rounds per magazine */
				20, 
	
				/* Ammunition type */
				_i_plasma_magazine, 
				
				/* Ticks per round, recovery ticks, charging ticks */
				NONE, TICKS_PER_SECOND/6, 0,
				
				/* recoil magnitude */
				5,
				
				/* firing, click, charging, shell casing, reload sound */
				_snd_fusion_firing, _snd_empty_gun, _snd_fusion_charging, NONE, NONE, NONE,
				
				/* projectile type */
				_projectile_fusion_bolt_minor,
				
				/* theta error */
				1,
				
				/* dx, dz */
				0, -4*NORMAL_WEAPON_DZ,
				
				/* shell casing type */
				NONE, 

				/* burst count */
				0
			},
			{
				/* rounds per magazine */
				20,  // this should not be used...
	
				/* Ammunition type */
				_i_plasma_magazine, 
				
				/* Ticks per round, recovery ticks, charging ticks */
				5, 4, TICKS_PER_SECOND/2,
				
				/* recoil magnitude */
				20,
				
				/* firing, click, charging, shell casing, reload sound */
				_snd_major_fusion_firing, _snd_empty_gun, _snd_fusion_charging, NONE, NONE, _snd_major_fusion_charged,
				
				/* projectile type */
				_projectile_fusion_bolt_major,
				
				/* theta error */
				1,
				
				/* dx, dz */
				0, 0,
				
				/* shell casing type */
				NONE, 

				/* burst count */
				0
			}
		}
	},

	/* Assault Rifle */
	{
		/* item type, powerup type, item class, item flags */
		_i_assault_rifle, NONE, _multipurpose_class, _weapon_is_automatic,

		3*FIXED_ONE/4, TICKS_PER_SECOND/5, /* firing intensity, firing decay */


		/* idle height, bob amplitude, kick height, reload height */
		FIXED_ONE+FIXED_ONE/6, FIXED_ONE/35, FIXED_ONE/16, 3*FIXED_ONE/4,

		/* horizontal positioning.. */
		FIXED_ONE_HALF, 0,
		
		/* collection, idle, firing, reloading shapes; shell casing, charging, charged */
		_weapon_in_hand_collection,
		_assault_rifle_idle, _assault_rifle_firing, _assault_rifle_reloading,
		NONE,
		NONE, NONE,

		/* ready/await/load/finish/powerup rounds ticks */
		TICKS_PER_SECOND/2, TICKS_PER_SECOND/3, TICKS_PER_SECOND/3, TICKS_PER_SECOND/3, 0,

		{
			{
				/* rounds per magazine */
				52, 
	
				/* Ammunition type */
				_i_assault_rifle_magazine, 
				
				/* Ticks per round, recovery ticks, charging ticks */
				NONE, 0, 0,
				
				/* recoil magnitude */
				5,
				
				/* firing, click, charging, shell casing, reload sound */
				_snd_assault_rifle_firing, _snd_empty_gun, NONE, _snd_assault_rifle_shell_casings, _snd_assault_rifle_reloading, NONE,
				
				/* projectile type */
				_projectile_rifle_bullet,
				
				/* theta error */
				10,
				
				/* dx, dz */
				0, -NORMAL_WEAPON_DZ,
				
				/* shell casing type */
				_shell_casing_assault_rifle,

				/* burst count */
				0
			},
			{
				/* rounds per magazine */
				7, 
	
				/* Ammunition type */
				_i_assault_grenade_magazine, 
				
				/* Ticks per round, recovery ticks, charging ticks */
				TICKS_PER_SECOND/6, (3*TICKS_PER_SECOND)/4 - TICKS_PER_SECOND/6, 0,
				
				/* recoil magnitude */
				40,
				
				/* firing, click, charging, shell casing, reload sound */
				_snd_grenade_launcher_firing, _snd_empty_gun, NONE, NONE, NONE, NONE,
				
				/* projectile type */
				_projectile_grenade,
				
				/* theta error */
				0,
				
				/* dx, dz */
				0, -5*NORMAL_WEAPON_DZ,
				
				/* shell casing type */
				NONE, 

				/* burst count */
				0
			}
		}
	},

	/* Rocket Launcher */
	{
		/* item type, powerup_type, item class, item flags */
		_i_missile_launcher, NONE, _normal_class, _no_flags,

		FIXED_ONE, TICKS_PER_SECOND/3, /* firing intensity, firing decay */

		/* idle height, bob amplitude, kick height, reload height */
		(3*FIXED_ONE)/4, FIXED_ONE/50, FIXED_ONE/20, FIXED_ONE,
		
		/* horizontal positioning.. */
		(FIXED_ONE_HALF/4), 0,

		/* collection, idle, firing, reloading shapes; shell casing, charging, charged */
		_weapon_in_hand_collection,
		_missile_launcher_idle, _missile_launcher_firing, NONE,
		NONE,
		NONE, NONE,

		/* ready/await/load/finish/powerup rounds ticks */
		TICKS_PER_SECOND, TICKS_PER_SECOND, TICKS_PER_SECOND, TICKS_PER_SECOND, 0,

		{
			{
				/* rounds per magazine */
				2, 
	
				/* Ammunition type */
				_i_missile_launcher_magazine, 
				
				/* Ticks per round, recovery ticks, charging ticks */
				TICKS_PER_SECOND/2, TICKS_PER_SECOND/10, 0,
				
				/* recoil magnitude */
				100,
				
				/* firing, click, charging, shell casing, reload sound */
				_snd_rocket_firing, _snd_empty_gun, NONE, NONE, NONE, NONE,
				
				/* projectile type */
				_projectile_rocket,
				
				/* theta error */
				0,
				
				/* dx, dz */
				-WORLD_ONE_FOURTH, 0,
				
				/* shell casing type */
				NONE, 

				/* burst count */
				0
			},
			
			/* unused */
			{
				/* rounds per magazine */
				1, 
	
				/* Ammunition type */
				NONE, 
				
				/* Ticks per round, recovery ticks, charging ticks */
				NONE, TICKS_PER_SECOND/3, 0,
				
				/* recoil magnitude */
				0,
				
				/* firing, click, charging, shell casing, reload sound */
				NONE, NONE, NONE, NONE, NONE, NONE,
				
				/* projectile type */
				_projectile_fist,
				
				/* theta error */
				0,
				
				/* dx, dz */
				0, 0,
				
				/* shell casing type */
				NONE, 

				/* burst count */
				0
			}
		}
	},

	/* flamethrower */
	{
		/* item type, powerup type, item class, item flags */
		_i_flamethrower, NONE, _normal_class, _weapon_is_automatic,

		3*FIXED_ONE/4, TICKS_PER_SECOND/3, /* firing intensity, firing decay */

		/* idle height, bob amplitude, kick height, reload height */
		FIXED_ONE, FIXED_ONE/35, FIXED_ONE/15, FIXED_ONE/2,
		
		/* horizontal positioning.. */
		FIXED_ONE_HALF, 0,

		/* collection, idle, firing, reloading shapes; shell casing, charging, charged */
		_weapon_in_hand_collection,
		_flamethrower_idle, _flamethrower_firing, NONE,
		NONE,
		NONE, NONE,

		/* ready/await/load/finish/powerup rounds ticks */
		TICKS_PER_SECOND, (2*TICKS_PER_SECOND)/3, (2*TICKS_PER_SECOND)/3, (2*TICKS_PER_SECOND)/3, 0,

		{
			{
				/* rounds per magazine */
				7*TICKS_PER_SECOND, 
	
				/* Ammunition type */
				_i_flamethrower_canister, 
				
				/* Ticks per round, recovery ticks, charging ticks */
				NONE, TICKS_PER_SECOND/3, 0,
				
				/* recoil magnitude */
				2,
				
				/* firing, click, charging, shell casing, reload sound */
				_snd_flamethrower, NONE, NONE, NONE, NONE, NONE,
				
				/* projectile type */
				_projectile_flamethrower_burst,
				
				/* theta error */
				0,
				
				/* dx, dz */
				20, -50,
				
				/* shell casing type */
				NONE, 

				/* burst count */
				0
			},
			
			/* unused */
			{
				/* rounds per magazine */
				1, 
	
				/* Ammunition type */
				NONE, 
				
				/* Ticks per round, recovery ticks, charging ticks */
				NONE, TICKS_PER_SECOND/3, 0,
				
				/* recoil magnitude */
				0,
				
				/* firing, click, charging, shell casing, reload sound */
				NONE, NONE, NONE, NONE, NONE, NONE,
				
				/* projectile type */
				_projectile_fist,
				
				/* theta error */
				0,
				
				/* dx, dz */
				0, 0,
				
				/* shell casing type */
				NONE, 

				/* burst count */
				0
			}
		}
	},

	/* alien weapon */
	{
		/* item type, powerup type, item class, item flags */
		_i_alien_shotgun, NONE, _multipurpose_class, _weapon_is_automatic | _weapon_disappears_after_use | _weapon_has_random_ammo_on_pickup | _weapon_triggers_share_ammo | _weapon_secondary_has_angular_flipping,

		3*FIXED_ONE/4, TICKS_PER_SECOND/5, /* firing intensity, firing decay */

		/* idle height, bob amplitude, kick height, reload height */
		FIXED_ONE+FIXED_ONE/6, FIXED_ONE/35, FIXED_ONE/16, 3*FIXED_ONE/4,

		/* horizontal positioning.. */
		FIXED_ONE_HALF, 0,
		
		/* collection, idle, firing, reloading shapes; shell casing, charging, charged */
		_weapon_in_hand_collection,
		_alien_weapon_idle, _alien_weapon_firing, NONE,
		NONE,
		NONE, NONE,

		/* ready/await/load/finish/powerup rounds ticks */
		TICKS_PER_SECOND/2, TICKS_PER_SECOND/3, TICKS_PER_SECOND/3, TICKS_PER_SECOND/3, 0,

		{
			{
				/* rounds per magazine */
				2000, 
	
				/* Ammunition type */
				_i_alien_shotgun_magazine, 
				
				/* Ticks per round, recovery ticks, charging ticks */
				NONE, 0, 0,
				
				/* recoil magnitude */
				5,
				
				/* firing, click, charging, shell casing, reload sound */
				_snd_enforcer_attack, NONE, NONE, NONE, NONE, NONE,
				
				/* projectile type */
				_projectile_alien_weapon,
				
				/* theta error */
				1,
				
				/* dx, dz */
				0, -8*NORMAL_WEAPON_DZ,
				
				/* shell casing type */
				NONE,

				/* burst count */
				0
			},

			{
				/* rounds per magazine */
				50, 
	
				/* Ammunition type */
				_i_alien_shotgun_magazine, 
				
				/* Ticks per round, recovery ticks, charging ticks */
				NONE, 0, 0,
				
				/* recoil magnitude */
				5,
				
				/* firing, click, charging, shell casing, reload sound */
				_snd_enforcer_attack, NONE, NONE, NONE, NONE, NONE,
				
				/* projectile type */
				_projectile_alien_weapon,
				
				/* theta error */
				1,
				
				/* dx, dz */
				0, -8*NORMAL_WEAPON_DZ,
				
				/* shell casing type */
				NONE,

				/* burst count */
				0
			}
		}
	},

#define SHOTGUN_BURST_COUNT 10
#define SHOTGUN_SPREAD 5
	/* Shotgun- dual fisted */
	{
		/* item type, powerup type, item class, item flags */
		_i_shotgun, NONE, _twofisted_pistol_class, _weapon_reloads_in_one_hand,

		3*FIXED_ONE/4, TICKS_PER_SECOND/8, /* firing intensity, firing decay */

		/* idle height, bob amplitude, kick height, reload height */
		FIXED_ONE+FIXED_ONE/15, FIXED_ONE/25, FIXED_ONE/8, FIXED_ONE,
		
		/* horizontal positioning.. */
		FIXED_ONE_HALF, 0,

		/* collection, idle, firing, reloading shapes; shell casing, charging, charged */
		_weapon_in_hand_collection,
		_shotgun_idle, _shotgun_firing, _shotgun_reloading,
		NONE,
		NONE, NONE,

		/* ready/await/load/finish/powerup rounds ticks */
		TICKS_PER_SECOND/3, 5, 5, 5, 0, // was NONE

		{
			{
				/* rounds per magazine */
				SHOTGUN_BURST_COUNT, 
	
				/* Ammunition type */
				_i_shotgun_magazine, 
				
				/* Ticks per round, recovery ticks, charging ticks */
				NONE, TICKS_PER_SECOND/3, 0,
				
				/* recoil magnitude */
				25,
				
				/* firing, click, charging, shell casing, reload sound */
				_snd_shotgun_firing, _snd_empty_gun, NONE, NONE, _snd_shotgun_reloading, NONE,
				
				/* projectile type */
				_projectile_shotgun_bullet,
				
				/* theta error */
				SHOTGUN_SPREAD,
				
				/* dx, dz */
				(WORLD_ONE_FOURTH/6), -NORMAL_WEAPON_DZ,		/* Primary */
				
				/* shell casing type */
				NONE, 

				/* burst count */
				SHOTGUN_BURST_COUNT
			},
			
			/* left weapon (for consistency)... */
			{
				/* rounds per magazine */
				SHOTGUN_BURST_COUNT, 
	
				/* Ammunition type */
				_i_shotgun_magazine, 
				
				/* Ticks per round, recovery ticks, charging ticks */
				NONE, TICKS_PER_SECOND/3, 0,
				
				/* recoil magnitude */
				25,
				
				/* firing, click, charging, shell casing, reload sound */
				_snd_shotgun_firing, _snd_empty_gun, NONE, NONE, _snd_shotgun_reloading, NONE,
				
				/* projectile type */
				_projectile_shotgun_bullet,
				
				/* theta error */
				SHOTGUN_SPREAD,
				
				/* dx, dz */
				(WORLD_ONE_FOURTH/6), -NORMAL_WEAPON_DZ,		/* Primary */
				
				/* shell casing type */
				NONE, 

				/* burst count */
				SHOTGUN_BURST_COUNT
			}
		}
	},
	
	/* The Ball- Don't Drop It. */
	{
		/* item type, powerup type, item class, item flags */
		_i_red_ball, NONE, _normal_class, 0,

		0, 0, /* firing intensity, firing decay */

		/* idle height, bob amplitude, kick height, reload height */
		FIXED_ONE+FIXED_ONE/15, FIXED_ONE/15, FIXED_ONE/16, 0,
		
		/* horizontal positioning.. */
		FIXED_ONE_HALF, 0,

		/* collection, idle, firing, reloading shapes; shell casing, charging, charged */
		_collection_player,
		30, 30, 30,
		NONE,
		NONE, NONE,

		/* ready/await/load/finish/powerup rounds ticks */
		TICKS_PER_SECOND/4, 0, 0, 0, 0,

		{
			{
				/* rounds per magazine */
				1, 
	
				/* Ammunition type */
				NONE, 
				
				/* Ticks per round, recovery ticks, charging ticks */
				NONE, TICKS_PER_SECOND/3, 0,
				
				/* recoil magnitude */
				0,
				
				/* firing, click, charging, shell casing, reload sound */
				NONE, NONE, NONE, NONE, NONE, NONE,
				
				/* projectile type */
				_projectile_ball_dropped,
				
				/* theta error */
				0,
				
				/* dx, dz */
				0, -150,		/* Primary */

				/* shell casing type */
				NONE, 
				
				/* burst count */
				0
			},
			
			{
				/* rounds per magazine */
				1, 
	
				/* Ammunition type */
				NONE, 
				
				/* Ticks per round, recovery ticks, charging ticks */
				NONE, TICKS_PER_SECOND/3, 0,
				
				/* recoil magnitude */
				0,
				
				/* firing, click, charging, shell casing, reload sound */
				NONE, NONE, NONE, NONE, NONE, NONE,
				
				/* projectile type */
				_projectile_ball_dropped,
				
				/* theta error */
				0,
				
				/* dx, dz */
				0, 0,		/* Primary */

				/* shell casing type */
				NONE, 
				
				/* burst count */
				0
			}
		}
	},

	/* LP addition: SMG */
	{
		/* item type, powerup type, item class, item flags */
		_i_smg, NONE, _normal_class, _weapon_is_automatic|_weapon_fires_under_media,

		3*FIXED_ONE/4, TICKS_PER_SECOND/5, /* firing intensity, firing decay */


		/* idle height, bob amplitude, kick height, reload height */
		FIXED_ONE+FIXED_ONE/6, FIXED_ONE/35, FIXED_ONE/16, 3*FIXED_ONE/4,

		/* horizontal positioning.. */
		FIXED_ONE_HALF, 0,
		
		/* collection, idle, firing, reloading shapes; shell casing, charging, charged */
		_weapon_in_hand_collection,
		_smg_idle, _smg_firing, _smg_reloading,
		NONE,
		NONE, NONE,

		/* ready/await/load/finish/powerup rounds ticks */
		TICKS_PER_SECOND/2, TICKS_PER_SECOND/3, TICKS_PER_SECOND/3, TICKS_PER_SECOND/3, 0,

		{
			{
				/* rounds per magazine */
				32, 
	
				/* Ammunition type */
				_i_smg_ammo, 
				
				/* Ticks per round, recovery ticks, charging ticks */
				NONE, 0, 0,
				
				/* recoil magnitude */
				5,
				
				/* firing, click, charging, shell casing, reload sound */
				_snd_smg_firing, _snd_empty_gun, NONE, _snd_assault_rifle_shell_casings, _snd_smg_reloading, NONE,
				
				/* projectile type */
				_projectile_smg_bullet,
				
				/* theta error */
				3,
				
				/* dx, dz */
				0, -NORMAL_WEAPON_DZ,
				
				/* shell casing type */
				_shell_casing_smg,

				/* burst count */
				2
			},
			{
				/* rounds per magazine */
				32, 
	
				/* Ammunition type */
				_i_smg_ammo, 
				
				/* Ticks per round, recovery ticks, charging ticks */
				NONE, 0, 0,
				
				/* recoil magnitude */
				5,
				
				/* firing, click, charging, shell casing, reload sound */
				_snd_smg_firing, _snd_empty_gun, NONE, _snd_assault_rifle_shell_casings, _snd_smg_reloading, NONE,
				
				/* projectile type */
				_projectile_smg_bullet,
				
				/* theta error */
				3,
				
				/* dx, dz */
				0, -NORMAL_WEAPON_DZ,
				
				/* shell casing type */
				_shell_casing_smg,

				/* burst count */
				2
			}
		}
	},
};

// Added for the convenience of the 1-2-3 Converter
uint8 *unpack_weapon_definition(uint8 *Stream, weapon_definition *Objects, size_t Count);
uint8 *pack_weapon_definition(uint8 *Stream, weapon_definition *Objects, size_t Count);

#endif
