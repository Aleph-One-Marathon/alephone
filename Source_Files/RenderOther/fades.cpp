/*
FADES.C

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

Friday, June 10, 1994 6:59:04 PM

Saturday, June 11, 1994 12:49:19 AM
	not doing signed math on the color deltas resulted in some very cool static-ish effects
	resulting from integer overflow.
Saturday, July 9, 1994 1:06:20 PM
	fade_finished() was acting like fade_not_finished().
Sunday, September 25, 1994 12:50:34 PM  (Jason')
	cool new fades.
Monday, April 3, 1995 11:22:58 AM  (Jason')
	fade effects for underwater/lava/goo/sewage.
Monday, July 10, 1995 8:23:03 PM  (Jason)
	random transparencies
Thursday, August 24, 1995 6:20:06 PM  (Jason)
	removed macintosh dependencies
Monday, October 30, 1995 8:02:12 PM  (Jason)
	fade prioritites for juggernaut flash

Jan 30, 2000 (Loren Petrich):
	Removed some "static" declarations that conflict with "extern"

May 17, 2000 (Loren Petrich):
	Added separate under-JjaroGoo fade effects

May 20, 2000 (Loren Petrich):
	Added XML-parser support.
	Note: "transparency" here should be called the opacity,
	since if it is zero, then colors will be unmodified.
	
May 29, 2000 (Loren Petrich):
	Extended the ranges of the opacities (values > 1 and < 0 now OK)

May 30, 2000 (Loren Petrich):
	Added stuff for transmitting fader info to OpenGL

Jul 1, 2000 (Loren Petrich):
	Added some idiot-proofing to the fader accessors

Jan 31, 2001 (Loren Petrich):
	Added delayed action for the fader effect, so as to get around certain MacOS oddities
*/

#include "cseries.h"
#include "fades.h"
#include "screen.h"
#include "interface.h"
#include "map.h" // for TICKS_PER_SECOND
#include "InfoTree.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

// LP additions:
#include <string.h>
#include "OGL_Faders.h"

#include "Music.h"
#include "Movie.h"

/* ---------- constants */

#define ADJUSTED_TRANSPARENCY_DOWNSHIFT 8

#define MINIMUM_FADE_RESTART (MACHINE_TICKS_PER_SECOND/2)

/* ---------- macros */

#define FADES_RANDOM() ((fades_random_seed&1) ? (fades_random_seed= (fades_random_seed>>1)^0xb400) : (fades_random_seed>>= 1))

/* ---------- types */

typedef void (*fade_proc)(struct color_table *original_color_table, struct color_table *animated_color_table, struct rgb_color *color, _fixed transparency);

/* ---------- structures */

/* an effect is something which is applied to the original color table before a fade begins */
struct fade_effect_definition
{
	short fade_type;
	_fixed transparency;
};

enum
{
	_full_screen_flag= 0x0001,
	_random_transparency_flag= 0x0002
};

struct fade_definition
{
	fade_proc proc;
	struct rgb_color color;
	_fixed initial_transparency, final_transparency; /* in [0,FIXED_ONE] */

	short period;
	
	uint16 flags;
	short priority; // higher is higher
};

#define FADE_IS_ACTIVE(f) ((f)->flags&(uint16)0x8000)
#define SET_FADE_ACTIVE_STATUS(f,s) ((void)((s)?((f)->flags|=(uint16)0x8000):((f)->flags&=(uint16)~0x8000)))

struct fade_data
{
	uint16 flags; /* [active.1] [unused.15] */
	
	short type;
	short fade_effect_type;
	
	uint32 last_update_tick;
	uint32 last_update_game_tick;
	
	struct color_table *original_color_table;
	struct color_table *animated_color_table;
};

/* ---------- globals */

static struct fade_data *fade = NULL;
static short last_fade_type = NUMBER_OF_FADE_TYPES;

static uint16 fades_random_seed= 0x1;

// LP addition: pointer to OpenGL fader to be used in the color-table functions below
// It is NULL if OpenGL is inactive
static OGL_Fader *CurrentOGLFader = NULL;

static int FadeEffectDelay = 0;

/* ---------- fade definitions */

static void tint_color_table(struct color_table *original_color_table, struct color_table *animated_color_table, struct rgb_color *color, _fixed transparency);
static void randomize_color_table(struct color_table *original_color_table, struct color_table *animated_color_table, struct rgb_color *color, _fixed transparency);
static void negate_color_table(struct color_table *original_color_table, struct color_table *animated_color_table, struct rgb_color *color, _fixed transparency);
static void dodge_color_table(struct color_table *original_color_table, struct color_table *animated_color_table, struct rgb_color *color, _fixed transparency);
static void burn_color_table(struct color_table *original_color_table, struct color_table *animated_color_table, struct rgb_color *color, _fixed transparency);
static void soft_tint_color_table(struct color_table *original_color_table, struct color_table *animated_color_table, struct rgb_color *color, _fixed transparency);

static struct fade_definition fade_definitions[NUMBER_OF_FADE_TYPES]=
{
	{tint_color_table, {0, 0, 0}, FIXED_ONE, FIXED_ONE, 0, _full_screen_flag, 0}, /* _start_cinematic_fade_in */
	{tint_color_table, {0, 0, 0}, FIXED_ONE, 0, MACHINE_TICKS_PER_SECOND/2, _full_screen_flag, 0}, /* _cinematic_fade_in */
	{tint_color_table, {0, 0, 0}, FIXED_ONE, 0, 3*MACHINE_TICKS_PER_SECOND/2, _full_screen_flag, 0}, /* _long_cinematic_fade_in */
	{tint_color_table, {0, 0, 0}, 0, FIXED_ONE, MACHINE_TICKS_PER_SECOND/2, _full_screen_flag, 0}, /* _cinematic_fade_out */
	{tint_color_table, {0, 0, 0}, 0, 0, 0, _full_screen_flag, 0}, /* _end_cinematic_fade_out */
	
	{tint_color_table, {65535, 0, 0}, (3*FIXED_ONE)/4, 0, MACHINE_TICKS_PER_SECOND/4, 0, 0}, /* _fade_red */
	{tint_color_table, {65535, 0, 0}, FIXED_ONE, 0, (3*MACHINE_TICKS_PER_SECOND)/4, 0, 0}, /* _fade_big_red */
	{tint_color_table, {0, 65535, 0}, FIXED_ONE_HALF, 0, MACHINE_TICKS_PER_SECOND/4, 0, 0}, /* _fade_bonus */
	{tint_color_table, {65535, 65535, 50000}, FIXED_ONE, 0, MACHINE_TICKS_PER_SECOND/3, 0, 0}, /* _fade_bright */
	{tint_color_table, {65535, 65535, 50000}, FIXED_ONE, 0, 4*MACHINE_TICKS_PER_SECOND, 0, 1}, /* _fade_long_bright */
	{tint_color_table, {65535, 65535, 0}, FIXED_ONE, 0, MACHINE_TICKS_PER_SECOND/2, 0, 0}, /* _fade_yellow */
	{tint_color_table, {65535, 65535, 0}, FIXED_ONE, 0, MACHINE_TICKS_PER_SECOND, 0, 0}, /* _fade_big_yellow */
	{tint_color_table, {215*256, 107*256, 65535}, (3*FIXED_ONE)/4, 0, MACHINE_TICKS_PER_SECOND/4, 0, 0}, /* _fade_purple */
	{tint_color_table, {169*256, 65535, 224*256}, (3*FIXED_ONE)/4, 0, MACHINE_TICKS_PER_SECOND/2, 0, 0}, /* _fade_cyan */
	{tint_color_table, {65535, 65535, 65535}, FIXED_ONE_HALF, 0, MACHINE_TICKS_PER_SECOND/4, 0, 0}, /* _fade_white */
	{tint_color_table, {65535, 65535, 65535}, FIXED_ONE, 0, MACHINE_TICKS_PER_SECOND/2, 0, 0}, /* _fade_big_white */
	{tint_color_table, {65535, 32768, 0}, FIXED_ONE, 0, MACHINE_TICKS_PER_SECOND/4, 0, 0}, /* _fade_orange */
	{tint_color_table, {65535, 32768, 0}, FIXED_ONE/4, 0, 3*MACHINE_TICKS_PER_SECOND, 0, 0}, /* _fade_long_orange */
	{tint_color_table, {0, 65535, 0}, 3*FIXED_ONE/4, 0, MACHINE_TICKS_PER_SECOND/2, 0, 0}, /* _fade_green */
	{tint_color_table, {65535, 0, 65535}, FIXED_ONE/4, 0, 3*MACHINE_TICKS_PER_SECOND, 0, 0}, /* _fade_long_green */

	{randomize_color_table, {0, 0, 0}, FIXED_ONE, 0, (3*MACHINE_TICKS_PER_SECOND)/8, 0, 0}, /* _fade_static */
	{negate_color_table, {65535, 65535, 65535}, FIXED_ONE, 0, MACHINE_TICKS_PER_SECOND/2, 0, 0}, /* _fade_negative */
	{negate_color_table, {65535, 65535, 65535}, FIXED_ONE, 0, (3*MACHINE_TICKS_PER_SECOND)/2, 0, 0}, /* _fade_big_negative */
	{negate_color_table, {0, 65535, 0}, FIXED_ONE, 0, MACHINE_TICKS_PER_SECOND/2, _random_transparency_flag, 0}, /* _fade_flicker_negative */
	{dodge_color_table, {0, 65535, 0}, FIXED_ONE, 0, (3*MACHINE_TICKS_PER_SECOND)/4, 0, 0}, /* _fade_dodge_purple */
	{burn_color_table, {0, 65535, 65535}, FIXED_ONE, 0, MACHINE_TICKS_PER_SECOND, 0, 0}, /* _fade_burn_cyan */
	{dodge_color_table, {0, 0, 65535}, FIXED_ONE, 0, (3*MACHINE_TICKS_PER_SECOND)/2, 0, 0}, /* _fade_dodge_yellow */
	{burn_color_table, {0, 65535, 0}, FIXED_ONE, 0, 2*MACHINE_TICKS_PER_SECOND, 0, 0}, /* _fade_burn_green */

	{soft_tint_color_table, {137*256, 0, 137*256}, FIXED_ONE, 0, 2*MACHINE_TICKS_PER_SECOND, 0, 0}, /* _fade_tint_purple */
	{soft_tint_color_table, {0, 0, 65535}, FIXED_ONE, 0, 2*MACHINE_TICKS_PER_SECOND, 0, 0}, /* _fade_tint_blue */
	{soft_tint_color_table, {65535, 16384, 0}, FIXED_ONE, 0, 2*MACHINE_TICKS_PER_SECOND, 0, 0}, /* _fade_tint_orange */
	{soft_tint_color_table, {32768, 65535, 0}, FIXED_ONE, 0, 2*MACHINE_TICKS_PER_SECOND, 0, 0}, /* _fade_tint_gross */
	
	// LP addition: underneath JjaroGoo (default: like sewage)
	{soft_tint_color_table, {32768, 65535, 0}, FIXED_ONE, 0, 2*MACHINE_TICKS_PER_SECOND, 0, 0}, /* _fade_tint_gross */
};

// LP change: rearranged to get order: water, lava, sewage, jjaro, pfhor
struct fade_effect_definition fade_effect_definitions[NUMBER_OF_FADE_EFFECT_TYPES]=
{
	{_fade_tint_blue, 3*FIXED_ONE/4}, /* _effect_under_water */
	{_fade_tint_orange, 3*FIXED_ONE/4}, /* _effect_under_lava */
	{_fade_tint_gross, 3*FIXED_ONE/4}, /* _effect_under_sewage */
	{_fade_tint_jjaro, 3*FIXED_ONE/4}, /* _effect_under_jjaro */ // LP addition
	{_fade_tint_green, 3*FIXED_ONE/4}, /* _effect_under_goo */
};

static float actual_gamma_values[NUMBER_OF_GAMMA_LEVELS]=
{
	1.3F,
	1.15F,
	1.0F, // default
	0.95F,
	0.90F,
	0.85F,
	0.77F,
	0.70F
};

/* ---------- private prototypes */

static fade_definition *get_fade_definition(
	const short index);

static fade_effect_definition *get_fade_effect_definition(
	const short index);

static void recalculate_and_display_color_table(short type, _fixed transparency,
	struct color_table *original_color_table, struct color_table *animated_color_table, bool fade_active);

// LP additions for OpenGL fader support; both of them use the pointer to the current
// OpenGL fader defined above.

// Set up OpenGL fader-queue entry -- arg is which one (liquid or other);
// the fader type is set to NONE.
static void SetOGLFader(int Index);

// Translate the color and opacity values
static void TranslateToOGLFader(rgb_color &Color, _fixed Opacity);

/* ---------- code */

// Accessors:
fade_definition *get_fade_definition(
	const short index)
{
	return GetMemberWithBounds(fade_definitions,index,NUMBER_OF_FADE_TYPES);
}

fade_effect_definition *get_fade_effect_definition(
	const short index)
{
	return GetMemberWithBounds(fade_effect_definitions,index,NUMBER_OF_FADE_EFFECT_TYPES);
}

void initialize_fades(
	void)
{
	/* allocate and initialize space for our fade_data structure */
	fade= new fade_data;
	assert(fade);
	fade->flags = 0;
	
	SET_FADE_ACTIVE_STATUS(fade, false);
	fade->fade_effect_type= NONE;
}

bool update_fades(
	bool game_in_progress)
{
  if (FADE_IS_ACTIVE(fade))
	{
		struct fade_definition *definition= get_fade_definition(fade->type);
		// LP change: idiot-proofing
		if (!definition) return false;
		
		_fixed transparency;
		short phase;
		if (game_in_progress)
		{
			phase = (dynamic_world->tick_count - fade->last_update_game_tick) * MACHINE_TICKS_PER_SECOND/TICKS_PER_SECOND;
		}
		else
		{
			phase = machine_tick_count() - fade->last_update_tick;
		}
		
		if (phase>=definition->period)
		{
			transparency= definition->final_transparency;
			SET_FADE_ACTIVE_STATUS(fade, false);
		}
		else
		{
			transparency= definition->initial_transparency + (phase*(definition->final_transparency-definition->initial_transparency))/definition->period;
			if (definition->flags&_random_transparency_flag) transparency+= FADES_RANDOM()%(definition->final_transparency-transparency);
		}
		
		recalculate_and_display_color_table(fade->type, transparency, fade->original_color_table, fade->animated_color_table, FADE_IS_ACTIVE(fade));
		Movie::instance()->AddFrame(Movie::FRAME_FADE);
	}
	
	return FADE_IS_ACTIVE(fade) ? true : false;
}

void SetFadeEffectDelay(int _FadeEffectDelay)
{
	FadeEffectDelay = _FadeEffectDelay;
}

void set_fade_effect(
	short type)
{
	bool ForceFEUpdate = false;
	if (FadeEffectDelay > 0)
	{
		ForceFEUpdate = true;
		FadeEffectDelay--;
	}
	
	if (ForceFEUpdate || fade->fade_effect_type!=type)
	{
		fade->fade_effect_type= type;
		
		if (!FADE_IS_ACTIVE(fade))
		{
			if (type==NONE)
			{
				// LP addition: OpenGL-fader handling
				for (int f=0; f<NUMBER_OF_FADER_QUEUE_ENTRIES; f++)
					SetOGLFader(f);
				
				// Only do the video-card fader if the OpenGL fader is inactive
#ifdef HAVE_OPENGL
				if (!OGL_FaderActive())
#endif
					animate_screen_clut(world_color_table, false);
			}
			else
			{
				recalculate_and_display_color_table(NONE, 0, world_color_table, visible_color_table, false);
			}
		}
	}
}

void start_fade(
	short type)
{
	explicit_start_fade(type, world_color_table, visible_color_table, true);
}

void explicit_start_fade(
	short type,
	struct color_table *original_color_table,
	struct color_table *animated_color_table,
	bool game_in_progress)
{
	struct fade_definition *definition= get_fade_definition(type);
	// LP change: idiot-proofing
	if (!definition) return;
		
	uint32 machine_ticks= machine_tick_count();
	bool do_fade= true;

	if (FADE_IS_ACTIVE(fade))
	{
		struct fade_definition *old_definition= get_fade_definition(fade->type);
		// LP change: idiot-proofing
		if (old_definition)
		{
			if (old_definition->priority>definition->priority) do_fade= false;
			if (fade->type==type)
			{
				short phase;
				if (game_in_progress)
				{
					phase = (dynamic_world->tick_count - fade->last_update_game_tick) * MACHINE_TICKS_PER_SECOND/TICKS_PER_SECOND;
				}
				else
				{
					phase = machine_ticks - fade->last_update_tick;
				}
				if (phase<MINIMUM_FADE_RESTART) do_fade= false;
			}
		}
	}

	if (do_fade)
	{
		SET_FADE_ACTIVE_STATUS(fade, false);
		last_fade_type = type;
	
		recalculate_and_display_color_table(type, definition->initial_transparency, original_color_table, animated_color_table, definition->period);
		if (definition->period)
		{
			fade->type= type;
			fade->last_update_tick= machine_ticks;
			fade->last_update_game_tick = game_in_progress ? dynamic_world->tick_count : 0;
			fade->original_color_table= original_color_table;
			fade->animated_color_table= animated_color_table;
			SET_FADE_ACTIVE_STATUS(fade, true);
		}
	}
}

void stop_fade(
	void)
{
	if (FADE_IS_ACTIVE(fade))
	{
		struct fade_definition *definition= get_fade_definition(fade->type);
		// LP change: idiot-proofing
		if (!definition) return;
		
		recalculate_and_display_color_table(fade->type, definition->final_transparency,
			fade->original_color_table, fade->animated_color_table, false);
		
		SET_FADE_ACTIVE_STATUS(fade, false);
	}
}

bool fade_finished(
	void)
{
	return FADE_IS_ACTIVE(fade) ? false : true;
}

void full_fade(
	short type,
	struct color_table *original_color_table)
{
	struct color_table animated_color_table;
	
	obj_copy(animated_color_table, *original_color_table);
	
	explicit_start_fade(type, original_color_table, &animated_color_table);
	while (update_fades())
		Music::instance()->Idle();
		;
}

short get_fade_period(
	short type)
{
	struct fade_definition *definition= get_fade_definition(type);
	// LP change: idiot-proofing
	if (!definition) return 0;	
	
	return definition->period;
}

void gamma_correct_color_table(
	struct color_table *uncorrected_color_table,
	struct color_table *corrected_color_table,
	short gamma_level)
{
	short i;
	float gamma;
	struct rgb_color *uncorrected= uncorrected_color_table->colors;
	struct rgb_color *corrected= corrected_color_table->colors;
	
	assert(gamma_level>=0 && gamma_level<NUMBER_OF_GAMMA_LEVELS);
	gamma= actual_gamma_values[gamma_level];
	if (Movie::instance()->IsRecording())
		gamma = 1.0;
	if (gamma > 0.999F && gamma < 1.001F) {
		memcpy(corrected_color_table, uncorrected_color_table, sizeof(struct color_table));
		return;
	}
	
	corrected_color_table->color_count= uncorrected_color_table->color_count;
	for (i= 0; i<uncorrected_color_table->color_count; ++i, ++corrected, ++uncorrected)
	{
		corrected->red = static_cast<uint16>(pow(static_cast<float>(uncorrected->red/65535.0), gamma)*65535.0);
		corrected->green = static_cast<uint16>(pow(static_cast<float>(uncorrected->green/65535.0), gamma)*65535.0);
		corrected->blue = static_cast<uint16>(pow(static_cast<float>(uncorrected->blue/65535.0), gamma)*65535.0);
	}
}

float get_actual_gamma_adjust(short gamma_level)
{
	return actual_gamma_values[gamma_level];
}

bool fade_blacked_screen(void)
{
	return (!FADE_IS_ACTIVE(fade) &&
			(last_fade_type == _start_cinematic_fade_in ||
			 last_fade_type == _cinematic_fade_out));
}

/* ---------- private code */

/*
struct fade_definition *get_fade_definition(
	short index)
{
	assert(index>=0 && index<NUMBER_OF_FADE_TYPES);
	
	return fade_definitions + index;
}

static struct fade_effect_definition *get_fade_effect_definition(
	short index)
{
	assert(index>=0 && index<NUMBER_OF_FADE_EFFECT_TYPES);
	
	return fade_effect_definitions + index;
}
*/

//void draw_intro_screen(void); // from screen.cpp

static void recalculate_and_display_color_table(
	short type,
	_fixed transparency,
	struct color_table *original_color_table,
	struct color_table *animated_color_table,
	bool fade_active)
{
	bool full_screen= false;
	
	// LP addition: set up the OGL queue entry for the liquid effects
	SetOGLFader(FaderQueue_Liquid);
	
	/* if a fade effect is active, apply it first */
	if (fade->fade_effect_type!=NONE)
	{
		struct fade_effect_definition *effect_definition= get_fade_effect_definition(fade->fade_effect_type);
		// LP change: idiot-proofing
		if (!effect_definition) return;
		
		struct fade_definition *definition= get_fade_definition(effect_definition->fade_type);
		// LP change: idiot-proofing
		if (!definition) return;
				
		definition->proc(original_color_table, animated_color_table, &definition->color, effect_definition->transparency);
		original_color_table= animated_color_table;
	}
	
	// LP addition: set up the OGL queue entry for the other effects
	SetOGLFader(FaderQueue_Other);
	
	if (type!=NONE)
	{
		struct fade_definition *definition= get_fade_definition(type);

		definition->proc(original_color_table, animated_color_table, &definition->color, transparency);	
		full_screen= (definition->flags&_full_screen_flag) ? true : false;

		// cancel OGL fader if we've reached the end point for this fade
		if (!fade_active)
			SetOGLFader(FaderQueue_Other);
	}
	
	// Only do the video-card fader if the OpenGL fader is inactive
#ifdef HAVE_OPENGL
	if (!OGL_FaderActive())
#endif
		animate_screen_clut(animated_color_table, full_screen);
	
	if (get_game_state() < _game_in_progress)  // main menu or chapter screen
		draw_intro_screen();
}

/* ---------- fade functions */

static void tint_color_table(
	struct color_table *original_color_table,
	struct color_table *animated_color_table,
	struct rgb_color *color,
	_fixed transparency)
{
	// LP addition: support for OpenGL faders
	if (CurrentOGLFader)
	{
		CurrentOGLFader->Type = _tint_fader_type;
		TranslateToOGLFader(*color,transparency);
		return;
	}

	short i;
	struct rgb_color *unadjusted= original_color_table->colors;
	struct rgb_color *adjusted= animated_color_table->colors;
	short adjusted_transparency= transparency>>ADJUSTED_TRANSPARENCY_DOWNSHIFT;

	animated_color_table->color_count= original_color_table->color_count;
	for (i= 0; i<original_color_table->color_count; ++i, ++adjusted, ++unadjusted)
	{
		adjusted->red= unadjusted->red + (((color->red-unadjusted->red)*adjusted_transparency)>>(FIXED_FRACTIONAL_BITS-ADJUSTED_TRANSPARENCY_DOWNSHIFT));
		adjusted->green= unadjusted->green + (((color->green-unadjusted->green)*adjusted_transparency)>>(FIXED_FRACTIONAL_BITS-ADJUSTED_TRANSPARENCY_DOWNSHIFT));
		adjusted->blue= unadjusted->blue + (((color->blue-unadjusted->blue)*adjusted_transparency)>>(FIXED_FRACTIONAL_BITS-ADJUSTED_TRANSPARENCY_DOWNSHIFT));
	}
}

static void randomize_color_table(
	struct color_table *original_color_table,
	struct color_table *animated_color_table,
	struct rgb_color *color,
	_fixed transparency)
{
	// LP addition: support for OpenGL faders
	if (CurrentOGLFader)
	{
		CurrentOGLFader->Type = _randomize_fader_type;
		// Create random colors, but transmit the opacity
		CurrentOGLFader->Color[0] = FADES_RANDOM()/float(SHRT_MAX);
		CurrentOGLFader->Color[1] = FADES_RANDOM()/float(SHRT_MAX);
		CurrentOGLFader->Color[2] = FADES_RANDOM()/float(SHRT_MAX);
		CurrentOGLFader->Color[3] = transparency/float(FIXED_ONE);
		return;
	}

	short i;
	struct rgb_color *unadjusted= original_color_table->colors;
	struct rgb_color *adjusted= animated_color_table->colors;
	uint16 mask, adjusted_transparency= PIN(transparency, 0, 0xffff);

	(void) (color);

	/* calculate a mask which has all bits including and lower than the high-bit in the
		transparency set */
	for (mask= 0; ~mask & adjusted_transparency; mask= (mask<<1)|1)
		;
	
	animated_color_table->color_count= original_color_table->color_count;
	for (i= 0; i<original_color_table->color_count; ++i, ++adjusted, ++unadjusted)
	{
		adjusted->red= unadjusted->red + (FADES_RANDOM()&mask);
		adjusted->green= unadjusted->green + (FADES_RANDOM()&mask);
		adjusted->blue= unadjusted->blue + (FADES_RANDOM()&mask);
	}
}

/* unlike pathways, all colors wonÕt pass through 50% gray at the same time */
static void negate_color_table(
	struct color_table *original_color_table,
	struct color_table *animated_color_table,
	struct rgb_color *color,
	_fixed transparency)
{
	// LP addition: support for OpenGL faders
	if (CurrentOGLFader)
	{
		CurrentOGLFader->Type = _negate_fader_type;
		TranslateToOGLFader(*color,transparency);
		return;
	}

	short i;
	struct rgb_color *unadjusted= original_color_table->colors;
	struct rgb_color *adjusted= animated_color_table->colors;
	
	transparency= FIXED_ONE-transparency;
	animated_color_table->color_count= original_color_table->color_count;
	for (i= 0; i<original_color_table->color_count; ++i, ++adjusted, ++unadjusted)
	{
		adjusted->red= (unadjusted->red>0x8000) ?
			CEILING((unadjusted->red^color->red)+transparency, (int32)unadjusted->red) :
			FLOOR((unadjusted->red^color->red)-transparency, (int32)unadjusted->red);
		adjusted->green= (unadjusted->green>0x8000) ?
			CEILING((unadjusted->green^color->green)+transparency, (int32)unadjusted->green) :
			FLOOR((unadjusted->green^color->green)-transparency, (int32)unadjusted->green);
		adjusted->blue= (unadjusted->blue>0x8000) ?
			CEILING((unadjusted->blue^color->blue)+transparency, (int32)unadjusted->blue) :
			FLOOR((unadjusted->blue^color->blue)-transparency, (int32)unadjusted->blue);
	}
}

static void dodge_color_table(
	struct color_table *original_color_table,
	struct color_table *animated_color_table,
	struct rgb_color *color,
	_fixed transparency)
{
	// LP addition: support for OpenGL faders
	if (CurrentOGLFader)
	{
		CurrentOGLFader->Type = _dodge_fader_type;
		TranslateToOGLFader(*color,transparency);
		return;
	}

	short i;
	struct rgb_color *unadjusted= original_color_table->colors;
	struct rgb_color *adjusted= animated_color_table->colors;
	
	animated_color_table->color_count= original_color_table->color_count;
	for (i= 0; i<original_color_table->color_count; ++i, ++adjusted, ++unadjusted)
	{
		int32 component;
		
		component= 0xffff - (int32(1LL*(color->red^0xffff)*unadjusted->red)>>FIXED_FRACTIONAL_BITS) - transparency, adjusted->red= CEILING(component, unadjusted->red);
		component= 0xffff - (int32(1LL*(color->green^0xffff)*unadjusted->green)>>FIXED_FRACTIONAL_BITS) - transparency, adjusted->green= CEILING(component, unadjusted->green);
		component= 0xffff - (int32(1LL*(color->blue^0xffff)*unadjusted->blue)>>FIXED_FRACTIONAL_BITS) - transparency, adjusted->blue= CEILING(component, unadjusted->blue);
	}
}

static void burn_color_table(
	struct color_table *original_color_table,
	struct color_table *animated_color_table,
	struct rgb_color *color,
	_fixed transparency)
{
	// LP addition: support for OpenGL faders
	if (CurrentOGLFader)
	{
		CurrentOGLFader->Type = _burn_fader_type;
		TranslateToOGLFader(*color,transparency);
		return;
	}

	short i;
	struct rgb_color *unadjusted= original_color_table->colors;
	struct rgb_color *adjusted= animated_color_table->colors;
	
	transparency= FIXED_ONE-transparency;
	animated_color_table->color_count= original_color_table->color_count;
	for (i= 0; i<original_color_table->color_count; ++i, ++adjusted, ++unadjusted)
	{
		int32 component;
		
		component= (int32(1LL*color->red*unadjusted->red)>>FIXED_FRACTIONAL_BITS) + transparency, adjusted->red= CEILING(component, unadjusted->red);
		component= (int32(1LL*color->green*unadjusted->green)>>FIXED_FRACTIONAL_BITS) + transparency, adjusted->green= CEILING(component, unadjusted->green);
		component= (int32(1LL*color->blue*unadjusted->blue)>>FIXED_FRACTIONAL_BITS) + transparency, adjusted->blue= CEILING(component, unadjusted->blue);
	}
}

static void soft_tint_color_table(
	struct color_table *original_color_table,
	struct color_table *animated_color_table,
	struct rgb_color *color,
	_fixed transparency)
{
	// LP addition: support for OpenGL faders
	if (CurrentOGLFader)
	{
		CurrentOGLFader->Type = _soft_tint_fader_type;
		TranslateToOGLFader(*color,transparency);
		return;
	}

	short i;
	struct rgb_color *unadjusted= original_color_table->colors;
	struct rgb_color *adjusted= animated_color_table->colors;
	uint16 adjusted_transparency= transparency>>ADJUSTED_TRANSPARENCY_DOWNSHIFT;
	
	animated_color_table->color_count= original_color_table->color_count;
	for (i= 0; i<original_color_table->color_count; ++i, ++adjusted, ++unadjusted)
	{
		uint16 intensity;
		
		intensity= MAX(unadjusted->red, unadjusted->green);
		intensity= MAX(intensity, unadjusted->blue)>>ADJUSTED_TRANSPARENCY_DOWNSHIFT;
		
		adjusted->red= unadjusted->red + (((((color->red*intensity)>>(FIXED_FRACTIONAL_BITS-ADJUSTED_TRANSPARENCY_DOWNSHIFT))-unadjusted->red)*adjusted_transparency)>>(FIXED_FRACTIONAL_BITS-ADJUSTED_TRANSPARENCY_DOWNSHIFT));
		adjusted->green= unadjusted->green + (((((color->green*intensity)>>(FIXED_FRACTIONAL_BITS-ADJUSTED_TRANSPARENCY_DOWNSHIFT))-unadjusted->green)*adjusted_transparency)>>(FIXED_FRACTIONAL_BITS-ADJUSTED_TRANSPARENCY_DOWNSHIFT));
		adjusted->blue= unadjusted->blue + (((((color->blue*intensity)>>(FIXED_FRACTIONAL_BITS-ADJUSTED_TRANSPARENCY_DOWNSHIFT))-unadjusted->blue)*adjusted_transparency)>>(FIXED_FRACTIONAL_BITS-ADJUSTED_TRANSPARENCY_DOWNSHIFT));
	}
}


// Arg is location in the OpenGL fader queue
void SetOGLFader(int Index)
{
#ifdef HAVE_OPENGL
	if (OGL_FaderActive())
	{
		CurrentOGLFader = GetOGL_FaderQueueEntry(Index);
		CurrentOGLFader->Type = NONE;
	} else
#endif
		CurrentOGLFader = NULL;
}

// Translate the color and opacity values
static void TranslateToOGLFader(rgb_color &Color, _fixed Opacity)
{
	assert(CurrentOGLFader);
	CurrentOGLFader->Color[0] = Color.red/float(FIXED_ONE-1);
	CurrentOGLFader->Color[1] = Color.green/float(FIXED_ONE-1);
	CurrentOGLFader->Color[2] = Color.blue/float(FIXED_ONE-1);
	CurrentOGLFader->Color[3] = Opacity/float(FIXED_ONE);
}


struct fade_definition *original_fade_definitions = NULL;
struct fade_effect_definition *original_fade_effect_definitions = NULL;

void reset_mml_faders()
{
	if (original_fade_definitions) {
		for (int i = 0; i < NUMBER_OF_FADE_TYPES; i++)
			fade_definitions[i] = original_fade_definitions[i];
		free(original_fade_definitions);
		original_fade_definitions = NULL;
	}

	if (original_fade_effect_definitions) {
		for (int i = 0; i < NUMBER_OF_FADE_EFFECT_TYPES; i++)
			fade_effect_definitions[i] = original_fade_effect_definitions[i];
		free(original_fade_effect_definitions);
		original_fade_effect_definitions = NULL;
	}
}

void parse_mml_faders(const InfoTree& root)
{
	// back up old values first
	if (!original_fade_definitions) {
		original_fade_definitions = (struct fade_definition *) malloc(sizeof(struct fade_definition) * NUMBER_OF_FADE_TYPES);
		assert(original_fade_definitions);
		for (int i = 0; i < NUMBER_OF_FADE_TYPES; i++)
			original_fade_definitions[i] = fade_definitions[i];
	}

	if (!original_fade_effect_definitions) {
		original_fade_effect_definitions = (struct fade_effect_definition *) malloc(sizeof(struct fade_effect_definition) * NUMBER_OF_FADE_EFFECT_TYPES);
		assert(original_fade_effect_definitions);
		for (int i = 0; i < NUMBER_OF_FADE_EFFECT_TYPES; i++)
			original_fade_effect_definitions[i] = fade_effect_definitions[i];
	}
	
	BOOST_FOREACH(InfoTree ftree, root.children_named("fader"))
	{
		int16 index;
		if (!ftree.read_indexed("index", index, NUMBER_OF_FADE_TYPES))
			continue;
		
		fade_definition& def = fade_definitions[index];
		int16 fade_type;
		if (ftree.read_indexed("type", fade_type, NUMBER_OF_FADER_FUNCTIONS))
		{
			switch (fade_type) {
				case _tint_fader_type:
					def.proc = tint_color_table;
					break;
				case _randomize_fader_type:
					def.proc = randomize_color_table;
					break;
				case _negate_fader_type:
					def.proc = negate_color_table;
					break;
				case _dodge_fader_type:
					def.proc = dodge_color_table;
					break;
				case _burn_fader_type:
					def.proc = burn_color_table;
					break;
				case _soft_tint_fader_type:
					def.proc = soft_tint_color_table;
					break;
				default:
					break;
			}
		}
		
		ftree.read_fixed("initial_opacity", def.initial_transparency);
		ftree.read_fixed("final_opacity", def.final_transparency);
		ftree.read_attr("flags", def.flags);
		ftree.read_attr("priority", def.priority);
		int16 period;
		if (ftree.read_attr("period", period))
			def.period = static_cast<int32>(period) * 1000 / MACHINE_TICKS_PER_SECOND;
		
		BOOST_FOREACH(InfoTree color, ftree.children_named("color"))
			color.read_color(def.color);
	}
	
	BOOST_FOREACH(InfoTree ltree, root.children_named("liquid"))
	{
		int16 index;
		if (!ltree.read_indexed("index", index, NUMBER_OF_FADE_EFFECT_TYPES))
			continue;
		
		fade_effect_definition& def = fade_effect_definitions[index];
		ltree.read_indexed("fader", def.fade_type, NUMBER_OF_FADE_TYPES, true);
		ltree.read_fixed("opacity", def.transparency);
	}
}
