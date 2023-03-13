/*
 
 Copyright (C) 2012 and beyond by Jeremiah Morris
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
 
 Decodes sound files with libav/ffmpeg
 
 */

#if defined _MSC_VER
#define NOMINMAX
#include <algorithm>
#endif

// make FFmpeg happy
#define __STDC_CONSTANT_MACROS

#include "FFmpegDecoder.h"

#ifdef HAVE_FFMPEG
#ifdef __cplusplus
extern "C"
{
#endif
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavutil/fifo.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "SDL_ffmpeg.h"
 

#ifdef __cplusplus
}
#endif

struct ffmpeg_vars {
    SDL_ffmpegFile* file;
    SDL_ffmpegAudioFrame* frame;
    AVFifoBuffer *fifo;
    bool started;
};
typedef struct ffmpeg_vars ffmpeg_vars_t;

FFmpegDecoder::FFmpegDecoder() :
	av(NULL)
{
    av = new ffmpeg_vars_t;
    memset(av, 0, sizeof(ffmpeg_vars_t));
    
    av->fifo = av_fifo_alloc(524288);
}

FFmpegDecoder::~FFmpegDecoder()
{
	Close();
    if (av && av->fifo)
        av_fifo_free(av->fifo);
}

bool FFmpegDecoder::Open(FileSpecifier& File)
{
	Close();
    
    // make sure one-time setup succeeded
    if (!av || !av->fifo)
        return false;

    av->file = SDL_ffmpegOpen(File.GetPath());
    if (!av->file || !av->file->as)
    {
        Close();
        return false;
    }
    if (SDL_ffmpegSelectAudioStream(av->file, av->file->as->id) == -1)
    {
        Close();
        return false;
    }
    if ((av->frame = SDL_ffmpegCreateAudioFrame(av->file, 8192)) == 0)
    {
        Close();
        return false;
    }

    channels = av->file->audioStream->_ffmpeg->codecpar->channels;
    rate = av->file->audioStream->_ffmpeg->codecpar->sample_rate;
    return true;
}

int32 FFmpegDecoder::Decode(uint8* buffer, int32 max_length)
{
	int32 total_bytes_read = 0;
    uint8* cur = buffer;
	while (total_bytes_read < max_length)
	{
        int32 fifo_size = av_fifo_size(av->fifo);
        if (!fifo_size)
        {
            if (!GetAudio())
                break;
            fifo_size = av_fifo_size(av->fifo);
        }
        int bytes_read = std::min(fifo_size, max_length - total_bytes_read);
        av_fifo_generic_read(av->fifo, cur, bytes_read, NULL);
        total_bytes_read += bytes_read;
        cur += bytes_read;
    }
    
	memset(&buffer[total_bytes_read], 0, max_length - total_bytes_read);
	return total_bytes_read;
}

void FFmpegDecoder::Rewind()
{
    if (av->started)
    {
        SDL_ffmpegSeekRelative(av->file, 0);
        av_fifo_reset(av->fifo);
        av->started = false;
    }
}

void FFmpegDecoder::Close()
{
    if (av && av->file)
        SDL_ffmpegFree(av->file);
    if (av && av->frame)
        SDL_ffmpegFreeAudioFrame(av->frame);
    if (av && av->fifo)
        av_fifo_reset(av->fifo);
    if (av)
        av->started = false;
}

bool FFmpegDecoder::GetAudio()
{
    if (!SDL_ffmpegGetAudioFrame(av->file, av->frame)) return false;
    av_fifo_generic_write(av->fifo, av->frame->buffer, av->frame->size, NULL);
    av->started = true;
    return true;
}

#endif
