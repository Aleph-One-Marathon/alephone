#ifndef __MEDIA_H
#define __MEDIA_H

/*
MEDIA.H
Saturday, March 25, 1995 1:27:18 AM  (Jason')

Feb 3, 2000 (Loren Petrich):
	Added Jjaro media type

May 17, 2000 (Loren Petrich):
	Added XML-parser support

June 3, 2000 (Loren Petrich):
	Made accessors return the null pointer for invalid index values;
	also added is-media-dangerous function for use in determining whether a monster
	should step into it.

July 1, 2000 (Loren Petrich):
	Inlined the media accessors; added map.h here to define SLOT_IS_USED
*/

#include "map.h"
#include "XML_ElementParser.h"

/* ---------- constants */

#define MAXIMUM_MEDIAS_PER_MAP 16

// LP addition: added JjaroGoo support
enum /* media types */
{
	_media_water,
	_media_lava,
	_media_goo,
	_media_sewage,
	_media_jjaro,
	NUMBER_OF_MEDIA_TYPES
};

enum /* media flags */
{
	_media_sound_obstructed_by_floor, // this media makes no sound when under the floor
	
	NUMBER_OF_MEDIA_FLAGS /* <= 16 */
};

#define MEDIA_SOUND_OBSTRUCTED_BY_FLOOR(m) TEST_FLAG16((m)->flags, _media_sound_obstructed_by_floor)

#define SET_MEDIA_SOUND_OBSTRUCTED_BY_FLOOR(m, v) SET_FLAG16((m)->flags, _media_sound_obstructed_by_floor, (v))

enum /* media detonation types */
{
	_small_media_detonation_effect,
	_medium_media_detonation_effect,
	_large_media_detonation_effect,
	_large_media_emergence_effect,
	NUMBER_OF_MEDIA_DETONATION_TYPES
};

enum /* media sounds */
{
	_media_snd_feet_entering,
	_media_snd_feet_leaving,
	_media_snd_head_entering,
	_media_snd_head_leaving,
	_media_snd_splashing,
	_media_snd_ambient_over,
	_media_snd_ambient_under,
	_media_snd_platform_entering,
	_media_snd_platform_leaving,
	
	NUMBER_OF_MEDIA_SOUNDS
};

/* ---------- macros */

#define UNDER_MEDIA(m, z) ((z)<=(m)->height)

/* ---------- structures */

struct media_data /* 32 bytes */
{
	int16 type;
	uint16 flags;

	/* this light is not used as a real light; instead, the intensity of this light is used to
		determine the height of the media: height= low + (high-low)*intensity ... this sounds
		gross, but it makes media heights as flexible as light intensities; clearly discontinuous
		light functions (e.g., strobes) should not be used */
	int16 light_index;

	/* this is the maximum external velocity due to current; acceleration is 1/32nd of this */
	angle current_direction;
	world_distance current_magnitude;
	
	world_distance low, high;
	
	world_point2d origin;
	world_distance height;

	fixed minimum_light_intensity;
	shape_descriptor texture;
	int16 transfer_mode;
	
	int16 unused[2];
};
const int SIZEOF_media_data = 32;

/* --------- globals */

extern struct media_data *medias;

/* --------- prototypes/MEDIA.C */

short new_media(struct media_data *data);

void update_medias(void);

void get_media_detonation_effect(short media_index, short type, short *detonation_effect);
short get_media_sound(short media_index, short type);
short get_media_submerged_fade_effect(short media_index);
struct damage_definition *get_media_damage(short media_index, fixed scale);

// LP addition: media dangerous?
bool IsMediaDangerous(short media_type);

bool media_in_environment(short media_type, short environment_code);

// LP change: made this inline
inline struct media_data *get_media_data(
	const short media_index)
{
	struct media_data *media = GetMemberWithBounds(medias,media_index,MAXIMUM_MEDIAS_PER_MAP);
	
	if (!media) return NULL;
	if (!(SLOT_IS_USED(media))) return NULL;
	
	return media;
}

/*
#ifdef DEBUG
struct media_data *get_media_data(short media_index);
#else
#define get_media_data(i) (medias+(i))
#endif
*/

// LP addition: count number of media types used,
// for better Infinity compatibility when saving games
short count_number_of_medias_used();

// LP change: added liquid-parser export
XML_ElementParser *Liquids_GetParser();

#endif

