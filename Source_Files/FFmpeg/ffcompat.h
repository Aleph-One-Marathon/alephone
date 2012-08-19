#ifndef __FFCOMPAT_H
#define __FFCOMPAT_H

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
 
 Handles FFmpeg API changes for SDL_ffmpeg
 
 */

#ifndef CODEC_TYPE_AUDIO
#define CODEC_TYPE_AUDIO AVMEDIA_TYPE_AUDIO
#endif

#ifndef CODEC_TYPE_VIDEO
#define CODEC_TYPE_VIDEO AVMEDIA_TYPE_VIDEO
#endif

#ifndef PKT_FLAG_KEY
#define PKT_FLAG_KEY AV_PKT_FLAG_KEY
#endif

#endif
