/*
SOUND_MACINTOSH.C - Sound management, MacOS specific stuff (included by mysound.c)
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
*/

#include <FixMath.h>

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
	kEXTRA_MEMORY_HEAP= 12*MEG
};

/* --------- macros */

#define BUILD_STEREO_VOLUME(l, r) ((((long)(r))<<16)|(l))

/* --------- globals */

/* --------- private prototypes */

static pascal void sound_callback_proc(SndChannelPtr channel, SndCommand command);

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
		bool initial_state= _sm_active;

		verify_sound_manager_parameters(parameters);
		
		/* if it was initially on, turn off the sound manager */
		if (initial_state) set_sound_manager_status(false);		
		
		/* we need to get rid of the sounds we have in memory */
		unload_all_sounds();
		
		/* stuff in our new parameters */
		*_sm_parameters= *parameters;
		
		/* if it was initially on, turn the sound manager back on */
		if (initial_state) set_sound_manager_status(true);
	}
	
	return;
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

				_sm_globals->sound_source= (_sm_parameters->flags&_16bit_sound_flag) ? _16bit_22k_source : _8bit_22k_source;
				_sm_globals->base_sound_definitions= sound_definitions + _sm_globals->sound_source*number_of_sound_definitions;

				GetDefaultOutputVolume(&_sm_globals->old_sound_volume);
				SetDefaultOutputVolume(sound_level_to_sound_volume(_sm_parameters->volume));
				
				for (i= 0, channel= _sm_globals->channels; i<_sm_globals->total_channel_count; ++i, ++channel)
				{
					/* initialize the channel */
					channel->flags= 0;
					channel->callback_count= false;
					channel->sound_index= NONE;
					obj_clear(*(channel->channel));
					channel->channel->qLength= stdQLength;
					channel->channel->userInfo= (long) &channel->callback_count;

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
	
	return;
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
				;
			SetDefaultOutputVolume(sound_level_to_sound_volume(_sm_parameters->volume));
		}
	}
	
	return;
}

/* ---------- private code (SOUND.C) */

static void initialize_machine_sound_manager(
	struct sound_manager_parameters *parameters)
{
	OSErr error;

	assert(kFullVolume==MAXIMUM_SOUND_VOLUME);

	_sm_globals->sound_callback_upp= NewSndCallBackProc((ProcPtr)sound_callback_proc);
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
					
					_sm_globals->available_flags= _stereo_flag | _dynamic_tracking_flag;
					{
						long heap_size= FreeMem();
						
						if (heap_size>kAMBIENT_SOUNDS_HEAP) _sm_globals->available_flags|= _ambient_sound_flag;
						if (heap_size>kMORE_SOUNDS_HEAP) _sm_globals->available_flags|= _more_sounds_flag;
						if (heap_size>k16BIT_SOUNDS_HEAP) _sm_globals->available_flags|= _16bit_sound_flag;
						if (heap_size>kEXTRA_MEMORY_HEAP) _sm_globals->available_flags|= _extra_memory_flag;
					}
					
					/* fake a set_sound_manager_parameters() call */
					_sm_parameters->flags= 0;
					_sm_initialized= _sm_active= true;
					GetDefaultOutputVolume(&_sm_globals->old_sound_volume);
					
					set_sound_manager_parameters(parameters);
				}
				else
					error = SoundFile.GetError();
			}
		}
	}

	vwarn(error==noErr, csprintf(temporary, "initialize_sound_manager() == #%d", error));

	return;
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
	// LP: don't need this anymore
	/*
	struct sound_definition *definition= get_sound_definition(sound_index);
	// LP change: idiot-proofing
	if (!definition) return;
	
	assert(definition->handle);
	
	if (definition->handle)
	{
		HUnlock((Handle)definition->handle);
	}
	*/
	return;
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
	/*	
	assert(definition->handle);
	
	_sm_globals->loaded_sounds_size-= GetHandleSize((Handle)definition->handle);
	DisposeHandle((Handle)definition->handle);
	definition->handle= 0;
	*/
	
	return;
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
	
	bool success= false;
	byte *data= NULL;
	// Handle data= NULL;
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
		
		/*
		if ((data= NewHandle(size))!=NULL)
		{
			ParamBlockRec param;
			
			HLock(data);
			
			param.ioParam.ioCompletion= (IOCompletionUPP) NULL;
			param.ioParam.ioRefNum= SoundFile.RefNum;
			// param.ioParam.ioRefNum= _sm_globals->sound_file_refnum;
			param.ioParam.ioBuffer= *data;
			param.ioParam.ioReqCount= size;
			param.ioParam.ioPosMode= fsFromStart;
			param.ioParam.ioPosOffset= definition->group_offset;
			
			error= PBReadSync(&param);
			if (error==noErr)
			{
				HUnlock(data);
				
				_sm_globals->loaded_sounds_size+= size;
			}
			else
			{
				DisposeHandle(data);
				
				data= NULL;
			}
			*/
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
	
	return;
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
	}

	vwarn(error==noErr, csprintf(temporary, "SndDoImmediate() == #%d in instantiate_sound_variables()", error));

	channel->variables= *variables;
	
	return;
}

static void buffer_sound(
	struct channel_data *channel,
	short sound_index,
	fixed pitch)
{
	struct sound_definition *definition= get_sound_definition(sound_index);
	// LP change: idiot-proofing
	if (!definition) return;

	short permutation= get_random_sound_permutation(sound_index);
	SoundHeaderPtr sound_header;
	SndCommand command;
	OSErr error;

	assert(definition->ptr);
	// assert(definition->handle);
	// HLock((Handle)definition->handle);
	
	assert(permutation>=0 && permutation<definition->permutations);
	sound_header= (SoundHeaderPtr) (definition->ptr + definition->sound_offsets[permutation]);
	// sound_header= (SoundHeaderPtr) ((*(byte **)definition->handle) + definition->sound_offsets[permutation]);

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
			if (pitch!=FIXED_ONE)
			{
				fixed rate;
				
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
		}
	}

	vassert(error==noErr, csprintf(temporary, "SndDoCommand() == #%d in buffer_sound()", error));
	
	return;
}

/* ---------- private code (SOUND_MACINTOSH.C) */

static void shutdown_sound_manager(
	void)
{
	set_sound_manager_status(false);

	close_sound_file();	
	
	return;
}

static pascal void sound_callback_proc(
	SndChannelPtr channel,
	SndCommand command)
{
	(void) (command);
	
	*((short *)channel->userInfo)+= 1;

	return;
}

static long sound_level_to_sound_volume(
	short level)
{
	short volume= level*SOUND_VOLUME_DELTA;
	
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
	
	return;
}
#endif
