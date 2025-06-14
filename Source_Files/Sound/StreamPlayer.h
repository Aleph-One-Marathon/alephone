#ifndef __STREAM_PLAYER_H
#define __STREAM_PLAYER_H

#include "AudioPlayer.h"

typedef int (*CallBackStreamPlayer)(uint8* data, int length, void* userdata);

class StreamPlayer : public AudioPlayer {
public:
	StreamPlayer(CallBackStreamPlayer callback, int rate, bool stereo, AudioFormat audioFormat, void* userdata); //Must not be used outside OpenALManager (public for make_shared)
	float GetPriority() const override { return 10; } //As long as it's only used for intro video, it doesn't really matter
private:
	int GetNextData(uint8* data, int length) override;
	CallBackStreamPlayer CallBackFunction;
	void* userdata;

	friend class OpenALManager;
};

#endif