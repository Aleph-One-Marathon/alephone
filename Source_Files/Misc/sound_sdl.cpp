/*
 *  sound_sdl.cpp - Sound/music management, SDL specific stuff (included by mysound.cpp)
 *
 *  Written in 2000 by Christian Bauer
 */

#include <SDL_endian.h>

// Unlike the Mac version, we also do the music handling here
#include "music.h"
#include "song_definitions.h"


// Number of sound channels used by Aleph One sound manager
const int SM_SOUND_CHANNELS = MAXIMUM_SOUND_CHANNELS + MAXIMUM_AMBIENT_SOUND_CHANNELS;

// Private channels
const int MUSIC_CHANNEL = SM_SOUND_CHANNELS;
const int RESOURCE_CHANNEL = SM_SOUND_CHANNELS + 1;

// Total number of sound channels
const int TOTAL_SOUND_CHANNELS = SM_SOUND_CHANNELS + 2;


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
static SDL_AudioSpec desired;

// Global sound volume (0x100 = nominal)
static int16 main_volume = 0x100;

// Data for music replay
static OpenedFile music_file;			// Music file object
static uint32 music_data_length;		// Total length of sample data in file
static uint32 music_data_remaining;		// Remaining unread sample data in file
static uint32 music_data_offset;		// Start offset of sample data
static uint32 music_sample_rate;		// Music sample rate in Hz
static bool music_initialized = false;	// Flag: music ready to play
static bool music_fading = false;		// Flag: music fading out
static uint32 music_fade_start;			// Music fade start tick
static uint32 music_fade_duration;		// Music fade duration in ticks

const int MUSIC_BUFFER_SIZE = 0x40000;
static uint8 music_buffer[MUSIC_BUFFER_SIZE];


// From FileHandler_SDL.cpp
extern void get_default_sounds_spec(FileSpecifier &file);

// From shell_sdl.cpp
extern bool option_nosound;

// Prototypes
static void close_sound_file(void);
static void shutdown_sound_manager(void);
static void shutdown_music_handler(void);
static void set_sound_manager_status(bool active);
static void load_sound_header(sdl_channel *c, uint8 *data, _fixed pitch);
static void sound_callback(void *userdata, uint8 *stream, int len);


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

static void shutdown_sound_manager(void)
{
	set_sound_manager_status(false);
	close_sound_file();
}


/*
 *  Initialize music handling
 */

bool initialize_music_handler(FileSpecifier &song_file)
{
	assert(NUMBER_OF_SONGS == sizeof(songs) / sizeof(struct song_definition));
	sdl_channel *c = sdl_channels + MUSIC_CHANNEL;

	atexit(shutdown_music_handler);

	// Open music file
	if (song_file.Open(music_file)) {

		SDL_RWops *p = music_file.GetRWops();

		// Read magic ID
		uint32 magic = SDL_ReadBE32(p);
		if (magic == FOUR_CHARS_TO_INT('F', 'O', 'R', 'M')) {

			// Maybe an AIFF file, check further
			uint32 total_size = SDL_ReadBE32(p);
			if (SDL_ReadBE32(p) != FOUR_CHARS_TO_INT('A', 'I', 'F', 'F'))
				return false;

			// Seems so, look for COMM and SSND chunks
			bool comm_found = false;
			bool ssnd_found = false;
			do {

				// Read chunk ID and size
				uint32 id = SDL_ReadBE32(p);
				uint32 size = SDL_ReadBE32(p);
				int pos = SDL_RWtell(p);

				switch (id) {
					case FOUR_CHARS_TO_INT('C', 'O', 'M', 'M'): {
						comm_found = true;

						c->stereo = (SDL_ReadBE16(p) == 2);
						SDL_RWseek(p, 4, SEEK_CUR);
						c->sixteen_bit = (SDL_ReadBE16(p) == 16);
						c->signed_8bit = true;

						c->bytes_per_frame = 1;
						if (c->stereo)
							c->bytes_per_frame *= 2;
						if (c->sixteen_bit)
							c->bytes_per_frame *= 2;

						uint32 srate = SDL_ReadBE32(p);	// This is a 6888x 80-bit floating point number, but we only read the first 4 bytes and try to guess the sample rate
						switch (srate) {
							case 0x400eac44:
								music_sample_rate = 44100;
								break;
							case 0x400dac44:
							default:
								music_sample_rate = 22050;
								break;
						}
						break;
					}

					case FOUR_CHARS_TO_INT('S', 'S', 'N', 'D'):
						ssnd_found = true;

						music_data_length = size;
						SDL_RWseek(p, 8, SEEK_CUR);
						music_data_offset = SDL_RWtell(p);
						break;
				}

				// Skip to next chunk
				if (size & 1)
					size++;
				SDL_RWseek(p, pos + size, SEEK_SET);

			} while (SDL_RWtell(p) < total_size);

			if (comm_found && ssnd_found)
				music_initialized = true;

		} else if (magic == FOUR_CHARS_TO_INT('R', 'I', 'F', 'F')) {

			// Maybe a WAV file, check further
			uint32 total_size = SDL_ReadLE32(p);
			if (SDL_ReadBE32(p) != FOUR_CHARS_TO_INT('W', 'A', 'V', 'E'))
				return false;

			// Seems so, look for fmt and data chunks
			bool fmt_found = false;
			bool data_found = false;
			do {

				// Read chunk ID and size
				uint32 id = SDL_ReadBE32(p);
				uint32 size = SDL_ReadLE32(p);
				int pos = SDL_RWtell(p);

				switch (id) {
					case FOUR_CHARS_TO_INT('f', 'm', 't', ' '):
						fmt_found = true;

						if (SDL_ReadLE16(p) != 1) // PCM encoding
							return false;
						c->stereo = (SDL_ReadLE16(p) == 2);
						music_sample_rate = SDL_ReadLE32(p);
						SDL_RWseek(p, 4, SEEK_CUR);
						c->bytes_per_frame = SDL_ReadLE16(p);
						c->sixteen_bit = (SDL_ReadLE16(p) == 16);
						c->signed_8bit = false;
						break;

					case FOUR_CHARS_TO_INT('d', 'a', 't', 'a'):
						data_found = true;

						music_data_length = size;
						music_data_offset = SDL_RWtell(p);
						break;
				}

				// Skip to next chunk
				if (size & 1)
					size++;
				SDL_RWseek(p, pos + size, SEEK_SET);

			} while (SDL_RWtell(p) < total_size);

			if (fmt_found && data_found)
				music_initialized = true;
		}
	}

	return music_initialized;
}


/*
 *  Shutdown music handling
 */

static void shutdown_music_handler(void)
{
	free_music_channel();
	music_file.Close();
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
				_sm_globals->base_sound_definitions = sound_definitions + _sm_globals->sound_source * number_of_sound_definitions;

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
				desired.samples = 1024;
				desired.callback = sound_callback;
				desired.userdata = NULL;
				if (option_nosound || SDL_OpenAudio(&desired, NULL) < 0) {
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
		if (initial_state)
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

static void buffer_sound(struct channel_data *channel, short sound_index, _fixed pitch)
{
	struct sound_definition *definition = get_sound_definition(sound_index);
	if (!definition)
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

		// Yes, queue mext header
		c->next_header = data;
		c->next_pitch = pitch;

	} else {

		// No, load sound header and start channel
		load_sound_header(c, data, pitch);
		c->active = true;
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
	SDL_RWops *p = SDL_RWFromMem(rsrc.GetPointer(), rsrc.GetLength());
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
			load_sound_header(c, (uint8 *)rsrc.GetPointer() + param2, FIXED_ONE);
			c->left_volume = c->right_volume = 0x100;
			c->active = true;
		}
	}

	// Unlock sound subsystem
	SDL_UnlockAudio();
	SDL_RWclose(p);
}


/*
 *  Stop playback of sound resource
 */

void stop_sound_resource(void)
{
	SDL_LockAudio();
	sdl_channels[RESOURCE_CHANNEL].active = false;
	SDL_UnlockAudio();
}


/*
 *  Start playback of music
 */

void queue_song(short song_index)
{
	if (!_sm_initialized || !_sm_active || !music_initialized)
		return;

	sdl_channel *c = sdl_channels + MUSIC_CHANNEL;
	SDL_RWops *p = music_file.GetRWops();

	// Read first buffer of music data
	SDL_RWseek(p, music_data_offset, SEEK_SET);
	music_data_remaining = music_data_length;
	uint32 to_read = music_data_remaining > MUSIC_BUFFER_SIZE ? MUSIC_BUFFER_SIZE : music_data_remaining;
	SDL_RWread(p, music_buffer, 1, to_read);
//!!	if (!c->sixteen_bit && !c->signed) {
//		for (int i=0; i<MUSIC_BUFFER_SIZE; i++)
//			music_buffer[i] ^= 0x80;
//	}
	music_data_remaining -= to_read;

	// Lock sound subsystem
	SDL_LockAudio();

	// Initialize and start channel
	c->data = music_buffer;
	c->length = to_read;
	c->loop_length = 0;
	c->rate = (music_sample_rate << 16) / desired.freq;
	c->left_volume = c->right_volume = 0x100;
	music_fading = false;
	c->counter = 0;
	c->active = true;

	// Unlock sound subsystem
	SDL_UnlockAudio();
}


/*
 *  Stop playback of music
 */

void free_music_channel(void)
{
	music_fading = false;
	SDL_LockAudio();
	sdl_channels[MUSIC_CHANNEL].active = false;
	SDL_UnlockAudio();
}


/*
 *  Is music playing?
 */

bool music_playing(void)
{
	return sdl_channels[MUSIC_CHANNEL].active;
}


/*
 *  Fade out music
 */

void fade_out_music(short duration)
{
	music_fading = true;
	music_fade_start = SDL_GetTicks();
	music_fade_duration = duration;
}

void music_idle_proc(void)
{
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
}


/*
 *  Preload per-level music
 */

void PreloadLevelMusic(void)
{
}


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
		c->rate = (pitch >> 8) * ((SDL_ReadBE32(p) >> 8) / desired.freq);
		uint32 loop_start = SDL_ReadBE32(p);
		c->loop = c->data + loop_start;
		c->loop_length = SDL_ReadBE32(p) - loop_start;
	} else if (header_type == 0xff) {	// Extended sound header
		//printf("extended sound header\n");
		c->data = data + 64;
		c->stereo = SDL_ReadBE32(p) == 2;
		if (c->stereo)
			c->bytes_per_frame *= 2;
		c->rate = (pitch >> 8) * ((SDL_ReadBE32(p) >> 8) / desired.freq);
		uint32 loop_start = SDL_ReadBE32(p);
		c->loop = c->data + loop_start;
		c->loop_length = SDL_ReadBE32(p) - loop_start;
		SDL_RWseek(p, 2, SEEK_CUR);
		c->length = SDL_ReadBE32(p) * c->bytes_per_frame;
		SDL_RWseek(p, 22, SEEK_CUR);
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


/*
 *  Sound callback function
 */

template <class T>
inline static void calc_buffer(T *p, int len, bool stereo)
{
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

							// Music channel, more data to play?
							uint32 to_read = music_data_remaining > MUSIC_BUFFER_SIZE ? MUSIC_BUFFER_SIZE : music_data_remaining;
							if (to_read) {

								// Read next buffer of music data
								SDL_RWread(music_file.GetRWops(), music_buffer, 1, to_read);
//!!								if (!c->sixteen_bit) {
//									// AIFF data is always signed
//									for (int i=0; i<MUSIC_BUFFER_SIZE; i++)
//										music_buffer[i] ^= 0x80;
//								}
								music_data_remaining -= to_read;
								c->data = music_buffer;
								c->length = to_read;

							} else {

								// Music replay finished, turn off channel
								c->active = false;
							}

						} else if (i == RESOURCE_CHANNEL) {

							// Resource channel, turn it off
							c->active = false;

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
		if (sizeof(T) == 1)						// Downscale for 8-bit output
			left >>= 8;
		*p++ = left;							// Write to output buffer

		// Finalize right channel
		if (stereo) {
			right = (right * main_volume) >> 8;	// Apply main volume setting
			if (right > 32767)					// Clip output value
				right = 32767;
			else if (right < -32768)
				right = -32768;
			if (sizeof(T) == 1)					// Downscale for 8-bit output
				right >>= 8;
			*p++ = right;						// Write to output buffer
		}
	}
}

static void sound_callback(void *usr, uint8 *stream, int len)
{
	bool stereo = (desired.channels == 2);
	if ((desired.format & 0xff) == 16) {
		if (stereo)	// try to inline as much as possible
			calc_buffer((int16 *)stream, len / 4, true);
		else
			calc_buffer((int16 *)stream, len / 2, false);
	} else {
		if (stereo)
			calc_buffer((int8 *)stream, len / 2, true);
		else
			calc_buffer((int8 *)stream, len, false);
	}
}
