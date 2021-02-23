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

    
// Use audio conversion from Movie.cpp
extern int convert_audio(int in_samples, int in_channels, int in_stride,
                         enum AVSampleFormat in_fmt, const uint8_t *in_buf,
                         int out_samples, int out_channels, int out_stride,
                         enum AVSampleFormat out_fmt, uint8_t *out_buf);
    

#ifdef __cplusplus
}
#endif

struct ffmpeg_vars {
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
    
    av_register_all();
    avcodec_register_all();
    av->temp_data = reinterpret_cast<uint8_t *>(av_malloc(262144));
    av->fifo = av_fifo_alloc(524288);
}

FFmpegDecoder::~FFmpegDecoder()
{
	Close();
    if (av && av->temp_data)
        av_free(av->temp_data);
    if (av && av->fifo)
        av_fifo_free(av->fifo);
}

bool FFmpegDecoder::Open(FileSpecifier& File)
{
	Close();
    
    // make sure one-time setup succeeded
    if (!av || !av->temp_data || !av->fifo)
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
    if (avcodec_open2(av->stream->codec, codec, NULL) < 0)
    {
        Close();
        return false;
    }
    channels = av->stream->codec->channels;
    rate = av->stream->codec->sample_rate;
	
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
    if (av && av->stream)
    {
        avcodec_close(av->stream->codec);
        av->stream = NULL;
    }
    if (av && av->ctx)
    {
        avio_close(av->ctx->pb);
        avformat_free_context(av->ctx);
        av->ctx = NULL;
    }
    if (av && av->fifo)
        av_fifo_reset(av->fifo);
    if (av)
        av->started = false;
}

bool FFmpegDecoder::GetAudio()
{
    AVPacket pkt;
    av_init_packet(&pkt);
    
    while (true)
    {
        int decode = av_read_frame(av->ctx, &pkt);
        if (decode < 0)
            return false;
        if (pkt.stream_index == av->stream_idx)
            break;
        av_free_packet(&pkt);
    }
    
    av->started = true;
    AVPacket pkt_temp;
    av_init_packet(&pkt_temp);
    pkt_temp.data = pkt.data;
    pkt_temp.size = pkt.size;
    
    AVCodecContext *dec_ctx = av->stream->codec;
    
    while (pkt_temp.size > 0)
    {
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,0)
        AVFrame *dframe = avcodec_alloc_frame();
#else
        AVFrame *dframe = av_frame_alloc();
#endif
        int got_frame = 0;
        int bytes_read = avcodec_decode_audio4(dec_ctx, dframe, &got_frame, &pkt_temp);
        if (bytes_read < 0)
        {
            av_free_packet(&pkt);
            return false;
        }
        if (got_frame && bytes_read > 0)
        {
            int channels = dec_ctx->channels;
            enum AVSampleFormat in_fmt = dec_ctx->sample_fmt;
            enum AVSampleFormat out_fmt = AV_SAMPLE_FMT_S16;
            
            int stride = -1;
            if (channels > 1 && av_sample_fmt_is_planar(in_fmt))
                stride = dframe->extended_data[1] - dframe->extended_data[0];

            int written = convert_audio(dframe->nb_samples, channels,
                                        stride,
                                        in_fmt, dframe->extended_data[0],
                                        dframe->nb_samples, channels,
                                        -1,
                                        out_fmt, av->temp_data);
            
            av_fifo_generic_write(av->fifo, av->temp_data, written, NULL);

            pkt_temp.data += bytes_read;
            pkt_temp.size -= bytes_read;
        }
        
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,0)
        av_freep(&dframe);
#else
        av_frame_free(&dframe);
#endif
    }
    
    av_free_packet(&pkt);
    return true;
}

#endif
