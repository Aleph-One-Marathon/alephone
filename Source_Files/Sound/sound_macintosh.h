/*
SOUND_MACINTOSH.C - Sound management, MacOS specific stuff (included by mysound.c)

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

Friday, August 25, 1995 4:51:06 PM  (Jason)

Tuesday, August 29, 1995 8:56:06 AM  (Jason)
	running.
Thursday, August 31, 1995 9:09:46 AM  (Jason)
	pitch changes without bogus sound headers.

Feb 2, 2000 (Loren Petrich):
	Discovered that SOUND_FILE_TAG should not be changed from 'snd2' to be Infinity-compatible

Mar 2, 2000 (Loren Petrich):
	Added alias resolution to opening of file

Jul 1, 2000 (Loren Petrich):
	Suppressed dprintf statements; all the remaining ones are vwarn's, which ought to be
	better-behaved.

Dec 3, 2000 (Loren Petrich):
	Added quadrupling of usual buffer size because RAM is now readily available

Apr 26, 2001 (Loren Petrich):
	Added support for loading external sounds; got that support working

Sept 10, 2001 (Loren Petrich):
	Added Ian Rickard's relative-sound 

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for Carbon.h
	Added usage of get/setRateMultiplierCmd for Carbon
		in place of use of unsupported get/setRateCmd
*/

#if defined(EXPLICIT_CARBON_HEADER)
    #include <Carbon/Carbon.h>
#else
//#include <FixMath.h>
#include <MediaHandlers.h>
#endif


/* --------- constants */

enum
{
	MINIMUM_SOUND_BUFFER_SIZE= 300*KILO,
	MORE_SOUND_BUFFER_SIZE= 600*KILO,
	AMBIENT_SOUND_BUFFER_SIZE= 1*MEG,
	MAXIMUM_SOUND_BUFFER_SIZE= 1*MEG,

	kAMBIENT_SOUNDS_HEAP= 6*MEG,
	kMORE_SOUNDS_HEAP= 4*MEG,
	k16BIT_SOUNDS_HEAP= 8*MEG,
	kEXTRA_MEMORY_HEAP= 12*MEG,
	 // Because RAM is more available
	kEXTRA_EXTRA_MEMORY_HEAP= 20*MEG
};

/* --------- macros */

#define BUILD_STEREO_VOLUME(l, r) ((((long)(r))<<16)|(l))

/* --------- globals */

/* --------- private prototypes */

// Thomas Herzog: Updated for Universal Interfaces 3.4
#if UNIVERSAL_INTERFACES_VERSION < 0x0340
	static pascal void sound_callback_proc(SndChannelPtr channel, SndCommand command);
#else
	static pascal void sound_callback_proc(SndChannelPtr channel, SndCommand *command);
#endif
// static pascal void sound_callback_proc(SndChannelPtr channel, SndCommand command);

static long sound_level_to_sound_volume(short level);

static void set_sound_manager_status(bool active);
static void close_sound_file(void);
static void shutdown_sound_manager(void);

/* --------- code */

void set_sound_manager_parameters(
	struct sound_manager_parameters *parameters)
{
	if (_sm_initialized)
	{
		verify_sound_manager_parameters(parameters);
		
		/* if it was initially on, turn off the sound manager */
		if (_sm_active) set_sound_manager_status(false);
		
		/* we need to get rid of the sounds we have in memory */
		unload_all_sounds();
		
		/* stuff in our new parameters */
		*_sm_parameters= *parameters;
		
		/* enable sound if volume is set */
		if (parameters->volume)
			set_sound_manager_status(true);
	}
}

/* passing false disposes of all existing sound channels and sets _sm_active to false,
	true reallocates everything and sets _sm_active to true */
static void set_sound_manager_status(
	bool active)
{
	if (_sm_initialized)
	{
		short i;
		OSErr error;
		struct channel_data *channel;
	
		if (active != _sm_active)
		{
			if (active)
			{
				_sm_globals->total_channel_count= _sm_parameters->channel_count;
				if (_sm_parameters->flags&_ambient_sound_flag) _sm_globals->total_channel_count+= MAXIMUM_AMBIENT_SOUND_CHANNELS;

				_sm_globals->total_buffer_size= (_sm_parameters->flags&_more_sounds_flag) ?
					MORE_SOUND_BUFFER_SIZE : MINIMUM_SOUND_BUFFER_SIZE;
				if (_sm_parameters->flags&_ambient_sound_flag) _sm_globals->total_buffer_size+= AMBIENT_SOUND_BUFFER_SIZE;
				if (_sm_parameters->flags&_16bit_sound_flag) _sm_globals->total_buffer_size*= 2;
				if (_sm_globals->available_flags&_extra_memory_flag) _sm_globals->total_buffer_size*= 2;
				 // Because RAM is more available
				if (_sm_globals->available_flags&_extra_extra_memory_flag) _sm_globals->total_buffer_size*= 2;
				
				_sm_globals->sound_source= (_sm_parameters->flags&_16bit_sound_flag) ? _16bit_22k_source : _8bit_22k_source;
				_sm_globals->base_sound_definitions= sound_definitions + _sm_globals->sound_source*number_of_sound_definitions;

				SetDefaultOutputVolume(sound_level_to_sound_volume(_sm_parameters->volume));
				
				for (i= 0, channel= _sm_globals->channels; i<_sm_globals->total_channel_count; ++i, ++channel)
				{
					/* initialize the channel */
					channel->flags= 0;
					channel->callback_count= 0;
					channel->sound_index= NONE;
					obj_clear(*(channel->channel));
					channel->channel->qLength= stdQLength;
					channel->channel->userInfo= (long) &channel->callback_count;
					channel->SndPtr = NULL;

					error= SndNewChannel(&channel->channel, sampledSynth, (_sm_parameters->flags&_stereo_flag) ? initStereo : initMono,
						_sm_globals->sound_callback_upp);
					if (error!=noErr)
					{
						for (channel= _sm_globals->channels; i; --i, ++channel)
						{
							error= SndDisposeChannel(channel->channel, true);
							assert(error==noErr);
						}
						
 						alert_user(infoError, strERRORS, badSoundChannels, error);
						_sm_globals->total_channel_count= 0;
						active= _sm_active= _sm_initialized= false;
						
						break;
					}
				}
			}
			else
			{
				stop_all_sounds();
				for (i= 0, channel= _sm_globals->channels; i<_sm_globals->total_channel_count; ++i, ++channel)
				{
					error= SndDisposeChannel(channel->channel, true);
					assert(error==noErr);
				}

				SetDefaultOutputVolume(_sm_globals->old_sound_volume);
				
				// noticing that the default sound volume wasnÕt correctly restored until the
				// next sound command was issued (sysbeep, etc.) we do this to assure volume
				// is correct on exit
				{
					long unused;
					
					GetDefaultOutputVolume(&unused);
				}

				_sm_globals->total_channel_count= 0;
			}
			
			_sm_active= active;
		}
	}
}

bool adjust_sound_volume_up(
	struct sound_manager_parameters *parameters,
	short sound_index)
{
	bool changed= false;

	if (_sm_active)
	{
		if (parameters->volume<NUMBER_OF_SOUND_VOLUME_LEVELS)
		{
			_sm_parameters->volume= (parameters->volume+= 1);
			SetDefaultOutputVolume(sound_level_to_sound_volume(parameters->volume));
			play_sound(sound_index, (world_location3d *) NULL, NONE);
			changed= true;
		}
	}
	
	return changed;
}

bool adjust_sound_volume_down(
	struct sound_manager_parameters *parameters,
	short sound_index)
{
	bool changed= false;

	if (_sm_active)
	{
		if (parameters->volume>0)
		{
			_sm_parameters->volume= (parameters->volume-= 1);
			SetDefaultOutputVolume(sound_level_to_sound_volume(parameters->volume));
			play_sound(sound_index, (world_location3d *) NULL, NONE);
			changed= true;
		}
	}
	
	return changed;
}

void test_sound_volume(
	short volume,
	short sound_index)
{
	if (_sm_active)
	{
		if ((volume= PIN(volume, 0, NUMBER_OF_SOUND_VOLUME_LEVELS))!=0)
		{
			SetDefaultOutputVolume(sound_level_to_sound_volume(volume));
			play_sound(sound_index, (world_location3d *) NULL, NONE);
			while (sound_is_playing(sound_index))
				{/* call an idle procedure here? */}
			SetDefaultOutputVolume(sound_level_to_sound_volume(_sm_parameters->volume));
		}
	}
}

/* ---------- private code (SOUND.C) */

static void initialize_machine_sound_manager(
	struct sound_manager_parameters *parameters)
{
	OSErr error;

	assert(static_cast<uint32>(kFullVolume)==static_cast<uint32>(MAXIMUM_SOUND_VOLUME));

 	// Thomas Herzog: Updated for Universal Interfaces 3.4
 	#if UNIVERSAL_INTERFACES_VERSION < 0x0340
 		_sm_globals->sound_callback_upp= NewSndCallBackProc((ProcPtr)sound_callback_proc);
 	#else
 		_sm_globals->sound_callback_upp= NewSndCallBackUPP(sound_callback_proc);
 	#endif
	// _sm_globals->sound_callback_upp= NewSndCallBackProc((ProcPtr)sound_callback_proc);
	if ((error= MemError())==noErr)
	{
		short i;
		
		// allocate sound channels
		for (i= 0; i<MAXIMUM_SOUND_CHANNELS+MAXIMUM_AMBIENT_SOUND_CHANNELS; ++i)
		{
			_sm_globals->channels[i].channel= (SndChannelPtr) NewPtr(sizeof(SndChannel));
		}
		
		if ((error= MemError())==noErr)
		{
			FileSpecifier InitialSoundFile;
			// FSSpec sounds_file;
			
			/* initialize _sm_globals */
			_sm_globals->loaded_sounds_size= 0;
			_sm_globals->total_channel_count= 0;
			// Auto-initialize by FileSpecifier_Mac constructor
			// _sm_globals->sound_file_refnum= -1;
			
			get_default_sounds_spec(InitialSoundFile);
			// error= get_file_spec(&sounds_file, strFILENAMES, filenameSOUNDS8, strPATHS);
			
			if (error==noErr)
			{
				/*
				error= open_sound_file(&sounds_file);
				if (error==noErr)
				*/
				if (open_sound_file(InitialSoundFile))
				{
					atexit(shutdown_sound_manager);
					
					// LP: need relative volume here
					_sm_globals->available_flags= _stereo_flag | _dynamic_tracking_flag | _relative_volume_flag;
					{
						long heap_size= FreeMem();
						
						if (heap_size>kAMBIENT_SOUNDS_HEAP) _sm_globals->available_flags|= _ambient_sound_flag;
						if (heap_size>kMORE_SOUNDS_HEAP) _sm_globals->available_flags|= _more_sounds_flag;
						if (heap_size>k16BIT_SOUNDS_HEAP) _sm_globals->available_flags|= _16bit_sound_flag;
						if (heap_size>kEXTRA_MEMORY_HEAP) _sm_globals->available_flags|= _extra_memory_flag;
						 // Because RAM is more available
						if (heap_size>kEXTRA_EXTRA_MEMORY_HEAP) _sm_globals->available_flags|= _extra_extra_memory_flag;
					}
					
					/* fake a set_sound_manager_parameters() call */
					_sm_parameters->flags= 0;
					_sm_initialized= _sm_active= true;
					GetDefaultOutputVolume(&_sm_globals->old_sound_volume);

#if TARGET_API_MAC_CARBON
					// JTP: There appears to be a problem where even if I get the default volume, then set it
					// atexit, we still miss the original volume and change further, softer in my experience
					// though others may get louder.
					// Let's try and find this delta and actually fix our stored volume with it to fix it...
					// If this all works with no glitch, then the delta will be 0 anyhow...
					int32 glitched_volume;
					SetDefaultOutputVolume(_sm_globals->old_sound_volume);
					GetDefaultOutputVolume(&glitched_volume);
					_sm_globals->old_sound_volume+= (_sm_globals->old_sound_volume - glitched_volume);
#endif
					
					set_sound_manager_parameters(parameters);
				}
				else
					error = SoundFile.GetError();
			}
		}
	}

	vwarn(error==noErr, csprintf(temporary, "initialize_sound_manager() == #%d", error));
}

static bool channel_busy(
	struct channel_data *channel)
{
	assert(SLOT_IS_USED(channel));
	
	return (channel->callback_count) ? false : true;
}

static void unlock_sound(
	short sound_index)
{
	// LP: unlocking a sound handle is now superfluous
}

static void dispose_sound(
	short sound_index)
{
	struct sound_definition *definition= get_sound_definition(sound_index);
	// LP change: idiot-proofing
	if (!definition) return;

	if (!definition->ptr) return;

	_sm_globals->loaded_sounds_size -= definition->size;
	delete []definition->ptr;
	definition->ptr = NULL;
	definition->size = 0;
}

// should be asynchronous
// should only read a single sound unless _sm_parameters->flags&_more_sounds_flag
static byte *read_sound_from_file(
	short sound_index, long& size)
	// short sound_index)
{
	struct sound_definition *definition= get_sound_definition(sound_index);
	// LP change: idiot-proofing
	size = 0;
	if (!definition) return NULL;
	
	// Load all the external-file sounds for each index; fill the slots appropriately.
	// Don't load any of them is QuickTime is not present.
	int NumSlots = (_sm_parameters->flags&_more_sounds_flag) ? definition->permutations : 1;
	if (!machine_has_quicktime()) NumSlots = 0;
	for (int k=0; k<NumSlots; k++)
	{
		FileSpecifier File;
		SoundOptions *SndOpts = GetSoundOptions(sound_index,k);
		if (!SndOpts) continue;
		
		try
		{
			// Load only if necessary
			if (SndOpts->Sound.IsLoaded()) throw 0;
			if (!File.SetNameWithPath(&SndOpts->File[0])) throw 0;
			if (!SndOpts->Sound.Load(File)) throw 0;
		}
		catch(...)
		{
			// Just in case attempted loading fails
			SndOpts->Sound.Unload();
		}
	}
	
	byte *data= NULL;
	OSErr error= noErr;
	
	if (SoundFile.IsOpen())
	// if (_sm_globals->sound_file_refnum!=-1)
	{
		size= (_sm_parameters->flags&_more_sounds_flag) ? definition->total_length : definition->single_length;
		
		if ((data = new byte[size]) != NULL)
		{
			if (!SoundFile.SetPosition(definition->group_offset))
			{
				delete []data;
				data = NULL;
				error = SoundFile.GetError();
			}
			if (SoundFile.Read(size,data))
			{
				_sm_globals->loaded_sounds_size+= size;
			}
			else
			{
				delete []data;
				data = NULL;
				error = SoundFile.GetError();
			}
		}
		else
		{
			error= MemError();
			// dprintf("read_sound_from_file() couldnÕt allocate #%d bytes", size);
		}
	}
	
	vwarn(error==noErr, csprintf(temporary, "read_sound_from_file(#%d) got error #%d", sound_index, error));
	
	return data;
}

/* send flushCmd, quietCmd */
static void quiet_channel(
	struct channel_data *channel)
{
	// If loaded sound had been active, shut it up
	LoadedSound *SndPtr = channel->SndPtr;
	if (SndPtr)
	{
		SndPtr->Stop();
		channel->SndPtr = NULL;
		return;
	}
	
	SndCommand command;
	OSErr error;
	
	command.cmd= flushCmd;
	command.param1= 0;
	command.param2= 0;
	error= SndDoImmediate(channel->channel, &command);
	if (error==noErr)
	{
		command.cmd= quietCmd;
		command.param1= 0;
		command.param2= 0;
		error= SndDoImmediate(channel->channel, &command);
		if (error==noErr)
		{
		}
	}
	
	vwarn(error==noErr, csprintf(temporary, "SndDoImmediate() == #%d in quiet_channel()", error));
}

static void instantiate_sound_variables(
	struct sound_variables *variables,
	struct channel_data *channel,
	bool first_time)
{
	OSErr error= noErr;
	SndCommand command;

	if (first_time || variables->right_volume!=channel->variables.right_volume || variables->left_volume!=channel->variables.left_volume)
	{
		/* set the sound volume */
		command.cmd= volumeCmd;
		command.param1= 0;
		command.param2= BUILD_STEREO_VOLUME(variables->left_volume, variables->right_volume);
		error= SndDoImmediate(channel->channel, &command);
		
		LoadedSound *SndPtr = channel->SndPtr;
		if (SndPtr)
			SndPtr->SetVolume(variables->left_volume,variables->right_volume);
	}

	vwarn(error==noErr, csprintf(temporary, "SndDoImmediate() == #%d in instantiate_sound_variables()", error));

	channel->variables= *variables;
}

static void buffer_sound(
	struct channel_data *channel,
	short sound_index,
	_fixed pitch,
	bool ExtPlayImmed)
{
	struct sound_definition *definition= get_sound_definition(sound_index);
	// LP change: idiot-proofing
	if (!definition) return;
	
	short permutation= get_random_sound_permutation(sound_index);
	
	// Handle externally-loaded sound
	LoadedSound *SndPtr = channel->SndPtr;
	if (SndPtr)
	{
		SndPtr->Stop();
		SndPtr = NULL;
	}
	
	SoundOptions *SndOpts = GetSoundOptions(sound_index,permutation);
	if (SndOpts)
	{
		SndPtr = &SndOpts->Sound;
		if (SndPtr->IsLoaded())
		{
			channel->SndPtr = SndPtr;
			SndPtr->SetPitch(calculate_pitch_modifier(sound_index, pitch));
			SndPtr->SetVolume(channel->variables.left_volume,channel->variables.right_volume);
			if (ExtPlayImmed)
				SndPtr->Play();
			else
				SndPtr->Queued = true;
			return;
		}
	}
	
	SoundHeaderPtr sound_header;
	SndCommand command;
	OSErr error;

	assert(definition->ptr);
	
	assert(permutation>=0 && permutation<definition->permutations);
	sound_header= (SoundHeaderPtr) (definition->ptr + definition->sound_offsets[permutation]);

	/* play the sound */
	command.cmd= bufferCmd; /* high bit not set: weÕre sending a real pointer */
	command.param1= 0;
	command.param2= (long) sound_header;
	error= SndDoCommand(channel->channel, &command, false);
	if (error==noErr)
	{
		/* queue the callback */
		command.cmd= callBackCmd;
		command.param1= 0;
		command.param2= 0;
		error= SndDoCommand(channel->channel, &command, false);
		if (error==noErr)
		{
#if defined(TARGET_API_MAC_CARBON)
			// JTP: Steve Bytnar's rateMultiplierCmd fix
			command.cmd= rateMultiplierCmd;
			command.param1= 0;
			command.param2= calculate_pitch_modifier(sound_index, pitch);
			error= SndDoImmediate(channel->channel, &command);
#else
			if (pitch!=FIXED_ONE)
			{
				_fixed rate;
				
				command.cmd= getRateCmd;
				command.param1= 0;
				command.param2= (long)&rate;
				error= SndDoImmediate(channel->channel, &command);
				if (error==noErr)
				{
					command.cmd= rateCmd;
					command.param1= 0;
					command.param2= FixMul(rate, calculate_pitch_modifier(sound_index, pitch));
					error= SndDoImmediate(channel->channel, &command);
				}
			}
#endif
		}
	}
}

/* ---------- private code (SOUND_MACINTOSH.C) */

static void shutdown_sound_manager(
	void)
{
	set_sound_manager_status(false);
	close_sound_file();	
}

static pascal void sound_callback_proc(
	SndChannelPtr channel,
// TH: Updated for Universal Interfaces 3.4
#if UNIVERSAL_INTERFACES_VERSION < 0x0340
	SndCommand command)
#else
	SndCommand *command)
#endif
{
	(void) (command);
	
	*((short *)channel->userInfo)+= 1;
}

static long sound_level_to_sound_volume(
	short level)
{
	// Implementing Ian Rickard's relative-volume feature in a simpler fashion
	// (OK, since there are 256 possible overall volume levels).
	// Its formula was inspired by the definition of SOUND_VOLUME_DELTA
	short volume = 0;
	if (_sm_parameters->flags&_relative_volume_flag)
	{
		int LeftVol = HiWord(_sm_globals->old_sound_volume);
		int RightVol = LoWord(_sm_globals->old_sound_volume);
		int OldSoundVolume = (LeftVol + RightVol)/2;
		volume = (2*level*OldSoundVolume)/NUMBER_OF_SOUND_VOLUME_LEVELS;
	}
	else
		volume= (level*SOUND_VOLUME_DELTA);
	
	return BUILD_STEREO_VOLUME(volume, volume);
}


#if 0
enum
{
	FADE_OUT_DURATION= 2*MACINTOSH_TICKS_PER_SECOND,
	FADE_OUT_STEPS= 20,
	
	TICKS_PER_FADE_OUT_STEP= FADE_OUT_DURATION/FADE_OUT_STEPS
};

// doesnÕt work
static synchronous_global_fade_to_silence(
	void)
{
	short i;
	bool fade_out= false;
	struct channel_data *channel;
	struct sound_variables old_variables[MAXIMUM_SOUND_CHANNELS+MAXIMUM_AMBIENT_SOUND_CHANNELS];
	
	for (i= 0, channel= _sm_globals->channels; i<_sm_globals->total_channel_count; ++i, ++channel)
	{
		if (SLOT_IS_USED(channel))
		{
			old_variables[i]= channel->variables;
			
			fade_out= true;
		}
	}
	
	if (fade_out)
	{
		short step;
		
		for (step= 0; step<FADE_OUT_STEPS; ++step)
		{
			long tick_at_last_update= TickCount();
			
			while (TickCount()-tick_at_last_update<TICKS_PER_FADE_OUT_STEP);
			
			for (i= 0, channel= _sm_globals->channels; i<_sm_globals->total_channel_count; ++i, ++channel)
			{
				if (SLOT_IS_USED(channel))
				{
					struct sound_variables *old= old_variables + i;
					struct sound_variables *new= &channel->variables;
					
					new->volume= (old->volume*(FADE_OUT_STEPS-step-1))/FADE_OUT_STEPS;
					new->left_volume= (old->left_volume*(FADE_OUT_STEPS-step-1))/FADE_OUT_STEPS;
					new->right_volume= (old->right_volume*(FADE_OUT_STEPS-step-1))/FADE_OUT_STEPS;
			
					instantiate_sound_variables(&channel->variables, channel, true);
				}
			}
		}
	}
}
#endif


// LP additions: QuickTime sound handling
	
bool LoadedSound::Load(FileSpecifier& File)
{
	Unload();
	
	OSErr Err;
	short RefNum;
	short ResID = 0;	// Only use the first resource
	Boolean WasChanged;
	Err = OpenMovieFile(&File.GetSpec(), &RefNum, fsRdPerm);
	if (Err != noErr) return false;
	Err = NewMovieFromFile(&QTSnd, RefNum, &ResID, NULL, newMovieActive, &WasChanged);
	CloseMovieFile(RefNum);
	if (Err != noErr) return false;
	
	// Want the sound to be ready to go when it's time to play
	PrerollMovie(QTSnd,0,FIXED_ONE);
	
	return true;
}

bool LoadedSound::Unload()
{
	if (!QTSnd) return true;
	
	DisposeMovie(QTSnd);
	QTSnd = NULL;
	return true;
}

bool LoadedSound::IsLoaded()
{
	return (QTSnd != NULL);
}

bool LoadedSound::Play()
{
	if (!QTSnd) return false;
	
	GoToBeginningOfMovie(QTSnd);
	StartMovie(QTSnd);
	return true;
}

bool LoadedSound::Stop()
{
	if (!QTSnd) return false;
	
	StopMovie(QTSnd);
	return true;
}

bool LoadedSound::IsPlaying()
{
	if (!QTSnd) return false;
	
	return !IsMovieDone(QTSnd);
}

bool LoadedSound::Update()
{
	if (!QTSnd) return false;
	
	MoviesTask(QTSnd,0);
	return true;
}

bool LoadedSound::SetPitch(_fixed Pitch)
{
	if (!QTSnd) return false;
	
	SetMovieRate(QTSnd,Pitch);
	return true;
}

bool LoadedSound::SetVolume(_fixed Left, _fixed Right)
{
	if (!QTSnd) return false;
	
	// Rather contorted way of doing stereo with QuickTime...
	
	_fixed Vol = MAX(Left,Right);
	if (Vol <= 0) return true;
	
	short Balance = short((1 << 8) * (Right - Left)/float(Vol));
	
	SetMovieVolume(QTSnd,Vol);
	
	int NumTracks = GetMovieTrackCount(QTSnd);
	for (int t=1; t<=NumTracks; t++) {
	
		Track Trk = GetMovieIndTrack(QTSnd,t);
		if (!Trk) continue;
		if (GetMoviesError() != noErr) continue;
		
		Media Med = GetTrackMedia(Trk);
		if (!Med) continue;
		if (GetMoviesError() != noErr) continue;
		
		MediaHandler Hdlr = GetMediaHandler(Med);
		if (!Hdlr) continue;
		if (GetMoviesError() != noErr) continue;
		
		MediaSetSoundBalance(Hdlr,Balance);
	}
	
	return true;
}
