/*
 *  SDL_rwops_ostream.h - create an SDL_RWops structure from an ostream
 
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

#ifndef _SDL_rwops_ostream_h
#define _SDL_rwops_ostream_h

#include <sstream>
#include <SDL_rwops.h>

SDL_RWops *SDL_RWFromOStream(std::ostream& strm);

#endif
