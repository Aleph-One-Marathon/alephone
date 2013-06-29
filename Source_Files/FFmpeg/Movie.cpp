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
  
  Movie export using libav/ffmpeg
  Thanks to https://sourceforge.net/p/libavexample/ by Arash Shafiei
 
 */


#include "cseries.h"

#ifdef HAVE_OPENGL
#include "OGL_Headers.h"
#endif

#include "Movie.h"
#include "interface.h"
#include "screen.h"
#include "Mixer.h"

Movie* Movie::m_instance = NULL;

#ifndef HAVE_FFMPEG

struct libav_vars {
    bool inited;
};

void Movie::PromptForRecording() {}
void Movie::StartRecording(std::string path) {}
bool Movie::IsRecording() { return false; }
void Movie::StopRecording() {}
void Movie::AddFrame(FrameType ftype) {}

bool Movie::Setup() { return false; }
int Movie::Movie_EncodeThread(void *arg) { return 0; }
void Movie::EncodeThread() {}
void Movie::EncodeVideo(bool last) {}
void Movie::EncodeAudio(bool last) {}
Movie::Movie() {}

#else

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
#ifdef __cplusplus
}
#endif

#include "SDL_ffmpeg.h"

struct libav_vars {
    bool inited;
    
    AVFrame *audio_frame;
    AVFifoBuffer *audio_fifo;
    uint8_t *audio_data;
    
    AVFrame *video_frame;
    uint8_t *video_buf;
    int video_bufsize;
    uint8_t *video_data;
    
    struct SwsContext *sws_ctx;
    AVFormatContext *fmt_ctx;
    int video_stream_idx;
    int audio_stream_idx;
};
typedef struct libav_vars libav_vars_t;

Movie::Movie() :
  moviefile(""),
  temp_surface(NULL),
  av(NULL),
  encodeThread(NULL),
  encodeReady(NULL),
  fillReady(NULL),
  stillEncoding(0)
{
    av = new libav_vars_t;
    memset(av, 0, sizeof(libav_vars_t));
}

void Movie::PromptForRecording()
{
	FileSpecifier dst_file;
	if (!dst_file.WriteDialog(_typecode_movie, "EXPORT FILM", "Untitled Movie.mp4"))
		return;
	StartRecording(dst_file.GetPath());
}

void Movie::StartRecording(std::string path)
{
	StopRecording();
	moviefile = path;
	SDL_PauseAudio(IsRecording());
}

bool Movie::IsRecording()
{
  return (moviefile.length() > 0);
}

bool Movie::Setup()
{
    if (!IsRecording())
        return false;
    if (!av)
        return false;

    bool success = true;
    
	alephone::Screen *scr = alephone::Screen::instance();
	view_rect = scr->window_rect();
	
	if (SDL_GetVideoSurface()->flags & SDL_OPENGL)
		view_rect.y = scr->height() - (view_rect.y + view_rect.h);

	temp_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, view_rect.w, view_rect.h, 32,
#if ALEPHONE_LITTLEENDIAN
										0x00ff0000, 0x0000ff00, 0x000000ff,
#else
										0x000000ff, 0x0000ff00, 0x00ff0000,
#endif
										0);
	success = (temp_surface != NULL);

    Mixer *mx = Mixer::instance();
    
    av_register_all();
    avcodec_register_all();
    
    // Open output file
    AVOutputFormat *fmt;
    if (success)
    {
        fmt = av_guess_format(NULL, moviefile.c_str(), NULL);
        success = fmt;
    }
    if (success)
    {
        av->fmt_ctx = avformat_alloc_context();
        success = av->fmt_ctx;
    }
    if (success)
    {
        av->fmt_ctx->oformat = fmt;
        strcpy(av->fmt_ctx->filename, moviefile.c_str());
        success = (0 <= avio_open(&av->fmt_ctx->pb, av->fmt_ctx->filename, AVIO_FLAG_WRITE));
    }
    
    // Open output video stream
    AVCodec *video_codec;
    AVStream *video_stream;
    if (success)
    {
        video_codec = avcodec_find_encoder(CODEC_ID_H264);
        success = video_codec;
    }
    if (success)
    {
        video_stream = avformat_new_stream(av->fmt_ctx, video_codec);
        success = video_stream;
    }
    if (success)
    {
        video_stream->codec->codec_id = video_codec->id;
        video_stream->codec->codec_type = AVMEDIA_TYPE_VIDEO;
        video_stream->codec->width = view_rect.w;
        video_stream->codec->height = view_rect.h;
        video_stream->codec->time_base = (AVRational){1, TICKS_PER_SECOND};
        video_stream->codec->pix_fmt = PIX_FMT_YUV420P;
        video_stream->codec->flags |= CODEC_FLAG_CLOSED_GOP;
        
        if (av->fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            video_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
        
        av->video_stream_idx = video_stream->index;
        
        // tuning options
        video_stream->codec->max_b_frames = 2;
        video_stream->codec->b_frame_strategy = 2;
        video_stream->codec->gop_size = TICKS_PER_SECOND/2;
        av_opt_set(video_stream->codec->priv_data, "preset", "slow", 0);
        av_opt_set(video_stream->codec->priv_data, "crf", "23", 0);
        
        success = (0 <= avcodec_open2(video_stream->codec, video_codec, NULL));
    }
    if (success)
    {
        av->video_bufsize = view_rect.w * view_rect.h * 4 + 10000;
        av->video_buf = static_cast<uint8_t *>(av_malloc(av->video_bufsize));
        success = av->video_buf;
    }
    if (success)
    {
        av->video_frame = avcodec_alloc_frame();
        success = av->video_frame;
    }
    if (success)
    {
        int numbytes = avpicture_get_size(video_stream->codec->pix_fmt, view_rect.w, view_rect.h);
        av->video_data = static_cast<uint8_t *>(av_malloc(numbytes));
        success = av->video_data;
    }
    if (success)
    {
        avpicture_fill(reinterpret_cast<AVPicture *>(av->video_frame), av->video_data, video_stream->codec->pix_fmt, view_rect.w, view_rect.h);
    }
    
    // Open output audio stream
    AVCodec *audio_codec;
    AVStream *audio_stream;
    if (success)
    {
        audio_codec = avcodec_find_encoder(CODEC_ID_AAC);
        success = audio_codec;
    }
    if (success)
    {
        audio_stream = avformat_new_stream(av->fmt_ctx, audio_codec);
        success = audio_stream;
    }
    if (success)
    {
        audio_stream->codec->codec_id = audio_codec->id;
        audio_stream->codec->codec_type = AVMEDIA_TYPE_AUDIO;
        audio_stream->codec->sample_rate = mx->obtained.freq;
        audio_stream->codec->channels = 2;
        audio_stream->codec->sample_fmt = AV_SAMPLE_FMT_S16;
        
        if (av->fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            audio_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
        
        av->audio_stream_idx = audio_stream->index;
        
        // tuning options
        // audio_stream->codec->bit_rate = 131072;
        
        success = (0 <= avcodec_open2(audio_stream->codec, audio_codec, NULL));
        if (!success)
        {
            // FFmpeg wants different options than libav
            audio_stream->codec->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
            audio_stream->codec->sample_fmt = AV_SAMPLE_FMT_FLT;
            success = (0 <= avcodec_open2(audio_stream->codec, audio_codec, NULL));
        }
    }
    if (success)
    {
        av->audio_frame = avcodec_alloc_frame();
        success = av->audio_frame;
    }
    if (success)
    {
        av->audio_fifo = av_fifo_alloc(262144);
    }
    if (success)
    {
        av->audio_data = reinterpret_cast<uint8_t *>(av_malloc(524288));
        success = av->audio_data;
    }
    
    // initialize conversion context
    if (success)
    {
        PixelFormat pix = PIX_FMT_NONE;
        switch (temp_surface->format->BitsPerPixel)
        {
            case 24:
                pix = PIX_FMT_RGB24;
                break;
            case 32:
                pix = PIX_FMT_BGR32;
                break;
            default:
                break;
        }
        if (pix != PIX_FMT_NONE)
        {
            av->sws_ctx = sws_getContext(temp_surface->w, temp_surface->h, pix,
                                         video_stream->codec->width,
                                         video_stream->codec->height,
                                         video_stream->codec->pix_fmt,
                                         SWS_BILINEAR,
                                         NULL, NULL, NULL);
        }
        success = av->sws_ctx;
    }
    
    // Start movie file
    if (success)
    {
        avformat_write_header(av->fmt_ctx, NULL);
    }
    
    // set up our threads and intermediate storage
    if (success)
    {
        videobuf.resize(av->video_bufsize);
        audiobuf.resize(2 * 2 * mx->obtained.freq / 30);
	}
	if (success)
	{
		encodeReady = SDL_CreateSemaphore(0);
		fillReady = SDL_CreateSemaphore(1);
		stillEncoding = true;
		success = encodeReady && fillReady;
	}
	if (success)
	{
		encodeThread = SDL_CreateThread(Movie_EncodeThread, this);
		success = encodeThread;
	}
	
	if (!success)
	{
	  StopRecording();
	  alert_user("Your movie could not be exported.");
	}
    av->inited = success;
	return success;
}

int Movie::Movie_EncodeThread(void *arg)
{
	reinterpret_cast<Movie *>(arg)->EncodeThread();
	return 0;
}

void Movie::EncodeVideo(bool last)
{
    // convert video
    AVStream *vstream = av->fmt_ctx->streams[av->video_stream_idx];
    AVCodecContext *vcodec = vstream->codec;
    
    AVFrame *frame = NULL;
    if (!last)
    {
        int pitch[] = { temp_surface->pitch, 0 };
        const uint8_t *const pdata[] = { reinterpret_cast<uint8_t *>(temp_surface->pixels), NULL };
    
        sws_scale(av->sws_ctx, pdata, pitch, 0, temp_surface->h,
                  av->video_frame->data, av->video_frame->linesize);
        av->video_frame->pts = vcodec->frame_number;
        frame = av->video_frame;
    }
    
    bool done = false;
    while (!done)
    {
        // add video
        int vsize = avcodec_encode_video(vcodec,
                                         av->video_buf, av->video_bufsize,
                                         frame);
        if (vsize > 0)
        {
            AVPacket pkt;
            av_init_packet(&pkt);
            if (vcodec->coded_frame->pts != AV_NOPTS_VALUE)
            {
                pkt.pts = av_rescale_q(vcodec->coded_frame->pts,
                                       vcodec->time_base,
                                       vstream->time_base);
            }
            if (vcodec->coded_frame->key_frame)
                pkt.flags |= AV_PKT_FLAG_KEY;
            pkt.stream_index = vstream->index;
            pkt.data = av->video_buf;
            pkt.size = vsize;
            av_interleaved_write_frame(av->fmt_ctx, &pkt);
            av_free_packet(&pkt);
        }
        
        if (!last || vsize <= 0)
            done = true;
    }
}

void Movie::EncodeAudio(bool last)
{
    AVStream *astream = av->fmt_ctx->streams[av->audio_stream_idx];
    AVCodecContext *acodec = astream->codec;
    
    
    av_fifo_generic_write(av->audio_fifo, &audiobuf[0], audiobuf.size(), NULL);
    
    // bps: bytes per sample
    int out_bps = acodec->channels * av_get_bytes_per_sample(acodec->sample_fmt);
    int in_bps = acodec->channels * 2;
    
    int max_read = acodec->frame_size * in_bps;
    int min_read = last ? in_bps : max_read;
    while (av_fifo_size(av->audio_fifo) >= min_read)
    {
        int read_bytes = MIN(av_fifo_size(av->audio_fifo), max_read);
        av_fifo_generic_read(av->audio_fifo, av->audio_data, read_bytes, NULL);
        
        int write_bytes = read_bytes;
        if (acodec->sample_fmt == AV_SAMPLE_FMT_FLT)
        {
            // rewrite samples in place; we allocated enough space to do this
            float *out_data = reinterpret_cast<float *>(av->audio_data);
            int16 *in_data = reinterpret_cast<int16 *>(av->audio_data);
            for (int j = (read_bytes/2) - 1; j >= 0; j--)
                out_data[j] = in_data[j] / 32768.0f;

            write_bytes = read_bytes * out_bps / in_bps;
        }
        
        avcodec_get_frame_defaults(av->audio_frame);
        if (read_bytes < max_read)
        {
            // shrink or pad audio frame
            if (acodec->codec->capabilities & CODEC_CAP_SMALL_LAST_FRAME)
                acodec->frame_size = write_bytes / out_bps;
            else
            {
                int frame_bytes = acodec->frame_size * out_bps;
                memset(&av->audio_data[write_bytes], 0, frame_bytes - write_bytes);
                write_bytes = frame_bytes;
            }
        }
        av->audio_frame->nb_samples = write_bytes / out_bps;
        int asize = avcodec_fill_audio_frame(av->audio_frame, acodec->channels,
                                             acodec->sample_fmt,
                                             av->audio_data, write_bytes, 1);
        if (asize >= 0)
        {
            AVPacket pkt;
            memset(&pkt, 0, sizeof(AVPacket));
            av_init_packet(&pkt);
            
            int got_pkt = 0;
            if (0 == avcodec_encode_audio2(acodec, &pkt, av->audio_frame, &got_pkt)
                && got_pkt)
            {
                pkt.stream_index = astream->index;
                pkt.flags |= AV_PKT_FLAG_KEY;
                av_interleaved_write_frame(av->fmt_ctx, &pkt);
                av_free_packet(&pkt);
            }
        }
    }
    
}

void Movie::EncodeThread()
{
	while (true)
	{
		SDL_SemWait(encodeReady);
		if (!stillEncoding)
		{
			// signal to quit
			SDL_SemPost(fillReady);
			return;
		}
        
        // add video and audio
        EncodeVideo(false);
        EncodeAudio(false);
		
		SDL_SemPost(fillReady);
	}
}

void Movie::AddFrame(FrameType ftype)
{
	if (!IsRecording())
		return;
	if (!av->inited)
	{
	  if (ftype == FRAME_FADE)
	    return;
	  if (!Setup())
	    return;
	}
	
	if (ftype == FRAME_FADE && get_keyboard_controller_status())
		return;
	
	SDL_SemWait(fillReady);
  	
	SDL_Surface *video = SDL_GetVideoSurface();
	if (!(video->flags & SDL_OPENGL))
	{
		SDL_BlitSurface(video, &view_rect, temp_surface, NULL);
	}
#ifdef HAVE_OPENGL
	else
	{
		// Read OpenGL frame buffer
		glReadPixels(view_rect.x, view_rect.y, view_rect.w, view_rect.h, GL_RGBA, GL_UNSIGNED_BYTE, &videobuf.front());
		
		// Copy pixel buffer (which is upside-down) to surface
		for (int y = 0; y < view_rect.h; y++)
			memcpy((uint8 *)temp_surface->pixels + temp_surface->pitch * y, &videobuf.front() + view_rect.w * 4 * (view_rect.h - y - 1), view_rect.w * 4);
	}
#endif
	
	int audio_bytes_per_frame = audiobuf.size();
	Mixer *mx = Mixer::instance();
	int old_vol = mx->main_volume;
	mx->main_volume = 0x100;
	mx->Mix(&audiobuf.front(), audio_bytes_per_frame / 4, true, true, true);
	mx->main_volume = old_vol;
	
	SDL_SemPost(encodeReady);
}

void Movie::StopRecording()
{
	if (encodeThread)
	{
		stillEncoding = false;
		SDL_SemPost(encodeReady);
		SDL_WaitThread(encodeThread, NULL);
		encodeThread = NULL;
	}
	if (encodeReady)
	{
		SDL_DestroySemaphore(encodeReady);
		encodeReady = NULL;
	}
	if (fillReady)
	{
		SDL_DestroySemaphore(fillReady);
		fillReady = NULL;
	}
	if (temp_surface)
	{
		SDL_FreeSurface(temp_surface);
		temp_surface = NULL;
	}
    
    if (av->inited)
    {
        // flush video and audio
        EncodeVideo(true);
        EncodeAudio(true);
        avcodec_flush_buffers(av->fmt_ctx->streams[av->audio_stream_idx]->codec);
        avcodec_flush_buffers(av->fmt_ctx->streams[av->video_stream_idx]->codec);
        av_write_trailer(av->fmt_ctx);
        av->inited = false;
    }
    
    if (av->audio_fifo)
    {
        av_fifo_free(av->audio_fifo);
        av->audio_fifo = NULL;
    }
    if (av->audio_data)
    {
        av_free(av->audio_data);
        av->audio_data = NULL;
    }
    if (av->audio_frame)
    {
        av_free(av->audio_frame);
        av->audio_frame = NULL;
    }
    
    if (av->video_buf)
    {
        av_free(av->video_buf);
        av->video_buf = NULL;
    }
    if (av->video_data)
    {
        av_free(av->video_data);
        av->video_data = NULL;
    }
    if (av->video_frame)
    {
        av_free(av->video_frame);
        av->video_frame = NULL;
    }
    
    if (av->sws_ctx)
    {
        av_free(av->sws_ctx);
        av->sws_ctx = NULL;
    }

    if (av->fmt_ctx)
    {
        for (int i = 0; i < av->fmt_ctx->nb_streams; i++)
        {
            avcodec_close(av->fmt_ctx->streams[i]->codec);
        }
        avio_close(av->fmt_ctx->pb);
        avformat_free_context(av->fmt_ctx);
        av->fmt_ctx = NULL;
    }

	moviefile = "";
	SDL_PauseAudio(false);
}

#endif
