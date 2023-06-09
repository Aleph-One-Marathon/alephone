#ifndef __MUSIC_PLAYER_H
#define __MUSIC_PLAYER_H

#include "AudioPlayer.h"

struct MusicParameters {
	float volume = 1;
	bool loop = true;
};

class MusicPlayer : public AudioPlayer {
public:
	MusicPlayer(std::shared_ptr<StreamDecoder> decoder, MusicParameters parameters); //Must not be used outside OpenALManager (public for make_shared)
	static void SetDefaultVolume(float volume) { default_volume = volume; } //Since we can only change global music volume in settings, we don't have to care about AL sync here
	float GetPriority() const override { return 5; } //Doesn't really matter, just be above maximum volume (1) to be prioritized over sounds
private:
	std::shared_ptr<StreamDecoder> decoder;
	MusicParameters parameters;
	int GetNextData(uint8* data, int length) override;
	SetupALResult SetUpALSourceIdle() override;
	static std::atomic<float> default_volume;

	friend class OpenALManager;
};

#endif