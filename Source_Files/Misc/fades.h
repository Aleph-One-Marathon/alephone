#ifndef __FADES_H
#define __FADES_H

/*
FADES.H
Tuesday, April 4, 1995 11:48:29 AM  (Jason')

May 17, 2000 (Loren Petrich):
	Added separate under-JjaroGoo fade effects

May 20, 2000 (Loren Petrich):
	Added XML-parser support
*/

#include "XML_ElementParser.h"

/* ---------- constants */

enum
{
	NUMBER_OF_GAMMA_LEVELS= 8,
	DEFAULT_GAMMA_LEVEL= 2
};

enum /* fade types */
{
	_start_cinematic_fade_in, /* force all colors to black immediately */
	_cinematic_fade_in, /* fade in from black */
	_long_cinematic_fade_in,
	_cinematic_fade_out, /* fade out from black */
	_end_cinematic_fade_out, /* force all colors from black immediately */

	_fade_red, /* bullets and fist */
	_fade_big_red, /* bigger bullets and fists */
	_fade_bonus, /* picking up items */
	_fade_bright, /* teleporting */
	_fade_long_bright, /* nuclear monster detonations */
	_fade_yellow, /* explosions */
	_fade_big_yellow, /* big explosions */
	_fade_purple, /* ? */
	_fade_cyan, /* fighter staves and projectiles */
	_fade_white, /* absorbed */
	_fade_big_white, /* rocket (probably) absorbed */
	_fade_orange, /* flamethrower */
	_fade_long_orange, /* marathon lava */
	_fade_green, /* hunter projectile */
	_fade_long_green, /* alien green goo */
	_fade_static, /* compiler projectile */
	_fade_negative, /* minor fusion projectile */
	_fade_big_negative, /* major fusion projectile */
	_fade_flicker_negative, /* hummer projectile */
	_fade_dodge_purple, /* alien weapon */
	_fade_burn_cyan, /* armageddon beast electricity */
	_fade_dodge_yellow, /* armageddon beast projectile */
	_fade_burn_green, /* hunter projectile */

	_fade_tint_green, /* under goo */
	_fade_tint_blue, /* under water */
	_fade_tint_orange, /* under lava */
	_fade_tint_gross, /* under sewage */
	_fade_tint_jjaro, /* under JjaroGoo */ // LP addition
	
	NUMBER_OF_FADE_TYPES
};

// LP change: rearranged to get order: water, lava, sewage, jjaro, pfhor
enum /* effect types */
{
	_effect_under_water,
	_effect_under_lava,
	_effect_under_sewage,
	_effect_under_jjaro,
	_effect_under_goo,
	NUMBER_OF_FADE_EFFECT_TYPES
};

// LP addition, since XML does not support direct specification of callbacks
// very well.
// Moved out here for the convenience of OpenGL fader implementations.
enum
{
	_tint_fader_type,
	_randomize_fader_type,
	_negate_fader_type,
	_dodge_fader_type,
	_burn_fader_type,
	_soft_tint_fader_type,
	NUMBER_OF_FADER_FUNCTIONS
};


/* ---------- prototypes/FADES.C */

void initialize_fades(void);
boolean update_fades(void);

void start_fade(short type);
void stop_fade(void);
boolean fade_finished(void);

void set_fade_effect(short type);

short get_fade_period(short type);

void gamma_correct_color_table(struct color_table *uncorrected_color_table, struct color_table *corrected_color_table, short gamma_level);

void explicit_start_fade(short type, struct color_table *original_color_table, struct color_table *animated_color_table);
void full_fade(short type, struct color_table *original_color_table);

// LP change: added fader-parser export
XML_ElementParser *Faders_GetParser();

#endif

