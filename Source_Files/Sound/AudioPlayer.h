/*
    Copyright (C) 2023 Benoit Hauquier and the "Aleph One" developers.

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
    static constexpr uint32_t queue_size = 5;
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

static constexpr uint32_t num_buffers = 4;
static constexpr uint32_t buffer_samples = 8192;

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

    uint32_t queued_rate;
    AudioFormat queued_format;
    bool queued_stereo;

    friend class OpenALManager;

public:
    void AskStop() { stop_signal = true; }
    bool IsActive() const { return is_active.load(); }
    void AskRewind() { rewind_signal = true; }
    virtual float GetPriority() const = 0;
protected:
    AudioPlayer(uint32_t rate, bool stereo, AudioFormat audioFormat);
    void Init(uint32_t rate, bool stereo, AudioFormat audioFormat);
    virtual uint32_t GetNextData(uint8* data, uint32_t length) = 0;
    virtual bool LoadParametersUpdates() { return false; }
    bool IsPlaying() const;
    virtual std::tuple<AudioFormat, uint32_t, bool> GetAudioFormat() const { return std::make_tuple(format, rate, stereo); }
    bool HasBufferFormatChanged() const;
    std::atomic_bool rewind_signal = { false };
    std::atomic_bool stop_signal = { false };
    std::atomic_bool is_active = { true };
    bool is_sync_with_al_parameters = false; //uses locks
    uint32_t rate;
    AudioFormat format;
    bool stereo;
    std::unique_ptr<AudioSource> audio_source;
    virtual void Rewind();
};

#endif