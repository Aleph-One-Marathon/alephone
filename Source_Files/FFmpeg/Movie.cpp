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
  
  Movie export using SDL_ffmpeg
  
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

void Movie::PromptForRecording() {}
void Movie::StartRecording(std::string path) {}
bool Movie::IsRecording() { return false; }
void Movie::StopRecording() {}
void Movie::AddFrame(FrameType ftype) {}

bool Movie::Setup() { return false; }
Movie::Movie() {}

#else
#include "SDL_ffmpeg.h"

#ifdef __cplusplus
extern "C"
{
#endif
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#ifdef __cplusplus
}
#endif

Movie::Movie() :
  moviefile(""),
  temp_surface(NULL),
  sffile(NULL),
  aframe(NULL),
  encodeThread(NULL),
  encodeReady(NULL),
  fillReady(NULL),
  stillEncoding(0)
{
}

void Movie::PromptForRecording()
{
	FileSpecifier dst_file;
	if (!dst_file.WriteDialog(_typecode_movie, "EXPORT FILM", "Untitled Movie.mpg"))
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
  if (sffile)
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
  SDL_ffmpegCodec codec;
  codec.videoCodecID = CODEC_ID_MPEG4;
  codec.width = view_rect.w;
  codec.height = view_rect.h;
  codec.framerateNum = 1;
  codec.framerateDen = 30;
  codec.videoBitrate = view_rect.w * view_rect.h * 16;
  codec.videoMinRate = -1;
  codec.videoMaxRate = -1;
  codec.audioCodecID = CODEC_ID_MP2;
  codec.channels = 2;
  codec.sampleRate = mx->obtained.freq;
  codec.audioBitrate = 128000;
  codec.audioMinRate = -1;
  codec.audiooMaxRate = -1;
  
	if (success)
	{
    sffile = SDL_ffmpegCreate(moviefile.c_str());
    success = (sffile != NULL);
  }
  
  if (success)
    success = (SDL_ffmpegAddVideoStream(sffile, codec) != NULL);
  if (success)
    success = (SDL_ffmpegSelectVideoStream(sffile, 0) >= 0);
  if (success)
    success = (SDL_ffmpegAddAudioStream(sffile, codec) != NULL);
  if (success)
    success = (SDL_ffmpegSelectAudioStream(sffile, 0) >= 0);

  int audio_bytes_per_frame = 2 * 2 * mx->obtained.freq / 30;
  if (success)
  {
	  aframe = SDL_ffmpegCreateAudioFrame(sffile, audio_bytes_per_frame);
	  success = (aframe != NULL);
	}

  if (success)
  {
		videobuf.resize(view_rect.w * view_rect.h * 4);
	  audiobuf.resize(audio_bytes_per_frame);
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
	return success;
}

int Movie::Movie_EncodeThread(void *arg)
{
	reinterpret_cast<Movie *>(arg)->EncodeThread();
	return 0;
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
		
		SDL_ffmpegAddVideoFrame(sffile, temp_surface);
		
		int audio_bytes_per_frame = audiobuf.size();
		int spare_used = audio_bytes_per_frame;
		int spare_off = 0;
		while (spare_used)
		{
			int bytes = MIN(spare_used, aframe->capacity - aframe->size);
			memcpy(&aframe->buffer[aframe->size], &audiobuf[spare_off], bytes);
			spare_used -= bytes;
			spare_off += bytes;
			aframe->size += bytes;
			if (aframe->size == aframe->capacity)
			{
				SDL_ffmpegAddAudioFrame(sffile, aframe);
				aframe->size = 0;
			}
		}
		
		SDL_SemPost(fillReady);
	}
}

void Movie::AddFrame(FrameType ftype)
{
	if (!IsRecording())
		return;
	if (!sffile)
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
	if (sffile)
	{
		SDL_ffmpegFree(sffile);
		sffile = NULL;
	}
	if (temp_surface)
	{
		SDL_FreeSurface(temp_surface);
		temp_surface = NULL;
	}
	if (aframe)
	{
		SDL_ffmpegFreeAudioFrame(aframe);
		aframe = NULL;
	}
	moviefile = "";
	SDL_PauseAudio(false);
}

#endif
