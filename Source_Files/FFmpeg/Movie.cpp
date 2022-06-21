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
#include "OpenALManager.h"

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
long Movie::GetCurrentAudioTimeStamp() { return 0; }
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
#include "SDL_ffmpeg.h"
#ifdef __cplusplus
}
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
    
    AVFifoBuffer *audio_fifo;
    
    SDL_ffmpegFile* ffmpeg_file;
    SDL_ffmpegAudioFrame* audio_frame;

    size_t video_counter;
    size_t audio_counter;
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
    if (!OpenALManager::Get()) return;

	StopRecording(); 
	moviefile = path;
    OpenALManager::Get()->Stop();
    OpenALManager::Get()->ToggleDeviceMode(IsRecording());
    OpenALManager::Get()->Start();
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
    if (!OpenALManager::Get())
        return false;

    alephone::Screen* scr = alephone::Screen::instance();
    view_rect = scr->window_rect();

    const auto fps = std::max(get_fps_target(), static_cast<int16_t>(30));

    temp_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, view_rect.w, view_rect.h, 32,
        0x00ff0000, 0x0000ff00, 0x000000ff,
        0);

    if (temp_surface == NULL) { ThrowUserError("Could not create SDL surface"); return false; }

    av->ffmpeg_file = SDL_ffmpegCreate(moviefile.c_str());

    if (!av->ffmpeg_file) { ThrowUserError("Could not create ffmpeg file: " + std::string(SDL_ffmpegGetError())); return false; }

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

    int vq = graphics_preferences->movie_export_video_quality;
    int aq = graphics_preferences->movie_export_audio_quality;
    std::string crf = std::to_string(ScaleQuality(vq, 63, 10, 4));

    SDL_ffmpegCodec codec = {};
    codec.videoCodecID = AV_CODEC_ID_VP8;
    codec.audioCodecID = AV_CODEC_ID_VORBIS;
    codec.sampleRate = OpenALManager::Get()->GetFrequency();
    codec.channels = 2;
    codec.width = view_rect.w;
    codec.height = view_rect.h;
    codec.videoBitrate = bitrate;
    codec.cpuCount = get_cpu_count();
    codec.videoMaxRate = ScaleQuality(vq, 63, 63, 50);
    codec.videoMinRate = ScaleQuality(vq, 10, 4, 0);
    codec.audioQuality = aq;
    codec.crf = crf.c_str();
    codec.framerateNum = 1;
    codec.framerateDen = fps;
    codec.audioFormat = mapping_openal_ffmpeg.at(OpenALManager::Get()->GetRenderingFormat());

    in_bps = av_get_bytes_per_sample(codec.audioFormat);

    auto video_stream = SDL_ffmpegAddVideoStream(av->ffmpeg_file, codec);
    if (!video_stream) { ThrowUserError("Could not add video stream: " + std::string(SDL_ffmpegGetError())); return false; }

    auto audio_stream = SDL_ffmpegAddAudioStream(av->ffmpeg_file, codec);
    if (!audio_stream) { ThrowUserError("Could not add audio stream: " + std::string(SDL_ffmpegGetError())); return false; }

    if (SDL_ffmpegSelectVideoStream(av->ffmpeg_file, video_stream->id) == -1) { ThrowUserError("Could not select video stream: " + std::string(SDL_ffmpegGetError())); return false; }
    if (SDL_ffmpegSelectAudioStream(av->ffmpeg_file, audio_stream->id) == -1) { ThrowUserError("Could not select audio stream: " + std::string(SDL_ffmpegGetError())); return false; }

    av->audio_frame = SDL_ffmpegCreateAudioFrame(av->ffmpeg_file, 0);
    if (!av->audio_frame) { ThrowUserError("Could not create audio frame"); return false; }

    if (avformat_write_header(av->ffmpeg_file->_ffmpeg, 0) < 0) { ThrowUserError("Could not write header"); return false; }

    av->audio_fifo = av_fifo_alloc(262144);
    if (!av->audio_fifo) { ThrowUserError("Could not allocate audio fifo"); return false; }

    // set up our threads and intermediate storage
    videobuf.resize(view_rect.w * view_rect.h * 4 + 10000);
    audiobuf.resize(2 * in_bps * OpenALManager::Get()->GetFrequency() / fps);

    // TODO: fixme!
    if (OpenALManager::Get()->GetFrequency() % fps != 0) { ThrowUserError("Audio buffer size is non-integer; try lowering FPS target"); return false; }

	encodeReady = SDL_CreateSemaphore(0);
	fillReady = SDL_CreateSemaphore(1);
	stillEncoding = true;
    if (!encodeReady || !fillReady) { ThrowUserError("Could not create movie thread semaphores"); return false; }

	encodeThread = SDL_CreateThread(Movie_EncodeThread, "MovieSetup_encodeThread", this);
    if (!encodeThread) { ThrowUserError("Could not create movie encoding thread"); return false; }

    if (MainScreenIsOpenGL())
    {
        frameBufferObject = std::unique_ptr<FBO>(new FBO(view_rect.w, view_rect.h));
    }

	return av->inited = true;
}

void Movie::ThrowUserError(std::string error_msg)
{
    StopRecording();
    std::string full_msg = "Your movie could not be exported. (";
    full_msg += error_msg;
    full_msg += ".)";
    logError(full_msg.c_str());
    alert_user(full_msg.c_str());
}

long Movie::GetCurrentAudioTimeStamp()
{
    return IsRecording() && av->inited && av->ffmpeg_file->audioStream ? av->ffmpeg_file->audioStream->lastTimeStamp : 0;
}

int Movie::Movie_EncodeThread(void *arg)
{
	reinterpret_cast<Movie *>(arg)->EncodeThread();
	return 0;
}

void Movie::EncodeVideo(bool last)
{
    SDL_ffmpegAddVideoFrame(av->ffmpeg_file, temp_surface, av->video_counter++, last);
}

void Movie::EncodeAudio(bool last)
{
    av_fifo_generic_write(av->audio_fifo, &audiobuf[0], audiobuf.size(), NULL);
    auto acodec = av->ffmpeg_file->audioStream->_ctx;
    
    // bps: bytes per sample
    int channels = acodec->channels;
    int read_bps = 2;
    
    int max_read = acodec->frame_size * read_bps * channels;
    int min_read = last ? read_bps * channels : max_read;
    while (av_fifo_size(av->audio_fifo) >= min_read)
    {
        int read_bytes = av->audio_frame->size = MIN(av_fifo_size(av->audio_fifo), max_read);
        av_fifo_generic_read(av->audio_fifo, av->audio_frame->buffer, read_bytes, NULL);
        SDL_ffmpegAddAudioFrame(av->ffmpeg_file, av->audio_frame, &av->audio_counter, last);
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
	
	int bytes = audiobuf.size();
    int frameSize = 2 * in_bps;
    auto oldVol = OpenALManager::Get()->GetComputedVolume();
    OpenALManager::Get()->SetDefaultVolume(OpenALManager::From_db(sound_preferences->video_export_volume_db));
    OpenALManager::Get()->GetPlayBackAudio(&audiobuf.front(), bytes / frameSize);
    OpenALManager::Get()->SetDefaultVolume(oldVol);
	
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
        SDL_ffmpegFree(av->ffmpeg_file);
        av->inited = false;
    }
    if (av->audio_frame)
    {
        SDL_ffmpegFreeAudioFrame(av->audio_frame);
        av->audio_frame = NULL;
    }
    if (av->audio_fifo)
    {
        av_fifo_free(av->audio_fifo);
        av->audio_fifo = NULL;
    }

	moviefile = "";
    if (OpenALManager::Get()) {
        OpenALManager::Get()->ToggleDeviceMode(false);
        OpenALManager::Get()->Start();
    }
}

#endif
