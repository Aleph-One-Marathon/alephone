#ifndef __NETWORK_SOUND_H
#define __NETWORK_SOUND_H

/*
NETWORK_SOUND.H
Sunday, August 14, 1994 3:36:17 AM- go nuts
*/

/* ---------- constants */
#define NETWORK_SOUND_CHUNK_BUFFER_SIZE 512

/* ---------- prototypes: NETWORK_SPEAKER.C */

OSErr open_network_speaker(short block_size, short connection_threshold);
void close_network_speaker(void);
void quiet_network_speaker(void);

void queue_network_speaker_data(byte *buffer, short count);
void network_speaker_idle_proc(void);

/* ---------- prototypes: NETWORK_MICROPHONE.C */

OSErr open_network_microphone(short network_distribution_type);
void close_network_microphone(void);

bool has_sound_input_capability(void);

/* This function is defined in interface.h */
// void handle_microphone_key(bool triggered);

#endif
