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

#define AV_FIFO_BUFFER_SIZE (1<<19)

struct ffmpeg_vars {
    SDL_ffmpegFile* file;
    SDL_ffmpegAudioFrame* frame;
#if USE_NEW_AV_FIFO_API
    AVFifo *fifo;
#else
    AVFifoBuffer *fifo;
#endif
    bool started;
};
typedef struct ffmpeg_vars ffmpeg_vars_t;

FFmpegDecoder::FFmpegDecoder() :
	av(NULL)
{
    av = new ffmpeg_vars_t;
    memset(av, 0, sizeof(ffmpeg_vars_t));
    
#if USE_NEW_AV_FIFO_API
    av->fifo = av_fifo_alloc2(AV_FIFO_BUFFER_SIZE / AV_FIFO_CHUNK_SIZE, AV_FIFO_CHUNK_SIZE, 0);
#else
    av->fifo = av_fifo_alloc(AV_FIFO_BUFFER_SIZE);
#endif
}

FFmpegDecoder::~FFmpegDecoder()
{
	Close();
    if (av && av->fifo)
#if USE_NEW_AV_FIFO_API
        av_fifo_freep2(&av->fifo);
#else
        av_fifo_free(av->fifo);
#endif
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

#if USE_NEW_AV_FIFO_API
    channels = av->file->audioStream->_ffmpeg->codecpar->ch_layout.nb_channels;
#else
    channels = av->file->audioStream->_ffmpeg->codecpar->channels;
#endif
    rate = av->file->audioStream->_ffmpeg->codecpar->sample_rate;
    return true;
}

int32 FFmpegDecoder::Decode(uint8* buffer, int32 max_length)
{
	size_t total_bytes_read = 0;

	while (total_bytes_read < max_length)
	{
#if USE_NEW_AV_FIFO_API
        size_t fifo_chunks_waiting = av_fifo_can_read(av->fifo);
        if (!fifo_chunks_waiting)
        {
            if (!GetAudio())
                break;
            fifo_chunks_waiting = av_fifo_can_read(av->fifo);
        }
        size_t chunks_to_read = std::min(fifo_chunks_waiting * AV_FIFO_CHUNK_SIZE, (max_length - total_bytes_read + AV_FIFO_CHUNK_SIZE - 1) / AV_FIFO_CHUNK_SIZE);
        if (!chunks_to_read || av_fifo_read(av->fifo, buffer + total_bytes_read, chunks_to_read) < 0)
            break;
        total_bytes_read += chunks_to_read * AV_FIFO_CHUNK_SIZE;
#else
        int32 fifo_size = av_fifo_size(av->fifo);
        if (!fifo_size)
        {
            if (!GetAudio())
                break;
            fifo_size = av_fifo_size(av->fifo);
		}
        int bytes_read = std::min(fifo_size, max_length - (int) total_bytes_read);
        av_fifo_generic_read(av->fifo, buffer + total_bytes_read, bytes_read, NULL);
        total_bytes_read += bytes_read;
#endif
    }

	memset(&buffer[total_bytes_read], 0, max_length - total_bytes_read);
	return total_bytes_read;
}

void FFmpegDecoder::Rewind()
{
    if (av->started)
    {
        SDL_ffmpegSeekRelative(av->file, 0);
#if USE_NEW_AV_FIFO_API
        av_fifo_reset2(av->fifo);
#else
        av_fifo_reset(av->fifo);
#endif
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
#if USE_NEW_AV_FIFO_API
        av_fifo_reset2(av->fifo);
#else
        av_fifo_reset(av->fifo);
#endif
    if (av)
        av->started = false;
}

bool FFmpegDecoder::GetAudio()
{
    if (!SDL_ffmpegGetAudioFrame(av->file, av->frame))
        return false;
#if USE_NEW_AV_FIFO_API
    for (size_t bytes_written = 0; bytes_written < av->frame->size; )
    {
        size_t free_chunks_in_fifo = av_fifo_can_write(av->fifo);
        size_t chunks_to_write = std::min((av->frame->size - bytes_written + AV_FIFO_CHUNK_SIZE - 1) / AV_FIFO_CHUNK_SIZE, free_chunks_in_fifo);
        if (!free_chunks_in_fifo || av_fifo_write(av->fifo, av->frame->buffer + bytes_written, chunks_to_write) < 0)
            break;
        bytes_written += chunks_to_write * AV_FIFO_CHUNK_SIZE; 
    }
#else
    av_fifo_generic_write(av->fifo, av->frame->buffer, av->frame->size, NULL);
#endif

    av->started = true;
    return true;
}

#endif
