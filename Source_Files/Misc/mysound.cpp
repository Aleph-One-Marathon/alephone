/*
SOUND.C
Friday, March 26, 1993 10:16:00 AM

Friday, March 26, 1993 10:16:01 AM
	this is the source file that will make minotaur’s sound manager look like a truckload of dead
	rats in a tampon factory.
Saturday, March 27, 1993 8:13:44 PM
	a few things have been changed, i think most of the coding is done.
Tuesday, April 6, 1993 9:26:54 PM
	is there any way to use SyncCmd’s to better synchronize the left and right channels of
	stereo sound?  why do all sounds end in a click when the sound manager is doing realtime
	sound mixing?
Tuesday, May 18, 1993 9:35:21 PM
	ha ha ha.  after wwdc93, hacking out jim reekes’ new stereo amplitude command, rewriting all
	of this crap so it’ll actually work like you might expect.  fixed a bug where the sample
	rate wasn’t restored after drinking the potion.  we used to have tons of problems in our
	amplitude calculations (that was why it didn’t initially work under the new sound driver).
Sunday, July 4, 1993 7:56:21 AM
	there exists a bug such that for every channel (especially the monster channel) there is
	a possibility that the channel will stop playing sounds but report no errors.
Friday, July 16, 1993 6:04:11 PM
	the last_played field of a sound definition was not being set by play_sound, so the sound
	which was first loaded will be disposed of first.  would that manifest in the ‘pause bug’?
	alex has a version of Suitcase which was marking all the sounds I loaded as purgable.
Friday, July 23, 1993 9:43:39 PM
	the pause bug was SCSI Probe’s COMMAND-SPACE hotkey.  reduced sound space to 250k.  i can’t
	wait to rewrite this and make it real.
Saturday, November 20, 1993 10:32:34 PM
	‘making this real’ means making it a true n-channel sound manager, where n is some function
	of the current cpu.  this means throwing away all that channel code garbage.
Sunday, January 2, 1994 11:21:41 AM
	sound_code was not being set in play_sound; added threshhold to best_channel
Friday, August 19, 1994 8:26:31 PM
	added custom depth-fading curves, which respect obstructions, and removed the MoveHHi
	before every sound is played.  this is bad news, because we might have sounds locked down
	in our heap when we tried to load shapes, other sounds, etc.  we try to improve this
	situation slightly with the addition of the sound_manager_idle_proc (to unlock handles)
	and by asking everyone who really needs memory to call stop_all_sounds, which will unlock
	the sound handles.
Friday, September 2, 1994 1:29:03 PM
	moving to slot-base collection model fixed a gnarly reentrancy bug while releasing sounds.
Tuesday, September 6, 1994 2:53:35 PM
	added _depth_fading_flag and _high_quality_flag (resamples 22k to 11k if FALSE).
Sunday, November 6, 1994 9:17:24 PM  (Jason)
	if the sound manager is never initialized, calling any of it’s procedures doesn’t crash.
Friday, December 2, 1994 5:26:10 PM  (Jason)
	old= CurResFile(); UseResFile(_sm_sound_file_handle); Get1Resource('snd', ...); UseResFile(old);
Saturday, December 10, 1994 4:32:46 PM  (Jason)
	in search of the golden sample rate: changing rate22khz to 0x56220000 on PowerMacs didn’t
	seem to make any difference.
Friday, January 27, 1995 12:49:25 AM  (Jason')
	the marathon 2 rewrite begins.
Friday, June 16, 1995 10:18:42 AM  (Jason)
	background, ambient and scenery sounds.
Friday, July 7, 1995 12:47:22 PM  (Jason)
	_sound_cannot_be_aborted, sounds which are not always played
Thursday, August 17, 1995 10:41:51 AM  (Jason)
	fixed sound pitches.
Tuesday, August 29, 1995 8:56:16 AM  (Jason)
	moved macintosh-specific code to SOUND_MACINTOSH.C.

Jan 30, 2000 (Loren Petrich):
	Added some typecasts
	Removed some "static" declarations that conflict with "extern"

Feb 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Mar 23, 2000 (Loren Petrich):
	Increased maximum number of sound channels from 4 to 8;
	also changed the resource-fork menu.

May 27, 2000 (Loren Petrich):
	Fixed directionality bug for long-distance sounds;
	made some distance calculations more long-distance friendly

Jun 3, 2000 (Loren Petrich):
	Idiot-proofed the sound accessors; when a sound index is out of range,
	an accessor returns NULL.

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler

Aug 26, 2000 (Loren Petrich):
	Moved get_default_sounds_spec() to preprocess_map_mac.c
*/

/*
sound pitches do not work

there should be no difference between ambient and normal sound channels
shortening radii on low-volume ambient sound sorces would be a good idea
*/

#include "cseries.h"

#include <stdlib.h>

#ifdef mac
#include <Gestalt.h>
#endif

#include "shell.h"

#include "world.h"
#include "interface.h"
#include "mysound.h"
#include "byte_swapping.h"
#include "FileHandler.h"

#include <string.h>

#ifdef env68k
#pragma segment sound
#endif

/* ---------- constants */

enum
{
	MAXIMUM_OUTPUT_SOUND_VOLUME= 2*MAXIMUM_SOUND_VOLUME,
	SOUND_VOLUME_DELTA= MAXIMUM_OUTPUT_SOUND_VOLUME/NUMBER_OF_SOUND_VOLUME_LEVELS,
	DEFAULT_SOUND_LEVEL= NUMBER_OF_SOUND_VOLUME_LEVELS/3,

	ABORT_AMPLITUDE_THRESHHOLD= (MAXIMUM_SOUND_VOLUME/6),
	MINIMUM_RESTART_TICKS= MACHINE_TICKS_PER_SECOND/12,
	
	// LP change: increase maximum number of sound channels and also
	// ambient ones
	MAXIMUM_SOUND_CHANNELS= 8,
	// MAXIMUM_SOUND_CHANNELS= 4,
	MAXIMUM_AMBIENT_SOUND_CHANNELS= 4,
	// MAXIMUM_AMBIENT_SOUND_CHANNELS= 2,

	MINIMUM_SOUND_PITCH= 1,
	MAXIMUM_SOUND_PITCH= 256*FIXED_ONE
};

enum /* channel flags */
{
	_sound_is_local= 0x0001 // .source is invalid
};

/* ---------- macros */

/* from marathon map.h */
#define SLOT_IS_USED(o) ((o)->flags&(word)0x8000)
#define SLOT_IS_FREE(o) (!SLOT_IS_USED(o))
#define MARK_SLOT_AS_FREE(o) ((o)->flags&=(word)~0x8000)
#define MARK_SLOT_AS_USED(o) ((o)->flags|=(word)0x8000)

#define AMBIENT_SOUND_CHANNELS (_sm_globals->channels + _sm_parameters->channel_count)

/* ---------- structures */

struct sound_variables
{
	short volume, left_volume, right_volume;
	fixed original_pitch, pitch;
	
	short priority;
};

struct channel_data
{
	word flags;

	short sound_index; /* sound_index being played in this channel */

	short identifier; /* unique sound identifier for the sound being played in this channel (object_index) */
	struct sound_variables variables; /* variables of the sound being played */	
	world_location3d *dynamic_source; /* can be NULL for immobile sounds */
	world_location3d source; /* must be valid */

	unsigned long start_tick;

#if defined(mac)
	SndChannelPtr channel;
#elif defined(SDL)
	void *channel;
#endif
	short callback_count;
};

struct sound_manager_globals
{
	short total_channel_count;
	long total_buffer_size;

	short sound_source; // 8-bit, 16-bit
	struct sound_definition *base_sound_definitions;
	
	word available_flags;
	
	long loaded_sounds_size;
	
	struct channel_data channels[MAXIMUM_SOUND_CHANNELS+MAXIMUM_AMBIENT_SOUND_CHANNELS];

#ifdef mac
	long old_sound_volume;
	SndCallBackUPP sound_callback_upp;
#endif
};

/* ---------- globals */

static boolean _sm_initialized, _sm_active;
static struct sound_manager_globals *_sm_globals;
static struct sound_manager_parameters *_sm_parameters;

// Moved out of _sm_globals so it could work correctly;
// there is apparently some sort of buffer overrun somwhere.
static OpenedFile SoundFile;

/* include globals */
#include "sound_definitions.h"

/* ---------- machine-specific prototypes */

static void initialize_machine_sound_manager(struct sound_manager_parameters *parameters);

static boolean channel_busy(struct channel_data *channel);
static void unlock_sound(short sound_index);
static void dispose_sound(short sound_index);
// Returns not the handle, but the pointer and size
static byte *read_sound_from_file(short sound_index, int32 &size);

static void quiet_channel(struct channel_data *channel);
static void instantiate_sound_variables(struct sound_variables *variables,
	struct channel_data *channel, boolean first_time);
static void buffer_sound(struct channel_data *channel, short sound_index, fixed pitch);

/* ---------- private prototypes */

static void track_stereo_sounds(void);
static void unlock_locked_sounds(void);

static boolean _load_sound(short sound);
static short _release_least_useful_sound(void);

static struct channel_data *best_channel(short sound_index, struct sound_variables *variables);
static void free_channel(struct channel_data *channel);

static short get_random_sound_permutation(short sound_index);

static void calculate_initial_sound_variables(short sound_index, world_location3d *source,
	struct sound_variables *variables, fixed pitch_modifier);
static void calculate_sound_variables(short sound_index, world_location3d *source,
	struct sound_variables *variables);
static fixed calculate_pitch_modifier(short sound_index, fixed pitch_modifier);

static void angle_and_volume_to_stereo_volume(angle delta, short volume, short *right_volume, short *left_volume);
static short distance_to_volume(struct sound_definition *definition, world_distance distance,
	word flags);

static void update_ambient_sound_sources(void);

/* ---------- private code */

// LP change: inlined all these accessors; they return NULL for an invalid index value:

struct sound_definition *get_sound_definition(
	const short sound_index)
{
	return GetMemberWithBounds(_sm_globals->base_sound_definitions,sound_index,NUMBER_OF_SOUND_DEFINITIONS);
}

struct ambient_sound_definition *get_ambient_sound_definition(
	const short ambient_sound_index)
{
	return GetMemberWithBounds(ambient_sound_definitions,ambient_sound_index,NUMBER_OF_AMBIENT_SOUND_DEFINITIONS);
}

struct random_sound_definition *get_random_sound_definition(
	const short random_sound_index)
{
	return GetMemberWithBounds(random_sound_definitions,random_sound_index,NUMBER_OF_RANDOM_SOUND_DEFINITIONS);
}

struct sound_behavior_definition *get_sound_behavior_definition(
	const short sound_behavior_index)
{
	return GetMemberWithBounds(sound_behavior_definitions,sound_behavior_index,NUMBER_OF_SOUND_BEHAVIOR_DEFINITIONS);
}

/*
#ifdef DEBUG
struct sound_definition *get_sound_definition(short sound_index);
struct ambient_sound_definition *get_ambient_sound_definition(short ambient_sound_index);
struct random_sound_definition *get_random_sound_definition(short random_sound_index);
struct sound_behavior_definition *get_sound_behavior_definition(short sound_behavior_index);
#else
#define get_sound_definition(i) (_sm_globals->base_sound_definitions+(i))
#define get_ambient_sound_definition(i) (ambient_sound_definitions+(i))
#define get_random_sound_definition(i) (random_sound_definitions+(i))
#define get_sound_behavior_definition(i) (sound_behavior_definitions+(i))
#endif
*/

/* ---------- machine-specific code */

#if defined(mac)
#include "sound_macintosh.c"
#elif defined(SDL)
#include "sound_sdl.cpp"
#endif

/* ---------- code */

void initialize_sound_manager(
	struct sound_manager_parameters *parameters)
{
	_sm_globals= (struct sound_manager_globals *) malloc(sizeof(struct sound_manager_globals));
	_sm_parameters= (struct sound_manager_parameters *) malloc(sizeof(struct sound_manager_parameters));
	sound_definitions= (struct sound_definition *) malloc(NUMBER_OF_SOUND_SOURCES*NUMBER_OF_SOUND_DEFINITIONS*sizeof(struct sound_definition));
	assert(_sm_globals && _sm_parameters && sound_definitions);

	obj_clear(*_sm_globals);
	obj_clear(*_sm_parameters);
	objlist_clear(sound_definitions, NUMBER_OF_SOUND_SOURCES * NUMBER_OF_SOUND_DEFINITIONS);
	
	initialize_machine_sound_manager(parameters);
	
	return;
}

void load_sound(
	short sound)
{
	if (_sm_active)
	{
		if (sound!=NONE)
		{
			_load_sound(sound);
		}
	}
	
	return;
}

void load_sounds(
	short *sounds,
	short count)
{
	short i;
	
	for (i= 0; i<count; ++i)
	{
		load_sound(sounds[i]);
	}
	
	return;
}

void sound_manager_idle_proc(
	void)
{
	if (_sm_active && _sm_globals->total_channel_count>0)
	{
		unlock_locked_sounds();
		track_stereo_sounds();
		cause_ambient_sound_source_update();
	}
	
	return;
}

void cause_ambient_sound_source_update(
	void)
{
	if (_sm_active && _sm_parameters->volume>0 && _sm_globals->total_channel_count>0)
	{
		if (_sm_parameters->flags&_ambient_sound_flag)
		{
			update_ambient_sound_sources();
		}
	}
	
	return;
}

void direct_play_sound(
	short sound_index,
	angle direction, // can be NONE
	short volume,
	fixed pitch)
{
	/* don’t do anything if we’re not initialized or active, or our sound_code is NONE,
		or our volume is zero, our we have no sound channels */
	if (sound_index!=NONE && _sm_active && sound_index<NUMBER_OF_SOUND_DEFINITIONS &&
		_sm_parameters->volume>0 && _sm_globals->total_channel_count>0)
	{
		struct sound_variables variables;
		struct channel_data *channel;

		/* make sure the sound data is in memory */
		if (_load_sound(sound_index))
		{
			/* get the channel, and free it for our new sound */
			if ((channel= best_channel(sound_index, &variables))!=NULL)
			{
				world_location3d *listener= _sound_listener_proc();
				
				variables.priority= 0;
				variables.volume= volume;
				
				if (direction==NONE || !listener)
				{
					variables.left_volume= variables.right_volume= volume;
				}
				else
				{
					angle_and_volume_to_stereo_volume(direction - listener->yaw,
						volume, &variables.right_volume, &variables.left_volume);
				}
		
				/* set the volume and pitch in this channel */
				instantiate_sound_variables(&variables, channel, TRUE);

				/* initialize the channel */
				channel->flags= _sound_is_local; // but possibly being played in stereo
				channel->callback_count= 0; // #MD
				channel->start_tick= machine_tick_count();
				channel->sound_index= sound_index;
				channel->identifier= NONE;
				channel->dynamic_source= (world_location3d *) NULL;
				MARK_SLOT_AS_USED(channel);

				/* start the sound playing */
				buffer_sound(channel, sound_index, pitch);
			}
		}
	}
	
	return;
}

void _play_sound(
	short sound_index,
	world_location3d *source,
	short identifier, /* NONE is no identifier and the sound is immediately orphaned */
	fixed pitch) /* on top of all existing pitch modifiers */
{
	/* don’t do anything if we’re not initialized or active, or our sound_code is NONE,
		or our volume is zero, our we have no sound channels */
	if (sound_index!=NONE && _sm_active && sound_index<NUMBER_OF_SOUND_DEFINITIONS &&
		_sm_parameters->volume>0 && _sm_globals->total_channel_count>0)
	{
		struct sound_variables variables;
		struct channel_data *channel;

		calculate_initial_sound_variables(sound_index, source, &variables, pitch);
		
		/* make sure the sound data is in memory */
		if (_load_sound(sound_index))
		{
			/* get the channel, and free it for our new sound */
			if ((channel= best_channel(sound_index, &variables))!=NULL)
			{
				/* set the volume and pitch in this channel */
				instantiate_sound_variables(&variables, channel, TRUE);
				
				/* initialize the channel */
				channel->flags= 0;
				channel->callback_count= 0; // #MD
				channel->start_tick= machine_tick_count();
				channel->sound_index= sound_index;
				channel->identifier= identifier;
				channel->dynamic_source= (identifier==NONE) ? (world_location3d *) NULL : source;
				MARK_SLOT_AS_USED(channel);
				
				/* start the sound playing */
				buffer_sound(channel, sound_index, pitch);

				/* if we have a valid source, copy it, otherwise remember that we don’t */
				if (source)
				{
					channel->source= *source;
				}
				else
				{
					channel->flags|= _sound_is_local;
				}
			}
		}
	}
	
	return;
}

void unload_all_sounds(
	void)
{
	if (_sm_active)
	{
		stop_all_sounds();
		
		while (_release_least_useful_sound()!=NONE)
			;
	}
	
	return;
}

void stop_sound(
	short identifier,
	short sound_index)
{
	if (_sm_active && _sm_globals->total_channel_count>0)
	{
		short i;
		struct channel_data *channel;

		// if we’re stopping everything...
		if (identifier==NONE && sound_index==NONE)
		{
			// can’t fade to silence here
			
			// stop the ambient sound channels, too
			if (_sm_parameters->flags&_ambient_sound_flag)
			{
				for (i= 0, channel= AMBIENT_SOUND_CHANNELS; i<MAXIMUM_AMBIENT_SOUND_CHANNELS; ++i, ++channel)
				{
					free_channel(channel);
				}
			}
		}

		for (i= 0, channel= _sm_globals->channels; i<_sm_parameters->channel_count; ++i, ++channel)
		{
			if (SLOT_IS_USED(channel) && (channel->identifier==identifier || identifier==NONE) &&
				(channel->sound_index==sound_index || sound_index==NONE))
			{
				free_channel(channel);
			}
		}
	}
	
	return;
 }

// doesn’t check ambient sounds
boolean sound_is_playing(
	short sound_index)
{
	boolean sound_playing= FALSE;
	
	if (_sm_active && _sm_globals->total_channel_count>0)
	{
		short i;
		struct channel_data *channel;
		
		for (i= 0, channel= _sm_globals->channels; i<_sm_globals->total_channel_count; ++i, ++channel)
		{
			if (SLOT_IS_USED(channel) && channel->sound_index==sound_index)
			{
				sound_playing= TRUE;
			}
		}
		
		unlock_locked_sounds();
	}
	
	return sound_playing;
}

void orphan_sound(
	short identifier)
{
	if (_sm_active && _sm_globals->total_channel_count>0)
	{
		short i;
		struct channel_data *channel;
		
		for (i= 0, channel= _sm_globals->channels; i<_sm_parameters->channel_count; ++i, ++channel)
		{
			if (channel->identifier==identifier || identifier==NONE)
			{
				channel->dynamic_source= (world_location3d *) NULL;
				channel->identifier= NONE;
			}
		}
	}
	
	return;
}

word available_sound_manager_flags(
	word flags)
{
	word available_flags= _sm_active ? _sm_globals->available_flags : 0;
	
	if (!(flags&_stereo_flag)) available_flags&= ~(word)_dynamic_tracking_flag;
	
	return available_flags;
}

void default_sound_manager_parameters(
	void *prefs)
{
	struct sound_manager_parameters *parameters=(struct sound_manager_parameters *)prefs;

	obj_clear(*parameters);
	
	parameters->channel_count= MAXIMUM_SOUND_CHANNELS;
	parameters->volume= DEFAULT_SOUND_LEVEL;
	parameters->flags= _more_sounds_flag;
	parameters->pitch= FIXED_ONE;
	
	return;
}

boolean verify_sound_manager_parameters(
	struct sound_manager_parameters *parameters)
{
	// pin parameters
	parameters->channel_count= PIN(parameters->channel_count, 0, MAXIMUM_SOUND_CHANNELS);
	parameters->volume= PIN(parameters->volume, 0, NUMBER_OF_SOUND_VOLUME_LEVELS);
	parameters->pitch= PIN(parameters->pitch, MINIMUM_SOUND_PITCH, MAXIMUM_SOUND_PITCH);

	// adjust flags	
	parameters->flags&= _sm_globals->available_flags;
	
	return TRUE;
}

short random_sound_index_to_sound_index(
	short random_sound_index)
{
	struct random_sound_definition *definition= get_random_sound_definition(random_sound_index);
	
	// LP change: make NONE in case this sound definition is invalid
	if (definition)
		return definition->sound_index;
	else
		return NONE;
}

/* ---------- private code */
/*
#ifdef DEBUG
// LP: "static" removed
struct sound_definition *get_sound_definition(
	short sound_index)
{
	struct sound_definition *definition= _sm_globals->base_sound_definitions + sound_index;
	
	// LP change: idiot-proofing
	if (!(sound_index>=0 && sound_index<NUMBER_OF_SOUND_DEFINITIONS)) return NULL;
	 vassert(sound_index>=0 && sound_index<NUMBER_OF_SOUND_DEFINITIONS,
		csprintf(temporary, "sound #%d is out of range [0,#%d)", sound_index, NUMBER_OF_SOUND_DEFINITIONS));
	return definition;
}

// LP: "static" removed
// LP change: if invalid, return NULL
struct ambient_sound_definition *get_ambient_sound_definition(
	short ambient_sound_index)
{
	// LP change: idiot-proofing
	if (!(ambient_sound_index>=0 && ambient_sound_index<NUMBER_OF_AMBIENT_SOUND_DEFINITIONS)) return NULL;
	vassert(ambient_sound_index>=0 && ambient_sound_index<NUMBER_OF_AMBIENT_SOUND_DEFINITIONS,
		csprintf(temporary, "ambient sound #%d is out of range [0,#%d)", ambient_sound_index, NUMBER_OF_AMBIENT_SOUND_DEFINITIONS));
	
	return ambient_sound_definitions + ambient_sound_index;
}

// LP: "static" removed
// LP change: if invalid, return NULL
struct random_sound_definition *get_random_sound_definition(
	short random_sound_index)
{
	// LP change: idiot-proofing
	if (!(random_sound_index>=0 && random_sound_index<NUMBER_OF_RANDOM_SOUND_DEFINITIONS)) return NULL;
	vassert(random_sound_index>=0 && random_sound_index<NUMBER_OF_RANDOM_SOUND_DEFINITIONS,
		csprintf(temporary, "random sound #%d is out of range [0,#%d)", random_sound_index, NUMBER_OF_RANDOM_SOUND_DEFINITIONS));
	
	return random_sound_definitions + random_sound_index;
}

// LP: "static" removed
struct sound_behavior_definition *get_sound_behavior_definition(
	short sound_behavior_index)
{
	// LP change: idiot-proofing
	if (!(sound_behavior_index>=0 && sound_behavior_index<NUMBER_OF_SOUND_BEHAVIOR_DEFINITIONS)) return NULL;
	vassert(sound_behavior_index>=0 && sound_behavior_index<NUMBER_OF_SOUND_BEHAVIOR_DEFINITIONS,
		csprintf(temporary, "sound behavior #%d is out of range [0,#%d)", sound_behavior_index, NUMBER_OF_SOUND_BEHAVIOR_DEFINITIONS));
	return sound_behavior_definitions + sound_behavior_index;
}
#endif
*/

static void unlock_locked_sounds(
	void)
{
	if (_sm_active && _sm_globals->total_channel_count>0)
	{
		short i;
		struct channel_data *channel;
		
		// if we're done playing a locked sound, dispose it
		for (i= 0, channel= _sm_globals->channels; i<_sm_parameters->channel_count; ++i, ++channel)
		{
			if (SLOT_IS_USED(channel) && !channel_busy(channel))
			{
				free_channel(channel);
			}
		}
	}
	
	return;
}

static void track_stereo_sounds(
	void)
{
	if (_sm_active && _sm_globals->total_channel_count>0 && (_sm_parameters->flags&_dynamic_tracking_flag))
	{
		short i;
		struct channel_data *channel;
		
		// if we're done playing a locked sound, dispose it
		for (i= 0, channel= _sm_globals->channels; i<_sm_parameters->channel_count; ++i, ++channel)
		{
			if (SLOT_IS_USED(channel) && channel_busy(channel) && !(channel->flags&_sound_is_local))
			{
				struct sound_variables variables= channel->variables;
				
				if (channel->dynamic_source) channel->source= *channel->dynamic_source;
				calculate_sound_variables(channel->sound_index, &channel->source, &variables);
				instantiate_sound_variables(&variables, channel, FALSE);
			}
		}
	}

	return;
}

static short get_random_sound_permutation(
	short sound_index)
{
	struct sound_definition *definition= get_sound_definition(sound_index);
	// LP change: idiot-proofing
	if (!definition) return 0;
	short permutation;
	
	// LP change: idiot-proofing
	if (!(definition->permutations>0)) return 0;
	// assert(definition->permutations>0);

	if (_sm_parameters->flags&_more_sounds_flag)
	{
		if ((definition->permutations_played&((1<<definition->permutations)-1))==((1<<definition->permutations)-1)) definition->permutations_played= 0;
		permutation= local_random()%definition->permutations;
		while (definition->permutations_played & (1<<permutation)) if ((permutation+= 1)>=definition->permutations) permutation= 0;
		definition->permutations_played|= 1<<permutation;
	}
	else
	{
		permutation= 0;
	}

	definition->last_played= machine_tick_count();

	return permutation;
}

static struct channel_data *best_channel(
	short sound_index,
	struct sound_variables *variables)
{
	short i;
	struct channel_data *channel;
	struct channel_data *best_channel;
	struct sound_definition *definition= get_sound_definition(sound_index);
	if (!definition) return NULL;
	
	best_channel= (struct channel_data *) NULL;
	if (!definition->chance || (local_random()>definition->chance))
	{
		for (i= 0, channel= _sm_globals->channels; i<_sm_parameters->channel_count; ++i, ++channel)
		{
			if (SLOT_IS_USED(channel) && channel_busy(channel))
			{
				/* if this channel is at a lower volume than the sound we are trying to play and is at the same
					priority, or the channel is at a lower priority, then we can abort it */
				if ((channel->variables.volume<=variables->volume+ABORT_AMPLITUDE_THRESHHOLD && channel->variables.priority==variables->priority) ||
					channel->variables.priority<variables->priority)
				{
					/* if this channel is already playing our sound, this is our channel (for
						better or for worse) */
					if (channel->sound_index==sound_index)
					{
						if (definition->flags&_sound_cannot_be_restarted)
						{
							best_channel= (struct channel_data *) NULL;
							break;
						}
						
						if (!(definition->flags&_sound_does_not_self_abort))
						{
							best_channel= (channel->start_tick+MINIMUM_RESTART_TICKS<machine_tick_count()) ? channel : (struct channel_data *) NULL;
							break;
						}
					}
					
					/* if we haven’t found an alternative channel or this channel is at a lower
						volume than our previously best channel (which isn’t an unused channel),
						then we’ve found a new best channel */
					if (!best_channel ||
						(SLOT_IS_USED(best_channel) && best_channel->variables.volume>channel->variables.volume) ||
						(SLOT_IS_USED(best_channel) && best_channel->variables.priority<channel->variables.priority))
					{
						best_channel= channel;
					}
				}
				else
				{
					if (channel->sound_index==sound_index && !(definition->flags&_sound_does_not_self_abort))
					{
						/* if we’re already playing this sound at a higher volume, don’t abort it */
						best_channel= (struct channel_data *) NULL;
						break;
					}
				}
			}
			else
			{
				/* unused channel (we won’t get much better than this!) */
				if (SLOT_IS_USED(channel)) free_channel(channel);
				best_channel= channel;
			}
		}
	}

	if (best_channel)
	{
		/* stop whatever sound is playing and unlock the old handle if necessary */
		free_channel(best_channel);
	}
	
	return best_channel;
}

static short _release_least_useful_sound(
	void)
{
	short sound_index, least_used_sound_index= NONE;
	struct sound_definition *least_used_definition= (struct sound_definition *) NULL;
	struct sound_definition *definition;

	for (sound_index= 0, definition= _sm_globals->base_sound_definitions; sound_index<NUMBER_OF_SOUND_DEFINITIONS; ++sound_index, ++definition)
	{
		// if (definition->handle && (!least_used_definition || least_used_definition->last_played>definition->last_played))
		if (definition->ptr && (!least_used_definition || least_used_definition->last_played>definition->last_played))
		{
			least_used_sound_index= sound_index;
			least_used_definition= definition;
		}
	}
	
	if (least_used_sound_index!=NONE)
	{
		stop_sound(NONE, least_used_sound_index);
		dispose_sound(least_used_sound_index);
	}
	
	return least_used_sound_index;
}

/* if .sound_handle isn’t NULL then we still have a locked handle in our heap; so unlock the
	handle and clear the .sound_handle field */
static void free_channel(
	struct channel_data *empty_channel)
{
	if (SLOT_IS_USED(empty_channel))
	{
		short sound_index= empty_channel->sound_index;
		
		quiet_channel(empty_channel);
	
		assert(sound_index!=NONE);
		empty_channel->sound_index= NONE;
		MARK_SLOT_AS_FREE(empty_channel);
		
		// if anybody else is playing this sound_index, we can’t unlock the handle
		if (!sound_is_playing(sound_index)) unlock_sound(sound_index);
	}
	
	return;
}

boolean _load_sound(
	short sound_index)
{
	struct sound_definition *definition= get_sound_definition(sound_index);
	// LP change: idiot-proofing
	if (!definition) return FALSE;
	boolean successful= FALSE;
		
	if (definition->sound_code!=NONE &&
		((_sm_parameters->flags&_ambient_sound_flag) || !(definition->flags&_sound_is_ambient)))
	{
		if (!definition->ptr)
		{
			definition->ptr = read_sound_from_file(sound_index, definition->size);
			definition->last_played= machine_tick_count();
			
			while (_sm_globals->loaded_sounds_size>_sm_globals->total_buffer_size) _release_least_useful_sound();
		}
		
		if (definition->ptr)
		{
			definition->permutations_played= 0;
		}
	}
	
	return definition->ptr ? TRUE : FALSE;
}

static void calculate_initial_sound_variables(
	short sound_index,
	world_location3d *source,
	struct sound_variables *variables,
	fixed pitch_modifier)
{
	struct sound_definition *definition= get_sound_definition(sound_index);
	// LP change: idiot-proofing
	if (!definition) return;

	(void) (pitch_modifier);

	/* if we have no sound source, play at full volume */
	if (!source)
	{
		variables->volume= variables->left_volume= variables->right_volume= MAXIMUM_SOUND_VOLUME;
	}

	/* and finally, do all the stuff we regularly do ... */
	calculate_sound_variables(sound_index, source, variables);

	return;
}

static fixed calculate_pitch_modifier(
	short sound_index,
	fixed pitch_modifier)
{
	struct sound_definition *definition= get_sound_definition(sound_index);
	// LP change: idiot-proofing
	if (!definition) return FIXED_ONE;

	if (!(definition->flags&_sound_cannot_change_pitch))
	{
		if (!(definition->flags&_sound_resists_pitch_changes))
		{
			pitch_modifier+= ((FIXED_ONE-pitch_modifier)>>1);
		}
	}
	else
	{
		pitch_modifier= FIXED_ONE;
	}
	
	return pitch_modifier;
}

static void calculate_sound_variables(
	short sound_index,
	world_location3d *source,
	struct sound_variables *variables)
{
	struct sound_definition *definition= get_sound_definition(sound_index);
	// LP change: idiot-proofing
	if (!definition) return;
	world_location3d *listener= _sound_listener_proc();

	if (source && listener)
	{
		world_distance distance= distance3d(&source->point, &listener->point);
		// LP change: made this long-distance friendly
		long dx= long(listener->point.x) - long(source->point.x);
		long dy= long(listener->point.y) - long(source->point.y);
		// world_distance dx= listener->point.x - source->point.x;
		// world_distance dy= listener->point.y - source->point.y;
		
		/* for now, a sound's priority is it's behavior_index */
		variables->priority= definition->behavior_index;
	
		/* calculate the relative volume due to the given depth curve */
		variables->volume= distance_to_volume(definition, distance, _sound_obstructed_proc(source));

		if (dx || dy)
		{
			/* set volume, left_volume, right_volume */
			angle_and_volume_to_stereo_volume(arctangent(dx, dy) - listener->yaw,
				variables->volume, &variables->right_volume, &variables->left_volume);
		}
		else
		{
			variables->left_volume= variables->right_volume= variables->volume;
		}
	}

	return;
}

static void angle_and_volume_to_stereo_volume(
	angle delta,
	short volume,
	short *right_volume,
	short *left_volume)
{
	if (_sm_parameters->flags&_stereo_flag)
	{
		short fraction= delta&((1<<(ANGULAR_BITS-2))-1);
		short maximum_volume= volume + (volume>>1);
		short minimum_volume= volume>>2;
		short middle_volume= volume-minimum_volume;
		
		switch (NORMALIZE_ANGLE(delta)>>(ANGULAR_BITS-2))
		{
			case 0: // rear right quarter [v,vmax] [v,vmin]
				*left_volume= middle_volume + ((fraction*(maximum_volume-middle_volume))>>(ANGULAR_BITS-2));
				*right_volume= middle_volume + ((fraction*(minimum_volume-middle_volume))>>(ANGULAR_BITS-2));
				break;
			
			case 1: // front right quarter [vmax,vmid] [vmin,vmid]
				*left_volume= maximum_volume + ((fraction*(volume-maximum_volume))>>(ANGULAR_BITS-2));
				*right_volume= minimum_volume + ((fraction*(volume-minimum_volume))>>(ANGULAR_BITS-2));
				break;
			
			case 2: // front left quarter [vmid,vmin] [vmid,vmax]
				*left_volume= volume + ((fraction*(minimum_volume-volume))>>(ANGULAR_BITS-2));
				*right_volume= volume + ((fraction*(maximum_volume-volume))>>(ANGULAR_BITS-2));
				break;
			
			case 3: // rear left quarter [vmin,v] [vmax,v]
				*left_volume= minimum_volume + ((fraction*(middle_volume-minimum_volume))>>(ANGULAR_BITS-2));
				*right_volume= maximum_volume + ((fraction*(middle_volume-maximum_volume))>>(ANGULAR_BITS-2));
				break;
			
			default:
				// LP change:
				assert(false);
				// halt();
		}
	}
	else
	{
		*left_volume= *right_volume= volume;
	}

	return;
}

static short distance_to_volume(
	struct sound_definition *definition,
	world_distance distance,
	word flags)
{
	struct sound_behavior_definition *behavior= get_sound_behavior_definition(definition->behavior_index);
	// LP change: idiot-proofing
	if (!behavior) return 0; // Silence
	
	struct depth_curve_definition *depth_curve;
	short volume;

	if (((flags&_sound_was_obstructed) && !(definition->flags&_sound_cannot_be_obstructed)) ||
		((flags&_sound_was_media_obstructed) && !(definition->flags&_sound_cannot_be_media_obstructed)))
	{
		depth_curve= &behavior->obstructed_curve;
	}
	else
	{
		depth_curve= &behavior->unobstructed_curve;
	}
	
	if (distance<=depth_curve->maximum_volume_distance)
	{
		volume= depth_curve->maximum_volume;
	}
	else
	{
		if (distance>depth_curve->minimum_volume_distance)
		{
			volume= depth_curve->minimum_volume;
		}
		else
		{
			volume= depth_curve->minimum_volume - ((depth_curve->minimum_volume-depth_curve->maximum_volume)*(depth_curve->minimum_volume_distance-distance)) /
				(depth_curve->minimum_volume_distance-depth_curve->maximum_volume_distance);
		}
	}
	
	if ((flags&_sound_was_media_muffled) && !(definition->flags&_sound_cannot_be_media_obstructed))
	{
		volume>>= 1;
	}
	
	return volume;
}

/* ---------- ambient sound sources */

enum
{
	MAXIMUM_PROCESSED_AMBIENT_SOUNDS= 5
};

struct ambient_sound_data
{
	word flags;
	short sound_index;

	struct sound_variables variables;
	
	struct channel_data *channel;
};

static void add_one_ambient_sound_source(struct ambient_sound_data *ambient_sounds,
	world_location3d *source, world_location3d *listener, short sound_index,
	short absolute_volume);

static void update_ambient_sound_sources(
	void)
{
	struct ambient_sound_data ambient_sounds[MAXIMUM_PROCESSED_AMBIENT_SOUNDS];
	struct ambient_sound_data *ambient;
	struct channel_data *channel;
	boolean channel_used[MAXIMUM_AMBIENT_SOUND_CHANNELS], sound_handled[MAXIMUM_PROCESSED_AMBIENT_SOUNDS];
	short i, j;

	// reset all local copies	
	for (i= 0, ambient= ambient_sounds; i<MAXIMUM_PROCESSED_AMBIENT_SOUNDS; ++i, ++ambient)
	{
		ambient->flags= 0;
		ambient->sound_index= NONE;
		
		sound_handled[i]= FALSE;
	}

	for (i= 0; i<MAXIMUM_AMBIENT_SOUND_CHANNELS; ++i)
	{
		channel_used[i]= FALSE;
	}
	
	// accumulate up to MAXIMUM_PROCESSED_AMBIENT_SOUNDS worth of sounds
	_sound_add_ambient_sources_proc(&ambient_sounds, add_one_ambient_sound_source);

	// remove all zero volume sounds
	for (i= 0, ambient= ambient_sounds; i<MAXIMUM_PROCESSED_AMBIENT_SOUNDS; ++i, ++ambient)
	{
		if (SLOT_IS_USED(ambient) && !ambient->variables.volume) MARK_SLOT_AS_FREE(ambient);
	}

	{
		struct ambient_sound_data *lowest_priority;
		short count;
		
		do
		{
			lowest_priority= (struct ambient_sound_data *) NULL;
			count= 0;
			
			for (i= 0, ambient= ambient_sounds; i<MAXIMUM_PROCESSED_AMBIENT_SOUNDS; ++i, ++ambient)
			{
				if (SLOT_IS_USED(ambient))
				{
					if (!lowest_priority ||
//						lowest_priority->sound_index>ambient->sound_index ||
						lowest_priority->variables.volume>ambient->variables.volume+ABORT_AMPLITUDE_THRESHHOLD)
					{
						lowest_priority= ambient;
					}
					
					count+= 1;
				}
			}
			
			if (count>MAXIMUM_AMBIENT_SOUND_CHANNELS)
			{
				assert(lowest_priority);
				MARK_SLOT_AS_FREE(lowest_priority);
				count-= 1;
			}
		}
		while (count>MAXIMUM_AMBIENT_SOUND_CHANNELS);
	}
	
	// update .variables of those sounds which we are already playing
	for (i= 0, ambient= ambient_sounds; i<MAXIMUM_PROCESSED_AMBIENT_SOUNDS; ++i, ++ambient)
	{
		if (SLOT_IS_USED(ambient))
		{
			for (j= 0, channel= AMBIENT_SOUND_CHANNELS; j<MAXIMUM_AMBIENT_SOUND_CHANNELS; ++j, ++channel)
			{
				if (SLOT_IS_USED(channel) && channel->sound_index==ambient->sound_index)
				{
					instantiate_sound_variables(&ambient->variables, channel, FALSE);
					
					sound_handled[i]= channel_used[j]= TRUE;
					
					break;
				}
			}
		}
	}
	
	// allocate a channel for a sound we just started playing
	for (i= 0, ambient= ambient_sounds; i<MAXIMUM_PROCESSED_AMBIENT_SOUNDS; ++i, ++ambient)
	{
		if (SLOT_IS_USED(ambient) && !sound_handled[i])
		{
			for (j= 0, channel= AMBIENT_SOUND_CHANNELS; j<MAXIMUM_AMBIENT_SOUND_CHANNELS; ++j, ++channel)
			{
				if (!channel_used[j])
				{
					// take over this channel for a new sound
					if (SLOT_IS_USED(channel)) free_channel(channel);
					
					channel->flags= 0;
					channel->callback_count= 2; // #MD as if two sounds had just stopped playing
					channel->sound_index= ambient->sound_index;
					// channel->identifier
					// channel->dynamic_source
					// channel->source
					MARK_SLOT_AS_USED(channel);
					
					channel_used[j]= TRUE;
					
					instantiate_sound_variables(&ambient->variables, channel, TRUE);

					break;
				}
			}
		}
	}
	
	// remove those sounds which are no longer being played
	for (i= 0, channel= AMBIENT_SOUND_CHANNELS; i<MAXIMUM_AMBIENT_SOUND_CHANNELS; ++i, ++channel)
	{
		if (SLOT_IS_USED(channel) && !channel_used[i])
		{
			free_channel(channel);
		}
	}

	// unlock those handles which need to be unlocked and buffer new sounds if necessary
	for (i= 0, channel= AMBIENT_SOUND_CHANNELS; i<MAXIMUM_AMBIENT_SOUND_CHANNELS; ++i, ++channel)
	{
		if (SLOT_IS_USED(channel))
		{
			if (_load_sound(channel->sound_index))
			{
				// make sure at least two sounds are buffered at all times			
				while (channel->callback_count) // #MD
				{
					buffer_sound(channel, channel->sound_index, FIXED_ONE);
					
					channel->callback_count-= 1;
				}
			}
		}
	}
	
	return;
}

enum
{
	MAXIMUM_AMBIENT_SOUND_VOLUME= 3*MAXIMUM_SOUND_VOLUME/2
};

// NULL source means directionless
static void add_one_ambient_sound_source(
	struct ambient_sound_data *ambient_sounds,
	world_location3d *source,
	world_location3d *listener,
	short ambient_sound_index,
	short absolute_volume)
{
	if (ambient_sound_index!=NONE)
	{
		// LP change; make NONE in case this sound definition is invalid
		struct ambient_sound_definition *SoundDef = get_ambient_sound_definition(ambient_sound_index);
		short sound_index = (SoundDef) ? SoundDef->sound_index : NONE;
		// short sound_index= get_ambient_sound_definition(ambient_sound_index)->sound_index;
		
		if (sound_index!=NONE)
		{
			struct sound_definition *definition= get_sound_definition(sound_index);
			
			// LP change: idiot-proofing
			if (definition)
			{
			if (definition->sound_code!=NONE)
			{
				struct sound_behavior_definition *behavior= get_sound_behavior_definition(definition->behavior_index);
				// LP change: idiot-proofing
				if (!behavior) return; // Silence
	
				struct ambient_sound_data *ambient;
				short distance;
				short i;
			
				if (source)
				{
					distance= distance3d(&listener->point, &source->point);
				}
			
				for (i= 0, ambient= ambient_sounds;
					i<MAXIMUM_PROCESSED_AMBIENT_SOUNDS;
					++i, ++ambient)
				{
					if (SLOT_IS_USED(ambient))
					{
						if (ambient->sound_index==sound_index) break;
					}
					else
					{
						MARK_SLOT_AS_USED(ambient);
						
						ambient->sound_index= sound_index;
			
						ambient->variables.priority= definition->behavior_index;
						ambient->variables.volume= ambient->variables.left_volume= ambient->variables.right_volume= 0;
			
						break;
					}
				}
				
				if (i!=MAXIMUM_PROCESSED_AMBIENT_SOUNDS)
				{
					if (!source || distance<behavior->unobstructed_curve.minimum_volume_distance)
					{
						short volume, left_volume, right_volume;
						
						if (source)
						{
							// LP change: made this long-distance friendly
							long dx= long(listener->point.x) - long(source->point.x);
							long dy= long(listener->point.y) - long(source->point.y);
							// world_distance dx= listener->point.x-source->point.x;
							// world_distance dy= listener->point.y-source->point.y;
							
							volume= distance_to_volume(definition, distance, _sound_obstructed_proc(source));
							volume= (absolute_volume*volume)>>MAXIMUM_SOUND_VOLUME_BITS;
							
							if (dx || dy)
							{
								angle_and_volume_to_stereo_volume(arctangent(dx, dy) - listener->yaw,
									volume, &right_volume, &left_volume);
							}
							else
							{
								left_volume= right_volume= volume;
							}
						}
						else
						{
							volume= left_volume= right_volume= absolute_volume;
						}
						
						{
							short maximum_volume= MAX(MAXIMUM_AMBIENT_SOUND_VOLUME, volume);
							short maximum_left_volume= MAX(MAXIMUM_AMBIENT_SOUND_VOLUME, left_volume);
							short maximum_right_volume= MAX(MAXIMUM_AMBIENT_SOUND_VOLUME, right_volume);
	
							ambient->variables.volume= CEILING(ambient->variables.volume+volume, maximum_volume);
							ambient->variables.left_volume= CEILING(ambient->variables.left_volume+left_volume, maximum_left_volume);
							ambient->variables.right_volume= CEILING(ambient->variables.right_volume+right_volume, maximum_right_volume);
						}
					}
				}
				else
				{
//					dprintf("warning: ambient sound buffer full;g;");
				}
			}
			}
		}
	}
	
	return;
}
