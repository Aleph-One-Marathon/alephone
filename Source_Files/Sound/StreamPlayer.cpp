#include "StreamPlayer.h"

StreamPlayer::StreamPlayer(CallBackStreamPlayer callback, int length, int rate, bool stereo, AudioFormat audioFormat)
	: AudioPlayer(rate, stereo, audioFormat) {
	data_length = length;
	CallBackFunction = callback;
	assert(data_length <= buffer_samples && "StreamPlayer not supported length");
}

int StreamPlayer::GetNextData(uint8* data, int length) {
	return CallBackFunction(data, std::min(length, data_length));
}