/*
LIGHTSOURCE.C

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

Aug 29, 2000 (Loren Petrich):
	Added packing routines for the light data; also moved old light stuff (M1) here

Jul 3, 2002 (Loren Petrich):
	Added support for Pfhortran Procedure: light_activated
*/

#include "cseries.h"

#include "map.h"
#include "lightsource.h"
#include "Packing.h"

//MH: Lua scripting
#include "lua_script.h"

/* ---------- globals */

// Turned the list of lights into a variable array;
// took over their maximum number as how many of them

vector<light_data> LightList;

// struct light_data *lights = NULL;

/* ---------- private prototypes */

static void rephase_light(short light_index);

// LP: "static" removed
static struct lighting_function_specification *get_lighting_function_specification(
	struct static_light_data *data, short state);

static _fixed lighting_function_dispatch(short function_index, _fixed initial_intensity,
	_fixed final_intensity, short phase, short period);

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

static light_definition *get_light_definition(
	const short type);

/* ---------- code */


light_data *get_light_data(
	const size_t light_index)
{
	struct light_data *light = GetMemberWithBounds(lights,light_index,MAXIMUM_LIGHTS_PER_MAP);
	
	if (!light) return NULL;
	if (!SLOT_IS_USED(light)) return NULL;
	
	return light;
}

// LP change: moved down here because it uses light definitions
light_definition *get_light_definition(
	const short type)
{
	return GetMemberWithBounds(light_definitions,type,NUMBER_OF_LIGHT_TYPES);
}

short new_light(
	struct static_light_data *data)
{
	int light_index;
	struct light_data *light;
	
	// LP change: idiot-proofing
	if (!data) return NONE;
	
	for (light_index= 0, light= lights; light_index<short(MAXIMUM_LIGHTS_PER_MAP); ++light_index, ++light)
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
	if (light_index == short(MAXIMUM_LIGHTS_PER_MAP)) light_index = NONE;
	
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
	int light_index;
	struct light_data *light;
	
	for (light_index= 0, light= lights; light_index<short(MAXIMUM_LIGHTS_PER_MAP); ++light_index, ++light)
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
}

bool get_light_status(
	size_t light_index)
{
	struct light_data *light= get_light_data(light_index);
	// LP change: idiot-proofing
	if (!light) return false;
	
	bool status;
	
	switch (light->state)
	{
		case _light_becoming_active:
		case _light_primary_active:
		case _light_secondary_active:
			status= true;
			break;
		
		case _light_becoming_inactive:
		case _light_primary_inactive:
		case _light_secondary_inactive:
			status= false;
			break;

		default:
			vhalt(csprintf(temporary, "what is light state #%d?", light->state));
			break;
	}
	
	return status;
}

bool set_light_status(
	size_t light_index,
	bool new_status)
{
	struct light_data *light= get_light_data(light_index);
	// LP change: idiot-proofing
	if (!light) return false;
	
	bool old_status= get_light_status(light_index);
	bool changed= false;
	
	if ((new_status&&!old_status) || (!new_status&&old_status))
	{
		if (!LIGHT_IS_STATELESS(light))
		{
			change_light_state(light_index, new_status ? _light_becoming_active : _light_becoming_inactive);
			assert(light_index == static_cast<size_t>(static_cast<short>(light_index)));
                        //MH: Lua script hook
                        L_Call_Light_Activated(light_index);
			assume_correct_switch_position(_panel_is_light_switch, static_cast<short>(light_index), new_status);
			changed= true;
		}
	}
	
	return changed;
}

bool set_tagged_light_statuses(
	short tag,
	bool new_status)
{
	bool changed= false;

	if (tag)
	{
		int light_index;
		struct light_data *light;
		
		for (light_index= 0, light= lights; light_index<short(MAXIMUM_LIGHTS_PER_MAP); ++light_index, ++light)
		{
			if (light->static_data.tag==tag)
			{
				if (set_light_status(light_index, new_status))
				{
					changed= true;
				}
			}
		}
	}
	
	return changed;
}

_fixed get_light_intensity(
	size_t light_index)
{
	// LP change: idiot-proofing / fallback
	light_data *light = get_light_data(light_index);
	if (!light) return 0;	// Blackness
	
	return light->intensity;
}

/* ---------- private code */

/* given a state, initialize .phase, .period, .initial_intensity, and .final_intensity */
void change_light_state(
	size_t light_index,
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
}
				
/* ---------- lighting functions */

static _fixed constant_lighting_proc(_fixed initial_intensity, _fixed final_intensity, short phase, short period);
static _fixed linear_lighting_proc(_fixed initial_intensity, _fixed final_intensity, short phase, short period);
static _fixed smooth_lighting_proc(_fixed initial_intensity, _fixed final_intensity, short phase, short period);
static _fixed flicker_lighting_proc(_fixed initial_intensity, _fixed final_intensity, short phase, short period);
static _fixed random_lighting_proc(_fixed initial_intensity, _fixed final_intensity, short phase, short period);
static _fixed fluorescent_lighting_proc(_fixed initial_intensity, _fixed final_intensity, short phase, short period);

typedef _fixed (*lighting_function)(_fixed initial_intensity, _fixed final_intensity,
	short phase, short period);

static lighting_function lighting_functions[NUMBER_OF_LIGHTING_FUNCTIONS]=
{
	constant_lighting_proc,
	linear_lighting_proc,
	smooth_lighting_proc,
	flicker_lighting_proc,
	random_lighting_proc,
	fluorescent_lighting_proc,
};

static _fixed lighting_function_dispatch(
	short function_index,
	_fixed initial_intensity,
	_fixed final_intensity,
	short phase,
	short period)
{
	assert(function_index>=0 && function_index<NUMBER_OF_LIGHTING_FUNCTIONS);
	
	return lighting_functions[function_index](initial_intensity, final_intensity, phase, period);
}

static _fixed constant_lighting_proc(
	_fixed initial_intensity,
	_fixed final_intensity,
	short phase,
	short period)
{
	(void) (initial_intensity);
	(void) (phase);
	(void) (period);
	
	return final_intensity;
}

static _fixed linear_lighting_proc(
	_fixed initial_intensity,
	_fixed final_intensity,
	short phase,
	short period)
{
	return initial_intensity + ((final_intensity-initial_intensity)*phase)/period;
}

static _fixed smooth_lighting_proc(
	_fixed initial_intensity,
	_fixed final_intensity,
	short phase,
	short period)
{
	return initial_intensity + (((final_intensity-initial_intensity)*(cosine_table[(phase*HALF_CIRCLE)/period+HALF_CIRCLE]+TRIG_MAGNITUDE))>>(TRIG_SHIFT+1));
}

static _fixed flicker_lighting_proc(
	_fixed initial_intensity,
	_fixed final_intensity,
	short phase,
	short period)
{
	_fixed smooth_intensity= smooth_lighting_proc(initial_intensity, final_intensity, phase, period);
	_fixed delta= final_intensity-smooth_intensity;
	
	return smooth_intensity + (delta ? global_random()%delta : 0);
}

static _fixed random_lighting_proc(
	_fixed initial_intensity,
	_fixed final_intensity,
	short phase,
	short period)
{
	_fixed delta;
	if (final_intensity > initial_intensity) 
	{
		delta = final_intensity - initial_intensity;
		return initial_intensity + (delta ? global_random()%delta : 0);
	} else {
		delta = initial_intensity - final_intensity;
		return final_intensity + (delta ? global_random()%delta : 0);	
	}
}

// should the probability of final_intensity increase with phase?
static _fixed fluorescent_lighting_proc(
	_fixed initial_intensity,
	_fixed final_intensity,
	short phase,
	short period)
{
	return (global_random()%2 ? final_intensity : initial_intensity);
}

uint8 *unpack_old_light_data(uint8 *Stream, old_light_data* Objects, size_t Count)
{
	uint8* S = Stream;
	old_light_data* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->flags);
		
		StreamToValue(S,ObjPtr->type);
		StreamToValue(S,ObjPtr->mode);
		StreamToValue(S,ObjPtr->phase);
		
		StreamToValue(S,ObjPtr->minimum_intensity);
		StreamToValue(S,ObjPtr->maximum_intensity);
		StreamToValue(S,ObjPtr->period);
		
		StreamToValue(S,ObjPtr->intensity);
		
		S += 5*2;
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_old_light_data));
	return S;
}

uint8 *pack_old_light_data(uint8 *Stream, old_light_data* Objects, size_t Count)
{
	uint8* S = Stream;
	old_light_data* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->flags);
		
		ValueToStream(S,ObjPtr->type);
		ValueToStream(S,ObjPtr->mode);
		ValueToStream(S,ObjPtr->phase);
		
		ValueToStream(S,ObjPtr->minimum_intensity);
		ValueToStream(S,ObjPtr->maximum_intensity);
		ValueToStream(S,ObjPtr->period);
		
		ValueToStream(S,ObjPtr->intensity);
		
		S += 5*2;
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_old_light_data));
	return S;
}

static void StreamToLightSpec(uint8* &S, lighting_function_specification& Object)
{
	StreamToValue(S,Object.function);
	
	StreamToValue(S,Object.period);
	StreamToValue(S,Object.delta_period);
	StreamToValue(S,Object.intensity);
	StreamToValue(S,Object.delta_intensity);
}

static void LightSpecToStream(uint8* &S, lighting_function_specification& Object)
{
	ValueToStream(S,Object.function);
	
	ValueToStream(S,Object.period);
	ValueToStream(S,Object.delta_period);
	ValueToStream(S,Object.intensity);
	ValueToStream(S,Object.delta_intensity);
}


uint8 *unpack_static_light_data(uint8 *Stream, static_light_data* Objects, size_t Count)
{
	uint8* S = Stream;
	static_light_data* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->type);
		StreamToValue(S,ObjPtr->flags);
		StreamToValue(S,ObjPtr->phase);

		StreamToLightSpec(S,ObjPtr->primary_active);
		StreamToLightSpec(S,ObjPtr->secondary_active);
		StreamToLightSpec(S,ObjPtr->becoming_active);
		StreamToLightSpec(S,ObjPtr->primary_inactive);
		StreamToLightSpec(S,ObjPtr->secondary_inactive);
		StreamToLightSpec(S,ObjPtr->becoming_inactive);

		StreamToValue(S,ObjPtr->tag);
		
		S += 4*2;
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_static_light_data));
	return S;
}

uint8 *pack_static_light_data(uint8 *Stream, static_light_data* Objects, size_t Count)
{
	uint8* S = Stream;
	static_light_data* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->type);
		ValueToStream(S,ObjPtr->flags);
		ValueToStream(S,ObjPtr->phase);

		LightSpecToStream(S,ObjPtr->primary_active);
		LightSpecToStream(S,ObjPtr->secondary_active);
		LightSpecToStream(S,ObjPtr->becoming_active);
		LightSpecToStream(S,ObjPtr->primary_inactive);
		LightSpecToStream(S,ObjPtr->secondary_inactive);
		LightSpecToStream(S,ObjPtr->becoming_inactive);

		ValueToStream(S,ObjPtr->tag);
		
		S += 4*2;
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_static_light_data));
	return S;
}


uint8 *unpack_light_data(uint8 *Stream, light_data* Objects, size_t Count)
{
	uint8* S = Stream;
	light_data* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		StreamToValue(S,ObjPtr->flags);
		StreamToValue(S,ObjPtr->state);
		
		StreamToValue(S,ObjPtr->intensity);
		
		StreamToValue(S,ObjPtr->phase);
		StreamToValue(S,ObjPtr->period);
		StreamToValue(S,ObjPtr->initial_intensity);
		StreamToValue(S,ObjPtr->final_intensity);
		
		S += 4*2;
		
		S = unpack_static_light_data(S,&ObjPtr->static_data,1);
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_light_data));
	return S;
}

uint8 *pack_light_data(uint8 *Stream, light_data* Objects, size_t Count)
{
	uint8* S = Stream;
	light_data* ObjPtr = Objects;
	
	for (size_t k = 0; k < Count; k++, ObjPtr++)
	{
		ValueToStream(S,ObjPtr->flags);
		ValueToStream(S,ObjPtr->state);
		
		ValueToStream(S,ObjPtr->intensity);
		
		ValueToStream(S,ObjPtr->phase);
		ValueToStream(S,ObjPtr->period);
		ValueToStream(S,ObjPtr->initial_intensity);
		ValueToStream(S,ObjPtr->final_intensity);
		
		S += 4*2;
		
		S = pack_static_light_data(S,&ObjPtr->static_data,1);
	}
	
	assert((S - Stream) == static_cast<ptrdiff_t>(Count*SIZEOF_light_data));
	return S;
}

static void FixIntensity(lighting_function_specification& LightState, old_light_data& OldLight)
{
	if (LightState.intensity > 0)
	{
		LightState.intensity = OldLight.maximum_intensity;
	} else {
		LightState.intensity = OldLight.minimum_intensity;
	}
}

static_light_data old_light_definitions[NUMBER_OF_OLD_LIGHTS] = 
{
	// _light_is_normal
	{
		_normal_light,
		FLAG(_light_is_initially_active)|FLAG(_light_has_slaved_intensities), 0,
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, FIXED_ONE, 0 },
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, FIXED_ONE, 0 },
		{ _constant_lighting_function, 1, 0, FIXED_ONE, 0 },
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, 0, 0 },
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, 0, 0 },
		{ _constant_lighting_function, 1, 0, FIXED_ONE, 0 }
	},

	// _light_is_rheostat
	{ 
		_normal_light,
		FLAG(_light_is_initially_active)|FLAG(_light_has_slaved_intensities), 0,
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, FIXED_ONE, 0 },
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, FIXED_ONE, 0 },
		{ _smooth_lighting_function, 3 * TICKS_PER_SECOND, 0, FIXED_ONE, 0 },
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, 0, 0 },
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, 0, 0 },
		{ _smooth_lighting_function, 3 * TICKS_PER_SECOND, 0, 0, 0 }
	},

	// _light_is_flourescent
	{
		_normal_light,
		FLAG(_light_is_initially_active)|FLAG(_light_has_slaved_intensities), 0,
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, FIXED_ONE, 0 },
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, FIXED_ONE, 0 },
		{ _fluorescent_lighting_function, 3 * TICKS_PER_SECOND, 0, FIXED_ONE, 0 },
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, 0, 0 },
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, 0, 0 },
		{ _constant_lighting_function, 1, 0, 0, 0 }
	},

	// _light_is_strobe
	{
		_normal_light,
		FLAG(_light_is_initially_active)|FLAG(_light_has_slaved_intensities), 0,
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, FIXED_ONE, 0 },
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, 0, 0 },
		{ _constant_lighting_function, 1, 0, FIXED_ONE, 0 },
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, 0, 0 },
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, FIXED_ONE, 0 },
		{ _constant_lighting_function, 1, 0, 0, 0 }
	},

	// _light_flickers
	{
		_normal_light,
		FLAG(_light_is_initially_active)|FLAG(_light_has_slaved_intensities), 0,
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, FIXED_ONE, 0 },
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, FIXED_ONE, 0 },
		{ _flicker_lighting_function, 3 * TICKS_PER_SECOND, 0, FIXED_ONE, 0 },
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, 0, 0 },
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, 0, 0 },
		{ _constant_lighting_function, 1, 0, 0, 0 }
	},

	// _light_pulsates
	{
		_normal_light,
		FLAG(_light_is_initially_active)|FLAG(_light_has_slaved_intensities), 0,
		{ _smooth_lighting_function, 2*TICKS_PER_SECOND, 0, FIXED_ONE, 0 },
		{ _smooth_lighting_function, 2*TICKS_PER_SECOND-1, 0, 0, 0 },
		{ _smooth_lighting_function, 2*TICKS_PER_SECOND-1, 0, FIXED_ONE, 0 },
		
		{ _smooth_lighting_function, 2*TICKS_PER_SECOND, 0, 0, 0 },
		{ _smooth_lighting_function, 2*TICKS_PER_SECOND-1, 0, FIXED_ONE, 0 },
		{ _smooth_lighting_function, 2*TICKS_PER_SECOND, 0, 0, 0 }
	},

	// _light_is_annoying
	{
		_normal_light,
		FLAG(_light_is_initially_active)|FLAG(_light_has_slaved_intensities), 0,
		{ _random_lighting_function, 2, 1, FIXED_ONE, 0 },
		{ _constant_lighting_function, 2, 0, 0, 0 },
		{ _random_lighting_function, 1, 0, FIXED_ONE, 0 },
		
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, 0, 0 },
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, 0, 0 },
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, 0, 0 }
	},

	// _light_is_energy_efficient
	{
		_normal_light,
		FLAG(_light_is_initially_active)|FLAG(_light_has_slaved_intensities), 0,
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, FIXED_ONE, 0 },
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, FIXED_ONE, 0 },
		{ _linear_lighting_function, 2 * TICKS_PER_SECOND, 0, FIXED_ONE, 0 },
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, 0, 0 },
		{ _constant_lighting_function, TICKS_PER_SECOND, 0, 0, 0 },
		{ _linear_lighting_function, 2 * TICKS_PER_SECOND, 0, 0, 0 }
	}
};	

void convert_old_light_data_to_new(static_light_data* NewLights, old_light_data* OldLights, int Count)
{
	// LP: code taken from game_wad.c and somewhat modified
	
	old_light_data *OldLtPtr = OldLights;
	static_light_data *NewLtPtr = NewLights;
	
	for (int k = 0; k < Count; k++, OldLtPtr++, NewLtPtr++)
	{
		obj_copy(*NewLtPtr, old_light_definitions[OldLtPtr->type]);
		FixIntensity(NewLtPtr->primary_active, *OldLtPtr);
		FixIntensity(NewLtPtr->secondary_active, *OldLtPtr);
		FixIntensity(NewLtPtr->becoming_active, *OldLtPtr);
		FixIntensity(NewLtPtr->primary_inactive, *OldLtPtr);
		FixIntensity(NewLtPtr->secondary_inactive, *OldLtPtr);
		FixIntensity(NewLtPtr->becoming_inactive, *OldLtPtr);

		if (OldLtPtr->type == _light_is_strobe) 
		{
			NewLtPtr->primary_active.period = OldLtPtr->period / 4 + 1;
			NewLtPtr->secondary_active.period = OldLtPtr->period / 4 + 1;
			NewLtPtr->primary_inactive.period = OldLtPtr->period / 4 + 1;
			NewLtPtr->secondary_inactive.period = OldLtPtr->period / 4 + 1;			
		}
		
		switch (OldLtPtr->mode)
		{
		case _light_mode_on:
		case _light_mode_turning_on:
			SET_FLAG(NewLtPtr->flags,FLAG(_light_is_initially_active),1);
			break;
		case _light_mode_off:
		default:
			SET_FLAG(NewLtPtr->flags,FLAG(_light_is_initially_active),0);
			break;
		}
	}
}

