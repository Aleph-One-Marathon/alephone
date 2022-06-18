#ifndef __AUDIO_PLAYER_H
#define __AUDIO_PLAYER_H

#include <AL/al.h>
#include <AL/alext.h>

#include "Decoder.h"
#include <atomic>
#include <algorithm>
#include <mutex>
#include <unordered_map>

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
    AudioSource RetrieveSource();
    bool AssignSource();
    virtual bool SetUpALSourceIdle() const; //Update of the source parameters (AL), done everytime the player is processed in the queue
    virtual bool SetUpALSourceInit() const; //Init of the source parameters (AL), done when the source is assigned to the player
    int GetCorrespondingFormat(bool stereo, bool isSixteenBit) const;

    /*This guarantees the player is not being handled
     by the queue manager when locked. It may still be in
     playing state. It is blocking for the queue manager.*/
    void Lock_Internal() { mutex_internal.lock(); Lock(); }
    void Unlock_Internal() { mutex_internal.unlock(); Unlock(); }
    friend class OpenALManager;
public:
    void Stop();
    bool IsActive() const { return is_active; }
    void SetVolume(float volume) { this->volume = volume; }
    void AskRewind() { rewind_state = true; }
    virtual short GetIdentifier() const { return NONE; }
    virtual short GetSourceIdentifier() const { return NONE; }
    void SetFilterable(bool filterable) { this->filterable = filterable; }
    void Lock() { mutex_public.lock(); }
    void Unlock() { mutex_public.unlock(); }
    virtual float GetPriority() const = 0;
protected:
    AudioPlayer(int rate, bool stereo, bool sixteen_bit);
    void UnqueueBuffers();
    virtual void FillBuffers();
    virtual int GetNextData(uint8* data, int length) = 0;
    void Load(int rate, bool stereo, bool sixteen_bit);
    std::atomic<bool> rewind_state = { false };
    std::atomic<bool> filterable = { true };
    std::atomic<bool> is_active = { true };
    std::atomic<float> volume = { 0 };
    std::mutex mutex_internal;
    std::mutex mutex_public;
    int rate = 0;
    ALenum format = 0; //Mono 8-16 or stereo 8-16
    AudioSource audio_source;
    virtual void Rewind();
};

#endif