#ifndef __LIGHTSOURCE_H
#define __LIGHTSOURCE_H

/*
LIGHTSOURCE.H
Wednesday, February 1, 1995 4:22:23 AM  (Jason')

Jul 1, 2000 (Loren Petrich):
	Made lights accessor an inline function
*/

/* ---------- constants */

#define MAXIMUM_LIGHTS_PER_MAP 64

enum /* default light types */
{
	_normal_light,
	_strobe_light,
	_media_light,
	NUMBER_OF_LIGHT_TYPES
};

enum /* states */
{
	_light_becoming_active,
	_light_primary_active,
	_light_secondary_active,
	_light_becoming_inactive,
	_light_primary_inactive,
	_light_secondary_inactive
};

/* ---------- static light data */

enum /* lighting functions */
{
	_constant_lighting_function, // maintain final intensity for period
	_linear_lighting_function, // linear transition between initial and final intensity over period
	_smooth_lighting_function, // sine transition between initial and final intensity over period
	_flicker_lighting_function, // intensity in [smooth_intensity(t),final_intensity]
	NUMBER_OF_LIGHTING_FUNCTIONS
};

/* as intensities, transition functions are given the primary periods of the active and inactive
	state, plus the intensity at the time of transition */
struct lighting_function_specification /* 7*2 == 14 bytes */
{
	short function;
	
	short period, delta_period;
	fixed intensity, delta_intensity;
};

enum /* static flags */
{
	_light_is_initially_active,
	_light_has_slaved_intensities,
	_light_is_stateless,
	NUMBER_OF_STATIC_LIGHT_FLAGS /* <=16 */
};

#define LIGHT_IS_INITIALLY_ACTIVE(s) TEST_FLAG16((s)->flags, _light_is_initially_active)
#define LIGHT_IS_STATELESS(s) TEST_FLAG16((s)->flags, _light_is_stateless)

#define SET_LIGHT_IS_INITIALLY_ACTIVE(s, v) SET_FLAG16((s)->flags, _light_is_initially_active, (v))
#define SET_LIGHT_IS_STATELESS(s, v) SET_FLAG16((s)->flags, _light_is_stateless, (v))

struct static_light_data /* 8*2 + 6*14 == 100 bytes */
{
	short type;
	uint16 flags;

	short phase; // initializer, so lights may start out-of-phase with each other
	
	struct lighting_function_specification primary_active, secondary_active, becoming_active;
	struct lighting_function_specification primary_inactive, secondary_inactive, becoming_inactive;
	
	short tag;
	
	short unused[4];
};

/* ---------- dynamic light data */

struct light_data /* 14*2 + 100 == 128 bytes */
{
	uint16 flags;
	short state;
	
	// result of lighting function
	fixed intensity;
	
	// data recalculated each function changed; passed to lighting_function each update
	short phase, period;
	fixed initial_intensity, final_intensity;

	short unused[4];

	struct static_light_data static_data;
};

/* --------- globals */

extern struct light_data *lights;

/* --------- prototypes/LIGHTSOURCE.C */

short new_light(struct static_light_data *data);
struct static_light_data *get_defaults_for_light_type(short type);

void update_lights(void);

boolean get_light_status(short light_index);
boolean set_light_status(short light_index, boolean active);
boolean set_tagged_light_statuses(short tag, boolean new_status);

fixed get_light_intensity(short light_index);

// LP change: inlined it
inline struct light_data *get_light_data(
	const short light_index)
{
	struct light_data *light = GetMemberWithBounds(lights,light_index,MAXIMUM_LIGHTS_PER_MAP);
	
	if (!light) return NULL;
	if (!SLOT_IS_USED(light)) return NULL;
	
	return light;
}

/*
#ifdef DEBUG
struct light_data *get_light_data(short light_index);
#else
#define get_light_data(i) (lights+(i))
#endif
*/

#endif
