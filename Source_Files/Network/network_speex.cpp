/*
 *  network_speex.cpp
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

    speex encoder / decoder functions
    
 */

#if !defined(DISABLE_NETWORKING)

#include "cseries.h"
#ifdef SPEEX
#include "network_speex.h"
#include "network_audio_shared.h"
#include "preferences.h"

#include <speex/speex_preprocess.h>

void *gEncoderState = 0;
SpeexBits gEncoderBits;
SpeexPreprocessState *gPreprocessState = 0;
void *gDecoderState = 0;
SpeexBits gDecoderBits;

void init_speex_encoder() {
    if (!gEncoderState) {
        gEncoderState = speex_encoder_init(&speex_nb_mode);
	int quality = 3; // 8000 bps
        speex_encoder_ctl(gEncoderState, SPEEX_SET_QUALITY, &quality);
	int complexity = 4; // net play is demanding, ok?
        speex_encoder_ctl(gEncoderState, SPEEX_SET_COMPLEXITY, &complexity);
        int tmp = kNetworkAudioSampleRate;
        speex_encoder_ctl(gEncoderState, SPEEX_SET_SAMPLING_RATE, &tmp);
        speex_bits_init(&gEncoderBits);

	// set up the preprocessor
	int frame_size;
	speex_encoder_ctl(gEncoderState, SPEEX_GET_FRAME_SIZE, &frame_size);
	gPreprocessState = speex_preprocess_state_init(frame_size, kNetworkAudioSampleRate);
	// turn on AGC and denoise
	tmp = 1;
	speex_preprocess_ctl(gPreprocessState, SPEEX_PREPROCESS_SET_DENOISE, &tmp);
	tmp = 1;
	speex_preprocess_ctl(gPreprocessState, SPEEX_PREPROCESS_SET_AGC, &tmp);
	
	float agc_level = 32768.0 * 0.7;
	speex_preprocess_ctl(gPreprocessState, SPEEX_PREPROCESS_SET_AGC_LEVEL, &agc_level);
    }
    
}

void destroy_speex_encoder() {
    if (gEncoderState != NULL) {
        speex_encoder_destroy(gEncoderState);
        speex_bits_destroy(&gEncoderBits);
        gEncoderState = NULL;
    }

    if (gPreprocessState)
    {
	    speex_preprocess_state_destroy(gPreprocessState);
	    gPreprocessState = 0;
    }
}

void init_speex_decoder() {
    if (gDecoderState == NULL) {
        gDecoderState = speex_decoder_init(&speex_nb_mode);
        int tmp = 1;
        speex_decoder_ctl(gDecoderState, SPEEX_SET_ENH, &tmp);
        tmp = kNetworkAudioSampleRate;
        speex_decoder_ctl(gDecoderState, SPEEX_SET_SAMPLING_RATE, &tmp);
        speex_bits_init(&gDecoderBits);
    }
}

void destroy_speex_decoder() {
    if (gDecoderState != NULL) {
        speex_decoder_destroy(gDecoderState);
        speex_bits_destroy(&gDecoderBits);
        gDecoderState = NULL;
    }
}

#endif //def SPEEX

#endif // !defined(DISABLE_NETWORKING)

