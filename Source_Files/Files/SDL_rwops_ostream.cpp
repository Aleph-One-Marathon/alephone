/*
 *  SDL_rwops_ostream.cpp - create an SDL_RWops structure from an ostream
 
	Copyright (C) 2015 and beyond by Jeremiah Morris
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
 
 */

#include "SDL_rwops_ostream.h"

static int stream_seek(struct SDL_RWops *context, int offset, int whence)
{
	std::ostream *strm = static_cast<std::ostream *>(context->hidden.unknown.data1);
	switch (whence)
	{
		case RW_SEEK_SET:
			strm->seekp(offset, std::ios::beg);
			break;
		case RW_SEEK_CUR:
			strm->seekp(offset, std::ios::cur);
			break;
		case RW_SEEK_END:
			strm->seekp(offset, std::ios::end);
			break;
	}
	return strm->tellp();
}

static int stream_read(struct SDL_RWops *context, void *ptr, int size, int maxnum)
{
	return -1;
}

static int stream_write(struct SDL_RWops *context, const void *ptr, int size, int num)
{
	std::ostream *strm = static_cast<std::ostream *>(context->hidden.unknown.data1);
	strm->write(static_cast<const char *>(ptr), size*num);
	if (strm->bad())
	{
		return -1;
	}
	else
	{
		return num;
	}
}

static int stream_close(struct SDL_RWops *context)
{
	if (context)
		SDL_FreeRW(context);
	return 0;
}

// the ostream must remain valid for RWops' lifetime
SDL_RWops *SDL_RWFromOStream(std::ostream& strm)
{
	SDL_RWops *ops = SDL_AllocRW();
	if (ops)
	{
		ops->seek = stream_seek;
		ops->read = stream_read;
		ops->write = stream_write;
		ops->close = stream_close;
		ops->hidden.unknown.data1 = &strm;
	}
	return ops;
}
