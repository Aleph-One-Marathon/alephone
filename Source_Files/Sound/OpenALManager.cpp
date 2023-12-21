#include "OpenALManager.h"
#include "sound_definitions.h"
#include "Logging.h"

LPALCLOOPBACKOPENDEVICESOFT OpenALManager::alcLoopbackOpenDeviceSOFT;
LPALCISRENDERFORMATSUPPORTEDSOFT OpenALManager::alcIsRenderFormatSupportedSOFT;
LPALCRENDERSAMPLESSOFT OpenALManager::alcRenderSamplesSOFT;
LPALGETSTRINGISOFT OpenALManager::alGetStringiSOFT;
LPALGENFILTERS OpenALManager::alGenFilters;
LPALDELETEFILTERS OpenALManager::alDeleteFilters;
LPALFILTERF OpenALManager::alFilterf;
LPALFILTERI OpenALManager::alFilteri;

OpenALManager* OpenALManager::instance = nullptr;

bool OpenALManager::Init(const AudioParameters& parameters) {

	if (instance) { //Don't bother recreating all the OpenAL context if nothing changed for it
		if (parameters.hrtf != instance->audio_parameters.hrtf || parameters.rate != instance->audio_parameters.rate
			|| parameters.stereo != instance->audio_parameters.stereo || parameters.sample_frame_size != instance->audio_parameters.sample_frame_size) {

			Shutdown();

		} else {
			instance->UpdateParameters(parameters);
			return true;
		}
	} else {
		if (alcIsExtensionPresent(NULL, "ALC_SOFT_loopback")) {
#define LOAD_PROC(T, x)  ((x) = (T)alGetProcAddress(#x))
			LOAD_PROC(LPALCLOOPBACKOPENDEVICESOFT, alcLoopbackOpenDeviceSOFT);
			LOAD_PROC(LPALCISRENDERFORMATSUPPORTEDSOFT, alcIsRenderFormatSupportedSOFT);
			LOAD_PROC(LPALCRENDERSAMPLESSOFT, alcRenderSamplesSOFT);
			LOAD_PROC(LPALGETSTRINGISOFT, alGetStringiSOFT);
			LOAD_PROC(LPALGENFILTERS, alGenFilters);
			LOAD_PROC(LPALDELETEFILTERS, alDeleteFilters);
			LOAD_PROC(LPALFILTERI, alFilteri);
			LOAD_PROC(LPALFILTERF, alFilterf);
#undef LOAD_PROC
		} else {
			logError("ALC_SOFT_loopback extension is not supported"); //Should never be the case as long as >= OpenAL 1.14
			return false;
		}
	}

	instance = new OpenALManager(parameters);
	return instance->OpenDevice() && instance->GenerateSources() && instance->GenerateEffects();
}

void OpenALManager::ProcessAudioQueue() {

	std::shared_ptr<AudioPlayer> audioPlayer;
	while (audio_players_shared.pop(audioPlayer)) {
		audio_players_queue.push_back(audioPlayer);
	}

	UpdateListener();
	for (int i = 0; i < audio_players_queue.size(); i++) {

		auto audio = audio_players_queue.front();
		bool mustStillPlay = !audio->stop_signal && audio->AssignSource() && audio->Update() && audio->Play();

		audio_players_queue.pop_front();

		if (!mustStillPlay) {
			RetrieveSource(audio);
			continue;
		}

		audio_players_queue.push_back(audio); //We have just processed a part of the data for you, now wait your next turn
	}
}

//we update our listener's position for 3D sounds
void OpenALManager::UpdateListener() {

	if (!audio_parameters.sounds_3d) return;

	const auto& listener = listener_location.Get();

	auto yaw = listener.yaw * angleConvert;
	auto pitch = listener.pitch * angleConvert;

	ALfloat u = std::cos(degreToRadian * yaw) * std::cos(degreToRadian * pitch);
	ALfloat	v = std::sin(degreToRadian * yaw) * std::cos(degreToRadian * pitch);
	ALfloat	w = std::sin(degreToRadian * pitch);

	//OpenAL uses the same coordinate system as OpenGL, so we have to swap Z <-> Y
	ALfloat vectorDirection[] = { u, w, v, 0, 1, 0 };

	ALfloat position[] = { (float)(listener.point.x) / WORLD_ONE,
						   (float)(listener.point.z) / WORLD_ONE,
						   (float)(listener.point.y) / WORLD_ONE };

	ALfloat velocity[] = { (float)listener.velocity.i / WORLD_ONE,
						   (float)listener.velocity.k / WORLD_ONE,
						   (float)listener.velocity.j / WORLD_ONE };

	alListenerfv(AL_ORIENTATION, vectorDirection);
	alListenerfv(AL_POSITION, position);
	alListenerfv(AL_VELOCITY, velocity);
}

void OpenALManager::SetMasterVolume(float volume) {
	volume = std::min(1.f, std::max(volume, 0.f));
	if (master_volume == volume) return;
	master_volume = volume;
	ResyncPlayers();
}

void OpenALManager::ResyncPlayers() {
	SDL_LockAudio();
	for (auto& player : audio_players_queue) player->is_sync_with_al_parameters = false;
	SDL_UnlockAudio();
}

void OpenALManager::Start() {
	process_audio_active = true;
	SDL_PauseAudio(is_using_recording_device); //Start playing only if not recording playback
}

void OpenALManager::Stop() {
	SDL_PauseAudio(true);
	StopAllPlayers();
	process_audio_active = false;
}

void OpenALManager::ToggleDeviceMode(bool recording_device) {
	is_using_recording_device = recording_device;
	SDL_PauseAudio(is_using_recording_device);
}

std::shared_ptr<SoundPlayer> OpenALManager::PlaySound(const Sound& sound, const SoundParameters& parameters) {
	if (!process_audio_active) return std::shared_ptr<SoundPlayer>();
	auto soundPlayer = std::make_shared<SoundPlayer>(sound, parameters);
	audio_players_shared.push(soundPlayer);
	return soundPlayer;
}

std::shared_ptr<MusicPlayer> OpenALManager::PlayMusic(std::shared_ptr<StreamDecoder> decoder, MusicParameters parameters) {
	if (!process_audio_active) return std::shared_ptr<MusicPlayer>();
	auto musicPlayer = std::make_shared<MusicPlayer>(decoder, parameters);
	audio_players_shared.push(musicPlayer);
	return musicPlayer;
}

//Used for video playback
std::shared_ptr<StreamPlayer> OpenALManager::PlayStream(CallBackStreamPlayer callback, int length, int rate, bool stereo, AudioFormat audioFormat) {
	if (!process_audio_active) return std::shared_ptr<StreamPlayer>();
	auto streamPlayer = std::make_shared<StreamPlayer>(callback, length, rate, stereo, audioFormat);
	audio_players_shared.push(streamPlayer);
	return streamPlayer;
}

//It's not a good idea generating dynamically a new source for each player
//It's slow so it's better having a pool, also we already know the max amount
//of supported simultaneous playing sources for the device
std::unique_ptr<AudioPlayer::AudioSource> OpenALManager::PickAvailableSource(const AudioPlayer& audioPlayer) {
	if (sources_pool.empty()) {
		const auto& victimPlayer = *std::min_element(audio_players_queue.begin(), audio_players_queue.end(),
			[](const std::shared_ptr<AudioPlayer>& a, const std::shared_ptr<AudioPlayer>& b)
			{  return a->audio_source && a->GetPriority() < b->GetPriority(); });

		return victimPlayer->GetPriority() < audioPlayer.GetPriority() ? victimPlayer->RetrieveSource() : nullptr;
	}

	auto source = std::move(sources_pool.front());
	sources_pool.pop();
	return source;
}

void OpenALManager::StopAllPlayers() {
	SDL_LockAudio();

	for (auto& audioPlayer : audio_players_queue) RetrieveSource(audioPlayer);
	audio_players_queue.clear();

	std::shared_ptr<AudioPlayer> audioPlayer;
	while (audio_players_shared.pop(audioPlayer)) {
		audioPlayer->is_active = false;
	}

	SDL_UnlockAudio();
}

void OpenALManager::RetrieveSource(const std::shared_ptr<AudioPlayer>& player) {
	auto audioSource = player->RetrieveSource();
	if (audioSource) sources_pool.push(std::move(audioSource));
	player->is_active = false;
}

//this is used with the recording device and this allows OpenAL to
//not output the audio once it has mixed it but instead, makes 
//the mixed data available with alcRenderSamplesSOFT
void OpenALManager::GetPlayBackAudio(uint8* data, int length) {
	ProcessAudioQueue();
	alcRenderSamplesSOFT(p_ALCDevice, data, length);
}

//This return true if the device supports switching from hrtf enabled <-> hrtf disabled
bool OpenALManager::Support_HRTF_Toggling() const {
	ALCint hrtfStatus;
	alcGetIntegerv(p_ALCDevice, ALC_HRTF_STATUS_SOFT, 1, &hrtfStatus);

	switch (hrtfStatus) {
		case ALC_HRTF_DENIED_SOFT:
		case ALC_HRTF_UNSUPPORTED_FORMAT_SOFT:
		case ALC_HRTF_REQUIRED_SOFT:
			return false;
		default:
			return true;
	}
}

bool OpenALManager::Is_HRTF_Enabled() const {
	ALCint hrtfStatus;
	alcGetIntegerv(p_ALCDevice, ALC_HRTF_SOFT, 1, &hrtfStatus);
	return hrtfStatus;
}

bool OpenALManager::OpenDevice() {
	if (p_ALCDevice) return true;

	p_ALCDevice = alcLoopbackOpenDeviceSOFT(nullptr);
	if (!p_ALCDevice) {
		logError("Could not open audio loopback device");
		return false;
	}

	ALCint channelsType = audio_parameters.stereo ? ALC_STEREO_SOFT : ALC_MONO_SOFT;
	ALCint format = rendering_format ? rendering_format : GetBestOpenALRenderingFormat(channelsType);

	rendering_format = format;

	if (format) {
		ALCint attrs[] = {
			ALC_FORMAT_TYPE_SOFT,     format,
			ALC_FORMAT_CHANNELS_SOFT, channelsType,
			ALC_FREQUENCY,            audio_parameters.rate,
			ALC_HRTF_SOFT,			  audio_parameters.hrtf,
			0,
		};

		p_ALCContext = alcCreateContext(p_ALCDevice, attrs);
		if (!p_ALCContext) {
			logError("Could not create audio context from loopback device");
			return false;
		}

		if (!alcMakeContextCurrent(p_ALCContext)) {
			logError("Could not make audio context from loopback device current");
			return false;
		}

		return true;
	}

	return false;
}

bool OpenALManager::CloseDevice() {
	if (!alcMakeContextCurrent(nullptr)) {
		logError("Could not remove current audio context");
		return false;
	}

	if (p_ALCContext) {
		alcDestroyContext(p_ALCContext);
		p_ALCContext = nullptr;
	}

	if (p_ALCDevice) {
		if (!alcCloseDevice(p_ALCDevice)) {
			logError("Could not close audio device");
			return false;
		}

		p_ALCDevice = nullptr;
	}

	return true;
}

bool OpenALManager::GenerateEffects() {
	alGenFilters(1, &low_pass_filter);
	alFilteri(low_pass_filter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
	alFilterf(low_pass_filter, AL_LOWPASS_GAIN, 1.f);
	alFilterf(low_pass_filter, AL_LOWPASS_GAINHF, 1.f);
	return alGetError() == AL_NO_ERROR;
}

ALuint OpenALManager::GetLowPassFilter(float highFrequencyGain) const {
	alFilterf(low_pass_filter, AL_LOWPASS_GAINHF, highFrequencyGain);
	return low_pass_filter;
}

bool OpenALManager::GenerateSources() {

	/* how many simultaneous sources are supported on this device ? */
	int monoSources, stereoSources;
	alcGetIntegerv(p_ALCDevice, ALC_MONO_SOURCES, 1, &monoSources);
	alcGetIntegerv(p_ALCDevice, ALC_STEREO_SOURCES, 1, &stereoSources);
	int nbSources = monoSources + stereoSources;

	std::vector<ALuint> sources_id(nbSources);
	alGenSources(nbSources, sources_id.data());
	for (auto source_id : sources_id) {

		alSourcei(source_id, AL_BUFFER, 0);
		alSourceRewind(source_id);

		if (alGetError() != AL_NO_ERROR) {
			logError("Could not set source parameters: [source id: %d] [number of sources: %d]", source_id, nbSources);
			return false;
		}

		AudioPlayer::AudioSource audioSource;
		audioSource.source_id = source_id;
		ALuint buffers_id[num_buffers];
		alGenBuffers(num_buffers, buffers_id);
		if (alGetError() != AL_NO_ERROR) {
			logError("Could not create source buffers: [source id: %d] [number of sources: %d]", source_id, nbSources);
			return false;
		}

		for (int i = 0; i < num_buffers; i++) {
			audioSource.buffers.insert({ buffers_id[i], false });
		}

		sources_pool.push(std::make_unique<AudioPlayer::AudioSource>(audioSource));
	}

	return !sources_id.empty();
}

OpenALManager::OpenALManager(const AudioParameters& parameters) {
	UpdateParameters(parameters);
	alListener3i(AL_POSITION, 0, 0, 0);

	auto openalFormat = GetBestOpenALRenderingFormat(parameters.stereo ? ALC_STEREO_SOFT : ALC_MONO_SOFT);
	assert(openalFormat && "Audio format not found or not supported");
	SDL_AudioSpec desired = {};
	desired.freq = parameters.rate;
	desired.format = openalFormat ? mapping_openal_sdl.at(openalFormat) : 0;
	desired.channels = parameters.stereo ? 2 : 1;
	desired.samples = parameters.sample_frame_size;
	desired.callback = MixerCallback;
	desired.userdata = reinterpret_cast<void*>(this);

	if (SDL_OpenAudio(&desired, &sdl_audio_specs_obtained) < 0) {
		CleanEverything();
	} else {
		audio_parameters.rate = sdl_audio_specs_obtained.freq;
		audio_parameters.stereo = sdl_audio_specs_obtained.channels == 2;
		rendering_format = mapping_sdl_openal.at(sdl_audio_specs_obtained.format);
	}
}

void OpenALManager::MixerCallback(void* usr, uint8* stream, int len) {
	auto manager = (OpenALManager*)usr;
	int frameSize = manager->sdl_audio_specs_obtained.channels * SDL_AUDIO_BITSIZE(manager->sdl_audio_specs_obtained.format) / 8;
	manager->GetPlayBackAudio(stream, len / frameSize);
}

void OpenALManager::CleanEverything() {
	Stop();

	while (!sources_pool.empty()) {
		const auto& audioSource = sources_pool.front();
		alDeleteSources(1, &audioSource->source_id);

		for (auto const& buffer : audioSource->buffers) {
			alDeleteBuffers(1, &buffer.first);
		}

		sources_pool.pop();
	}

	alDeleteFilters(1, &low_pass_filter);
	bool closedDevice = CloseDevice();
	assert(closedDevice && "Could not close audio device");
}

int OpenALManager::GetBestOpenALRenderingFormat(ALCint channelsType) {
	auto device = p_ALCDevice ? p_ALCDevice : alcLoopbackOpenDeviceSOFT(nullptr);
	if (!device) {
		logError("Could not open audio loopback device to find best rendering format");
		return 0;
	}

	ALCint format = 0;
	for (int i = 0; i < format_type.size(); i++) {

		ALCint attrs[] = {
			ALC_FORMAT_TYPE_SOFT,     format_type[i],
			ALC_FORMAT_CHANNELS_SOFT, channelsType,
			ALC_FREQUENCY,            audio_parameters.rate
		};

		if (alcIsRenderFormatSupportedSOFT(device, attrs[5], attrs[3], attrs[1]) == AL_TRUE) {
			format = format_type[i];
			break;
		}
	}

	if (!p_ALCDevice) {
		if (!alcCloseDevice(device)) {
			logError("Could not close audio loopback device to find best rendering format");
			return 0;
		}
	}

	return format;
}

void OpenALManager::UpdateParameters(const AudioParameters& parameters) {
	audio_parameters = parameters;
	master_volume = parameters.volume;
}

void OpenALManager::Shutdown() {
	delete instance;
	instance = nullptr;
}

OpenALManager::~OpenALManager() {
	CleanEverything();
	SDL_CloseAudio();
}
