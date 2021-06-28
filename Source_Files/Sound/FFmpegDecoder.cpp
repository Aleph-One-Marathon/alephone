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
 

#ifdef __cplusplus
}
#endif

struct ffmpeg_vars {
    AVCodecContext* codec_ctx;
    SwrContext* swr_context;
    AVFormatContext *ctx;
    AVStream *stream;
    uint8_t *temp_data;
    AVFifoBuffer *fifo;
    int stream_idx;
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
    
    // open the file
    if (avformat_open_input(&av->ctx, File.GetPath(), NULL, NULL) != 0)
        return false;
    
    // retrieve format info
    if (avformat_find_stream_info(av->ctx, NULL) < 0)
    {
        Close();
        return false;
    }
    
    // find the audio
    AVCodec *codec;
    av->stream_idx = av_find_best_stream(av->ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
    if (av->stream_idx < 0)
    {
        Close();
        return false;
    }
    av->stream = av->ctx->streams[av->stream_idx];
    av->codec_ctx = avcodec_alloc_context3(codec);
    if (!av->codec_ctx || avcodec_parameters_to_context(av->codec_ctx, av->stream->codecpar) < 0 || avcodec_open2(av->codec_ctx, NULL, NULL) < 0)
    {
        Close();
        return false;
    }
    channels = av->stream->codecpar->channels;
    rate = av->stream->codecpar->sample_rate;

    // init resampler
    av->swr_context = swr_alloc_set_opts(av->swr_context, av->codec_ctx->channel_layout, AV_SAMPLE_FMT_S16, rate,
        av->codec_ctx->channel_layout, av->codec_ctx->sample_fmt, rate, 0, NULL);

    if (!av->swr_context || swr_init(av->swr_context) < 0)
    {
        Close();
        return false;
    }

    if (av_samples_alloc(&av->temp_data, NULL, channels, 1024, AV_SAMPLE_FMT_S16, 0) < 0)
    {
        Close();
        return false;
    }

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
        av_seek_frame(av->ctx, -1, 0, AVSEEK_FLAG_BACKWARD);
        av_fifo_reset(av->fifo);
        av->started = false;
    }
}

void FFmpegDecoder::Close()
{
    if (av && av->codec_ctx)
    {
        avcodec_free_context(&av->codec_ctx);
    }
    if (av && av->ctx)
    {
        avio_close(av->ctx->pb);
        avformat_free_context(av->ctx);
        av->ctx = NULL;
    }
    if (av && av->fifo)
        av_fifo_reset(av->fifo);
    if (av && av->swr_context) 
        swr_free(&av->swr_context);
    if (av && av->temp_data)
        av_freep(&av->temp_data);
    if (av)
        av->started = false;
}

bool FFmpegDecoder::GetAudio()
{
    AVPacket* pkt = av_packet_alloc();
    
    while (true)
    {
        int decode = av_read_frame(av->ctx, pkt);
        if (decode < 0)
            return false;
        if (pkt->stream_index == av->stream_idx)
            break;
        av_packet_unref(pkt);
    }
    
    av->started = true;
    AVCodecContext *dec_ctx = av->codec_ctx;

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,0)
    AVFrame *dframe = avcodec_alloc_frame();
#else
    AVFrame *dframe = av_frame_alloc();
#endif

    int sent_packet = avcodec_send_packet(dec_ctx, pkt);
    if (sent_packet < 0)
    {
        av_packet_unref(pkt);
        return false;
    }
    while (avcodec_receive_frame(dec_ctx, dframe) == 0)
    {
        int nb_samples = swr_convert(av->swr_context, &av->temp_data, dframe->nb_samples, (const uint8_t**)dframe->extended_data, dframe->nb_samples);
        int nb_bytes = nb_samples * channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
        av_fifo_generic_write(av->fifo, av->temp_data, nb_bytes, NULL);
    }
        
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,0)
    av_freep(&dframe);
#else
    av_frame_free(&dframe);
#endif
    
    av_packet_free(&pkt);
    return true;
}

#endif
