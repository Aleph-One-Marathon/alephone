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
#include "csalerts.h"
#include "Logging.h"

#include <algorithm>

// for CPU count
#ifdef HAVE_SYSCONF
#include <unistd.h>
#endif
#ifdef HAVE_SYSCTLBYNAME
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#ifdef HAVE_OPENGL
#include "OGL_Headers.h"
#endif

#include "Movie.h"
#include "interface.h"
#include "screen.h"
#include "Mixer.h"
#include "preferences.h"

#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

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
#include "libavutil/imgutils.h"
#include "libavutil/fifo.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#ifdef __cplusplus
}
#endif

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(56, 34, 2)
#define AV_CODEC_CAP_SMALL_LAST_FRAME CODEC_CAP_SMALL_LAST_FRAME
#define AV_CODEC_FLAG_QSCALE CODEC_FLAG_QSCALE
#define AV_CODEC_FLAG_GLOBAL_HEADER CODEC_FLAG_GLOBAL_HEADER
#define AV_CODEC_FLAG_CLOSED_GOP CODEC_FLAG_CLOSED_GOP
#endif

// shamelessly stolen from SDL 2.0
static int get_cpu_count(void)
{
    static int cpu_count = 0;
    if (cpu_count == 0) {
#if defined(HAVE_SYSCONF) && defined(_SC_NPROCESSORS_ONLN)
        cpu_count = (int)sysconf(_SC_NPROCESSORS_ONLN);
#endif
#ifdef HAVE_SYSCTLBYNAME
        size_t size = sizeof(cpu_count);
        sysctlbyname("hw.ncpu", &cpu_count, &size, NULL, 0);
#endif
#ifdef __WIN32__
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        cpu_count = info.dwNumberOfProcessors;
#endif
        /* There has to be at least 1, right? :) */
        if (cpu_count <= 0)
            cpu_count = 1;
    }
    return cpu_count;
}


#define MAX_AUDIO_CHANNELS 2

struct libav_vars {
    bool inited;
    
    AVFrame *audio_frame;
    AVFifoBuffer *audio_fifo;
    uint8_t *audio_data;
    uint8_t **audio_data_conv;

    AVFrame *video_frame;
    uint8_t *video_buf;
    int video_bufsize;
    uint8_t *video_data;
    
    struct SwsContext *sws_ctx;
    AVFormatContext *fmt_ctx;
    int video_stream_idx;
    int audio_stream_idx;

    AVCodecContext* video_ctx;
    AVCodecContext* audio_ctx;

    size_t video_counter;
    size_t audio_counter;

    SwrContext* swr_context;
};
typedef struct libav_vars libav_vars_t;

int ScaleQuality(int quality, int zeroLevel, int fiftyLevel, int hundredLevel)
{
    switch (quality)
    {
        case 50:
            return fiftyLevel;
        case 100:
            return hundredLevel;
        case 0:
            return zeroLevel;
    }
    
    float min, diff, frac;
    if (quality < 50)
    {
        min = zeroLevel;
        diff = fiftyLevel - zeroLevel;
        frac = quality / 50.0f;
    }
    else
    {
        min = fiftyLevel;
        diff = hundredLevel - fiftyLevel;
        frac = (quality - 50) / 50.0f;
    }
    return min + (diff * frac);
}

Movie::Movie() :
  moviefile(""),
  temp_surface(NULL),
  av(NULL),
  encodeThread(NULL),
  encodeReady(NULL),
  frameBufferObject(nullptr),
  fillReady(NULL),
  stillEncoding(0)
{
    av = new libav_vars_t;
    memset(av, 0, sizeof(libav_vars_t));
}

void Movie::PromptForRecording()
{
	FileSpecifier dst_file;
	if (!dst_file.WriteDialog(_typecode_movie, "EXPORT FILM", "Untitled Movie.webm"))
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
    std::string err_msg;
    
	alephone::Screen *scr = alephone::Screen::instance();
	view_rect = scr->window_rect();

	temp_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, view_rect.w, view_rect.h, 32,
										0x00ff0000, 0x0000ff00, 0x000000ff,
										0);
	success = (temp_surface != NULL);
	if (!success) err_msg = "Could not create SDL surface";

    Mixer *mx = Mixer::instance();

	const auto fps = std::max(get_fps_target(), static_cast<int16_t>(30));
    
    // Open output file
    AVOutputFormat *fmt;
    if (success)
    {
        fmt = av_guess_format("webm", NULL, NULL);
        success = fmt;
        if (!success) err_msg = "Could not find output format";
    }
    if (success)
    {
        av->fmt_ctx = avformat_alloc_context();
        success = av->fmt_ctx;
        if (!success) err_msg = "Could not allocate movie format context";
    }
    if (success)
    {
        av->fmt_ctx->oformat = fmt;
        av->fmt_ctx->url = (char*)av_malloc(moviefile.size() + 1);
        snprintf(av->fmt_ctx->url, moviefile.size() + 1, moviefile.c_str());
        success = (0 <= avio_open(&av->fmt_ctx->pb, av->fmt_ctx->url, AVIO_FLAG_WRITE));
        if (!success) err_msg = "Could not open movie file for writing";
    }
    
    // Open output video stream
    AVCodec *video_codec;
    AVStream *video_stream;
    if (success)
    {
        video_codec = avcodec_find_encoder(AV_CODEC_ID_VP8);
        success = video_codec;
        if (!success) err_msg = "Could not find VP8 encoder";
    }
    if (success)
    {
        video_stream = avformat_new_stream(av->fmt_ctx, video_codec);
        success = video_stream;
        if (!success) err_msg = "Could not open output video stream";
    }
    if (success)
    {
        int bitrate = graphics_preferences->movie_export_video_bitrate;

        if (bitrate <= 0) // auto, based on YouTube's SDR standard frame rate
                          // recommendations
        {
            if (view_rect.h >= 2160) bitrate = 40 * 1024 * 1024;
            else if (view_rect.h >= 1440) bitrate = 16 * 1024 * 1024;
            else if (view_rect.h >= 1080) bitrate = 8 * 1024 * 1024;
            else if (view_rect.h >= 720) bitrate = 5 * 1024 * 1024;
            else if (view_rect.h >= 480) bitrate = 5 * 1024 * 1024 / 2;
            else                          bitrate = 1024 * 1024;

            // YouTube recommends 50% more bitrate for 60 fps, so extrapolate
            // from there
            bitrate += std::log2(fps / 30) * bitrate / 2;
        }

        video_stream->codecpar->codec_id = video_codec->id;
        video_stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
        video_stream->codecpar->width = view_rect.w;
        video_stream->codecpar->height = view_rect.h;
        video_stream->codecpar->bit_rate = bitrate;
        video_stream->codecpar->format = AV_PIX_FMT_YUV420P;

        av->video_ctx = avcodec_alloc_context3(video_codec);
        success = av->video_ctx && avcodec_parameters_to_context(av->video_ctx, video_stream->codecpar) >= 0;
        if (!success) err_msg = "Could not setup video context";
    }
    if (success)
    {
        av->video_ctx->time_base = AVRational{ 1, fps };
        av->video_ctx->flags |= AV_CODEC_FLAG_CLOSED_GOP;
        av->video_ctx->thread_count = get_cpu_count();

        if (av->fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            av->video_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        av->video_stream_idx = video_stream->index;
        
        // tuning options
        int vq = graphics_preferences->movie_export_video_quality;	
        av->video_ctx->qmin = ScaleQuality(vq, 10, 4, 0);
        av->video_ctx->qmax = ScaleQuality(vq, 63, 63, 50);
        std::string crf = std::to_string(ScaleQuality(vq, 63, 10, 4));
        av_opt_set(av->video_ctx->priv_data, "crf", crf.c_str(), 0);
        
        //avcodec_open2 may fill some fields we need to have in codecpar
        success = avcodec_open2(av->video_ctx, NULL, NULL) >= 0 && avcodec_parameters_from_context(video_stream->codecpar, av->video_ctx) >= 0;
        if (!success) err_msg = "Could not open video codec";
    }
    if (success)
    {
        av->video_bufsize = view_rect.w * view_rect.h * 4 + 10000;
        av->video_buf = static_cast<uint8_t *>(av_malloc(av->video_bufsize));
        success = av->video_buf;
        if (!success) err_msg = "Could not allocate video buffer";
    }
    if (success)
    {
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,0)
        av->video_frame = avcodec_alloc_frame();
#else
        av->video_frame = av_frame_alloc();
#endif
        success = av->video_frame;
        if (!success) err_msg = "Could not allocate video frame";
    }
    if (success)
    {
        int numbytes = av_image_get_buffer_size(av->video_ctx->pix_fmt, view_rect.w, view_rect.h, 1);
        av->video_data = static_cast<uint8_t *>(av_malloc(numbytes));
        success = av->video_data;
        if (!success) err_msg = "Could not allocate video data buffer";
    }
    if (success)
    {
        success = av_image_fill_arrays(av->video_frame->data, av->video_frame->linesize, av->video_data, av->video_ctx->pix_fmt, view_rect.w, view_rect.h, 1) >= 0;
        if (!success) err_msg = "Could not setup video data buffer";
    }
    
    // Open output audio stream
    AVCodec *audio_codec;
    AVStream *audio_stream;
    if (success)
    {
        audio_codec = avcodec_find_encoder(AV_CODEC_ID_VORBIS);
        success = audio_codec;
        if (!success) err_msg = "Could not find Vorbis encoder";
    }
    if (success)
    {
        audio_stream = avformat_new_stream(av->fmt_ctx, audio_codec);
        success = audio_stream;
        if (!success) err_msg = "Could not open output audio stream";
    }
    if (success)
    {
        audio_stream->codecpar->codec_id = audio_codec->id;
        audio_stream->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
        audio_stream->codecpar->format = AV_SAMPLE_FMT_FLTP;
        audio_stream->codecpar->sample_rate = mx->obtained.freq;
        audio_stream->codecpar->channel_layout = AV_CH_LAYOUT_STEREO;
        audio_stream->codecpar->channels = 2;

        av->audio_ctx = avcodec_alloc_context3(audio_codec);
        success = av->audio_ctx && avcodec_parameters_to_context(av->audio_ctx, audio_stream->codecpar) >= 0;
        if (!success) err_msg = "Could not setup audio context";
    }
    if (success)
    {     
        av->audio_ctx->time_base = AVRational{ 1, mx->obtained.freq };

        if (av->fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            av->audio_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        
        av->audio_stream_idx = audio_stream->index;
        av->audio_ctx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

        // tuning options
        int aq = graphics_preferences->movie_export_audio_quality;
        av->audio_ctx->global_quality = FF_QP2LAMBDA * (aq / 10);
        av->audio_ctx->flags |= AV_CODEC_FLAG_QSCALE;
        
        //avcodec_open2 may fill some fields we need to have in codecpar
        success = avcodec_open2(av->audio_ctx, NULL, NULL) >= 0 && avcodec_parameters_from_context(audio_stream->codecpar, av->audio_ctx) >= 0;
        if (!success) err_msg = "Could not open audio codec";
    }
    if (success)
    {
        // init resampler
        av->swr_context = swr_alloc_set_opts(av->swr_context, av->audio_ctx->channel_layout, av->audio_ctx->sample_fmt, av->audio_ctx->sample_rate,
            av->audio_ctx->channel_layout, AV_SAMPLE_FMT_S16, av->audio_ctx->sample_rate, 0, NULL);

        success = av->swr_context && swr_init(av->swr_context) >= 0;
        if (!success) err_msg = "Could not initialize resampler";
    }
    if (success)
    {
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,0)
        av->audio_frame = avcodec_alloc_frame();
#else
        av->audio_frame = av_frame_alloc();
#endif
        success = av->audio_frame;
        if (!success) err_msg = "Could not allocate audio frame";
    }
    if (success)
    {
        av->audio_fifo = av_fifo_alloc(262144);
        success = av->audio_fifo;
        if (!success) err_msg = "Could not allocate audio fifo";
    }
    if (success)
    {
        success = av_samples_alloc(&av->audio_data, NULL, av->audio_ctx->channels, av->audio_ctx->frame_size, AV_SAMPLE_FMT_S16, 0) >= 0;
        if (!success) err_msg = "Could not allocate audio data buffer";
    }
    if (success)
    {
        success = av_samples_alloc_array_and_samples(&av->audio_data_conv, NULL, av->audio_ctx->channels, av->audio_ctx->frame_size, av->audio_ctx->sample_fmt, 0) >= 0;
        if (!success) err_msg = "Could not allocate audio conversion buffer";
    }
    
    // initialize conversion context
    if (success)
    {
        av->sws_ctx = sws_getContext(temp_surface->w, temp_surface->h, AV_PIX_FMT_RGB32,
                                     av->video_ctx->width,
                                     av->video_ctx->height,
                                     av->video_ctx->pix_fmt,
                                     SWS_BILINEAR,
                                     NULL, NULL, NULL);
        success = av->sws_ctx;
        if (!success) err_msg = "Could not create video conversion context";
    }
    
    // Start movie file
    if (success)
    {
        video_stream->time_base = AVRational{1, fps};
        audio_stream->time_base = AVRational{1, mx->obtained.freq};
        success = avformat_write_header(av->fmt_ctx, NULL) >= 0;
        if (!success) err_msg = "Could not create write video header";
    }
    
    // set up our threads and intermediate storage
    if (success)
    {
        videobuf.resize(av->video_bufsize);
        audiobuf.resize(2 * 2 * mx->obtained.freq / fps);

		if (mx->obtained.freq % fps != 0)
		{
			// TODO: fixme!
			success = false;
			err_msg = "Audio buffer size is non-integer; try lowering FPS target";
		}
	}
	if (success)
	{
		encodeReady = SDL_CreateSemaphore(0);
		fillReady = SDL_CreateSemaphore(1);
		stillEncoding = true;
		success = encodeReady && fillReady;
		if (!success) err_msg = "Could not create movie thread semaphores";
	}
	if (success)
	{
		encodeThread = SDL_CreateThread(Movie_EncodeThread, "MovieSetup_encodeThread", this);
		success = encodeThread;
		if (!success) err_msg = "Could not create movie encoding thread";
	}
	
	if (!success)
	{
		StopRecording();
		std::string full_msg = "Your movie could not be exported. (";
		full_msg += err_msg;
		full_msg += ".)";
        logError(full_msg.c_str());
		alert_user(full_msg.c_str());
	}

    if (success && MainScreenIsOpenGL())
    {
        frameBufferObject = std::unique_ptr<FBO>(new FBO(view_rect.w, view_rect.h));
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
    AVCodecContext *vcodec = av->video_ctx;
    
    AVFrame *frame = NULL;
    if (!last)
    {
        int pitch[] = { temp_surface->pitch, 0 };
        const uint8_t *const pdata[] = { reinterpret_cast<uint8_t *>(temp_surface->pixels), NULL };
    
        sws_scale(av->sws_ctx, pdata, pitch, 0, temp_surface->h,
                  av->video_frame->data, av->video_frame->linesize);
        av->video_frame->pts = av->video_counter++;
        frame = av->video_frame;

        //Needed since ffmpeg version 4.4
        frame->format = vcodec->pix_fmt;
        frame->width = vcodec->width;
        frame->height = vcodec->height;
    }
    
    bool done = false;
    AVPacket* pkt = av_packet_alloc();
    while (!done)
    {
        // add video
        pkt->data = av->video_buf;
        pkt->size = av->video_bufsize;
        
        int vsize = avcodec_send_frame(vcodec, frame);
        int got_packet = avcodec_receive_packet(vcodec, pkt);
        if (vsize == 0 && got_packet == 0)
        {
            if (pkt->pts != AV_NOPTS_VALUE && pkt->pts < pkt->dts)
                pkt->pts = pkt->dts;
            if (pkt->pts != AV_NOPTS_VALUE)
                pkt->pts = av_rescale_q(pkt->pts, vcodec->time_base, vstream->time_base);
            if (pkt->dts != AV_NOPTS_VALUE)
                pkt->dts = av_rescale_q(pkt->dts, vcodec->time_base, vstream->time_base);
            pkt->duration = av_rescale_q(pkt->duration, vcodec->time_base, vstream->time_base);
            pkt->stream_index = vstream->index;
            av_interleaved_write_frame(av->fmt_ctx, pkt);
            av_packet_unref(pkt);
        }
        if (!last || vsize < 0 || got_packet < 0)
            done = true;
    }
    av_packet_free(&pkt);
}

void Movie::EncodeAudio(bool last)
{
    AVStream *astream = av->fmt_ctx->streams[av->audio_stream_idx];
    AVCodecContext *acodec = av->audio_ctx;
    
    
    av_fifo_generic_write(av->audio_fifo, &audiobuf[0], audiobuf.size(), NULL);
    
    // bps: bytes per sample
    int channels = acodec->channels;
    int read_bps = 2;
    int write_bps = av_get_bytes_per_sample(acodec->sample_fmt);
    
    int max_read = acodec->frame_size * read_bps * channels;
    int min_read = last ? read_bps * channels : max_read;
    while (av_fifo_size(av->audio_fifo) >= min_read)
    {
        int read_bytes = MIN(av_fifo_size(av->audio_fifo), max_read);
        av_fifo_generic_read(av->audio_fifo, av->audio_data, read_bytes, NULL);
        
        // convert
        int read_samples = read_bytes / (read_bps * channels);
        int write_samples = read_samples;
        if (read_samples < acodec->frame_size)
        {
            // shrink or pad audio frame
            if (acodec->codec->capabilities & AV_CODEC_CAP_SMALL_LAST_FRAME)
                acodec->frame_size = write_samples;
            else
                write_samples = acodec->frame_size;
        }

        write_samples = swr_convert(av->swr_context, av->audio_data_conv, write_samples, (const uint8_t**)&av->audio_data, read_samples);
                      
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
        avcodec_get_frame_defaults(av->audio_frame);
#else
        av_frame_unref(av->audio_frame);
#endif
        //Needed since ffmpeg 4.4
        av->audio_frame->channels = acodec->channels;
        av->audio_frame->format = acodec->sample_fmt;
        av->audio_frame->channel_layout = acodec->channel_layout;
        av->audio_frame->sample_rate = acodec->sample_rate;
        av->audio_frame->nb_samples = write_samples;

        av->audio_frame->pts = av_rescale_q(av->audio_counter,
                                            AVRational{1, acodec->sample_rate},
                                            acodec->time_base);

        av->audio_counter += write_samples;
        int asize = avcodec_fill_audio_frame(av->audio_frame, acodec->channels,
                                             acodec->sample_fmt,
                                             av->audio_data_conv[0],
                                             write_samples * write_bps * channels, 1);

        if (asize >= 0)
        {
            AVPacket* pkt = av_packet_alloc();        
            int vsize = avcodec_send_frame(acodec, av->audio_frame);
            if (0 == vsize)
            {
                while (avcodec_receive_packet(acodec, pkt) == 0) {
                    if (pkt->pts != AV_NOPTS_VALUE && pkt->pts < pkt->dts)
                        pkt->pts = pkt->dts;
                    if (pkt->pts != AV_NOPTS_VALUE)
                        pkt->pts = av_rescale_q(pkt->pts, acodec->time_base, astream->time_base);
                    if (pkt->dts != AV_NOPTS_VALUE)
                        pkt->dts = av_rescale_q(pkt->dts, acodec->time_base, astream->time_base);
                    pkt->duration = av_rescale_q(pkt->duration, acodec->time_base, astream->time_base);
                    pkt->stream_index = astream->index;
                    av_interleaved_write_frame(av->fmt_ctx, pkt);
                    av_packet_unref(pkt);
                }
            }
            av_packet_free(&pkt);
        }
    }
    if (last)
    {
        bool done = false;
        AVPacket* pkt = av_packet_alloc();
        int flush = avcodec_send_frame(acodec, NULL);
        while (flush == 0 && !done)
        {
            int got_pkt = avcodec_receive_packet(acodec, pkt);
            if (got_pkt == 0)
            {
                if (pkt->pts != AV_NOPTS_VALUE && pkt->pts < pkt->dts)
                    pkt->pts = pkt->dts;
                if (pkt->pts != AV_NOPTS_VALUE)
                    pkt->pts = av_rescale_q(pkt->pts, acodec->time_base, astream->time_base);
                if (pkt->dts != AV_NOPTS_VALUE)
                    pkt->dts = av_rescale_q(pkt->dts, acodec->time_base, astream->time_base);
                pkt->duration = av_rescale_q(pkt->duration, acodec->time_base, astream->time_base);
                pkt->stream_index = astream->index;
                av_interleaved_write_frame(av->fmt_ctx, pkt);
                av_packet_unref(pkt);
            }
            else
            {
                done = true;
            }
        }
        av_packet_free(&pkt);
        
    }
    
}

void Movie::EncodeThread()
{
	av->video_counter = 0;
	av->audio_counter = 0;
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
  	
	if (!MainScreenIsOpenGL())
	{
		SDL_Surface *video = MainScreenSurface();
		SDL_BlitSurface(video, &view_rect, temp_surface, NULL);
	}
#ifdef HAVE_OPENGL
	else
	{
        SDL_Rect viewportDimensions = alephone::Screen::instance()->OpenGLViewPort();
        GLint fbx = viewportDimensions.x, fby = viewportDimensions.y, fbWidth = viewportDimensions.w, fbHeight = viewportDimensions.h;

        // Copy default frame buffer to another one with correct viewport resized/pixels rescaled
        frameBufferObject->activate(true, GL_DRAW_FRAMEBUFFER_EXT);
        glBlitFramebufferEXT(fbx, fby, fbWidth + fbx, fbHeight + fby, view_rect.x, view_rect.y, view_rect.w, view_rect.h, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        frameBufferObject->deactivate();

        // Read our new frame buffer with rescaled pixels
        frameBufferObject->activate(true, GL_READ_FRAMEBUFFER_EXT);
        glReadPixels(view_rect.x, view_rect.y, view_rect.w, view_rect.h, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, &videobuf.front());
        frameBufferObject->deactivate();

		// Copy pixel buffer (which is upside-down) to surface
		for (int y = 0; y < view_rect.h; y++)
			memcpy((uint8 *)temp_surface->pixels + temp_surface->pitch * y, &videobuf.front() + view_rect.w * 4 * (view_rect.h - y - 1), view_rect.w * 4);
	}
#endif
	
	int audio_bytes_per_frame = audiobuf.size();
	Mixer *mx = Mixer::instance();
	float old_vol = mx->main_volume;
	mx->SetVolume(sound_preferences->video_export_volume_db);
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
        avcodec_flush_buffers(av->audio_ctx);
        avcodec_flush_buffers(av->video_ctx);
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
        av_freep(&av->audio_data);
    }
    if (av->audio_data_conv)
    {
        av_freep(&av->audio_data_conv[0]);
        av_freep(&av->audio_data_conv);
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

    if (av->video_ctx)
    {
        avcodec_free_context(&av->video_ctx);
    }

    if (av->audio_ctx)
    {
        avcodec_free_context(&av->audio_ctx);
    }
    
    if (av->swr_context)
    {
        swr_free(&av->swr_context);
    }

    if (av->fmt_ctx)
    {
        avio_close(av->fmt_ctx->pb);
        avformat_free_context(av->fmt_ctx);
        av->fmt_ctx = NULL;
    }

	moviefile = "";
	SDL_PauseAudio(false);
}

#endif
