#ifndef __PROJECTILES_H
#define __PROJECTILES_H

/*
PROJECTILES.H
Tuesday, June 28, 1994 7:12:00 PM

Feb 4, 2000 (Loren Petrich):
	Added SMG bullet

Feb 6, 2000 (Loren Petrich):
	Added access to size of projectile-definition structure
	
Feb 10, 2000 (Loren Petrich):
	Added dynamic-limits setting of MAXIMUM_PROJECTILES_PER_MAP

Aug 30, 2000 (Loren Petrich):
	Added stuff for unpacking and packing
*/

/* ---------- projectile structure */

// LP addition:
#include "dynamic_limits.h"

// LP change: made this settable from the resource fork
#define MAXIMUM_PROJECTILES_PER_MAP (get_dynamic_limit(_dynamic_limit_projectiles))
// #define MAXIMUM_PROJECTILES_PER_MAP 32

enum /* projectile types */
{
	_projectile_rocket,
	_projectile_grenade,
	_projectile_pistol_bullet,
	_projectile_rifle_bullet,
	_projectile_shotgun_bullet,
	_projectile_staff,
	_projectile_staff_bolt,
	_projectile_flamethrower_burst,
	_projectile_compiler_bolt_minor,
	_projectile_compiler_bolt_major,
	_projectile_alien_weapon,
	_projectile_fusion_bolt_minor,
	_projectile_fusion_bolt_major,
	_projectile_hunter,
	_projectile_fist,
	_projectile_armageddon_sphere,
	_projectile_armageddon_electricity,
	_projectile_juggernaut_rocket,
	_projectile_trooper_bullet,
	_projectile_trooper_grenade,
	_projectile_minor_defender,
	_projectile_major_defender,
	_projectile_juggernaut_missile,
	_projectile_minor_energy_drain,
	_projectile_major_energy_drain,
	_projectile_oxygen_drain,
	_projectile_minor_hummer,
	_projectile_major_hummer,
	_projectile_durandal_hummer,
	_projectile_minor_cyborg_ball,
	_projectile_major_cyborg_ball,
	_projectile_ball,
	_projectile_minor_fusion_dispersal,
	_projectile_major_fusion_dispersal,
	_projectile_overloaded_fusion_dispersal,
	_projectile_yeti,
	_projectile_sewage_yeti,
	_projectile_lava_yeti,
	// LP additions:
	_projectile_smg_bullet,
	NUMBER_OF_PROJECTILE_TYPES
};

#define PROJECTILE_HAS_MADE_A_FLYBY(p) ((p)->flags&(uint16)0x4000)
#define SET_PROJECTILE_FLYBY_STATUS(p,v) ((void)((v)?((p)->flags|=(uint16)0x4000):((p)->flags&=(uint16)~0x4000)))

/* only used for persistent projectiles */
#define PROJECTILE_HAS_CAUSED_DAMAGE(p) ((p)->flags&(uint16)0x2000)
#define SET_PROJECTILE_DAMAGE_STATUS(p,v) ((void)((v)?((p)->flags|=(uint16)0x2000):((p)->flags&=(uint16)~0x2000)))

/* uses SLOT_IS_USED(), SLOT_IS_FREE(), MARK_SLOT_AS_FREE(), MARK_SLOT_AS_USED() macros (0x8000 bit) */

struct projectile_data /* 32 bytes */
{
	short type;
	
	short object_index;

	short target_index; /* for guided projectiles, the current target index */

	angle elevation; /* facing is stored in the projectile’s object */
	
	short owner_index; /* ownerless if NONE */
	short owner_type; /* identical to the monster type which fired this projectile (valid even if owner==NONE) */
	uint16 flags; /* [slot_used.1] [played_flyby_sound.1] [has_caused_damage.1] [unused.13] */
	
	/* some projectiles leave n contrails effects every m ticks */
	short ticks_since_last_contrail, contrail_count;

	world_distance distance_travelled;
	
	world_distance gravity; /* velocity due to gravity for projectiles affected by it */
	
	fixed damage_scale;
	
	short permutation; /* item type if we create one */
	
	short unused[2];
};
const int SIZEOF_projectile_data = 32;

const int SIZEOF_projectile_definition = 48;

/* ---------- globals */

extern struct projectile_data *projectiles;

/* ---------- prototypes/PROJECTILES.C */

bool preflight_projectile(world_point3d *origin, short polygon_index, world_point3d *vector,
	angle delta_theta, short type, short owner, short owner_type, short *target_index);
short new_projectile(world_point3d *origin, short polygon_index, world_point3d *vector,
	angle delta_theta, short type, short owner_index, short owner_type, short intended_target_index,
	fixed damage_scale);
void detonate_projectile(world_point3d *origin, short polygon_index, short type,
	short owner_index, short owner_type, fixed damage_scale);

void move_projectiles(void); /* assumes ∂t==1 tick */

void remove_projectile(short projectile_index);
void remove_all_projectiles(void);

void orphan_projectiles(short monster_index);

void mark_projectile_collections(short type, bool loading);
void load_projectile_sounds(short type);

void drop_the_ball(world_point3d *origin, short polygon_index, short owner_index,
	short owner_type, short item_type);

// LP change: made this inline
inline struct projectile_data *get_projectile_data(
	const short projectile_index)
{
	struct projectile_data *projectile =  GetMemberWithBounds(projectiles,projectile_index,MAXIMUM_PROJECTILES_PER_MAP);
	
	vassert(projectile, csprintf(temporary, "projectile index #%d is out of range", projectile_index));
	vassert(SLOT_IS_USED(projectile), csprintf(temporary, "projectile index #%d (%p) is unused", projectile_index, projectile));
	
	return projectile;
}

/*
#ifdef DEBUG
struct projectile_data *get_projectile_data(short projectile_index);
#else
#define get_projectile_data(i) (projectiles+(i))
#endif
*/

// LP: to pack and unpack this data;
// these do not make the definitions visible to the outside world

uint8 *unpack_projectile_data(uint8 *Stream, projectile_data *Objects, int Count);
uint8 *pack_projectile_data(uint8 *Stream, projectile_data *Objects, int Count);
uint8 *unpack_projectile_definition(uint8 *Stream, int Count);
uint8 *pack_projectile_definition(uint8 *Stream, int Count);

#endif

