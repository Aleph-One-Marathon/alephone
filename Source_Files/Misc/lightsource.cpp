/*
LIGHTSOURCE.C
Wednesday, February 1, 1995 4:21:43 AM  (Jason')

Monday, March 6, 1995 9:41:50 PM  (Jason')
	nearly finished; looking toward cataclysm.  we need a good interface for editing intensities:
	a _normal_light only has two intensities, but each is found three places.
Thursday, April 27, 1995 11:00:36 AM  (Jason')
	functions with zero periods are skipped.
Tuesday, June 13, 1995 6:13:29 PM  (Jason)
	support for phases greater than a light’s initial period
Monday, July 10, 1995 5:20:26 PM  (Jason)
	stateless (six phase) lights.

June 2, 2000 (Loren Petrich):
	Added fallback for absent lights

July 1, 2000 (Loren Petrich):
	Modified light accessors to be more C++-like
*/

#include "cseries.h"

#include "map.h"
#include "lightsource.h"

#ifdef env68k
#pragma segment marathon
#endif

/* ---------- globals */

struct light_data *lights;

/* ---------- private prototypes */


/*
#ifdef DEBUG
static struct light_definition *get_light_definition(short type);
#else
#define get_light_definition(t) (light_definitions+(t))
#endif
*/

static void rephase_light(short light_index);

// LP: "static" removed
void change_light_state(short light_index, short new_state);
static struct lighting_function_specification *get_lighting_function_specification(
	struct static_light_data *data, short state);

static fixed lighting_function_dispatch(short function_index, fixed initial_intensity,
	fixed final_intensity, short phase, short period);

/* ---------- structures */

struct light_definition
{
	// it remains unclear where these sounds should come from
	short on_sound, off_sound;
	
	struct static_light_data defaults;
};

/* ---------- globals */

struct light_definition light_definitions[NUMBER_OF_LIGHT_TYPES]=
{
	// _normal_light
	{
		NONE, NONE, // on, off sound
		{
			_normal_light, // type
			FLAG(_light_is_initially_active)|FLAG(_light_has_slaved_intensities), 0, // flags, phase
			
			{_constant_lighting_function, TICKS_PER_SECOND, 0, FIXED_ONE, 0}, // primary_active
			{_constant_lighting_function, TICKS_PER_SECOND, 0, FIXED_ONE, 0}, // secondary_active
			{_smooth_lighting_function, TICKS_PER_SECOND, 0, FIXED_ONE, 0}, // becoming_active

			{_constant_lighting_function, TICKS_PER_SECOND, 0, 0, 0}, // primary_inactive
			{_constant_lighting_function, TICKS_PER_SECOND, 0, 0, 0}, // secondary_inactive
			{_smooth_lighting_function, TICKS_PER_SECOND, 0, 0, 0}, // becoming_inactive
		}
	},

	// _strobe_light
	{
		NONE, NONE, // on, off sound
		{
			_normal_light, // type
			FLAG(_light_is_initially_active)|FLAG(_light_has_slaved_intensities), 0, // flags, phase
			
			{_constant_lighting_function, TICKS_PER_SECOND/2, 0, FIXED_ONE, 0}, // primary_active
			{_constant_lighting_function, TICKS_PER_SECOND/2, 0, FIXED_ONE_HALF, 0}, // secondary_active
			{_smooth_lighting_function, TICKS_PER_SECOND, 0, FIXED_ONE_HALF, 0}, // becoming_active

			{_constant_lighting_function, TICKS_PER_SECOND, 0, 0, 0}, // primary_inactive
			{_constant_lighting_function, TICKS_PER_SECOND, 0, 0, 0}, // secondary_inactive
			{_smooth_lighting_function, TICKS_PER_SECOND, 0, 0, 0}, // becoming_inactive
		}
	},

	// _lava_light
	{
		NONE, NONE, // on, off sound
		{
			_normal_light, // type
			FLAG(_light_is_initially_active)|FLAG(_light_has_slaved_intensities), 0, // flags, phase
			
			{_smooth_lighting_function, 10*TICKS_PER_SECOND, 0, FIXED_ONE, 0}, // primary_active
			{_smooth_lighting_function, 10*TICKS_PER_SECOND, 0, 0, 0}, // secondary_active
			{_smooth_lighting_function, TICKS_PER_SECOND, 0, FIXED_ONE_HALF, 0}, // becoming_active

			{_constant_lighting_function, TICKS_PER_SECOND, 0, 0, 0}, // primary_inactive
			{_constant_lighting_function, TICKS_PER_SECOND, 0, 0, 0}, // secondary_inactive
			{_smooth_lighting_function, TICKS_PER_SECOND, 0, 0, 0}, // becoming_inactive
		}
	},
};

/* ---------- code */

// LP change: moved down here because it uses light definitions
inline struct light_definition *get_light_definition(const short type)
{
	return GetMemberWithBounds(light_definitions,type,NUMBER_OF_LIGHT_TYPES);
}


short new_light(
	struct static_light_data *data)
{
	short light_index;
	struct light_data *light;
	
	// LP change: idiot-proofing
	if (!data) return NONE;
	
	for (light_index= 0, light= lights; light_index<MAXIMUM_LIGHTS_PER_MAP; ++light_index, ++light)
	{
		if (SLOT_IS_FREE(light))
		{
			light->static_data= *data;
//			light->flags= 0;
			MARK_SLOT_AS_USED(light);
			
			light->intensity= 0;
			change_light_state(light_index, LIGHT_IS_INITIALLY_ACTIVE(data) ? _light_secondary_active : _light_secondary_inactive);
			light->intensity= light->final_intensity;
			change_light_state(light_index, LIGHT_IS_INITIALLY_ACTIVE(data) ? _light_primary_active : _light_primary_inactive);

			light->phase= data->phase;
			rephase_light(light_index);		
				
			light->intensity= lighting_function_dispatch(get_lighting_function_specification(&light->static_data, light->state)->function,
				light->initial_intensity, light->final_intensity, light->phase, light->period);

			break;
		}
	}
	if (light_index==MAXIMUM_LIGHTS_PER_MAP) light_index= NONE;
	
	return light_index;
}

struct static_light_data *get_defaults_for_light_type(
	short type)
{
	struct light_definition *definition= get_light_definition(type);
	// LP addition: idiot-proofing
	if (!definition) return NULL;
	
	return &definition->defaults;
}

void update_lights(
	void)
{
	short light_index;
	struct light_data *light;
	
	for (light_index= 0, light= lights; light_index<MAXIMUM_LIGHTS_PER_MAP; ++light_index, ++light)
	{
		if (SLOT_IS_USED(light))
		{
			/* update light phase; if we’ve overflowed our period change to the next state */
			light->phase+= 1;
			rephase_light(light_index);
			
			/* calculate and remember intensity for this ii, fi, phase, period */
			light->intensity= lighting_function_dispatch(get_lighting_function_specification(&light->static_data, light->state)->function,
				light->initial_intensity, light->final_intensity, light->phase, light->period);
		}
	}

	return;
}

boolean get_light_status(
	short light_index)
{
	struct light_data *light= get_light_data(light_index);
	// LP change: idiot-proofing
	if (!light) return false;
	
	boolean status;
	
	switch (light->state)
	{
		case _light_becoming_active:
		case _light_primary_active:
		case _light_secondary_active:
			status= TRUE;
			break;
		
		case _light_becoming_inactive:
		case _light_primary_inactive:
		case _light_secondary_inactive:
			status= FALSE;
			break;

		default:
			vhalt(csprintf(temporary, "what is light state #%d?", light->state));
			break;
	}
	
	return status;
}

boolean set_light_status(
	short light_index,
	boolean new_status)
{
	struct light_data *light= get_light_data(light_index);
	// LP change: idiot-proofing
	if (!light) return false;
	
	boolean old_status= get_light_status(light_index);
	boolean changed= FALSE;
	
	if ((new_status&&!old_status) || (!new_status&&old_status))
	{
		if (!LIGHT_IS_STATELESS(light))
		{
			change_light_state(light_index, new_status ? _light_becoming_active : _light_becoming_inactive);
			assume_correct_switch_position(_panel_is_light_switch, light_index, new_status);
			changed= TRUE;
		}
	}
	
	return changed;
}

boolean set_tagged_light_statuses(
	short tag,
	boolean new_status)
{
	boolean changed= FALSE;

	if (tag)
	{
		short light_index;
		struct light_data *light;
		
		for (light_index= 0, light= lights; light_index<MAXIMUM_LIGHTS_PER_MAP; ++light_index, ++light)
		{
			if (light->static_data.tag==tag)
			{
				if (set_light_status(light_index, new_status))
				{
					changed= TRUE;
				}
			}
		}
	}
	
	return changed;
}

fixed get_light_intensity(
	short light_index)
{
	// LP change: idiot-proofing / fallback
	light_data *light = get_light_data(light_index);
	if (!light) return 0;	// Blackness
	
	return light->intensity;
	// return get_light_data(light_index)->intensity;
}

/*
#ifdef DEBUG
struct light_data *get_light_data(
	short light_index)
{
	struct light_data *light;
	
	// LP change: made this return NULL
	if (!(light_index>=0&&light_index<MAXIMUM_LIGHTS_PER_MAP)) return NULL;;
	// vassert(light_index>=0&&light_index<MAXIMUM_LIGHTS_PER_MAP, csprintf(temporary, "light index #%d is out of range", light_index));
	
	light= lights+light_index;
	if (!SLOT_IS_USED(light)) return NULL;
	/// vassert(SLOT_IS_USED(light), csprintf(temporary, "light index #%d is unused", light_index));
	
	return light;
}
#endif
*/

/* ---------- private code */

/*
#ifdef DEBUG
static struct light_definition *get_light_definition(
	short type)
{
	// LP change: put in fallback
	if (!(type>=0&&type<NUMBER_OF_LIGHT_TYPES)) return NULL;
	// assert(type>=0&&type<NUMBER_OF_LIGHT_TYPES);
	return light_definitions+type;
}
#endif
*/

/* given a state, initialize .phase, .period, .initial_intensity, and .final_intensity */
// LP: "static" removed
void change_light_state(
	short light_index,
	short new_state)
{
	struct light_data *light= get_light_data(light_index);
	// LP change: idiot-proofing
	if (!light) return;
	struct lighting_function_specification *function= get_lighting_function_specification(&light->static_data, new_state);
	
	light->phase= 0;
	light->period= function->period + global_random()%(function->delta_period+1);
	
	light->initial_intensity= light->intensity;
	light->final_intensity= function->intensity + global_random()%(function->delta_intensity+1);
	
	light->state= new_state;
	
	return;
}

static struct lighting_function_specification *get_lighting_function_specification(
	struct static_light_data *data,
	short state)
{
	struct lighting_function_specification *function;
	
	switch (state)
	{
		case _light_becoming_active: function= &data->becoming_active; break;
		case _light_primary_active: function= &data->primary_active; break;
		case _light_secondary_active: function= &data->secondary_active; break;
		case _light_becoming_inactive: function= &data->becoming_inactive; break;
		case _light_primary_inactive: function= &data->primary_inactive; break;
		case _light_secondary_inactive: function= &data->secondary_inactive; break;
		default: vhalt(csprintf(temporary, "what is light state #%d?", state));
	}
	
	return function;
}

static void rephase_light(
	short light_index)
{
	struct light_data *light= get_light_data(light_index);
	// LP change: idiot-proofing
	if (!light) return;
	short phase= light->phase;
	
	while (phase>=light->period)
	{
		short new_state;
		
		phase-= light->period;
		
		switch (light->state)
		{
			case _light_becoming_active: new_state= _light_primary_active; break;
			case _light_primary_active: new_state= _light_secondary_active; break;
			case _light_secondary_active: new_state= LIGHT_IS_STATELESS(light) ? _light_becoming_inactive : _light_primary_active; break;
			case _light_becoming_inactive: new_state= _light_primary_inactive; break;
			case _light_primary_inactive: new_state= _light_secondary_inactive; break;
			case _light_secondary_inactive: new_state= LIGHT_IS_STATELESS(light) ? _light_becoming_active : _light_primary_inactive; break;
			default: vhalt(csprintf(temporary, "what is light state #%d?", light->state));
		}
		
		change_light_state(light_index, new_state);
	}
	light->phase= phase;
	
	return;
}
				
/* ---------- lighting functions */

static fixed constant_lighting_proc(fixed initial_intensity, fixed final_intensity, short phase, short period);
static fixed linear_lighting_proc(fixed initial_intensity, fixed final_intensity, short phase, short period);
static fixed smooth_lighting_proc(fixed initial_intensity, fixed final_intensity, short phase, short period);
static fixed flicker_lighting_proc(fixed initial_intensity, fixed final_intensity, short phase, short period);

typedef fixed (*lighting_function)(fixed initial_intensity, fixed final_intensity,
	short phase, short period);

static lighting_function lighting_functions[NUMBER_OF_LIGHTING_FUNCTIONS]=
{
	constant_lighting_proc,
	linear_lighting_proc,
	smooth_lighting_proc,
	flicker_lighting_proc
};

static fixed lighting_function_dispatch(
	short function_index,
	fixed initial_intensity,
	fixed final_intensity,
	short phase,
	short period)
{
	assert(function_index>=0 && function_index<NUMBER_OF_LIGHTING_FUNCTIONS);
	
	return lighting_functions[function_index](initial_intensity, final_intensity, phase, period);
}

static fixed constant_lighting_proc(
	fixed initial_intensity,
	fixed final_intensity,
	short phase,
	short period)
{
	(void) (initial_intensity, phase, period);
	
	return final_intensity;
}

static fixed linear_lighting_proc(
	fixed initial_intensity,
	fixed final_intensity,
	short phase,
	short period)
{
	return initial_intensity + ((final_intensity-initial_intensity)*phase)/period;
}

static fixed smooth_lighting_proc(
	fixed initial_intensity,
	fixed final_intensity,
	short phase,
	short period)
{
	return initial_intensity + (((final_intensity-initial_intensity)*(cosine_table[(phase*HALF_CIRCLE)/period+HALF_CIRCLE]+TRIG_MAGNITUDE))>>(TRIG_SHIFT+1));
}

static fixed flicker_lighting_proc(
	fixed initial_intensity,
	fixed final_intensity,
	short phase,
	short period)
{
	fixed smooth_intensity= smooth_lighting_proc(initial_intensity, final_intensity, phase, period);
	fixed delta= final_intensity-smooth_intensity;
	
	return smooth_intensity + (delta ? global_random()%delta : 0);
}


// Split and join the misaligned 4-byte values

#include <string.h>

inline void pack_lighting_function(lighting_function_specification& source, saved_lighting_function& dest)
{
	dest.function = source.function;
	
	dest.period = source.period;
	dest.delta_period = source.delta_period;
	
	memcpy(dest.intensity,&source.intensity,4);
	memcpy(dest.delta_intensity,&source.delta_intensity,4);
}

inline void unpack_lighting_function(saved_lighting_function& source, lighting_function_specification& dest)
{
	dest.function = source.function;
	
	dest.period = source.period;
	dest.delta_period = source.delta_period;
	
	memcpy(&dest.intensity,source.intensity,4);
	memcpy(&dest.delta_intensity,source.delta_intensity,4);
}

void pack_light_data(static_light_data& source, saved_static_light& dest)
{
	dest.type = source.type;
	dest.flags = source.flags;
	
	dest.phase = source.phase;
	
	pack_lighting_function(source.primary_active,dest.primary_active);
	pack_lighting_function(source.secondary_active,dest.secondary_active);
	pack_lighting_function(source.becoming_active,dest.becoming_active);
	pack_lighting_function(source.primary_inactive,dest.primary_inactive);
	pack_lighting_function(source.secondary_inactive,dest.secondary_inactive);
	pack_lighting_function(source.becoming_inactive,dest.becoming_inactive);
	
	dest.tag = source.tag;
}

void unpack_light_data(saved_static_light& source, static_light_data& dest)
{
	dest.type = source.type;
	dest.flags = source.flags;
	
	dest.phase = source.phase;
	
	unpack_lighting_function(source.primary_active,dest.primary_active);
	unpack_lighting_function(source.secondary_active,dest.secondary_active);
	unpack_lighting_function(source.becoming_active,dest.becoming_active);
	unpack_lighting_function(source.primary_inactive,dest.primary_inactive);
	unpack_lighting_function(source.secondary_inactive,dest.secondary_inactive);
	unpack_lighting_function(source.becoming_inactive,dest.becoming_inactive);
	
	dest.tag = source.tag;
}
