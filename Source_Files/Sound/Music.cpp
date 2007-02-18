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

#include "Music.h"
#include "Mixer.h"
#include "XML_LevelScript.h"

Music* Music::m_instance = 0;

Music::Music() : music_initialized(false), music_intro(false), music_play(false), music_prelevel(false),
		 music_level(false), music_fading(false), music_fade_start(0), music_fade_duration(0)
#ifdef __MACOS__
	       , macos_file_done(false), macos_read_more(false), macos_buffer_length(0)
#endif
{
#ifndef HAVE_SDL_SOUND
	music_buffer.resize(MUSIC_BUFFER_SIZE);
#endif
#ifdef __MACOS__
	macos_music_buffer.resize(MUSIC_BUFFER_SIZE);
#endif
}

void Music::Open(FileSpecifier *file)
{
	if (music_initialized)
	{
		if (file && *file == music_file)
		{
			Rewind();
			return;
		}

		Close();
	}

	if (file)
	{
#ifdef __MACOS__
		macos_read_more = true;
		macos_file_done = false;
#endif

		music_initialized = Load(*file);
		music_file = *file;
	}
		
}

bool Music::SetupIntroMusic(FileSpecifier &file)
{
	music_intro_file = file;
	Open(&file);
	if (music_initialized)
		music_intro = true;
	return music_initialized;
}

void Music::RestartIntroMusic()
{
	if (music_intro)
	{
		Open(&music_intro_file);
		Play();
		music_play = true;
	}
}

void Music::FadeOut(short duration)
{
	if (music_play)
	{
		music_play = false;
		
		music_fading = true;
		music_fade_start = SDL_GetTicks();
		music_fade_duration = duration;
	}
}

bool Music::Playing()
{
	return Mixer::instance()->MusicPlaying();
}

void Music::Restart()
{
	if (music_play)
	{
		if (music_level)
		{
			LoadLevelMusic();
			Play();
		}
		else if (music_intro)
		{
			Open(&music_intro_file);
			Play();
		}
	}
}

void Music::Idle()
{
	if (music_prelevel)
	{
		music_prelevel = false;
		Play();
	}

	if (!Playing())
		Restart();

	if (music_fading)
	{
		uint32 elapsed = SDL_GetTicks() - music_fade_start;
		int vol = 0x100 - (elapsed * 0x100) / music_fade_duration;
		if (vol <= 0)
			Pause();
		else
		{
			if (vol > 0x100)
				vol = 0x100;
			// set music channel volume
		}
	}

#ifdef __MACOS__
	FillBuffer();
#endif
}

void Music::Pause()
{
	Mixer::instance()->StopMusicChannel();
	music_fading = false;
}

//extern void free_music_channel();

void Music::Close()
{
	if (music_initialized)
	{
		music_initialized = false;
		Pause();
#ifdef HAVE_SDL_SOUND
		Sound_FreeSample(music_sample);
#endif
	}
}

bool Music::Load(FileSpecifier &song_file)
{
//	if (!_sm_initialized || !_sm_active) return false;
	//sdl_channel *c = sdl_channels + MUSIC_CHANNEL;	// Music channel
	uint32 music_sample_rate; // Music sample rate in Hz
	
	// We use the file abstraction classes to open the file,
	// but then we steal the SDL_RWops it created for us
	OpenedFile music_file;
	if (!song_file.Open(music_file)) {
		puts ("failed open music file 1");
		return false;
	}
	
	music_rw = music_file.TakeRWops();
	
#ifdef HAVE_SDL_SOUND
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
	Sound_AudioInfo out = in; // music format to play
	bool convert = false; // conversion necesary?

	switch (in.format) {
		case AUDIO_U8:
			sixteen_bit = false;
			signed_8bit = false;
			break;
		case AUDIO_S8:
			sixteen_bit = false;
			signed_8bit = true;
			break;
		case AUDIO_S16MSB:
			sixteen_bit = true;
			signed_8bit = false;
			break;
		default:
			// Sound channels are big endian
			// regardless of platform
			out.format = AUDIO_S16MSB;
			sixteen_bit = true;
			convert = true;
	}

	switch (in.channels) {
		case 1:
			stereo = false;
			break;
		case 2:
			stereo = true;
			break;
		default:  // just in case music has more than two channels
			convert = true;

			switch (Mixer::instance()->obtained.channels) {
				case 1:
					out.channels = 1;
					stereo = false;
					break;
				case 2:
					out.channels = 2;
					stereo = true;
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

					stereo = (SDL_ReadBE16(music_rw) == 2);
					SDL_RWseek(music_rw, 4, SEEK_CUR);
					sixteen_bit = (SDL_ReadBE16(music_rw) == 16);
					signed_8bit = true;

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
					stereo = (SDL_ReadLE16(music_rw) == 2);
					music_sample_rate = SDL_ReadLE32(music_rw);
					SDL_RWseek(music_rw, 4, SEEK_CUR);
					bytes_per_frame = SDL_ReadLE16(music_rw);
					sixteen_bit = (SDL_ReadLE16(music_rw) == 16);
					signed_8bit = false;
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
	bytes_per_frame = 1;
	if (stereo)
		bytes_per_frame *= 2;
	if (sixteen_bit)
		bytes_per_frame *= 2;

	rate = (music_sample_rate << 16) / Mixer::instance()->obtained.freq;

	return true;
}

void Music::Rewind()
{
#ifdef HAVE_SDL_SOUND
	Sound_Rewind(music_sample);
#else
	SDL_RWseek(music_rw, music_data_offset, SEEK_SET);
	music_data_remaining = music_data_length;
#endif
#ifdef __MACOS__
	macos_file_done = false;
	InterruptFillBuffer();
#endif
}

void Music::Play()
{
	if (FillBuffer()) {
#ifdef __MACOS__
		InterruptFillBuffer();
#endif
		// let the mixer handle it
		Mixer::instance()->StartMusicChannel(sixteen_bit, stereo, signed_8bit, bytes_per_frame, rate);
	}
}

#ifdef __MACOS__
bool Music::InterruptFillBuffer()
{
	if (macos_file_done) return false;

	// otherwise, copy out of the buffer (I know), and set the flag to read more when we're not at interrupt time
	memcpy(&macos_music_buffer.front(), &music_buffer.front(), macos_buffer_length);
	Mixer::instance()->UpdateMusicChannel(&macos_music_buffer.front(), macos_buffer_length);
	macos_read_more = true;
	return true;
}
#endif

bool Music::FillBuffer()
{
#ifdef __MACOS__
	if (!macos_read_more) return false;
#endif

#ifdef HAVE_SDL_SOUND
	Uint32 decoded;
	int attempt = 5;

	do {
		decoded = Sound_Decode(music_sample);
		if (decoded > 0) {
#ifdef __MACOS__
			macos_buffer_length = decoded;
			macos_read_more = false;
#else
			Mixer::instance()->UpdateMusicChannel(reinterpret_cast<uint8 *>(music_sample->buffer), decoded);
#endif
			return true;
		}

		if (!(music_sample->flags & SOUND_SAMPLEFLAG_EAGAIN))
		{
			// this is not a temporary error
			return false;
		}
	} while (attempt-- > 0);
#else
	uint32 to_read = music_data_remaining > MUSIC_BUFFER_SIZE ? MUSIC_BUFFER_SIZE : music_data_remaining;
	if (to_read > 0)
	{
		SDL_RWread(music_rw, &music_buffer.front(), 1, to_read);
		music_data_remaining -= to_read;
#ifdef __MACOS__
		macos_buffer_length = to_read;
		macos_read_more = false;
#else
		Mixer::instance()->UpdateMusicChannel(&music_buffer.front(), to_read);
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

void Music::LoadLevelMusic()
{
	FileSpecifier* level_song_file = GetLevelMusic();
	Open(level_song_file);
}

void Music::PreloadLevelMusic()
{
	LoadLevelMusic();

	if (music_initialized)
	{
		music_prelevel = true;
		music_level = true;
		music_play = true;
	}
}

void Music::StopLevelMusic()
{
	music_level = false;
	music_play = false;
	Open(0);
}
