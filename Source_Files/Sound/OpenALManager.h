#ifndef __OPENAL_MANAGER_H
#define __OPENAL_MANAGER_H

#include "MusicPlayer.h"
#include "SoundPlayer.h"
#include "StreamPlayer.h"
#include "SoundManager.h"
#include <queue>
#include <thread>
#include <unordered_map>

#if defined (_MSC_VER) && !defined (M_PI)
#define _USE_MATH_DEFINES
#endif 

#include <math.h>

#define LOAD_PROC(T, x)  ((x) = (T)alGetProcAddress(#x))

constexpr float abortAmplitudeThreshold = MAXIMUM_SOUND_VOLUME / 6.f / 256;
constexpr float angleConvert = 360 / float(FULL_CIRCLE);
constexpr float degreToRadian = M_PI / 180.f;

#ifdef  HAVE_FFMPEG
#ifdef __cplusplus
extern "C"
{
#endif
#include "libavutil/samplefmt.h"
#ifdef __cplusplus
}
#endif

const std::unordered_map<ALCint, AVSampleFormat> mapping_openal_ffmpeg = {
	{ALC_FLOAT_SOFT, AV_SAMPLE_FMT_FLT},
	{ALC_INT_SOFT, AV_SAMPLE_FMT_S32},
	{ALC_SHORT_SOFT, AV_SAMPLE_FMT_S16},
	{ALC_UNSIGNED_BYTE_SOFT, AV_SAMPLE_FMT_U8}
};

#endif //  HAVE_FFMPEG

struct AudioParameters {
	int rate;
	bool stereo;
	bool balance_rewind;
	bool hrtf;
	bool sounds_3d;
	float volume;
};

class OpenALManager {
public:
	static OpenALManager* Get();
	static bool Init(SoundManager::AudioBackend backend, AudioParameters parameters);
	static float From_db(float db, bool music = false) { return db <= (SoundManager::MINIMUM_VOLUME_DB / (music ? 2 : 1)) ? 0 : std::pow(10.f, db / 20.f); }
	virtual void Start();
	virtual void Stop();
	void StopAllPlayers();
	std::shared_ptr<SoundPlayer> PlaySound(const SoundInfo& header, const SoundData& data, SoundParameters parameters);
	std::shared_ptr<SoundPlayer> PlaySound(LoadedResource& rsrc, SoundParameters parameters);
	std::shared_ptr<MusicPlayer> PlayMusic(StreamDecoder* decoder);
	std::shared_ptr<StreamPlayer> PlayStream(uint8* data, int length, int rate, bool stereo, bool sixteen_bit);
	std::shared_ptr<CallBackableStreamPlayer> PlayStream(CallBackStreamPlayer callback, int length, int rate, bool stereo, bool sixteen_bit);
	void StopSound(short sound_identifier, short source_identifier);
	AudioPlayer::AudioSource PickAvailableSource(const AudioPlayer* player);
	std::shared_ptr<SoundPlayer> GetSoundPlayer(short identifier, short source_identifier, bool sound_identifier_only = false) const;
	void UpdateListener(world_location3d listener);
	world_location3d GetListener() const { return listener_location; }
	void SetDefaultVolume(float volume);
	float GetComputedVolume(bool filtered = true) const { return default_volume * (filters_volume.empty() || !filtered ? 1 : filters_volume.front()); }
	virtual void SetUpRecordingDevice();
	virtual void SetUpPlayingDevice();
	int GetFrequency() const;
	void GetPlayBackAudio(uint8* data, int length);
	void ApplyVolumeFilter(float volume_filter) { filters_volume.push(volume_filter); }
	void RemoveVolumeFilter() { if(!filters_volume.empty()) filters_volume.pop(); }
	bool Support_HRTF_Toggling() const;
	bool Is_HRTF_Enabled() const;
	bool IsBalanceRewindSound() const { return audio_parameters.balance_rewind; }
	ALCint GetRenderingFormat() const { return rendering_format; }
private:
	static OpenALManager* instance;
	SoundManager::AudioBackend audio_backend;
	ALCdevice* p_ALCDevice = nullptr;
	ALCcontext* p_ALCContext = nullptr;
	OpenALManager(AudioParameters parameters);
	virtual ~OpenALManager();
	std::queue<float> filters_volume;
	std::atomic<float> default_volume;
	world_location3d listener_location = {};
	void CleanEverything();
	bool GenerateSources();
	virtual bool OpenPlayingDevice();
	bool OpenRecordingDevice();
	bool CloseDevice();
	void ConsumeAudioQueue();
	void ProcessAudioQueue();
	void QueueAudio(std::shared_ptr<AudioPlayer> audioPlayer);
	bool is_using_recording_device = false;
	std::atomic<bool> consuming_audio_enable = { false };
	std::thread process_consuming_audio;
	std::queue<AudioPlayer::AudioSource> sources_pool;
	std::deque<std::shared_ptr<AudioPlayer>> audio_players;
	int GetBestOpenALRenderingFormat(ALCint channelsType);
	void RetrieveSource(std::shared_ptr<AudioPlayer> player);

	/* Loopback device functions (SDL backend and video export) */
	static LPALCLOOPBACKOPENDEVICESOFT alcLoopbackOpenDeviceSOFT;
	static LPALCISRENDERFORMATSUPPORTEDSOFT alcIsRenderFormatSupportedSOFT;
	static LPALCRENDERSAMPLESSOFT alcRenderSamplesSOFT;

	class SDLBackend;
	AudioParameters audio_parameters;
	ALCint rendering_format = 0;

	/* format type we supports for mixing / rendering
	* those are used from the first to the last of the list
	  and we stop when our device support the corresponding format 
	  Short is first, because there is no real purpose to use other format now */
	const std::vector<ALCint> formatType = {
		ALC_SHORT_SOFT,
		ALC_FLOAT_SOFT,
		ALC_INT_SOFT,
		ALC_UNSIGNED_BYTE_SOFT
	};
};

class OpenALManager::SDLBackend : public OpenALManager {
public:
	SDLBackend(AudioParameters parameters);
	~SDLBackend();

	void SetUpRecordingDevice();
	void SetUpPlayingDevice();
	void Start();
	void Stop();
private:
	const static int number_samples = 512;
	SDL_AudioSpec desired, obtained;
	static void MixerCallback(void* usr, uint8* stream, int len);
	bool OpenPlayingDevice();

	//should probably replace those by boost bimap
	const std::unordered_map<ALCint, int> mapping_openal_sdl = {
		{ALC_FLOAT_SOFT, AUDIO_F32SYS},
		{ALC_INT_SOFT, AUDIO_S32SYS},
		{ALC_SHORT_SOFT, AUDIO_S16SYS},
		{ALC_UNSIGNED_BYTE_SOFT, AUDIO_U8}
	};

	const std::unordered_map<int, ALCint> mapping_sdl_openal = {
		{AUDIO_F32SYS, ALC_FLOAT_SOFT},
		{AUDIO_S32SYS, ALC_INT_SOFT},
		{AUDIO_S16SYS, ALC_SHORT_SOFT},
		{AUDIO_U8, ALC_UNSIGNED_BYTE_SOFT}
	};
};

#endif