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
	       ,decoder(0)
{
	music_buffer.resize(MUSIC_BUFFER_SIZE);
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
		delete decoder;
		decoder = 0;
	}
}

bool Music::Load(FileSpecifier &song_file)
{

	delete decoder;
	decoder = Decoder::Get(song_file);

	if (decoder)
	{
		sixteen_bit = decoder->IsSixteenBit();
		stereo = decoder->IsStereo();
		signed_8bit = decoder->IsSigned();
		bytes_per_frame = decoder->BytesPerFrame();
		rate = (_fixed) (decoder->Rate() / Mixer::instance()->obtained.freq) * (1 << FIXED_FRACTIONAL_BITS);

		return true;
		
	}
	else
	{
		return false;
	}
}

void Music::Rewind()
{
	
	if (decoder)
		decoder->Rewind();
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

	if (!decoder) return false;
	int32 bytes_read = decoder->Decode(&music_buffer.front(), MUSIC_BUFFER_SIZE);
	if (bytes_read)
	{
#ifdef __MACOS__
		macos_buffer_length = bytes_read;
		macos_read_more = false;
#else
		Mixer::instance()->UpdateMusicChannel(&music_buffer.front(), bytes_read);
#endif
		return true;
	}

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
