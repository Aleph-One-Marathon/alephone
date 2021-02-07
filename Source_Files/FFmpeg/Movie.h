#ifndef __MOVIE_H
#define __MOVIE_H

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
  
 */

#include "cseries.h"
#include "OGL_FBO.h"
#include <memory>
#include <string.h>
#include <vector>
#include <SDL_thread.h>

class Movie
{
public:
	static Movie *instance() { 
		static Movie *m_instance = nullptr;
		if (!m_instance
				) m_instance = new Movie(); 
		return m_instance; 
	}
	
	void PromptForRecording();
	void StartRecording(std::string path);
	bool IsRecording();
	void StopRecording();
	
	enum FrameType {
	  FRAME_NORMAL,
	  FRAME_FADE,
	  FRAME_CHAPTER
	};
	void AddFrame(FrameType ftype = FRAME_NORMAL);

private:
  
  std::string moviefile;
  SDL_Rect view_rect;
  SDL_Surface *temp_surface;
  
  std::vector<uint8> videobuf;
  std::vector<uint8> audiobuf;
  
  struct libav_vars *av;
  
  SDL_Thread *encodeThread;
  SDL_sem *encodeReady;
  SDL_sem *fillReady;
  bool stillEncoding;
  std::unique_ptr<FBO> frameBufferObject;
  
  Movie();  
  bool Setup();
  static int Movie_EncodeThread(void *arg);
  void EncodeThread();
  void EncodeVideo(bool last);
  void EncodeAudio(bool last);
};
	
#endif
