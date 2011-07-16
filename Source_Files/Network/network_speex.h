/*
 *  network_speex.h
 *  created for Marathon: Aleph One <http://source.bungie.org/>

	Copyright (C) 2003 and beyond by Gregory Smith
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

    headers to store the speex encoder/decoder
    
 */

#ifndef __NETWORK_SPEEX_H
#define __NETWORK_SPEEX_H

#include "cseries.h"
#ifdef SPEEX
#include <speex/speex.h>
#include <speex/speex_preprocess.h>

// encoder
extern void *gEncoderState;
extern SpeexBits gEncoderBits;
extern void *gDecoderState;
extern SpeexBits gDecoderBits;
extern SpeexPreprocessState* gPreprocessState;

void init_speex_encoder();
void destroy_speex_encoder();
void init_speex_decoder();
void destroy_speex_decoder();
#endif //def SPEEX

#endif
