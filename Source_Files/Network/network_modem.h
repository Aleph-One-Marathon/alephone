#ifndef __NETWORK_MODEM_H
#define __NETWORK_MODEM_H

/*

	network_modem.h
	Friday, September 29, 1995 11:05:14 PM- rdm created.

*/

enum {
	kNoTimeout= -1,
	kBestTry= 0
	/* anything else is the ticks for timeout.. */
};

OSErr initialize_modem_endpoint(void);
OSErr teardown_modem_endpoint(void);

bool call_modem_player(void);
bool answer_modem_player(void);

/* Ideally, packet data has only a checksum, and stream data knows how to ask */
/*  that the data be resent. */
OSErr write_modem_endpoint(void *data, uint16 length, long timeout);
OSErr read_modem_endpoint(void *data, uint16 length, long timeout);

void write_modem_endpoint_asynchronous(void *data, uint16 length, long timeout);
bool asynchronous_write_completed(void);

void idle_modem_endpoint(void);

bool modem_available(void);
long modem_read_bytes_available(void);

#endif
