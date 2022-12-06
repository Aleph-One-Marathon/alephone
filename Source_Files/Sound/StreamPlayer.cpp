#include "StreamPlayer.h"
#include "OpenALManager.h"

StreamPlayer::StreamPlayer(CallBackStreamPlayer callback, int length, int rate, bool stereo, bool sixteen_bit)
	: AudioPlayer(rate, stereo, sixteen_bit) {
	data_length = length;
	CallBackFunction = callback;
	assert(data_length <= buffer_samples && "StreamPlayer not supported length");
}

int StreamPlayer::GetNextData(uint8* data, int length) {
	return CallBackFunction(data, length);
}

void StreamPlayer::FillBuffers() {

	UnqueueBuffers(); //First we unqueue buffers that can be

	for (auto& buffer : audio_source->buffers) {
		if (!buffer.second) {

			std::vector<uint8> data;
			size_t bufferOffset = 0;
			int actualDataLength = 0;

			while (buffer_samples >= bufferOffset + data_length) {
				data.resize(bufferOffset + data_length);
				actualDataLength = GetNextData(data.data() + bufferOffset, data_length);
				data.resize(data.size() + actualDataLength - data_length);
				if (actualDataLength <= 0) break;
				bufferOffset += actualDataLength;
			}

			if (data.size() <= 0) return;
			alBufferData(buffer.first, format, data.data(), data.size(), rate);
			alSourceQueueBuffers(audio_source->source_id, 1, &buffer.first);
			buffer.second = true;
		}
	}
}