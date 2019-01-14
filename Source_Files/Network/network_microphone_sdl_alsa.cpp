/*
 *  network_microphone_sdl_alsa.cpp
 *  created for Marathon: Aleph One <http://source.bungie.org/>

	Copyright (C) 2007 and beyond by Gregory Smith
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

#include "cseries.h"
#ifdef HAVE_ALSA
#include <alsa/asoundlib.h>

#ifdef SPEEX
#include "network_speex.h"
#include "preferences.h"
#endif

#include "network_microphone_shared.h"

static snd_pcm_t *capture_handle = 0;
static snd_pcm_hw_params_t *hw_params;

static bool initialized = false;

static const int bytes_per_frame = 2; // 16-bit, mono
static snd_pcm_uframes_t frames = 0;  // period

OSErr
open_network_microphone() {
	int err;

	if ((err = snd_pcm_open(&capture_handle, "default", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		fprintf(stderr, "snd_pcm_open\n");
		return -1;
	}

	if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
		fprintf(stderr, "snd_pcm_hw_params_malloc\n");
		return -1;
	}

	if ((err = snd_pcm_hw_params_any(capture_handle, hw_params)) < 0) {
		fprintf(stderr, "snd_pcm_hw_params_any\n");
		return -1;
	}

	if ((err = snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf(stderr, "snd_pcm_hw_params_set_access\n");
		return -1;
	}

	snd_pcm_format_t format;
	// In C++17, we could use if constexpr to do the same thing as the original
	// ifdef preprocessor statement like so:
	// if constexpr(PlatformIsLittleEndian()) {
	// 	format = SND_PCM_FORMAT_S16_LE; 
	// } else {
	// 	format = SND_PCM_FORMAT_S16_BE;
	// }
	//
	// Until that point, the following will be equivalent but we are depending
	// on the optimizer to do the right thing.
	if (PlatformIsLittleEndian()) {
		format = SND_PCM_FORMAT_S16_LE ;
	} else {
		format = SND_PCM_FORMAT_S16_BE;
	}

	if ((err = snd_pcm_hw_params_set_format(capture_handle, hw_params, format)) < 0) {
		fprintf(stderr, "snd_pcm_hw_params_set_format\n");
		return -1;
	}

	unsigned int rate = 8000;
	if ((err = snd_pcm_hw_params_set_rate_near(capture_handle, hw_params, &rate, 0)) < 0) {
		fprintf(stderr, "snd_pcm_hw_params_set_rate_near\n");
		return -1;
	}

	if (!announce_microphone_capture_format(rate, false, true)) {
		fprintf(stderr, "network microphone support code rejected audio format (rate=%i)\n", rate);
		return -1;
	}

	if ((err = snd_pcm_hw_params_set_channels(capture_handle, hw_params, 1)) < 0) {
		fprintf(stderr, "snd_pcm_hw_params_set_channels\n");
		return -1;
	}

	frames = get_capture_byte_count_per_packet() / bytes_per_frame;
	if ((err = snd_pcm_hw_params_set_period_size_near(capture_handle, hw_params, &frames, 0)) < 0) {
		fprintf(stderr, "snd_pcm_hw_params_set_period_size_near\n");
		return -1;
	}

	if ((err = snd_pcm_hw_params(capture_handle, hw_params)) < 0) {
		fprintf(stderr, "snd_pcm_hw_params\n");
		return -1;
	}

	snd_pcm_hw_params_free(hw_params);

	snd_pcm_sw_params_t *sw_params;
	if ((err = snd_pcm_sw_params_malloc(&sw_params)) < 0) {
		fprintf(stderr, "snd_pcm_sw_params_malloc\n");
		return -1;
	}

	if ((err = snd_pcm_sw_params_current(capture_handle, sw_params)) < 0) {
		fprintf(stderr, "snc_pcm_sw_params_current\n");
		return -1;
	}

	if ((err = snd_pcm_sw_params_set_avail_min(capture_handle, sw_params, frames)) < 0)
	{
		fprintf(stderr, "snd_pcm_params_set_avail_min\n");
		return -1;
	}

	if ((err = snd_pcm_sw_params_set_start_threshold(capture_handle, sw_params, frames)) < 0) {
		fprintf(stderr, "snd_pcm_params_set_start_threshold\n");
		return -1;
	}

#ifdef SPEEX
	if (network_preferences->use_speex_encoder) {
		init_speex_encoder();
	}
#endif

	initialized = true;
	
	return 0;
}

void
close_network_microphone() {
	initialized = false;
	snd_pcm_close(capture_handle);
	capture_handle = 0;

#ifdef SPEEX
	if (network_preferences->use_speex_encoder) {
		destroy_speex_encoder();
	}
#endif
}

void CaptureCallback(snd_async_handler_t *)
{
	snd_pcm_sframes_t avail = snd_pcm_avail_update(capture_handle);
	while (avail >= frames) {
		static uint8 buffer[16384];
		int frames_read = snd_pcm_readi(capture_handle, buffer, frames);
		if (frames_read == -EPIPE) {
			snd_pcm_prepare(capture_handle);
		} else if (frames_read > 0) {
			copy_and_send_audio_data(buffer, frames_read * bytes_per_frame, NULL, 0, true);
		}

		avail = snd_pcm_avail_update(capture_handle);
	}
}

static bool mic_active = false;

static snd_async_handler_t *pcm_callback;

void
set_network_microphone_state(bool inActive) {
	if (!initialized) return;
	if (inActive && !mic_active) {
		// prepare the pcm
		if (snd_pcm_prepare(capture_handle) < 0) {
			fprintf(stderr, "preparing stream failed\n");
		}
		if (snd_async_add_pcm_handler(&pcm_callback, capture_handle, CaptureCallback, NULL) < 0) {
			fprintf(stderr, "adding pcm handler failed\n");
			return;
		} 
		if (snd_pcm_start(capture_handle) < 0) {
			fprintf(stderr, "starting pcm failed\n");
		}
		mic_active = true;
	} else if (!inActive && mic_active) {
		snd_async_del_handler(pcm_callback);
		snd_pcm_drop(capture_handle);
		mic_active = false;
	}

}

bool
is_network_microphone_implemented() {
	return true;
}

void
network_microphone_idle_proc() {
	// do nothing
}

#else
#include "network_microphone_sdl_dummy.cpp"
#endif
