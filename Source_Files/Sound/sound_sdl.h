/*

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

*/

/*
 *  sound_sdl.h - Sound/music management, SDL specific stuff (included by mysound.cpp)
 *
 *  Written in 2000 by Christian Bauer
 *
 *  Feb 22, 2002 (Woody Zenfell)
 *     Added ability to disable DirectShow-based music playback for building on systems without the SDK.
 *     Define preprocessor symbol WIN32_DISABLE_MUSIC if this is desired.
 *
 *  Mar 3-8, 2002 (Woody Zenfell)
 *     Realtime network microphone audio playback support
 *
 *  Jan 14, 2003 (Woody Zenfell)
 *     Safer memory management scheme for network audio playback buffers
 */

#include <SDL_endian.h>
// Unlike the Mac version, we also do the music handling here
#include "music.h"
#include "song_definitions.h"
#include "XML_LevelScript.h"   // For getting level music. 
#include "network_speaker_sdl.h"
#include "network_audio_shared.h"

// Number of sound channels used by Aleph One sound manager
const int SM_SOUND_CHANNELS = MAXIMUM_SOUND_CHANNELS + MAXIMUM_AMBIENT_SOUND_CHANNELS;

// Private channels
const int MUSIC_CHANNEL = SM_SOUND_CHANNELS;
const int RESOURCE_CHANNEL = SM_SOUND_CHANNELS + 1;
// ZZZ: network audio channel
const int NETWORK_AUDIO_CHANNEL = SM_SOUND_CHANNELS + 2;

// Total number of sound channels
const int TOTAL_SOUND_CHANNELS = SM_SOUND_CHANNELS + 3;


// SDL sound channel data
struct sdl_channel {
	bool active;			// Flag: currently playing sound

	bool sixteen_bit;		// Flag: 16-bit sound data (8-bit otherwise)
	bool stereo;			// Flag: stereo sound data (mono otherwise)
	bool signed_8bit;		// Flag: 8-bit sound data is signed (unsigned otherwise, 16-bit data is always signed)
	int bytes_per_frame;	// Bytes per sample frame (1, 2 or 4)

	uint8 *data;			// Current pointer to sound data
	int32 length;			// Length in bytes remaining to be played
	uint8 *loop;			// Pointer to loop start
	int32 loop_length;		// Loop length in bytes (0 = don't loop)

	_fixed rate;				// Sample rate (relative to output sample rate)
	_fixed counter;			// Counter for up/downsampling

	int16 left_volume;		// Volume (0x100 = nominal)
	int16 right_volume;

	uint8 *next_header;		// Pointer to next queued sound header (NULL = none)
	_fixed next_pitch;		// Pitch of next queued sound header
} sdl_channels[TOTAL_SOUND_CHANNELS];


// Sound buffer sizes
const int MINIMUM_SOUND_BUFFER_SIZE = 300*KILO;
const int MORE_SOUND_BUFFER_SIZE = 600*KILO;
const int AMBIENT_SOUND_BUFFER_SIZE = 1*MEG;
const int MAXIMUM_SOUND_BUFFER_SIZE = 1*MEG;

// Desired and obtained audio formats
static SDL_AudioSpec desired, obtained;

// Global sound volume (0x100 = nominal)
static int16 main_volume = 0x100;


/*
 *  Music
 */

static bool music_initialized = false;	// Flag: music ready to play
static bool music_play = false;			// Flag: music playing
static bool music_prelevel = false;		// Flag: level music loaded but not playing
static bool music_level = false;		// Flag: play level music

static bool music_fading = false;		// Flag: music fading out
static uint32 music_fade_start;			// Music fade start tick
static uint32 music_fade_duration;		// Music fade duration in ticks

static FileSpecifier music_file;		// Current music file
static FileSpecifier music_intro_file;	// Introductory music
static bool music_intro = false;		// Flag: introductory music available

#ifdef __MACOS__
bool macos_read_more = false;
bool macos_file_done = false;
#endif

void set_music_file(FileSpecifier *file);
bool load_music(FileSpecifier &file);

void restart_music();
void LoadLevelMusic();
void play_music();
void rewind_music();
bool fill_music_buffer();
#ifdef __MACOS__
bool fill_music_buffer_at_interrupt();
#endif

// Win32 music support.
// ZZZ: could not build with this in, so added preprocessor symbol to disable it.
#ifdef WIN32
#ifndef WIN32_DISABLE_MUSIC
#include <dshow.h>
#include "SDL_syswm.h"

#define MUSIC_WIN32

static IGraphBuilder* gp_graph_builder = NULL;
static IMediaControl* gp_media_control = NULL;
static IMediaSeeking* gp_media_seeking = NULL;
static IMediaEventEx* gp_media_event_ex = NULL;

void process_music_event_win32(const SDL_Event& event);
#endif
#endif

// SDL music support
#ifndef MUSIC_WIN32			// If native music support unavailable try using SDL
#if !(defined(SDL_RFORK_HACK))		// If SDL audio is disabled we can't do music either
#define MUSIC_SDL

static SDL_RWops *music_rw;	// music file object
#ifdef __APPLE__
const uint32 MUSIC_BUFFER_SIZE = 1024; // necessary to help avoid skipping, for now :(
#else
const uint32 MUSIC_BUFFER_SIZE = 0x40000;
#endif

bool load_music_sdl(FileSpecifier &song_file);

#ifdef HAVE_SDL_SOUND
// Use SDL_Sound to read music files
#define MUSIC_SDL_SOUND

#include "SDL_sound.h"
static Sound_Sample* music_sample;

#else
// Read music ourself (AIFF and WAV only)
#define MUSIC_PURE_SDL

static uint32 music_data_length;		// Total length of sample data in file
static uint32 music_data_remaining;		// Remaining unread sample data in file
static uint32 music_data_offset;		// Start offset of sample data
static uint8 music_buffer[MUSIC_BUFFER_SIZE];
#endif
#endif
#endif


// ZZZ: network_speaker support
static NetworkSpeakerSoundBufferDescriptor* sNetworkAudioBufferDesc;

// From FileHandler_SDL.cpp
extern void get_default_sounds_spec(FileSpecifier &file);

// From shell_sdl.cpp
extern bool option_nosound;

// Prototypes
static void close_sound_file();
static void shutdown_sound_manager();
static void shutdown_music_handler();
static void set_sound_manager_status(bool active);
static void load_sound_header(sdl_channel *c, uint8 *data, _fixed pitch);
static void sound_callback(void *userdata, uint8 *stream, int len);

TakeSDLAudioControl::TakeSDLAudioControl()
{
	set_sound_manager_status(false);
}

TakeSDLAudioControl::~TakeSDLAudioControl()
{
	set_sound_manager_status(true);
}

/*
 *  Initialize sound manager
 */

static void initialize_machine_sound_manager(struct sound_manager_parameters *parameters)
{
	// Assign channels
	for (int i=0; i<SM_SOUND_CHANNELS; i++)
		_sm_globals->channels[i].channel = sdl_channels + i;

	// Initialize _sm_globals
	_sm_globals->loaded_sounds_size = 0;
	_sm_globals->total_channel_count = 0;

	// Open sounds file
	FileSpecifier InitialSoundFile;
	get_default_sounds_spec(InitialSoundFile);
	if (open_sound_file(InitialSoundFile)) {
		atexit(shutdown_sound_manager);

		_sm_globals->available_flags = _stereo_flag | _dynamic_tracking_flag
		                             | _ambient_sound_flag | _more_sounds_flag
		                             | _16bit_sound_flag | _extra_memory_flag;

		// Set parameters and start sound manager
		_sm_parameters->flags = 0;
		_sm_initialized = true;
		_sm_active = false;
		set_sound_manager_parameters(parameters);
		set_sound_manager_status(true);
	}
}


/*
 *  Shutdown sound manager
 */

static void shutdown_sound_manager()
{
	set_sound_manager_status(false);
	close_sound_file();
}


/*
 *	Initialize introductory music
 */

bool initialize_music_handler(FileSpecifier &file)
{
	music_intro_file = file;
	set_music_file(&file);

	if (music_initialized)
		music_intro = true;

	return music_initialized;
}


/*
 *	Make the necessary preperations to play a music file
 */

void set_music_file (FileSpecifier *file)
{
	if (music_initialized) {
		// Music file is already loaded
		if (file != NULL && *file == music_file) {
			if (music_initialized)
				rewind_music();
			return;
		}

		// Close the old music file
		shutdown_music_handler();
		music_initialized = false;
	}

	if (file != NULL) {
#ifdef __MACOS__
		macos_read_more = true;
		macos_file_done = false;
#endif
		// Open the new music file
		music_initialized = load_music (*file);
		music_file = *file;
	}
}


/*
 *  Initialize music handling
 */

bool load_music (FileSpecifier &song_file)
{
	assert(NUMBER_OF_SONGS == sizeof(songs) / sizeof(struct song_definition));

#ifdef MUSIC_SDL
	return load_music_sdl(song_file);
#endif
#ifdef MUSIC_WIN32
    CoInitialize(NULL);
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

    // Create an IGraphBuilder object, through which
    //  we will create a DirectShow graph.
    CoCreateInstance(CLSID_FilterGraph, NULL,
                     CLSCTX_INPROC, IID_IGraphBuilder,
                     (void **)&gp_graph_builder);
    // Get the IMediaControl Interface
    gp_graph_builder->QueryInterface(IID_IMediaControl,
                                 (void **)&gp_media_control);
   // Get the IMediaSeeking Interface
    gp_graph_builder->QueryInterface(IID_IMediaSeeking,
                                 (void **)&gp_media_seeking);
    // Get the IMediaEventEx Interface
    gp_graph_builder->QueryInterface(IID_IMediaEventEx,
                                 (void **)&gp_media_event_ex);

    // Set the event window.
    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    SDL_GetWMInfo(&wmi);
    gp_media_event_ex->SetNotifyWindow((OAHWND)wmi.window, WM_DSHOW_GRAPH_NOTIFY, 0);

    gp_media_control->Stop();

    char song_name[_MAX_PATH];
    WCHAR wsong_name[_MAX_PATH];

    strcpy(song_name, song_file.GetPath());

    MultiByteToWideChar(CP_ACP, 0, song_name, -1,
                        wsong_name, _MAX_PATH);

    fwprintf(stderr, L"Starting music (%s)...\n", wsong_name);
    gp_graph_builder->RenderFile(wsong_name, NULL);

	// start it here, because I dont know how
	// to check whether it is playing in music_idle_proc
	play_music ();

	// I don't know how to check if the music has been initialized,
	// but it isn't very important on MUSIC_WIN32
	return true;
#endif
	return false;
}


/*
 * Initialise music handling under SDL
 */

#ifdef MUSIC_SDL
bool load_music_sdl(FileSpecifier &song_file)
{
	if (!_sm_initialized || !_sm_active) return false;
	sdl_channel *c = sdl_channels + MUSIC_CHANNEL;	// Music channel
	uint32 music_sample_rate;						// Music sample rate in Hz

	// We use the file abstraction classes to open the file,
	// but then we steal the SDL_RWops it created for us
	OpenedFile music_file;
	if (!song_file.Open(music_file)) {
		puts ("failed open music file 1");
		return false;
	}

	music_rw = music_file.TakeRWops();

#ifdef MUSIC_SDL_SOUND
	// If this is an mp3 file, we need to tell SDL_sound
	const char *song_name = song_file.GetPath();
	const char *song_ext = strrchr(song_name, '.');
	if (song_ext != NULL)
		song_ext++;

	// Start reading music
	music_sample = Sound_NewSample(music_rw, song_ext, NULL, MUSIC_BUFFER_SIZE);

	if (music_sample == NULL) {
		fprintf(stderr, "Error reading music file (%s)\n", Sound_GetError());
		return false;
	}

	Sound_AudioInfo in = music_sample->actual;	// music format of file
	Sound_AudioInfo out = in;					// music format to play
	bool convert = false;						// conversion necesary?

	switch (in.format) {
		case AUDIO_U8:
			c->sixteen_bit = false;
			c->signed_8bit = false;
			break;
		case AUDIO_S8:
			c->sixteen_bit = false;
			c->signed_8bit = true;
			break;
		case AUDIO_S16MSB:
			c->sixteen_bit = true;
			c->signed_8bit = false;
			break;
		default:
			// Sound channels are big endian
			// regardless of platform
			out.format = AUDIO_S16MSB;
			c->sixteen_bit = true;
			convert = true;
	}

	switch (in.channels) {
		case 1:
			c->stereo = false;
			break;
		case 2:
			c->stereo = true;
			break;
		default:  // just in case music has more than two channels
			convert = true;

			switch (obtained.channels) {
				case 1:
					out.channels = 1;
					c->stereo = false;
					break;
				case 2:
					out.channels = 2;
					c->stereo = true;
					break;
				default:
					fprintf(stderr, "This should not happen: sound device should be opened as either mono or stereo\n");
					return false;
			}
	}

	// Recreate sample with necessary conversion
	if (convert) {
		Sound_FreeSample(music_sample);

		OpenedFile music_file;
		if (!song_file.Open(music_file)) {
			puts ("error opening music file 2");
			return false;
		}
		music_rw = music_file.TakeRWops();

		music_sample = Sound_NewSample(music_rw, song_ext, &out, MUSIC_BUFFER_SIZE);
		if (music_sample == NULL) {
			fprintf(stderr, "Error converting music file (%s)\n", Sound_GetError());
			return false;
		}
	}

	music_sample_rate = out.rate;
#else // MUSIC_PURE_SDL
	// Read magic ID
	uint32 magic = SDL_ReadBE32(music_rw);
	if (magic == FOUR_CHARS_TO_INT('F', 'O', 'R', 'M')) {

		// Maybe an AIFF file, check further
		uint32 total_size = SDL_ReadBE32(music_rw);
		if (SDL_ReadBE32(music_rw) != FOUR_CHARS_TO_INT('A', 'I', 'F', 'F'))
			return false;

		// Seems so, look for COMM and SSND chunks
		bool comm_found = false;
		bool ssnd_found = false;
		do {

			// Read chunk ID and size
			uint32 id = SDL_ReadBE32(music_rw);
			uint32 size = SDL_ReadBE32(music_rw);
			int pos = SDL_RWtell(music_rw);

			switch (id) {
				case FOUR_CHARS_TO_INT('C', 'O', 'M', 'M'): {
					comm_found = true;

					c->stereo = (SDL_ReadBE16(music_rw) == 2);
					SDL_RWseek(music_rw, 4, SEEK_CUR);
					c->sixteen_bit = (SDL_ReadBE16(music_rw) == 16);
					c->signed_8bit = true;

					uint32 srate = SDL_ReadBE32(music_rw);	// This is a 6888x 80-bit floating point number, but we only read the first 4 bytes and try to guess the sample rate
					switch (srate) {
						case 0x400eac44:
							music_sample_rate = 44100;
							break;
						case 0x400dac44:
							music_sample_rate = 22050;
							break;
						case 0x400cac44:
						default:
							music_sample_rate = 11025;
					}

					break;
				}

				case FOUR_CHARS_TO_INT('S', 'S', 'N', 'D'):
					ssnd_found = true;

					music_data_length = size;
					SDL_RWseek(music_rw, 8, SEEK_CUR);
					music_data_offset = SDL_RWtell(music_rw);
					break;
			}

			// Skip to next chunk
			if (size & 1)
				size++;
			SDL_RWseek(music_rw, pos + size, SEEK_SET);

		} while (uint32(SDL_RWtell(music_rw)) < total_size);

		if (!comm_found)
			return false;
		if (!ssnd_found)
			return false;
	} else if (magic == FOUR_CHARS_TO_INT('R', 'I', 'F', 'F')) {

		// Maybe a WAV file, check further
		uint32 total_size = SDL_ReadLE32(music_rw);
		if (SDL_ReadBE32(music_rw) != FOUR_CHARS_TO_INT('W', 'A', 'V', 'E'))
			return false;

		// Seems so, look for fmt and data chunks
		bool fmt_found = false;
		bool data_found = false;
		do {

			// Read chunk ID and size
			uint32 id = SDL_ReadBE32(music_rw);
			uint32 size = SDL_ReadLE32(music_rw);
			int pos = SDL_RWtell(music_rw);

			switch (id) {
				case FOUR_CHARS_TO_INT('f', 'm', 't', ' '):
					fmt_found = true;

					if (SDL_ReadLE16(music_rw) != 1) // PCM encoding
						return false;
					c->stereo = (SDL_ReadLE16(music_rw) == 2);
					music_sample_rate = SDL_ReadLE32(music_rw);
					SDL_RWseek(music_rw, 4, SEEK_CUR);
					c->bytes_per_frame = SDL_ReadLE16(music_rw);
					c->sixteen_bit = (SDL_ReadLE16(music_rw) == 16);
					c->signed_8bit = false;
					break;

				case FOUR_CHARS_TO_INT('d', 'a', 't', 'a'):
					data_found = true;

					music_data_length = size;
					music_data_offset = SDL_RWtell(music_rw);
					break;
			}

			// Skip to next chunk
			if (size & 1)
				size++;
			SDL_RWseek(music_rw, pos + size, SEEK_SET);

		} while (uint32(SDL_RWtell(music_rw)) < total_size);

		if (!fmt_found)
			return false;
		if (!data_found)
			return false;
	} else {
		return false;
	}
#endif
	c->bytes_per_frame = 1;
	if (c->stereo)
		c->bytes_per_frame *= 2;
	if (c->sixteen_bit)
		c->bytes_per_frame *= 2;

	c->rate = (music_sample_rate << 16) / obtained.freq;
	c->loop_length = 0;

	return true;
}
#endif

/*
 *  Shutdown music handling
 */

static void shutdown_music_handler()
{
	if (music_initialized) {
		music_initialized = false;

#ifdef MUSIC_SDL
		free_music_channel();
#ifdef MUSIC_SDL_SOUND
		Sound_FreeSample(music_sample);
#else
		SDL_RWclose(music_rw);
#endif
#endif
#ifdef MUSIC_WIN32
	    // Release everything.
		gp_media_control->Stop();
		gp_media_control->Release();
		gp_media_control = NULL;

		gp_media_event_ex->Release();
		gp_media_event_ex = NULL;

		gp_media_seeking->Release();
		gp_media_seeking = NULL;

		gp_graph_builder->Release();
		gp_graph_builder = NULL;

		CoUninitialize();
#endif
	}
}


/*
 *  Enable/disable sound manager
 */

static void set_sound_manager_status(bool active)
{
	if (_sm_initialized) {
		if (active != _sm_active) {
			if (active) {

				// Set channel count
				_sm_globals->total_channel_count = _sm_parameters->channel_count;
				if (_sm_parameters->flags & _ambient_sound_flag)
					_sm_globals->total_channel_count += MAXIMUM_AMBIENT_SOUND_CHANNELS;

				// Set buffer size
				_sm_globals->total_buffer_size = (_sm_parameters->flags & _more_sounds_flag) ? MORE_SOUND_BUFFER_SIZE : MINIMUM_SOUND_BUFFER_SIZE;
				if (_sm_parameters->flags & _ambient_sound_flag)
					_sm_globals->total_buffer_size += AMBIENT_SOUND_BUFFER_SIZE;
				if (_sm_parameters->flags & _16bit_sound_flag)
					_sm_globals->total_buffer_size *= 2;
				if (_sm_globals->available_flags & _extra_memory_flag)
					_sm_globals->total_buffer_size *= 2;

				_sm_globals->sound_source = (_sm_parameters->flags & _16bit_sound_flag) ? _16bit_22k_source : _8bit_22k_source;
				_sm_globals->base_sound_definitions= sound_definitions;
				_sm_globals->used_sound_definitions= &sound_definitions[(_sm_globals->sound_source == _16bit_22k_source)?number_of_sound_definitions:0];
				
				// Initialize channels
				struct channel_data *channel = _sm_globals->channels;
				for (int i=0; i<SM_SOUND_CHANNELS; i++, channel++) {
					channel->flags = 0;
					channel->callback_count = 0;
					channel->sound_index = NONE;
					sdl_channel *c = (sdl_channel *)channel->channel;
					memset(c, 0, sizeof(sdl_channel));
					c->left_volume = c->right_volume = 0x100;
				}

				// Set global volume
				main_volume = _sm_parameters->volume * SOUND_VOLUME_DELTA;

				// Activate SDL audio
				desired.freq = (_sm_parameters->pitch >> 16) * 22050;
				desired.format = _sm_parameters->flags & _16bit_sound_flag ? AUDIO_S16SYS : AUDIO_S8;
				desired.channels = _sm_parameters->flags & _stereo_flag ? 2 : 1;
				desired.samples = 2048;
				desired.callback = sound_callback;
				desired.userdata = NULL;
				if (option_nosound || SDL_OpenAudio(&desired, &obtained) < 0) {
					if (!option_nosound)
						alert_user(infoError, strERRORS, badSoundChannels, -1);
					_sm_globals->total_channel_count = 0;
					active = _sm_active = _sm_initialized = false;
				}
				SDL_PauseAudio(false);


			} else {

				// Stop sound manager
				stop_all_sounds();
				SDL_CloseAudio();
				_sm_globals->total_channel_count = 0;
			}
			_sm_active = active;
		}
	}
}


/*
 *  Set sound manager preferences
 */

void set_sound_manager_parameters(struct sound_manager_parameters *parameters)
{
	if (_sm_initialized) {
		bool initial_state = _sm_active;

		verify_sound_manager_parameters(parameters);

		// If it was initially on, turn off the sound manager
		if (initial_state)
			set_sound_manager_status(false);

		// We need to get rid of the sounds we have in memory
		unload_all_sounds();

		// Stuff in our new parameters
		*_sm_parameters = *parameters;

		// If it was initially on, turn the sound manager back on
		if (initial_state && parameters->volume)
			set_sound_manager_status(true);
	}
}


/*
 *  Adjust sound volume
 */

bool adjust_sound_volume_up(struct sound_manager_parameters *parameters, short sound_index)
{
	if (_sm_active && parameters->volume < NUMBER_OF_SOUND_VOLUME_LEVELS) {
		_sm_parameters->volume = (parameters->volume += 1);
		main_volume = parameters->volume * SOUND_VOLUME_DELTA;
		play_sound(sound_index, NULL, NONE);
		return true;
	}
	return false;
}

bool adjust_sound_volume_down(struct sound_manager_parameters *parameters, short sound_index)
{
	if (_sm_active && parameters->volume > 0) {
		_sm_parameters->volume = (parameters->volume -= 1);
		main_volume = parameters->volume * SOUND_VOLUME_DELTA;
		play_sound(sound_index, NULL, NONE);
		return true;
	}
	return false;
}

void test_sound_volume(short volume, short sound_index)
{
	if (_sm_active) {
		if ((volume = PIN(volume, 0, NUMBER_OF_SOUND_VOLUME_LEVELS)) != 0) {
			play_sound(sound_index, NULL, NONE);
			main_volume = volume * SOUND_VOLUME_DELTA;
			while (sound_is_playing(sound_index))
				SDL_Delay(10);
			main_volume = _sm_parameters->volume * SOUND_VOLUME_DELTA;
		}
	}
}


/*
 *  Is channel busy?
 */

static bool channel_busy(struct channel_data *channel)
{
	assert(SLOT_IS_USED(channel));
	sdl_channel *c = (sdl_channel *)channel->channel;
	return c->active;
}


/*
 *  Unlock sound
 */

static void unlock_sound(short sound_index)
{
	// nothing to do
}


/*
 *  Dispose of sound
 */

static void dispose_sound(short sound_index)
{
	struct sound_definition *definition = get_sound_definition(sound_index);
	if (!definition)
		return;
	
	if (!definition->ptr)
		return;

	_sm_globals->loaded_sounds_size -= definition->size;
	free(definition->ptr);
	definition->ptr = NULL;
	definition->size = 0;
}


/*
 *  Read sound from file, return pointer to data (should be asynchronous and only read
 *  a single sound unless _sm_parameters->flags & _more_sounds_flag)
 */

static byte *read_sound_from_file(short sound_index, int32 &size)
{
	size = 0;

	struct sound_definition *definition = get_sound_definition(sound_index);
	if (!definition)
		return NULL;

	uint8 *data = NULL;
	OSErr error= noErr;
	
	if (SoundFile.IsOpen()) {
		size = (_sm_parameters->flags&_more_sounds_flag) ? definition->total_length : definition->single_length;

		if ((data = (uint8 *)malloc(size)) != NULL) {
			if (!SoundFile.SetPosition(definition->group_offset)) {
				free(data);
				data = NULL;
				error = SoundFile.GetError();
			}
			if (SoundFile.Read(size, data))
				_sm_globals->loaded_sounds_size += size;
			else {
				free(data);
				data = NULL;
				error = SoundFile.GetError();
			}
		} else
			error = -1;
	}
	
	vwarn(error==noErr, csprintf(temporary, "read_sound_from_file(#%d) got error #%d", sound_index, error));
	
	return data;
}


/*
 *  Stop sounds on channel
 */

static void quiet_channel(struct channel_data *channel)
{
	sdl_channel *c = (sdl_channel *)channel->channel;
	c->active = false;
}


/*
 *  Set variables for sound channel
 */

static void instantiate_sound_variables(struct sound_variables *variables, struct channel_data *channel, bool first_time)
{
	if (first_time || variables->right_volume != channel->variables.right_volume || variables->left_volume != channel->variables.left_volume) {
		sdl_channel *c = (sdl_channel *)channel->channel;
		c->left_volume = variables->left_volume;
		c->right_volume = variables->right_volume;
	}
	channel->variables = *variables;
}


/*
 *  Start sound playback (asynchronously)
 */

static void buffer_sound(struct channel_data *channel, short sound_index, _fixed pitch, bool ext_play_immed)
{
	struct sound_definition *definition = get_sound_definition(sound_index);
	if (!definition || !definition->permutations)
		return;
	assert(definition->ptr);

	int permutation = get_random_sound_permutation(sound_index);
	assert(permutation>=0 && permutation<definition->permutations);

	// Lock sound subsystem
	sdl_channel *c = (sdl_channel *)channel->channel;
	SDL_LockAudio();

	// Get pointer to sound header
	uint8 *data = (uint8 *)definition->ptr + definition->sound_offsets[permutation];

	// Channel active? Then queue next header
	if (c->active) {

		// Yes, queue next header
		c->next_header = data;
		c->next_pitch = calculate_pitch_modifier(sound_index, pitch);
		
	} else {

		// No, load sound header and start channel
		c->active = true;
		load_sound_header(c, data, calculate_pitch_modifier(sound_index, pitch));
	}

	// Unlock sound subsystem
	SDL_UnlockAudio();
}


/*
 *  Start playback of MacOS sound resource
 */

void play_sound_resource(LoadedResource &rsrc)
{
	if (!_sm_initialized || !_sm_active)
		return;

	sdl_channel *c = sdl_channels + RESOURCE_CHANNEL;

	// Open stream to resource
	SDL_RWops *p = SDL_RWFromMem(rsrc.GetPointer(), (int)rsrc.GetLength());
	if (p == NULL)
		return;

	// Get resource format
	uint16 format = SDL_ReadBE16(p);
	if (format != 1 && format != 2) {
		fprintf(stderr, "Unknown sound resource format %d\n", format);
		SDL_RWclose(p);
		return;
	}

	// Skip sound data types or reference count
	if (format == 1) {
		uint16 num_data_formats = SDL_ReadBE16(p);
		SDL_RWseek(p, num_data_formats * 6, SEEK_CUR);
	} else if (format == 2)
		SDL_RWseek(p, 2, SEEK_CUR);

	// Lock sound subsystem
	SDL_LockAudio();

	// Scan sound commands for bufferCmd
	uint16 num_cmds = SDL_ReadBE16(p);
	for (int i=0; i<num_cmds; i++) {
		uint16 cmd = SDL_ReadBE16(p);
		uint16 param1 = SDL_ReadBE16(p);
		uint32 param2 = SDL_ReadBE32(p);
		//printf("cmd %04x %04x %08x\n", cmd, param1, param2);

		if (cmd == 0x8051) {

			// bufferCmd, load sound header and start channel
			c->active = true;
			load_sound_header(c, (uint8 *)rsrc.GetPointer() + param2, FIXED_ONE);
			c->left_volume = c->right_volume = 0x100;
		}
	}

	// Unlock sound subsystem
	SDL_UnlockAudio();
	SDL_RWclose(p);
}


/*
 *  Stop playback of sound resource
 */

void stop_sound_resource()
{
	SDL_LockAudio();
	sdl_channels[RESOURCE_CHANNEL].active = false;
	SDL_UnlockAudio();
}


/*
 *  (Re)start playback of introduction music
 */

void queue_song(short song_index)
{
	if (music_intro) {
		set_music_file(&music_intro_file);
		play_music();
		music_play = true;
	}
}


void rewind_music ()
{
#ifdef MUSIC_SDL_SOUND
	Sound_Rewind (music_sample);
#endif
#ifdef MUSIC_PURE_SDL
	SDL_RWseek(music_rw, music_data_offset, SEEK_SET);
	music_data_remaining = music_data_length;
#endif
#ifdef __MACOS__
	macos_file_done = false;
	fill_music_buffer_at_interrupt();
#endif
#ifdef MUSIC_WIN32
	// Re-seek the graph to the beginning
	LONGLONG llPos = 0;
	gp_media_seeking->SetPositions(&llPos, AM_SEEKING_AbsolutePositioning,
	                               &llPos, AM_SEEKING_NoPositioning);
#endif
}

void play_music ()
{
	if (!_sm_initialized || !_sm_active || !music_initialized) {
		return;
	}
#ifdef MUSIC_SDL
	if (fill_music_buffer()) {
#ifdef __MACOS__
		fill_music_buffer_at_interrupt();
#endif
		sdl_channel *c = sdl_channels + MUSIC_CHANNEL;
		c->counter = 0;
		c->left_volume = c->right_volume = 0x100;
		c->active = true;
	}
#endif
#ifdef MUSIC_WIN32
    gp_media_control->Run();
#endif
}


/*
 *  Stop playback of music
 */

void free_music_channel()
{
	SDL_LockAudio();
	sdl_channels[MUSIC_CHANNEL].active = false;
	SDL_UnlockAudio();

	music_fading = false;
}


/*
 *  Is music playing?
 */

bool music_playing()
{
	return sdl_channels[MUSIC_CHANNEL].active;
}


/*
 *  Fade out music
 */

void fade_out_music(short duration)
{
	if (music_play) {
		// don't restart music
		music_play = false;

		music_fading = true;
		music_fade_start = SDL_GetTicks();
		music_fade_duration = duration;
	}
}


void music_idle_proc()
{
#ifdef MUSIC_SDL
	// Start preloaded music
	if (music_prelevel) {
		music_prelevel = false;
		play_music();
	}

	// Keep music going
	if (!music_playing())
		restart_music();
#endif

	if (music_fading) {

		// Calculate volume level
		uint32 elapsed = SDL_GetTicks() - music_fade_start;
		int vol = 0x100 - (elapsed * 0x100) / music_fade_duration;
		if (vol <= 0) {
			// Fading done, stop music
			free_music_channel();
		} else {

			// Set new volume level
			if (vol > 0x100)
				vol = 0x100;
			sdl_channels[MUSIC_CHANNEL].left_volume = sdl_channels[MUSIC_CHANNEL].right_volume = vol;
		}
	}

#ifdef __MACOS__
	fill_music_buffer();
#endif
}

/*
 * (Re)start appropriate music when none is playing
 */

void restart_music ()
{
	if (music_play) {
		if (music_level) {
			// Load next song in playlist
			LoadLevelMusic();
			// Play song
			play_music();
		} else if (music_intro) {
			// (Re)start intro music
			set_music_file(&music_intro_file);
			play_music();
		}
	}
}

/*
 *  Preload per-level music
 */

void PreloadLevelMusic()
{
	LoadLevelMusic();

	if (music_initialized) {
		music_prelevel = true;
		music_level = true;
		music_play = true;
	}
}

void LoadLevelMusic()
{
	FileSpecifier* level_song_file = GetLevelMusic();
	set_music_file(level_song_file);
}


void StopLevelMusic()
{
 	music_level = false;
	music_play = false;
	set_music_file(NULL);
}


#ifdef MUSIC_WIN32
void process_music_event_win32(const SDL_Event& event)
{
    long evCode, param1, param2;
    HRESULT hr;

    if (gp_media_event_ex == NULL)
        return;

    // We must restart the music if it stops playing.
    while (hr = gp_media_event_ex->GetEvent(&evCode, &param1, &param2, 0), SUCCEEDED(hr)) {
        hr = gp_media_event_ex->FreeEventParams(evCode, param1, param2);
        if ((EC_COMPLETE == evCode) || (EC_USERABORT == evCode)) {
			restart_music ();
			break;
		}
	}
}
#endif


/*
 *  Load sound header into channel
 */

static void load_sound_header(sdl_channel *c, uint8 *data, _fixed pitch)
{
	// Open stream to header
	SDL_RWops *p = SDL_RWFromMem(data, 64);
	if (p == NULL) {
		c->active = false;
		return;
	}

	// Get sound header type, skip unused sample pointer
	uint8 header_type = data[20];
	SDL_RWseek(p, 4, SEEK_CUR);

	// Parse sound header
	c->bytes_per_frame = 1;
	c->signed_8bit = false;
	if (header_type == 0x00) {			// Standard sound header
		//printf("standard sound header\n");
		c->data = data + 22;
		c->sixteen_bit = c->stereo = false;
		c->length = SDL_ReadBE32(p);
		c->rate = (pitch >> 8) * ((SDL_ReadBE32(p) >> 8) / obtained.freq);
		uint32 loop_start = SDL_ReadBE32(p);
		c->loop = c->data + loop_start;
		c->loop_length = SDL_ReadBE32(p) - loop_start;
	} else if (header_type == 0xff || header_type == 0xfe) {	// Extended/compressed sound header
		//printf("extended/compressed sound header\n");
		c->data = data + 64;
		c->stereo = SDL_ReadBE32(p) == 2;
		if (c->stereo)
			c->bytes_per_frame *= 2;
		c->rate = (pitch >> 8) * ((SDL_ReadBE32(p) >> 8) / obtained.freq);
		uint32 loop_start = SDL_ReadBE32(p);
		c->loop = c->data + loop_start;
		c->loop_length = SDL_ReadBE32(p) - loop_start;
		SDL_RWseek(p, 2, SEEK_CUR);
		c->length = SDL_ReadBE32(p) * c->bytes_per_frame;
		if (header_type == 0xfe) {
			SDL_RWseek(p, 14, SEEK_CUR);
			uint32 format = SDL_ReadBE32(p);
			SDL_RWseek(p, 12, SEEK_CUR);
			int16 comp_id = SDL_ReadBE16(p);
			if (format != FOUR_CHARS_TO_INT('t', 'w', 'o', 's') || comp_id != -1) {
				fprintf(stderr, "Unsupported compressed sound header format '%c%c%c%c', ID %d\n", data[40], data[41], data[42], data[43], comp_id);
				c->active = false;
				SDL_RWclose(p);
				return;
			}
			SDL_RWseek(p, 4, SEEK_CUR);
		} else {
			SDL_RWseek(p, 22, SEEK_CUR);
		}
		c->sixteen_bit = (SDL_ReadBE16(p) == 16);
		if (c->sixteen_bit) {
			c->bytes_per_frame *= 2;
			c->length *= 2;
		}
	} else {							// Unknown header type
		fprintf(stderr, "Unknown sound header type %02x\n", header_type);
		c->active = false;
		SDL_RWclose(p);
		return;
	}

	// Correct loop count
	if (c->loop_length < 4)
		c->loop_length = 0;

	//printf(" data %p, length %d, loop %p, loop_length %d, rate %08x, stereo %d, 16 bit %d\n", c->data, c->length, c->loop, c->loop_length, c->rate, c->stereo, c->sixteen_bit);

	// Reset sample counter
	c->counter = 0;

	SDL_RWclose(p);
}


// ZZZ: realtime microphone stuff
// Is the locking necessary?  If checking c->active is the first thing the interrupt proc does,
// but setting c->active is the last thing we do, there's no way we can get mixed up, right?
void
ensure_network_audio_playing() {
#if !defined(DISABLE_NETWORKING)
    if(!sdl_channels[NETWORK_AUDIO_CHANNEL].active) {
        // Get the audio to play
        sNetworkAudioBufferDesc = dequeue_network_speaker_data();

        if(sNetworkAudioBufferDesc != NULL) {
            // Lock sound subsystem
	        SDL_LockAudio();

        	sdl_channel *c = sdl_channels + NETWORK_AUDIO_CHANNEL;

            // Initialize and start channel
            c->stereo           = kNetworkAudioIsStereo;
            c->sixteen_bit      = kNetworkAudioIs16Bit;
            c->signed_8bit      = kNetworkAudioIsSigned8Bit;
            c->bytes_per_frame  = kNetworkAudioBytesPerFrame;
	        c->data             = sNetworkAudioBufferDesc->mData;
	        c->length           = sNetworkAudioBufferDesc->mLength;
	        c->loop_length      = 0;
	        c->rate             = (kNetworkAudioSampleRate << 16) / obtained.freq;
	        c->left_volume      = 0x100;
            c->right_volume     = 0x100;
	        c->counter          = 0;
	        c->active           = true;

	        // Unlock sound subsystem
	        SDL_UnlockAudio();
        }
    }
#endif // !defined(DISABLE_NETWORKING)
}

// I can see locking here because we're invalidating some storage, and we want to
// make sure the play routine does not trail on a little bit using that storage.
void
stop_network_audio() {
#if !defined(DISABLE_NETWORKING)
	SDL_LockAudio();
	sdl_channels[NETWORK_AUDIO_CHANNEL].active = false;
    if(sNetworkAudioBufferDesc != NULL) {
        if(is_sound_data_disposable(sNetworkAudioBufferDesc))
            release_network_speaker_buffer(sNetworkAudioBufferDesc->mData);
        sNetworkAudioBufferDesc = NULL;
    }
	SDL_UnlockAudio();
#endif // !defined(DISABLE_NETWORKING)
}


/*
 *  Sound callback function
 */

template <class T>
static inline void calc_buffer(T *p, int len, bool stereo, bool is_signed)
{
#ifndef SDL_RFORK_HACK
	while (len--) {
		int32 left = 0, right = 0;	// 16-bit internally

		// Mix all channels
		sdl_channel *c = sdl_channels;
		for (int i=0; i<TOTAL_SOUND_CHANNELS; i++, c++) {
			// Channel active?
			if (c->active) {
				// Yes, read sound data
				int32 dleft, dright;
				if (c->stereo) {
					if (c->sixteen_bit) {
						dleft = (int16)SDL_SwapBE16(0[(int16 *)c->data]);
						dright = (int16)SDL_SwapBE16(1[(int16 *)c->data]);
					} else if (c->signed_8bit) {
						dleft = (int32)(int8)(0[c->data]) * 256;
						dright = (int32)(int8)(1[c->data]) * 256;
					} else {
						dleft = (int32)(int8)(0[c->data] ^ 0x80) * 256;
						dright = (int32)(int8)(1[c->data] ^ 0x80) * 256;
					}
				} else {
					if (c->sixteen_bit)
						dleft = dright = (int16)SDL_SwapBE16(*(int16 *)c->data);
					else if (c->signed_8bit)
						dleft = dright = (int32)(int8)(*(c->data)) << 8;
					else
						dleft = dright = (int32)(int8)(*(c->data) ^ 0x80) << 8;
				}

				// Mix into output
				left += (dleft * c->left_volume) >> 8;
				right += (dright * c->right_volume) >> 8;

				// Advance sound data pointer
				c->counter += c->rate;
				if (c->counter >= 0x10000) {
					int count = c->counter >> 16;
					c->counter &= 0xffff;
					c->data += c->bytes_per_frame * count;
					c->length -= c->bytes_per_frame * count;

					// Sound finished? Then enter loop or load next sound header
					if (c->length <= 0) {

						// Yes, loop present?
						if (c->loop_length) {

							// Yes, enter loop
							c->data = c->loop;
							c->length = c->loop_length;

						} else if (i == MUSIC_CHANNEL) {

							// More music data?
#ifdef __MACOS__
							if (!fill_music_buffer_at_interrupt()) {
#else
							if (!fill_music_buffer ()) {
#endif
								// Music finished, turn it off
								c->active = false;
							}

						} else if (i == RESOURCE_CHANNEL) {

							// Resource channel, turn it off
							c->active = false;

						}
						else if (i == NETWORK_AUDIO_CHANNEL) {
#if !defined(DISABLE_NETWORKING)

							// ZZZ: if we're supposed to dispose of the storage, so be it
							if (is_sound_data_disposable(sNetworkAudioBufferDesc))
								release_network_speaker_buffer(sNetworkAudioBufferDesc->mData);

							// Get the next buffer of data
							sNetworkAudioBufferDesc = dequeue_network_speaker_data();

							// If we have a buffer to play, set it up; else deactivate the channel.
							if (sNetworkAudioBufferDesc != NULL) {
								c->data     = sNetworkAudioBufferDesc->mData;
								c->length   = sNetworkAudioBufferDesc->mLength;
							}
							else
								c->active   = false;
#endif // !defined(DISABLE_NETWORKING)
						} else {

							// No loop, another sound header queued?
							_sm_globals->channels[i].callback_count++;
							if (c->next_header) {

								// Yes, load sound header and continue
								load_sound_header(c, c->next_header, c->next_pitch);
								c->next_header = NULL;

							} else {

								// No, turn off channel
								c->active = false;
							}
						}
					}
				}
			}
		}

		// Mix left+right for mono
		if (!stereo)
			left = (left + right) / 2;

		// Finalize left channel
		left = (left * main_volume) >> 8;		// Apply main volume setting
		if (left > 32767)						// Clip output value
			left = 32767;
		else if (left < -32768)
			left = -32768;
		if (sizeof(T) == 1)
			if (is_signed)
				left >>= 8;
			else
				left = (left >> 8) ^ 0x80;
		*p++ = left;							// Write to output buffer

		// Finalize right channel
		if (stereo) {
			right = (right * main_volume) >> 8;	// Apply main volume setting
			if (right > 32767)					// Clip output value
				right = 32767;
			else if (right < -32768)
				right = -32768;
			if (sizeof(T) == 1)
				if (is_signed)
// Downscale for 8-bit output
					right >>= 8;
				else
					right = (right >> 8) ^ 0x80;
			*p++ = right;						// Write to output buffer
		}
	}
#endif
}

static void sound_callback(void *usr, uint8 *stream, int len)
{
	bool stereo = (obtained.channels == 2);
	if ((obtained.format & 0xff) == 16) {
		if (stereo)	// try to inline as much as possible
			calc_buffer((int16 *)stream, len / 4, true, true);
		else
			calc_buffer((int16 *)stream, len / 2, false, true);
	} else if (obtained.format & 0x1000) {
		if (stereo)
			calc_buffer((int16 *)stream, len / 2, true, true);
		else
			calc_buffer((int16 *)stream, len, false, true);
	} else {
		if (stereo)
			calc_buffer((int8 *)stream, len / 2, true, false);
		else
			calc_buffer((int8 *)stream, len, false, false);
	}
}


/*
 * Fill music buffer from file
 */

#ifdef __MACOS__

uint32 macos_buffer_length = 0;

bool fill_music_buffer_at_interrupt()
{
	static uint8 macos_music_buffer[MUSIC_BUFFER_SIZE];
	if (macos_file_done) return false;

	// otherwise, copy out of the buffer (I know), and set the flag to read more when we're not at interrupt time
	memcpy(macos_music_buffer, music_buffer, macos_buffer_length);
	sdl_channel *c = sdl_channels + MUSIC_CHANNEL;
	c->data = macos_music_buffer;
	c->length = macos_buffer_length;
	macos_read_more = true;
	return true;
}

#endif

bool fill_music_buffer ()
{
#ifdef __MACOS__
	if (!macos_read_more) return false;
#endif
	sdl_channel *c = sdl_channels + MUSIC_CHANNEL;

#ifdef MUSIC_SDL_SOUND
	Uint32 decoded;
	int attempt = 5;	// retry up to five times

	do {
		decoded = Sound_Decode (music_sample);
		if (decoded > 0) {
#ifdef __MACOS__
			macos_buffer_length = decoded;
			macos_read_more = false;
#else
			c->data = (uint8 *) music_sample->buffer;
			c->length = decoded;
#endif
			return true;
		}

		if (!(music_sample->flags & SOUND_SAMPLEFLAG_EAGAIN)) {
			// This is not a temporary error
			return false;
		}
	}  while (attempt-- > 0);

#endif
#ifdef MUSIC_PURE_SDL
	uint32 to_read = music_data_remaining > MUSIC_BUFFER_SIZE ? MUSIC_BUFFER_SIZE : music_data_remaining;
	if (to_read > 0) {
		// Read next buffer of music data
		SDL_RWread(music_rw, music_buffer, 1, to_read);
		music_data_remaining -= to_read;
#ifdef __MACOS__
		macos_buffer_length = to_read;
		macos_read_more = false;
#else
		c->data = music_buffer;
		c->length = to_read;
#endif
		return true;
	}
#endif
	// Failed
#ifdef __MACOS__
	macos_file_done = true;
#endif
	return false;
}


/*
 *  External sounds stuff (unimplemented)
 */

bool LoadedSound::Load(FileSpecifier &file)
{
	return false;
}

bool LoadedSound::Unload()
{
	return true;
}
