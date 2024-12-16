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
   
 */


#include "cseries.h"
#include "csalerts.h"
#include "Logging.h"
#include "OpenALManager.h"
#include "alephversion.h"

#include <algorithm>

// for CPU count
#ifdef HAVE_SYSCONF
#include <unistd.h>
#endif
#ifdef HAVE_SYSCTLBYNAME
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#include "OGL_Headers.h"

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

#include <libwebm/mkvmuxer/mkvmuxer.h>
#include <libwebm/mkvmuxer/mkvwriter.h>
#include <vorbis/vorbisenc.h>
#include <libyuv/convert.h>
#include <vpx/vpx_encoder.h>
#include <vpx/vp8cx.h>


struct libav_vars {
    bool inited;
    
	// libwebm data
	mkvmuxer::MkvWriter *writer;
	mkvmuxer::Segment *segment;
	mkvmuxer::VideoTrack *vtrack;
	mkvmuxer::AudioTrack *atrack;

	// libvorbis data
	ogg_packet       op; /* one raw packet of data for decode */
	vorbis_info      vi; /* struct that stores all the static vorbis bitstream settings */
	vorbis_comment   vc; /* struct that stores all the user comments */
	vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
	vorbis_block     vb; /* local working space for packet->PCM decode */
	
	// libvpx data
	vpx_image_t *yuv;
	vpx_codec_ctx_t codec;
	unsigned long deadline;
		

	int fps;
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
#ifdef HAVE_OPENGL
  frameBufferObject(nullptr),
#endif
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

    const float pixel_scale = scr->pixel_scale();
    view_rect.x *= pixel_scale;
    view_rect.y *= pixel_scale;
    view_rect.h *= pixel_scale;
    view_rect.w *= pixel_scale;

    const auto fps = std::max(get_fps_target(), static_cast<int16_t>(30));
	av->fps = fps;
	last_written_timestamp = 0;

    temp_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, view_rect.w, view_rect.h, 32,
        0x00ff0000, 0x0000ff00, 0x000000ff,
        0);

    if (temp_surface == NULL) { ThrowUserError("Could not create SDL surface"); return false; }
	
	av->writer = new mkvmuxer::MkvWriter();
	if (!av->writer || !av->writer->Open(moviefile.c_str())) { ThrowUserError("Could not create webm file at " + moviefile); return false; }
	
	av->segment = new mkvmuxer::Segment();
	if (!av->segment || !av->segment->Init(av->writer)) { ThrowUserError("Could not start webm file"); return false; }
	av->segment->set_mode(mkvmuxer::Segment::kFile);
	av->segment->OutputCues(true);
	
	mkvmuxer::SegmentInfo* const info = av->segment->GetSegmentInfo();
	info->set_timecode_scale(1000000);
	info->set_writing_app(A1_DISPLAY_NAME " " A1_VERSION_STRING);

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
	
	// video setup
	// set up video track
	uint64_t vtracknum = av->segment->AddVideoTrack(view_rect.w, view_rect.h, mkvmuxer::Tracks::kVideo);
	if (!vtracknum) { ThrowUserError("Could not add video track"); return false; }
	av->vtrack = static_cast<mkvmuxer::VideoTrack*>(av->segment->GetTrackByNumber(vtracknum));
	if (!av->vtrack) { ThrowUserError("Could not find video track"); return false; }
	av->vtrack->set_codec_id(mkvmuxer::Tracks::kVp8CodecId);
	av->vtrack->set_frame_rate(fps);
	
	// set up vpx
	vpx_codec_enc_cfg_t cfg;
	vpx_codec_iface_t *cif = vpx_codec_vp8_cx();
	if (vpx_codec_enc_config_default(cif, &cfg, 0)) { ThrowUserError("Failed to get default codec config"); return false; }
	cfg.g_w = view_rect.w;
	cfg.g_h = view_rect.h;
	cfg.g_timebase.num = 1;
	cfg.g_timebase.den = fps;
	cfg.rc_end_usage = vq == 100 ? VPX_VBR : VPX_CQ;
	cfg.rc_target_bitrate = bitrate;
	cfg.rc_max_quantizer = ScaleQuality(vq, 63, 63, 50);
	cfg.rc_min_quantizer = ScaleQuality(vq, 10, 4, 0);
	if (vpx_codec_enc_init(&(av->codec), cif, &cfg, 0)) { ThrowUserError("Failed to initialize vpx encoder"); return false; }
	vpx_codec_control(&(av->codec), VP8E_SET_CQ_LEVEL, ScaleQuality(vq, 63, 10, 4));
	av->deadline = ScaleQuality(vq, VPX_DL_REALTIME, VPX_DL_GOOD_QUALITY, VPX_DL_GOOD_QUALITY * 2);
	if (vq == 100) { av->deadline = VPX_DL_BEST_QUALITY; }
	
	// audio setup
	// set in_bps from rendering format
	switch (OpenALManager::Get()->GetRenderingFormat()) {
		case ALC_SHORT_SOFT:
			in_bps = 16;
			break;
		case ALC_FLOAT_SOFT:
			in_bps = 32;
			break;
		case ALC_INT_SOFT:
			in_bps = 32;
			break;
		case ALC_UNSIGNED_BYTE_SOFT:
			in_bps = 8;
			break;
	}
	
	// set up audio track
	uint64_t atracknum = av->segment->AddAudioTrack(OpenALManager::Get()->GetFrequency(), 2, mkvmuxer::Tracks::kAudio);
	if (!atracknum) { ThrowUserError("Could not add audio track"); return false; }
	av->atrack = static_cast<mkvmuxer::AudioTrack*>(av->segment->GetTrackByNumber(atracknum));
	if (!av->atrack) { ThrowUserError("Could not find audio track"); return false; }
	av->atrack->set_codec_id(mkvmuxer::Tracks::kVorbisCodecId);
	
	// set up vorbis
	float vorbisq = (aq == 0) ? -.1f : aq/100.f;
	vorbis_info_init(&(av->vi));
	if (vorbis_encode_init_vbr(&(av->vi), 2, OpenALManager::Get()->GetFrequency(), vorbisq)) { ThrowUserError("Could not init vorbis vbr"); return false; }
	vorbis_analysis_init(&(av->vd), &(av->vi));

	// encode vorbis headers
	vorbis_comment_init(&(av->vc));
	vorbis_comment_add_tag(&(av->vc), "ENCODER", A1_DISPLAY_NAME " " A1_VERSION_STRING);
	ogg_packet header_iden;
	ogg_packet header_comm;
	ogg_packet header_code;
	vorbis_analysis_headerout(&(av->vd), &(av->vc), &header_iden, &header_comm, &header_code);
	long header_iden_lace = header_iden.bytes / 255;
	long header_iden_last = header_iden.bytes % 255;
	long header_comm_lace = header_comm.bytes / 255;
	long header_comm_last = header_comm.bytes % 255;
	std::vector<uint8> aprivatebuf;
	aprivatebuf.reserve(3 + header_iden_lace + header_comm_lace + header_iden.bytes + header_comm.bytes + header_code.bytes);
	aprivatebuf.push_back(2);
	aprivatebuf.insert(aprivatebuf.end(), header_iden_lace, 255);
	aprivatebuf.push_back(header_iden_last);
	aprivatebuf.insert(aprivatebuf.end(), header_comm_lace, 255);
	aprivatebuf.push_back(header_comm_last);
	aprivatebuf.insert(aprivatebuf.end(), header_iden.packet, header_iden.packet + header_iden.bytes);
	aprivatebuf.insert(aprivatebuf.end(), header_comm.packet, header_comm.packet + header_comm.bytes);
	aprivatebuf.insert(aprivatebuf.end(), header_code.packet, header_code.packet + header_code.bytes);
	av->atrack->SetCodecPrivate(aprivatebuf.data(), aprivatebuf.size());

	vorbis_block_init(&(av->vd), &(av->vb));

	// set up webm cues
	mkvmuxer::Cues* const cues = av->segment->GetCues();
	cues->set_output_block_number(true);
	av->segment->CuesTrack(vtracknum);
	
    // set up our threads and intermediate storage
    videobuf.resize(view_rect.w * view_rect.h * 4 + 10000);
	av->yuv = vpx_img_alloc(av->yuv, VPX_IMG_FMT_I420, view_rect.w, view_rect.h, 1);
	if (!av->yuv) { ThrowUserError("VPX image could not be allocated"); return false; }
    audiobuf.resize(2 * in_bps * OpenALManager::Get()->GetFrequency() / fps);
	last_written_timestamp = 0;
	current_audio_timestamp = 0;

    // TODO: fixme!
    if (OpenALManager::Get()->GetFrequency() % fps != 0) { ThrowUserError("Audio buffer size is non-integer; try lowering FPS target"); return false; }

	encodeReady = SDL_CreateSemaphore(0);
	fillReady = SDL_CreateSemaphore(1);
	stillEncoding = true;
    if (!encodeReady || !fillReady) { ThrowUserError("Could not create movie thread semaphores"); return false; }

	encodeThread = SDL_CreateThread(Movie_EncodeThread, "MovieSetup_encodeThread", this);
    if (!encodeThread) { ThrowUserError("Could not create movie encoding thread"); return false; }

#ifdef HAVE_OPENGL
    if (MainScreenIsOpenGL())
    {
        frameBufferObject = std::make_unique<FBO>(view_rect.w, view_rect.h);
    }
#endif

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
	return IsRecording() && av->inited ? current_audio_timestamp : 0;
}

int Movie::Movie_EncodeThread(void *arg)
{
	reinterpret_cast<Movie *>(arg)->EncodeThread();
	return 0;
}

void Movie::EncodeVideo(bool last)
{
	if (av->yuv)
	{
		int flags = 0;
		if (!(av->video_counter % (7 * av->fps)))
		{
			flags |= VPX_EFLAG_FORCE_KF;
		}
		if (vpx_codec_encode(&(av->codec), av->yuv, av->video_counter++, 1, flags, av->deadline))
		{
			fprintf(stderr, "vpx encode failed at %zu\n", av->video_counter);
		}
	}
	
	if (last)
	{
		if (vpx_codec_encode(&(av->codec), nullptr, av->video_counter++, 1, 0, 0))
		{
			fprintf(stderr, "vpx encode failed at last\n");
		}
	}
	
	vpx_codec_iter_t iter = nullptr;
	const vpx_codec_cx_pkt_t *pkt = nullptr;
	while ((pkt = vpx_codec_get_cx_data(&(av->codec), &iter)) != nullptr)
	{
		if (pkt->kind == VPX_CODEC_CX_FRAME_PKT)
		{
			const uint64_t frameTime = pkt->data.frame.pts * 1000000000ll / av->fps;
			// fprintf(stderr, "video pts = %lld, frameTime = %lld, key = %d\n", pkt->data.frame.pts, frameTime, (pkt->data.frame.flags & VPX_FRAME_IS_KEY));
			if (frameTime < last_written_timestamp)
			{
				// if this happens often, increase min_queue_time in DequeueFrames
				fprintf(stderr, "Video packet delivered too late (%lld > %lld)\n", last_written_timestamp, frameTime);
			}
			else if (video_queue.find(frameTime) != video_queue.end())
			{
				fprintf(stderr, "Video packet has duplicate timestamp (%lld)\n", frameTime);
			}
			else
			{
				video_queue[frameTime] = std::make_unique<StoredFrame>(static_cast<const uint8 *>(pkt->data.frame.buf), pkt->data.frame.sz, frameTime, pkt->data.frame.flags & VPX_FRAME_IS_KEY);
			}
		}
	}
}

void Movie::EncodeAudio(bool last)
{
	// feed data into vorbis
	if (audiobuf.size() >= in_bps * 2)
	{
		size_t samples = audiobuf.size() / (in_bps * 2);
		float **buffer = vorbis_analysis_buffer(&(av->vd), samples);
		
		ALCint fmt = OpenALManager::Get()->GetRenderingFormat();
		if (fmt == ALC_SHORT_SOFT)
		{
			int16_t *shortData = reinterpret_cast<int16_t *>(audiobuf.data());
			for (size_t i = 0; i < samples; ++i)
			{
				buffer[0][i] = shortData[i*2] / 32768.f;
				buffer[1][i] = shortData[i*2+1] / 32768.f;
			}
		}
		else if (fmt == ALC_FLOAT_SOFT)
		{
			float *floatData = reinterpret_cast<float *>(audiobuf.data());
			for (size_t i = 0; i < samples; ++i)
			{
				buffer[0][i] = floatData[i*2];
				buffer[1][i] = floatData[i*2+1];
			}
		}
		else if (fmt == ALC_INT_SOFT)
		{
			int32_t *intData = reinterpret_cast<int32_t *>(audiobuf.data());
			for (size_t i = 0; i < samples; ++i)
			{
				buffer[0][i] = intData[i*2] / 2147483647.f;
				buffer[1][i] = intData[i*2+1] / 2147483647.f;
			}
		}
		else if (fmt == ALC_UNSIGNED_BYTE_SOFT)
		{
			uint8_t *ubyteData = reinterpret_cast<uint8_t *>(audiobuf.data());
			for (size_t i = 0; i < samples; ++i)
			{
				buffer[0][i] = (ubyteData[i*2] / 128.f) - 1.f;
				buffer[1][i] = (ubyteData[i*2+1] / 128.f) - 1.f;
			}
		}
		vorbis_analysis_wrote(&(av->vd), samples);
	}
	
	if (last)
	{
		// signal end of data
		vorbis_analysis_wrote(&(av->vd), 0);
	}
	
	// feed vorbis packets into webm
	while (vorbis_analysis_blockout(&(av->vd), &(av->vb)) == 1)
	{
		vorbis_analysis(&(av->vb), NULL);
		vorbis_bitrate_addblock(&(av->vb));
		while (vorbis_bitrate_flushpacket(&(av->vd), &(av->op)))
		{
			// vorbis_granule_time provides seconds, mkv wants nanoseconds
			double packetTime = vorbis_granule_time(&(av->vd), av->op.granulepos);
			uint64_t frameTime = packetTime * 1000000000ll;
//			fprintf(stderr, "audio packetTime = %.6f, frameTime = %lld\n", packetTime, frameTime);
			if (frameTime < last_written_timestamp)
			{
				// if this happens often, increase min_queue_time in DequeueFrames
				fprintf(stderr, "Audio packet delivered too late (%lld > %lld)\n", last_written_timestamp, frameTime);
			}
			else if (audio_queue.find(frameTime) != audio_queue.end())
			{
				fprintf(stderr, "Audio packet has duplicate timestamp (%lld)\n", frameTime);
			}
			else
			{
				current_audio_timestamp = frameTime;
				audio_queue[frameTime] = std::make_unique<StoredFrame>(av->op.packet, av->op.bytes, frameTime, false);
			}
		}
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
		DequeueFrames(false);
		
		SDL_SemPost(fillReady);
	}
}

void Movie::DequeueFrame(FrameMap &queue, uint64_t tracknum)
{
	auto it = queue.begin();
	if (!av->segment->AddFrame(it->second->buf.data(), it->second->buf.size(), tracknum, it->second->timestamp, it->second->keyframe))
	{
		fprintf(stderr, "Frame rejected (track %lld, ts %lld)\n", tracknum, it->second->timestamp);
	}
	else
	{
		last_written_timestamp = it->second->timestamp;
	}
	queue.erase(it);
}

void Movie::DequeueFrames(bool last)
{
	// Rules for frame ordering:
	// - frame timecodes must monotonically increase
	// - audio should come before video when both have the same timecode
	// Clustering rules should be handled by libwebm.
	
	// Increase min_queue_size if an encoder needs more flexibility
	// to deliver frames out of order.
	size_t min_queue_size = last ? 1 : 2;
	while (video_queue.size() >= min_queue_size && audio_queue.size() >= min_queue_size)
	{
		uint64_t next_video_ts = video_queue.begin()->first;
		uint64_t next_audio_ts = audio_queue.begin()->first;
		assert(next_video_ts >= last_written_timestamp);
		assert(next_audio_ts >= last_written_timestamp);
		if (next_video_ts < next_audio_ts)
		{
			DequeueFrame(video_queue, av->vtrack->number());
		}
		else
		{
			DequeueFrame(audio_queue, av->atrack->number());
		}
	}
	if (last)
	{
		// flush whichever queue was not already emptied
		while (audio_queue.size())
		{
			DequeueFrame(audio_queue, av->atrack->number());
		}
		while (video_queue.size())
		{
			DequeueFrame(video_queue, av->vtrack->number());
		}
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
		int yuvRet = 0;
		SDL_Surface *video = MainScreenSurface();
		if (video->format == temp_surface->format)
		{
			// skip copy and read straight from main surface
			yuvRet = libyuv::ARGBToI420(static_cast<const uint8_t *>(video->pixels) + view_rect.x*4 + view_rect.y*video->pitch, video->pitch, av->yuv->planes[0], av->yuv->stride[0], av->yuv->planes[1], av->yuv->stride[1], av->yuv->planes[2], av->yuv->stride[2], view_rect.w, view_rect.h);
		}
		else
		{
			SDL_BlitSurface(video, &view_rect, temp_surface, NULL);
			yuvRet = libyuv::ARGBToI420(static_cast<const uint8_t *>(temp_surface->pixels), temp_surface->pitch, av->yuv->planes[0], av->yuv->stride[0], av->yuv->planes[1], av->yuv->stride[1], av->yuv->planes[2], av->yuv->stride[2], view_rect.w, view_rect.h);
		}
		if (yuvRet)
		{
			fprintf(stderr, "libyuv error %d in Movie::AddFrame (SDL)\n", yuvRet);
		}

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

		// Convert pixel buffer (which is upside-down) to YUV
		int yuvRet = libyuv::ARGBToI420(videobuf.data(), view_rect.w * 4, av->yuv->planes[0], av->yuv->stride[0], av->yuv->planes[1], av->yuv->stride[1], av->yuv->planes[2], av->yuv->stride[2], view_rect.w, -view_rect.h);
		if (yuvRet)
		{
			fprintf(stderr, "libyuv error %d in Movie::AddFrame (OpenGL)\n", yuvRet);
		}
	}
#endif
	
	int bytes = audiobuf.size();
    int frameSize = 2 * in_bps;
    auto oldVol = OpenALManager::Get()->GetMasterVolume();
    OpenALManager::Get()->SetMasterVolume(SoundManager::From_db(sound_preferences->video_export_volume_db));
    OpenALManager::Get()->GetPlayBackAudio(&audiobuf.front(), bytes / frameSize);
    OpenALManager::Get()->SetMasterVolume(oldVol);
	
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
	if (av->yuv)
	{
		vpx_img_free(av->yuv);
		av->yuv = nullptr;
	}
    if (av->inited)
    {
        // flush video and audio
        EncodeVideo(true);
        EncodeAudio(true);
		DequeueFrames(true);
		
		av->segment->Finalize();
		vorbis_block_clear(&(av->vb));
		vorbis_dsp_clear(&(av->vd));
		vorbis_comment_clear(&(av->vc));
		vorbis_info_clear(&(av->vi));
		vpx_codec_destroy(&(av->codec));
        av->inited = false;
    }
	av->atrack = nullptr;
	av->vtrack = nullptr;
	if (av->segment)
	{
		delete av->segment;	
		av->segment = nullptr;
	}
	if (av->writer)
	{
		av->writer->Close();
		delete av->writer;
		av->writer = nullptr;
	}

	moviefile = "";
    if (OpenALManager::Get()) {
        OpenALManager::Get()->ToggleDeviceMode(false);
        OpenALManager::Get()->Start();
    }
}

#endif
