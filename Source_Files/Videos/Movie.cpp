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
#if defined(_MSC_VER)
#define NOMINMAX
#endif
#include <windows.h>
#endif

#ifndef FILM_EXPORT

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
uint64_t Movie::GetCurrentAudioTimeStamp() { return 0; }
Movie::Movie() {}

#else

#include <vorbis/vorbisenc.h>
#include <libyuv/convert.h>
#include <vpx/vpx_encoder.h>
#include <vpx/vp8cx.h>

#include <ebml/EbmlTypes.h>
#include <ebml/EbmlHead.h>
#include <ebml/EbmlVoid.h>
#include <ebml/EbmlSubHead.h>
#include <matroska/KaxBlock.h>
#include <matroska/KaxSegment.h>
#include <matroska/KaxSeekHead.h>
#include <matroska/KaxTracks.h>
#include <matroska/KaxInfo.h>
#include <matroska/KaxCluster.h>
#include <matroska/KaxTrackAudio.h>
#include <matroska/KaxTrackVideo.h>
#include <matroska/KaxCues.h>

using namespace libmatroska;

#define NANOS_PER_SECOND 1000000000ll
#define WEBM_TIMECODE_SCALE 1000000ll
#define VIDEO_TRACK_NUMBER 1
#define AUDIO_TRACK_NUMBER 2

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

class MovieFileWrapper : public IOCallback
{
public:
	~MovieFileWrapper() {
		this->close();
	}
	bool _open(const char *filename) {
		fp = SDL_RWFromFile(filename, "wb");
		return !!fp;
	}
	uint32 read(void *Buffer, size_t Size) {
		return SDL_RWread(fp, Buffer, 1, Size);
	}
	void setFilePointer(int64 Offset, seek_mode Mode=seek_beginning) {
		SDL_RWseek(fp, Offset, Mode);
	}
	size_t write(const void *Buffer, size_t Size) {
		return SDL_RWwrite(fp, Buffer, 1, Size);
	}
	uint64 getFilePointer() {
		return SDL_RWtell(fp);
	}
	void close() {
		if (fp) {
			SDL_RWclose(fp);
			fp = nullptr;
		}
	}
private:
	SDL_RWops *fp;
};

struct libav_vars {
    bool inited;
    
	// libmatroska data
	MovieFileWrapper *fileio;
	KaxSegment *k_segment;
	KaxInfo *k_info;
	KaxDuration *k_duration;
	KaxSeekHead *k_seek_head;
	EbmlVoid *k_void;
	KaxCues *k_cues;
	KaxTracks *k_tracks;
	KaxTrackEntry *k_vtrack;
	KaxTrackEntry *k_atrack;
	KaxCluster *k_cluster;
	uint64_t total_duration;
	KaxBlockGroup *k_block_group;
	
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
	int64_t prev_video_pts;
    size_t audio_counter;
	int64_t prev_audio_packetno;
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
	
	// set up matroska headers
	av->fileio = new MovieFileWrapper();
	if (!(av->fileio) || !(av->fileio->_open(moviefile.c_str()))) { ThrowUserError("Could not create webm file at " + moviefile); return false; }
	av->k_segment = new KaxSegment();
	
	EbmlHead FileHead;
	GetChild<EDocType>(FileHead).SetValue("webm");
	GetChild<EDocTypeVersion>(FileHead).SetValue(4);
	GetChild<EDocTypeReadVersion>(FileHead).SetValue(2);
	FileHead.Render(*(av->fileio));
	av->k_segment->WriteHead(*(av->fileio), 8);
	
	av->k_seek_head = new KaxSeekHead();
	av->k_void = new EbmlVoid();
	av->k_void->SetSize(4096);
	av->k_void->Render(*(av->fileio));

	av->k_info = &GetChild<KaxInfo>(*(av->k_segment));
	UTFstring verustr;
	verustr.SetUTF8(std::string(A1_DISPLAY_NAME " " A1_VERSION_STRING));
	GetChild<KaxWritingApp>(*(av->k_info)).SetValue(verustr);
	GetChild<KaxMuxingApp>(*(av->k_info)).SetValue(verustr);
	GetChild<KaxDateUTC>(*(av->k_info)).SetEpochDate(time(NULL));
	GetChild<KaxTimecodeScale>(*(av->k_info)).SetValue(WEBM_TIMECODE_SCALE);

	// we will set the actual duration at the end
	av->k_duration = &GetChild<KaxDuration>(*(av->k_info));
	av->k_duration->SetPrecision(EbmlFloat::FLOAT_64);
	av->k_duration->SetValue(0.0);
	
	av->k_info->Render(*(av->fileio));
	av->k_seek_head->IndexThis(*(av->k_info), *(av->k_segment));
	av->k_cues = &GetChild<KaxCues>(*(av->k_segment));
	av->k_cues->SetGlobalTimecodeScale(WEBM_TIMECODE_SCALE);
	
	av->k_tracks = &GetChild<KaxTracks>(*(av->k_segment));
	av->k_cluster = nullptr;
	av->total_duration = 0;
	// end matroska headers	

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
	av->k_vtrack = &GetChild<KaxTrackEntry>(*(av->k_tracks));
	GetChild<KaxTrackNumber>(*(av->k_vtrack)).SetValue(VIDEO_TRACK_NUMBER);
	GetChild<KaxTrackUID>(*(av->k_vtrack)).SetValue(rand());
	av->k_vtrack->SetGlobalTimecodeScale(WEBM_TIMECODE_SCALE);
	GetChild<KaxCodecID>(*(av->k_vtrack)).SetValue("V_VP8");
	GetChild<KaxTrackType>(*(av->k_vtrack)).SetValue(track_video);
	KaxTrackVideo *vtinfo = &GetChild<KaxTrackVideo>(*(av->k_vtrack));
	GetChild<KaxVideoPixelWidth>(*vtinfo).SetValue(view_rect.w);
	GetChild<KaxVideoPixelHeight>(*vtinfo).SetValue(view_rect.h);
	GetChild<KaxVideoDisplayWidth>(*vtinfo).SetValue(view_rect.w);
	GetChild<KaxVideoDisplayHeight>(*vtinfo).SetValue(view_rect.h);
	
	// set up vpx
	vpx_codec_enc_cfg_t cfg;
	vpx_codec_iface_t *cif = vpx_codec_vp8_cx();
	if (vpx_codec_enc_config_default(cif, &cfg, 0)) { ThrowUserError("Failed to get default codec config"); return false; }
	cfg.g_w = view_rect.w;
	cfg.g_h = view_rect.h;
	cfg.g_threads = get_cpu_count();
	cfg.g_timebase.num = 1;
	cfg.g_timebase.den = fps;
	cfg.rc_end_usage = vq == 100 ? VPX_VBR : VPX_CQ;
	cfg.rc_target_bitrate = bitrate / 1000;
	cfg.rc_max_quantizer = ScaleQuality(vq, 63, 63, 50);
	cfg.rc_min_quantizer = ScaleQuality(vq, 10, 4, 0);
	cfg.g_lag_in_frames = 0;	// deliver encoded frames in order
	if (cfg.kf_mode != VPX_KF_AUTO) {	// ensure key frames are created
		cfg.kf_mode = VPX_KF_AUTO;
		cfg.kf_min_dist = 0;
		cfg.kf_max_dist = 128;
	}
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
	av->k_atrack = &GetNextChild<KaxTrackEntry>(*(av->k_tracks), *(av->k_vtrack));
	GetChild<KaxTrackNumber>(*(av->k_atrack)).SetValue(AUDIO_TRACK_NUMBER);
	GetChild<KaxTrackUID>(*(av->k_atrack)).SetValue(rand());
	av->k_atrack->SetGlobalTimecodeScale(WEBM_TIMECODE_SCALE);
	GetChild<KaxCodecID>(*(av->k_atrack)).SetValue("A_VORBIS");
	GetChild<KaxTrackType>(*(av->k_atrack)).SetValue(track_audio);
	KaxTrackAudio *atinfo = &GetChild<KaxTrackAudio>(*(av->k_atrack));
	GetChild<KaxAudioChannels>(*atinfo).SetValue(2);
	GetChild<KaxAudioSamplingFreq>(*atinfo).SetValue(OpenALManager::Get()->GetFrequency());
	
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
	GetChild<KaxCodecPrivate>(*(av->k_atrack)).CopyBuffer(aprivatebuf.data(), aprivatebuf.size());

	vorbis_block_init(&(av->vd), &(av->vb));

	// write matroska track headers
	av->k_tracks->Render(*(av->fileio));
	av->k_seek_head->IndexThis(*(av->k_tracks), *(av->k_segment));
		
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

uint64_t Movie::GetCurrentAudioTimeStamp()
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
		if (vpx_codec_encode(&(av->codec), av->yuv, av->video_counter++, 1, 0, av->deadline))
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
			if (pkt->data.frame.pts < av->prev_video_pts) {
				fprintf(stderr, "Dropping out-of-order video packet: %lld < %lld\n", pkt->data.frame.pts, av->prev_video_pts);
				continue;
			}
			const uint64_t frameTime = pkt->data.frame.pts * NANOS_PER_SECOND / av->fps;
			video_queue.push(std::make_unique<StoredFrame>(static_cast<const uint8 *>(pkt->data.frame.buf), pkt->data.frame.sz, frameTime, NANOS_PER_SECOND / av->fps, pkt->data.frame.flags & VPX_FRAME_IS_KEY));
			av->prev_video_pts = pkt->data.frame.pts;
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
			if (av->op.packetno < av->prev_audio_packetno) {
				fprintf(stderr, "Dropping out-of-order audio packet: %lld < %lld\n", av->op.packetno, av->prev_audio_packetno);
				continue;
			}
			// vorbis_granule_time provides seconds, mkv wants nanoseconds
			double packetTime = vorbis_granule_time(&(av->vd), av->op.granulepos);
			uint64_t frameTime = packetTime * NANOS_PER_SECOND;
			audio_queue.push(std::make_unique<StoredFrame>(av->op.packet, av->op.bytes, current_audio_timestamp, frameTime - current_audio_timestamp, false));
			av->prev_audio_packetno = av->op.packetno;
			current_audio_timestamp = frameTime;
		}
	}
}

void Movie::EncodeThread()
{
	av->video_counter = 0;
	av->prev_video_pts = 0;
	av->audio_counter = 0;
	av->prev_audio_packetno = 0;
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

void Movie::DequeueFrame(FrameQueue &queue, uint64_t tracknum, bool start_cluster)
{
	std::unique_ptr<StoredFrame> frame = std::move(queue.front());
	queue.pop();
	
	if (start_cluster)
	{
		// end the current cluster
		if (av->k_cluster)
		{
			av->k_cluster->UpdateSize();
			av->k_cluster->Render(*(av->fileio), *(av->k_cues));
			av->k_seek_head->IndexThis(*(av->k_cluster), *(av->k_segment));
			av->k_cluster->ReleaseFrames();
			while (cached_frames.size())
			{
				cached_frames.pop();
			}
			av->k_cluster = nullptr;
		}
	}
	if (!av->k_cluster)
	{
		av->k_cluster = new KaxCluster();
		av->k_cluster->SetParent(*(av->k_segment));
		av->k_cluster->InitTimecode(frame->timestamp, 1);
		av->k_cluster->SetPreviousTimecode(av->total_duration, WEBM_TIMECODE_SCALE);
		GetChild<KaxClusterTimecode>(*(av->k_cluster)).SetValue(frame->timestamp);
	}
	
	KaxBlockBlob *blob = new KaxBlockBlob(BLOCK_BLOB_ALWAYS_SIMPLE);
	av->k_cluster->AddBlockBlob(blob);
	blob->SetParent(*(av->k_cluster));
	if (frame->keyframe)
	{
		av->k_cues->AddBlockBlob(*blob);
	}
	KaxTrackEntry *track = (tracknum == VIDEO_TRACK_NUMBER) ? av->k_vtrack : av->k_atrack;
	
	SimpleDataBuffer *buf = new SimpleDataBuffer(frame->buf.data(), frame->buf.size(), 0);
	blob->AddFrameAuto(*track, frame->timestamp, *buf, LACING_NONE);
	av->total_duration = std::max(av->total_duration, frame->timestamp + frame->duration);
	last_written_timestamp = frame->timestamp;
	cached_frames.push(std::move(frame));
}

void Movie::DequeueFrames(bool last)
{
	// Rules for frame ordering and clustering:
	// - frame timecodes must monotonically increase
	// - audio should come before video when both have the same timecode
	// - each key frame should be the first video frame of a cluster
	// - audio during key frame should be in the key frame's cluster
	while (video_queue.size() >= 1 && audio_queue.size() >= 1)
	{
		uint64_t next_video_start = video_queue.front()->timestamp;
		uint64_t next_video_end = next_video_start + video_queue.front()->duration;
		uint64_t next_audio_start = audio_queue.front()->timestamp;
		uint64_t next_audio_end = next_audio_start + audio_queue.front()->duration;
		assert(next_video_start >= last_written_timestamp);
		assert(next_audio_start >= last_written_timestamp);
		if (next_video_start < next_audio_start)
		{
			// We never start a cluster on a video frame, since
			// we should have started one when the timecode's
			// corresponding audio packet was added.
			DequeueFrame(video_queue, VIDEO_TRACK_NUMBER, false);
		}
		else if (next_audio_start < next_video_start && next_audio_end <= next_video_start)
		{
			// This audio does not overlap with the next video
			// frame, so it belongs in the existing cluster.
			DequeueFrame(audio_queue, AUDIO_TRACK_NUMBER, false);
		}
		else if (video_queue.front()->keyframe)
		{
			// We are heading into a new cluster. Wait until we
			// have all audio for the keyframe before draining
			// the queue, unless we're at the end.
			if (!last && (audio_queue.back()->timestamp + audio_queue.back()->duration) < next_video_end)
			{
				break;
			}
			DequeueFrame(audio_queue, AUDIO_TRACK_NUMBER, true);
			while (audio_queue.size() > 0 && audio_queue.front()->timestamp <= next_video_start)
			{
				DequeueFrame(audio_queue, AUDIO_TRACK_NUMBER, false);
			}
			DequeueFrame(video_queue, VIDEO_TRACK_NUMBER, false);
		}
		else
		{
			// Next video frame is not key, and we have audio
			// extending into the frame. Both belong in the
			// existing cluster.
			DequeueFrame(audio_queue, AUDIO_TRACK_NUMBER, false);
			DequeueFrame(video_queue, VIDEO_TRACK_NUMBER, false);
		}
	}
	if (last)
	{
		// flush whichever queue was not already emptied
		while (audio_queue.size())
		{
			DequeueFrame(audio_queue, AUDIO_TRACK_NUMBER, false);
		}
		while (video_queue.size())
		{
			DequeueFrame(video_queue, VIDEO_TRACK_NUMBER, video_queue.front()->keyframe);
		}
		// close final cluster
		if (av->k_cluster)
		{
			av->k_cluster->Render(*(av->fileio), *(av->k_cues));
			av->k_seek_head->IndexThis(*(av->k_cluster), *(av->k_segment));
			av->k_seek_head->UpdateSize();
			av->k_cluster->ReleaseFrames();
			while (cached_frames.size())
			{
				cached_frames.pop();
			}
			av->k_cluster = nullptr;
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
		
		if (av->k_cues->ListSize() > 0)
		{
			av->k_cues->Render(*(av->fileio));
			av->k_seek_head->IndexThis(*(av->k_cues), *(av->k_segment));
			av->k_seek_head->UpdateSize();
		}
		
		av->k_duration->SetValue(av->total_duration / static_cast<double>(WEBM_TIMECODE_SCALE));
		av->k_info->UpdateSize();
		uint64_t oldPos = av->fileio->getFilePointer();
		av->fileio->setFilePointer(av->k_info->GetElementPosition());
		av->k_info->Render(*(av->fileio));
		av->fileio->setFilePointer(oldPos);

		if (av->k_seek_head->GetSize() > av->k_void->GetSize())
		{
			fprintf(stderr, "Seek head size (%lld) can't fit in reserved size (%lld), skipping\n", av->k_seek_head->GetSize(), av->k_void->GetSize());
		}
		else
		{
			av->k_void->ReplaceWith(*(av->k_seek_head), *(av->fileio));
		}
		
		av->k_segment->ForceSize(av->fileio->getFilePointer() - av->k_segment->GetElementPosition() - av->k_segment->HeadSize());
		av->k_segment->OverwriteHead(*(av->fileio));
		
		av->fileio->close();
		delete av->fileio;
		av->fileio = nullptr;
		av->k_cues = nullptr;
		av->k_atrack = nullptr;
		av->k_tracks = nullptr;
		av->k_vtrack = nullptr;
		av->k_duration = nullptr;

		delete av->k_seek_head;
		av->k_seek_head = nullptr;
		delete av->k_void;
		av->k_void = nullptr;
		delete av->k_segment;
		av->k_segment = nullptr;
		
		vorbis_block_clear(&(av->vb));
		vorbis_dsp_clear(&(av->vd));
		vorbis_comment_clear(&(av->vc));
		vorbis_info_clear(&(av->vi));
		vpx_codec_destroy(&(av->codec));
        av->inited = false;
    }

	moviefile = "";
    if (OpenALManager::Get()) {
        OpenALManager::Get()->ToggleDeviceMode(false);
        OpenALManager::Get()->Start();
    }
}

#endif
