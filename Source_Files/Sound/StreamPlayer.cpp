#include "StreamPlayer.h"
#include "OpenALManager.h"

StreamPlayer::StreamPlayer(uint8* data, int length, int rate, bool stereo, bool sixteen_bit) : AudioPlayer(rate, stereo, sixteen_bit) {
	stream_data = std::vector<uint8>(data, data + length);
}

void StreamPlayer::FeedData(uint8* data, int length) {
	std::lock_guard<std::mutex> guard(mutex_internal);
	size_t currentSize = stream_data.size();
	stream_data.resize(currentSize + length);
	std::copy(data, data + length, stream_data.data() + currentSize);
}

int StreamPlayer::GetNextData(uint8* data, int length) {
	int remainingDataLength = stream_data.size() - current_index_data;
	if (remainingDataLength <= 0) return 0;
	int returnedDataLength = std::min(remainingDataLength, length);
	std::copy(stream_data.data() + current_index_data, stream_data.data() + current_index_data + returnedDataLength, data);
	current_index_data += returnedDataLength;
	return returnedDataLength;
}

CallBackableStreamPlayer::CallBackableStreamPlayer(CallBackStreamPlayer callback, int length, int rate, bool stereo, bool sixteen_bit)
	: AudioPlayer(rate, stereo, sixteen_bit) {
	data_length = length;
	CallBackFunction = callback;
	assert(data_length <= buffer_samples && "CallBackableStreamPlayer not supported length");
}

int CallBackableStreamPlayer::GetNextData(uint8* data, int length) {
	return CallBackFunction(data, length);
}

void CallBackableStreamPlayer::FillBuffers() {

	UnqueueBuffers(); //First we unqueue buffers that can be

	for (auto& buffer : audio_source.buffers) {
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
			alSourceQueueBuffers(audio_source.source_id, 1, &buffer.first);
			buffer.second = true;
		}
	}
}