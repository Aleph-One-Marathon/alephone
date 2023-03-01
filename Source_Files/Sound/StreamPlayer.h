#ifndef __STREAM_PLAYER_H
#define __STREAM_PLAYER_H

#include "AudioPlayer.h"

typedef int (*CallBackStreamPlayer)(uint8* data, int length);

class StreamPlayer : public AudioPlayer {
public:
	//Length must be <= buffer_samples variable
	StreamPlayer(CallBackStreamPlayer callback, int length, int rate, bool stereo, bool sixteen_bit); //Must not be used outside OpenALManager (public for make_shared)
	float GetPriority() const override { return 10; } //As long as it's only used for intro video, it doesn't really matter
private:
	int GetNextData(uint8* data, int length) override;
	CallBackStreamPlayer CallBackFunction;
	int data_length;

	friend class OpenALManager;
};

#endif