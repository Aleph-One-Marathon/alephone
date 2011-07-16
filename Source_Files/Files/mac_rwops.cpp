/*

	Copyright (C) 2006 and beyond by Gregory Smith
 
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

*/

/* mac_rwops.cpp: more efficient RWops for reading from classic Mac forks */

#include "cseries.h"

#undef fnfErr
#include <Files.h>
#include <Resources.h>

static int SDLCALL rw_seek(SDL_RWops *context, int offset, int whence)
{
	short macWhence;
	int32 file_pos;

	if (!context) return -1;

	switch (whence) {
	case RW_SEEK_SET:
		macWhence = fsFromStart; break;
	case RW_SEEK_CUR:
		macWhence = fsFromMark; break;
	case RW_SEEK_END:
		macWhence = fsFromLEOF; break;
	default:
		return -1;
	}

	if (SetFPos((short) context->hidden.unknown.data1, macWhence, offset) != noErr)
		return -1;
	
	if (GetFPos((short) context->hidden.unknown.data1, &file_pos) != noErr)
		return -1;

	return file_pos;
}

static int SDLCALL rw_read(SDL_RWops *context, void *ptr, int size, int maxnum)
{
	int32 total_bytes = size * maxnum;
	
	if (!context) return 0;

	if (FSRead((short) context->hidden.unknown.data1, &total_bytes, ptr) != noErr)
		return 0;
	return (total_bytes / size);
	
}
static int SDLCALL rw_write(SDL_RWops *context, const void *ptr, int size, int num)
{
	assert(true);
}

static int SDLCALL rw_close(SDL_RWops *context)
{
	if (context) {
		FSClose((short) context->hidden.unknown.data1);

		SDL_FreeRW(context);
	}

	return 0;
}

SDL_RWops *
open_fork_from_existing_path(const char *inPath, bool resFork)
{

	// create a file spec for the path
	FSSpec fss;
	char buf[256];
	strcpy(&buf[1], inPath);
	buf[0] = strlen(&buf[1]);
	
	if (FSMakeFSSpec(0, 0, (unsigned char *) buf, &fss) != noErr)
		return NULL;

	short RefNum;
	if ((resFork) ? FSpOpenRF(&fss, fsRdPerm, &RefNum) : FSpOpenDF(&fss, fsRdPerm, &RefNum) != noErr)
		return NULL;

	// make an SDL_RWops from it!
	SDL_RWops* rwops = SDL_AllocRW();
	rwops->hidden.unknown.data1 = (void *) RefNum;
	rwops->seek = rw_seek;
	rwops->read = rw_read;
	rwops->write = rw_write;
	rwops->close = rw_close;

	return rwops;
}
