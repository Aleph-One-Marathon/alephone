/*

	Copyright (C) 2007 Gregory Smith
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

	Handles sound files with libsndfile

*/

#include "SndfileDecoder.h"

#ifdef HAVE_SNDFILE

// these should really probably use the OpenedFile abstraction rather
// than SDL_RWops? but seek would be more complex and I'm so lazy

static sf_count_t sfd_get_filelen(void* pv)
{
	SDL_RWops* rwops = reinterpret_cast<SDL_RWops*>(pv);
	int pos = SDL_RWtell(rwops);
	SDL_RWseek(rwops, 0, SEEK_END);
	sf_count_t len = SDL_RWtell(rwops);
	SDL_RWseek(rwops, pos, SEEK_SET);
	return len;
}

static sf_count_t sfd_seek(sf_count_t offset, int whence, void *pv)
{
	return SDL_RWseek(reinterpret_cast<SDL_RWops*>(pv), offset, whence);
}

static sf_count_t sfd_read(void *ptr, sf_count_t count, void *pv)
{
	return SDL_RWread(reinterpret_cast<SDL_RWops*>(pv), ptr, 1, count);
}

static sf_count_t sfd_write(const void* ptr, sf_count_t count, void* pv)
{
	return SDL_RWwrite(reinterpret_cast<SDL_RWops*>(pv), ptr, 1, count);
}

static sf_count_t sfd_tell(void* pv)
{
	return SDL_RWtell(reinterpret_cast<SDL_RWops*>(pv));
}

static SF_VIRTUAL_IO sf_virtual = {
	&sfd_get_filelen,
	&sfd_seek,
	&sfd_read,
	&sfd_write,
	&sfd_tell
};

SndfileDecoder::SndfileDecoder() :
	sndfile(0), rwops(0)
{
}

SndfileDecoder::~SndfileDecoder()
{
	Close();
}

bool SndfileDecoder::Open(FileSpecifier& File)
{
	Close();

	sfinfo.format = 0;
	OpenedFile openedFile;
	if (File.Open(openedFile))
	{
		rwops = openedFile.TakeRWops();
		sndfile = sf_open_virtual(&sf_virtual, SFM_READ, &sfinfo, rwops);
	}

	return sndfile;
}

int32 SndfileDecoder::Decode(uint8* buffer, int32 max_length)
{
	if (!sndfile) return 0;

	return sf_read_short(sndfile, (int16*) buffer, max_length / 2) * 2;

}

void SndfileDecoder::Rewind()
{
	if (!sndfile) return;
	sf_seek(sndfile, 0, SEEK_SET);
}	

void SndfileDecoder::Close()
{
	if (sndfile)
	{
		sf_close(sndfile);
		sndfile = 0;
	}

	if (rwops)
	{
		SDL_RWclose(rwops);
		rwops = 0;
	}
}
#endif
