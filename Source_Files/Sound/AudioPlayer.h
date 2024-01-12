#ifndef __AUDIO_PLAYER_H
#define __AUDIO_PLAYER_H

#include <AL/al.h>
#include <AL/alext.h>

#include "Decoder.h"
#include <atomic>
#include <algorithm>
#include <unordered_map>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/unordered/unordered_map.hpp>

using SetupALResult = std::pair<bool, bool>; //first is source configuration suceeded for this pass, second is source is fully setup and doesn't need another pass

template <typename T>
struct AtomicStructure {
private:
    static constexpr int queue_size = 5;
    boost::lockfree::spsc_queue<T, boost::lockfree::capacity<queue_size>> shared_queue;
    std::atomic_int index = { 0 };
    T structure[2];
public:
    AtomicStructure& operator= (const T& structure) {
        this->structure[index] = structure;
        return *this;
    }

    const T& Get() const { return structure[index]; }

    void Store(const T& value) {
        shared_queue.push(value);
    }

    void Set(const T& value) {
        int swappedIndex = index ^ 1;
        structure[swappedIndex] = value;
        index = swappedIndex;
    }

    bool Consume(T& returnValue) {
        return shared_queue.pop(returnValue);
    }

    bool Update() {
        T returnValue[queue_size];
        auto size = shared_queue.pop(returnValue, queue_size);
        if (size) Set(returnValue[size - 1]);
        return size;
    }
};

static constexpr int num_buffers = 4;
static constexpr int buffer_samples = 8192;

class AudioPlayer {
private:
   
    typedef std::unordered_map<ALuint, bool> AudioPlayerBuffers; //<buffer id, is queued for processing>

    struct AudioSource {
        ALuint source_id = 0; //Source used by this player
        AudioPlayerBuffers buffers;
    };

    bool Play();
    void ResetSource();
    bool Update();
    void UnqueueBuffers();
    void FillBuffers();
    std::unique_ptr<AudioSource> RetrieveSource();
    bool AssignSource();
    virtual SetupALResult SetUpALSourceIdle(); //Update of the source parameters (AL), done everytime the player is processed in the queue
    virtual bool SetUpALSourceInit(); //Init of the source parameters (AL), done when the source is assigned to the player

    static inline const boost::unordered_map<std::pair<AudioFormat, bool>, int> mapping_audio_format_openal = {
        {{AudioFormat::_8_bit, false}, AL_FORMAT_MONO8},
        {{AudioFormat::_8_bit, true}, AL_FORMAT_STEREO8},
        {{AudioFormat::_16_bit, false}, AL_FORMAT_MONO16},
        {{AudioFormat::_16_bit, true}, AL_FORMAT_STEREO16},
        {{AudioFormat::_32_float, false}, AL_FORMAT_MONO_FLOAT32},
        {{AudioFormat::_32_float, true}, AL_FORMAT_STEREO_FLOAT32}
    };

    friend class OpenALManager;

public:
    void AskStop() { stop_signal = true; }
    bool IsActive() const { return is_active.load(); }
    bool HasRewind() const { return rewind_signal.load(); }
    void AskRewind() { rewind_signal = true; }
    virtual float GetPriority() const = 0;
protected:
    AudioPlayer(int rate, bool stereo, AudioFormat audioFormat);
    void Init(int rate, bool stereo, AudioFormat audioFormat);
    virtual int GetNextData(uint8* data, int length) = 0;
    virtual bool LoadParametersUpdates() { return false; }
    std::atomic_bool rewind_signal = { false };
    std::atomic_bool stop_signal = { false };
    std::atomic_bool is_active = { true };
    bool is_sync_with_al_parameters = false; //uses locks
    int rate = 0;
    ALenum format = 0; //Mono 8-16-32f or stereo 8-16-32f
    std::unique_ptr<AudioSource> audio_source;
    virtual void Rewind();
};

#endif