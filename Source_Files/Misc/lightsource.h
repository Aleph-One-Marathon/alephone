#ifndef __LIGHTSOURCE_H
#define __LIGHTSOURCE_H

/*
LIGHTSOURCE.H
Wednesday, February 1, 1995 4:22:23 AM  (Jason')

Jul 1, 2000 (Loren Petrich):
	Made lights accessor an inline function

Aug 29, 2000 (Loren Petrich):
	Added packing routines for the light data; also moved old light stuff (M1) here
*/

#include <vector>

/* ---------- constants */

// #define MAXIMUM_LIGHTS_PER_MAP 64

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
	int16 function;
	
	int16 period, delta_period;
	_fixed intensity, delta_intensity;
};

// Misaligned 4-byte values (intensity, delta_intensity) split in it
struct saved_lighting_function /* 7*2 == 14 bytes */
{
	int16 function;
	
	int16 period, delta_period;
	uint16 intensity[2], delta_intensity[2];
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

struct static_light_data /* size platform-specific */
{
	int16 type;
	uint16 flags;
	int16 phase; // initializer, so lights may start out-of-phase with each other
	
	struct lighting_function_specification primary_active, secondary_active, becoming_active;
	struct lighting_function_specification primary_inactive, secondary_inactive, becoming_inactive;
	
	int16 tag;
	
	int16 unused[4];
};
const int SIZEOF_static_light_data = 100;

// Misaligned 4-byte values (in lighting_function_specification) split in it
struct saved_static_light /* 8*2 + 6*14 == 100 bytes */
{
	int16 type;
	uint16 flags;

	int16 phase; // initializer, so lights may start out-of-phase with each other
	
	struct saved_lighting_function primary_active, secondary_active, becoming_active;
	struct saved_lighting_function primary_inactive, secondary_inactive, becoming_inactive;
	
	int16 tag;
	
	int16 unused[4];
};

/* ---------- dynamic light data */

struct light_data /* 14*2 + 100 == 128 bytes */
{
	uint16 flags;
	int16 state;
	
	// result of lighting function
	_fixed intensity;
	
	// data recalculated each function changed; passed to lighting_function each update
	int16 phase, period;
	_fixed initial_intensity, final_intensity;

	int16 unused[4];

	struct static_light_data static_data;
};
const int SIZEOF_light_data = 128;

/* --------- Marathon 1 light definitions */

enum /* old light types */
{
	_light_is_normal,
	_light_is_rheostat,
	_light_is_flourescent,
	_light_is_strobe, 
	_light_flickers,
	_light_pulsates,
	_light_is_annoying,
	_light_is_energy_efficient
};

/* Borrowed from the old lightsource.h, to allow Marathon II to open/use Marathon I maps */
struct old_light_data {
	uint16 flags;
	
	int16 type;
	int16 mode; /* on, off, etc. */
	int16 phase;
	
	_fixed minimum_intensity, maximum_intensity;
	int16 period; /* on, in ticks (turning on and off periods are always the same for a given light type,
		or else are some function of this period) */
	
	_fixed intensity; /* current intensity */
	
	int16 unused[5];	
};
const int SIZEOF_old_light_data = 32;

/* --------- globals */

// Turned the list of lights into a variable array;
// took over their maximum number as how many of them

extern vector<light_data> LightList;
#define lights (&LightList[0])
#define MAXIMUM_LIGHTS_PER_MAP (LightList.size())

// extern struct light_data *lights;

/* --------- prototypes/LIGHTSOURCE.C */

short new_light(struct static_light_data *data);
struct static_light_data *get_defaults_for_light_type(short type);

void update_lights(void);

bool get_light_status(short light_index);
bool set_light_status(short light_index, bool active);
bool set_tagged_light_statuses(short tag, bool new_status);

_fixed get_light_intensity(short light_index);

light_data *get_light_data(
	const short light_index);

// Split and join the misaligned 4-byte values
uint8 *pack_light_data(static_light_data& source, saved_static_light& dest);
uint8 *unpack_light_data(saved_static_light& source, static_light_data& dest);

uint8 *unpack_old_light_data(uint8 *Stream, old_light_data* Objects, int Count);
uint8 *pack_old_light_data(uint8 *Stream, old_light_data* Objects, int Count);
uint8 *unpack_static_light_data(uint8 *Stream, static_light_data* Objects, int Count);
uint8 *pack_static_light_data(uint8 *Stream, static_light_data* Objects, int Count);
uint8 *unpack_light_data(uint8 *Stream, light_data* Objects, int Count);
uint8 *pack_light_data(uint8 *Stream, light_data* Objects, int Count);

void convert_old_light_data_to_new(static_light_data* NewLights, old_light_data* OldLights, int Count);

#endif
