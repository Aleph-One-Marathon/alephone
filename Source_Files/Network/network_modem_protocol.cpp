/*
	network_modem_protocol.c

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

	Wednesday, October 11, 1995 11:58:44 AM- rdm created.

	Ideally this code would all be in the standard network.c file, and the protocols
		would all be the same.  Unfortunately this would be a large pain in the ass,
		so it is done this way instead. (Since this is an adhoc addition)

Jan 30, 2000 (Loren Petrich):
	Added some typecasts

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging
*/

/*
	ToDo:
	1) Add the code to retry on syncing if we don't get a response...
	2) Retries!!!
*/

#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "macintosh_cseries.h"
#include "mytm.h"
#include "progress.h"

/* For stupid things.. */
#include "map.h"
#include "interface.h"

#include "macintosh_network.h"
#include "network_modem.h"
#include "network_modem_protocol.h"
#include "game_errors.h"

#ifdef env68k
	#pragma segment network_modem
#endif

#define DEBUG_MODEM 1

/* --------- #defines  */
/* Hopefully these are invalid flags.. */
#define ACKNOWLEDGE_ACTION_FLAG (0x7ffffffd)
#define MAXIMUM_MODEM_PLAYERS (2)
#define ACTION_FLAGS_PER_MODEM_PACKET 1

#define DATA_TRANSFER_CHUNK_SIZE (512) // This should be a size that a 16bit CRC checksum can guarantee correctness for...
#define MAP_TRANSFER_TIME_OUT   (70*MACHINE_TICKS_PER_SECOND) // 70 seconds to wait for map.
#define MODEM_JOIN_TIMEOUT (5*MACHINE_TICKS_PER_SECOND)

#define NET_SYNC_TIME_OUT       (50*MACHINE_TICKS_PER_SECOND) // 50 seconds to time out of syncing. 
#define NET_UNSYNC_TIME_OUT (3*MACHINE_TICKS_PER_SECOND) // 3 seconds

#define MAXIMUM_PLAYER_DATA_SIZE     128
#define MAXIMUM_GAME_DATA_SIZE       256

#define MAX_SERVER_ABORTED_READS 4*128
#define MAX_CLIENT_ABORTED_READS 4*128

/* Whatever this is should be statistically unlikely for the data you are */
/*  sending.... */
#define HEADER_START_BYTE (0xA5) /* binary 10100101 */

#define errTIMEOUT (42)

/* --------- structures  */
struct ModemPlayer
{
	short identifier;

	byte player_data[MAXIMUM_PLAYER_DATA_SIZE];
};
typedef struct ModemPlayer ModemPlayer, *ModemPlayerPtr;

struct ModemTopology
{
	short tag;
	short player_count;
	
	short nextIdentifier;
	
	byte game_data[MAXIMUM_GAME_DATA_SIZE];
	
	struct ModemPlayer players[MAXIMUM_MODEM_PLAYERS];
};
typedef struct ModemTopology ModemTopology, *ModemTopologyPtr;

struct ModemNetStatus
{
	long lastValidRingSequence; /* the sequence number of the last valid ring packet we received */
	long ringPacketCount; /* the number of ring packets we have received */
	
	short update_latency;
	long flags_processed;

	short time_between_retries;
	
	bool iAmTheServer;
	bool single_player;
	short localPlayerIndex;
	short localPlayerIdentifier;
	short server_player_index;

	short maximum_aborted_reads;
	short aborted_read_timer;

	long localNetTime;
	unsigned long sequence;

	bool outgoing_packet_type;
	bool outgoing_packet_valid;

};
typedef struct ModemNetStatus ModemNetStatus, *ModemNetStatusPtr;

#define NET_QUEUE_SIZE (17)

struct NetQueue
{
	short read_index, write_index;
	long buffer[NET_QUEUE_SIZE];
};
typedef struct NetQueue NetQueue, *NetQueuePtr;

enum {
	_incoming_packet,
	_outgoing_packet,
	NUMBER_OF_PACKET_BUFFERS
};

enum {
	_query_player_packet,
	_lookup_response_packet,
	_client_packet,
	_server_packet,
	_join_player_packet,
	_accept_join_packet,
	_topology_packet,
	_sync_packet,
	_acknowledge_packet,
	
	_stream_size_packet,
	_stream_packet,
	_stream_acknowledge_packet,
	NUMBER_OF_PACKET_TYPES
};

enum {
	_request_resend,
	_acknowledge,
	_normal_client_packet,
	_no_packet,
	NUMBER_OF_POSSIBLE_CLIENT_PACKETS
};

enum {
	tagNEW_PLAYER,
	tagCANCEL_GAME,
	tagSTART_GAME
};

struct join_player_data {
	short new_local_player_identifier;
};

struct accept_join_data {
	bool accepted;
	ModemPlayer player;
};

/* Before all the packets.. */
struct stream_header {
	byte unique_byte;
	byte packet_type;
	uint16 checksum; /* Should be 16bit crc? */
};

struct stream_read_queue {
	short read_index;
	short write_index;
	byte *data;
};

struct stream_size_packet {
	long size;
};

struct stream_packet_data {
	short sequence;
	char data[DATA_TRANSFER_CHUNK_SIZE];
};

struct acknowledge_packet {
	bool unused;
};

struct stream_acknowledge_packet {
	short sequence;
};

struct lookup_packet_data {
	NetEntityName entity;
};

struct lookup_query_data {
	short unused;
};

struct sync_packet_data 
{
	unsigned long localNetTime;
};

struct acknowledge_packet_data {
	unsigned long sequence;
};

struct player_packet_data {
#if (MAXIMUM_MODEM_PLAYERS>2)
	byte player_index;
#endif
	unsigned long sequence;
	long action_flag;
};

struct server_packet_data {
	unsigned long sequence;
	uint32 flags[1]; // this is actually number_of_players flags....
};

/*
	The response of a player_packet_data with a given sequence is essentially the acknowledgement.
*/

#if (MAXIMUM_MODEM_PLAYERS<=2)
	#define GET_CLIENT_PLAYER_IDENTIFIER(p) (1)
	#define SET_CLIENT_PLAYER_IDENTIFIER(p, i)
#else
	#define GET_CLIENT_PLAYER_IDENTIFIER(p) ((p)->player_index)
	#define SET_CLIENT_PLAYER_IDENTIFIER(p, i) ((p)->player_index==(i))
#endif

/* --------- conditional code */
#ifdef DEBUG_MODEM
struct modem_stats_data {
	long client_packets_sent;
	long server_packets_sent;
	long server_unsyncing;
	long numSmears;
	long action_flags_processed;
	long aborted_read_attempts;
	long max_consecutive_aborted_reads;
	long client_packets_received;
	long server_packets_received;
	long old_acknowledges;
	long stream_packets_sent;
	long stream_packets_necessary;
	long stream_early_acks;
	long number_of_stalls;
	long flags_processed;
	long missed_chances;
};

static struct modem_stats_data modem_stats;
#endif

/* ----------- local prototypes */
static long max_packet_length(void);
static void client_send_action_flag(long action_flag, unsigned long sequence);

bool packet_tickler(void);
static void build_and_post_async_server_packet(void);
static bool network_queueing_task(void);

static void build_server_packet(struct server_packet_data *packet);
static short size_of_players_queue(short player_index);
static void print_modem_stats(void);
static short server_packet_size(void);
static void allocate_modem_buffer(void);
static void reset_modem_buffer(void);
static OSErr distribute_topology(short tag);
static OSErr distribute_map_to_all_players(byte *wad, long wad_length);
static byte *receive_map(void);
static void modem_interrupt_idle(void);
static void modem_initialize_topology(void *game_data, short game_data_size, void *player_data,
	short player_data_size);
static void update_topology(void);
static OSErr modem_respond_to_lookup(void);
static OSErr send_acknowledged_packet(short packet_type, void *data, long timeout);
static void send_acknowledge(unsigned long sequence);
static void process_server_packet(struct server_packet_data *packet_data);
static void handle_single_player(void);

/* Modem name registration... */
OSErr ModemRegisterName(unsigned char *player_name, unsigned char *type, short version, short socket);
void ModemUnRegisterName(void);
void ModemLookupRemove(short index);

/* ------------ actual modem writing functions */
static bool modem_read_packet(byte *packet_type, void *buffer);
static OSErr modem_send_packet(byte packet_type, void *buffer);
static uint16 packet_type_size(short packet_type);

/* ------------ my version of ADSP */
static long receive_stream_size(long timeout);
static OSErr receive_stream_data_with_progress(byte *data, long data_length,long timeout,
	void (*progress_function_ptr)(long bytes_written, void *data), void *user_data);
static OSErr send_stream_data(byte *data, long data_length, long timeout);
static OSErr send_stream_data_with_progress(byte *data, long data_length, long timeout,
	void (*progress_function)(long bytes_written, void *data), void *user_data);

/* ----------- progress */
struct progress_statistics {
	long sent;
	long total;
};

static void progress_function(long bytes_written, void *data);

/* ----------- globals */
static ModemTopologyPtr topology;
static ModemNetStatusPtr status;
static short modemState= netUninitialized;

#define MODEM_QUEUE_SIZE (1024)
static struct stream_read_queue stream_read_queue;

/* Should be able to share this.  Notice the normal one is volatile! */
/* The local queue is where you get/store your own flags.  Only the server needs */
/*  the player_queues */
static NetQueue local_queue; 
static NetQueue player_queues[MAXIMUM_MODEM_PLAYERS];

static void *packet_buffers[NUMBER_OF_PACKET_BUFFERS];

static myTMTaskPtr serverQueueingTMTask = (myTMTaskPtr) NULL;
static myTMTaskPtr clientQueueingTMTask= (myTMTaskPtr) NULL;

/* ------------ code */
bool ModemEnter(
	void)
{
	bool success= true;
	OSErr error= noErr;
	
	assert(modemState==netUninitialized);

	/* Allocate the stream connection data (initializes the modem..) */
	error= initialize_modem_endpoint();
	if (!error)
	{
		static bool added_exit_procedure= false;

		/* Make sure we have an exit procedure.. */
		if(!added_exit_procedure)
		{
			atexit(ModemExit);
			added_exit_procedure= true;
		}

		topology= (ModemTopologyPtr) NewPtrClear(sizeof(ModemTopology));
		status= (ModemNetStatusPtr) NewPtrClear(sizeof(ModemNetStatus));

		error= MemError();
		if(!error && topology && status) 
		{
			short index;
			
			for(index= 0; !error && index<NUMBER_OF_PACKET_BUFFERS; ++index)
			{
				packet_buffers[index]= (char *) NewPtrClear(max_packet_length());
				error= MemError();
			}

			/* Allocate our stream buffer */
			allocate_modem_buffer();
			reset_modem_buffer();

			if(!error)
			{
				/* Set the server player identifier */
				status->server_player_index= 0;
				status->sequence= 0;
				modemState= netDown;
#ifdef DEBUG_NET
				obj_clear(modem_stats);
	#ifdef STREAM_NET
				open_stream_file();
	#endif
#endif
			}
		}
	}

	/* Handle our own errors.. */
	if(error)
	{
		alert_user(infoError, strNETWORK_ERRORS, netErrCantContinue, error);
		NetExit();
		success= false;
	}
	
	return success;
}

void ModemExit(
	void)
{
	/* These functions do the right thing for NULL pointers */
	serverQueueingTMTask= myTMRemove(serverQueueingTMTask);
	clientQueueingTMTask= myTMRemove(clientQueueingTMTask);

	if (modemState!=netUninitialized)
	{
		OSErr error;

		error= teardown_modem_endpoint();
		vwarn(!error, csprintf(temporary, "teardown_modem_endpoing returned %d", error));
		if (!error || error==4)
		{
			short index;
		
#ifdef DEBUG_MODEM
			print_modem_stats();
#ifdef STREAM_NET
			close_stream_file();
#endif
#endif
			DisposePtr((Ptr)topology);
			DisposePtr((Ptr)status);

			for(index= 0; index<NUMBER_OF_PACKET_BUFFERS; ++index)
			{
				DisposePtr((Ptr) packet_buffers[index]);
				packet_buffers[index]= NULL;
			}

			status= NULL;
			topology= NULL;

			modemState= netUninitialized;
		}
	}
}

bool ModemSync(
	void)
{
	bool success= true;
	OSErr error= noErr;
	long initial_ticks;

	status->lastValidRingSequence= 0;
	status->ringPacketCount= 0;
	status->outgoing_packet_valid= false;

	local_queue.read_index= local_queue.write_index= 0;

	modemState= netStartingUp;

	if (status->localPlayerIndex==status->server_player_index)
	{
		status->maximum_aborted_reads= MAX_SERVER_ABORTED_READS; /* Lots allowed while syncing */
		status->aborted_read_timer= MAX_SERVER_ABORTED_READS;

		/* Build the first packet.. */
		build_and_post_async_server_packet();
	}
	else
	{
		status->maximum_aborted_reads= MAX_CLIENT_ABORTED_READS; /* Lots allowed while syncing.. */
		status->aborted_read_timer= MAX_CLIENT_ABORTED_READS;
	}

	/* Packet tickler gets called 1000 times/second.. */
	serverQueueingTMTask= myXTMSetup(1000/(2*TICKS_PER_SECOND), packet_tickler);
	clientQueueingTMTask= myXTMSetup(1000/TICKS_PER_SECOND, network_queueing_task);

	/* Wait.. */
	initial_ticks= machine_tick_count();
	while(modemState != netActive && machine_tick_count()-initial_ticks<NET_SYNC_TIME_OUT)
		;

	if(modemState != netActive)
	{
		alert_user(infoError, strNETWORK_ERRORS, netErrSyncFailed, error);

		/* How did Alain do this? */
		modemState= netDown;
	}
	
	return success;
}

bool ModemUnsync(
	void)
{
	bool success= true;
	long ticks;
	
	if (modemState==netStartingUp || modemState==netActive)
	{
		modemState= netComingDown;
		
		/* Next time the server receives a packet, and if the modemState==netComingDown */
		/*  the server will send a packet with zero flags, which means process the remaining */
		/*  flags, and get ready to change level.  Once the packet with zero flags gets back */
		/*  to the server, the server sends an unsync ring packet.  This will cause all the other */
		/*  machines to unsync, and when the server gets the packet back, it turns the net off */

		ticks= TickCount();
		while(modemState != netDown && (TickCount()-ticks<NET_UNSYNC_TIME_OUT))
			;
	}

	if(TickCount()-ticks>=NET_UNSYNC_TIME_OUT) 
	{
		success= false;
	}
	modemState= netDown;

#ifdef DEBUG_NET
	dprintf("Flags processed: %d Time: %d;g", modem_stats.action_flags_processed, TickCount()-ticks);
	modem_stats.action_flags_processed= 0;
#endif
	
	return success;
}

short ModemAddDistributionFunction(
	NetDistributionProc proc, 
	bool lossy)
{
	(void)(proc, lossy);
	return NONE;
}

void ModemDistributeInformation(
	short type, 
	void *buffer, 
	short buffer_size, 
	bool send_to_self)
{
	(void)(type, buffer, buffer_size, send_to_self);
}

void ModemRemoveDistributionFunction(
	short type)
{
	(void)(type);
}

bool ModemGather(
	void *game_data, 
	short game_data_size, 
	void *player_data, 
	short player_data_size)
{
	bool success= false;

	modem_initialize_topology(game_data, game_data_size, player_data, player_data_size);
	modemState= netGathering;

	/* Call the person! */	
	if(call_modem_player())
	{
		success= true;
	}

	return success;
}

bool ModemGatherPlayer(
	short player_index, 
	CheckPlayerProcPtr check_player)
{
	OSErr error= noErr;
	bool success= false;
	long initial_tick_count= machine_tick_count();

	(void) (player_index)	;
	assert(modemState==netGathering);
	assert(topology->player_count<MAXIMUM_NUMBER_OF_NETWORK_PLAYERS);
	
	/* Setup the gather data. */
	while(!error && !success)
	{
		byte packet_type;
		bool got_packet;
		struct accept_join_data	*accept_packet= (struct accept_join_data *)packet_buffers[_incoming_packet];
		struct join_player_data packet;

		/* Send the join player packet.. */
		packet.new_local_player_identifier= topology->nextIdentifier;
		error= modem_send_packet(_join_player_packet, &packet);
		
		/* Read a packet.. */
		got_packet= modem_read_packet(&packet_type, accept_packet);
		if(got_packet && packet_type==_accept_join_packet)
		{
			if(accept_packet->accepted)
			{
				/* make sure everybody gets a unique identifier */
				topology->nextIdentifier+= 1;

				/* Copy in the player data */
				obj_copy(topology->players[topology->player_count], accept_packet->player);
			
				/* successfully added a player */
				topology->player_count+= 1;
					
				check_player(topology->player_count-1, topology->player_count);	

				/* recalculate our local information & distribute the new topology */
				update_topology();
				error= distribute_topology(tagNEW_PLAYER);
			
				/* We be done with this. */
				if(!error)
				{
					/* And remove them from the listbox.. */
					ModemLookupRemove(player_index);
				
					success= true;
				}
			} else {
				/* Joined player refused.... */
				error= 2;
			}
		} 

		/* Join timeout .. */
		if(machine_tick_count()-initial_tick_count>MODEM_JOIN_TIMEOUT)
		{
			error= errTIMEOUT;
		}
	}

	if(error)
	{
		alert_user(infoError, strNETWORK_ERRORS, netErrCantAddPlayer, error);
	}
	
	return success;
}

bool ModemGameJoin(
	unsigned char *player_name, 
	unsigned char *player_type, 
	void *player_data, 
	short player_data_size, 
	short version_number)
{
	bool success= false;

	/* initialize default topology (no game data) */
	modem_initialize_topology((void *) NULL, 0, player_data, player_data_size);

	/* register our downring socket with the net so gather dialogs can find us */
	if(answer_modem_player())
	{
		OSErr error= noErr;

//		error= NetStreamWaitForConnection();
		if (!error)
		{
			/* Register our modem name.. */
			error= ModemRegisterName(player_name, player_type, version_number, 
				0);

			if(!error)
			{
				/* weÕre registered and awaiting a connection request */
				modemState= netJoining;
				success= true;
			}
		} 
		
		if(error)
		{
			alert_user(infoError, strNETWORK_ERRORS, netErrCouldntJoin, error);
		}
	}
	
	return success;
}


void ModemCancelJoin(
	void)
{
	assert(modemState==netJoining||modemState==netWaiting||modemState==netCancelled||modemState==netJoinErrorOccurred);
	
	ModemUnRegisterName();
#if 0
	OSErr error;
	
	error= NetCloseStreamConnection(true); /* this should stop the ocPassive OpenConnection */
	if (error==noErr)
	{
		/* our name has been unregistered and our connection end has been closed */
	}
#endif
}

short ModemGetLocalPlayerIndex(
	void)
{
	assert(modemState!=netUninitialized&&modemState!=netDown&&modemState!=netJoining);

	return status->localPlayerIndex;
}

short ModemGetPlayerIdentifier(
	short player_index)
{
	assert(modemState!=netUninitialized&&modemState!=netDown&&modemState!=netJoining);
	assert(player_index>=0&&player_index<topology->player_count);
	
	return topology->players[player_index].identifier;
}

bool ModemNumberOfPlayerIsValid(
	void)
{
	bool valid;
	
	switch(modemState)
	{
		case netUninitialized:
		case netJoining:
			valid= false;
			break;
		default:
			valid= true;
			break;
	}

	return valid;
}

short ModemGetNumberOfPlayers(
	void)
{
	assert(modemState!=netUninitialized && modemState!=netJoining);
	
	return topology->player_count;
}

void *ModemGetPlayerData(	
	short player_index)
{	
	assert(modemState!=netUninitialized && modemState!=netJoining);
	assert(player_index>=0&&player_index<topology->player_count);
	
	return topology->players[player_index].player_data;
}

void *ModemGetGameData(
	void)
{
	assert(modemState!=netUninitialized && modemState!=netDown && modemState!=netJoining);
	
	return topology->game_data;
}

bool ModemStart(	
	void)
{
	OSErr error;
	bool success;
	
	assert(modemState==netGathering);

	// how about we sort the players before we pass them out to everyone?
	// This is an attempt to have a slightly more efficent ring in a multi-zone network.
	// we should really do some sort of pinging to determine an optimal order (or perhaps
	// sort on hop counts) but we'll just order them by their network numbers.
	// however, we need to leave the server player index 0 because we assume that the person
	// that starts out at index 0 is the server.
#if 0	
	if (topology->player_count > 2)
	{
		qsort(topology->players+1, topology->player_count-1, sizeof(struct NetPlayer), net_compare);
	}
#endif

	update_topology();
	error= distribute_topology(tagSTART_GAME);

	status->single_player= false;

	if(error)
	{
		alert_user(infoError, strNETWORK_ERRORS, netErrCouldntDistribute, error);
		success= false;
	} else {
		success= true;
	}

	return success;
}

void ModemCancelGather(
	void)
{
	assert(modemState==netGathering);

	distribute_topology(tagCANCEL_GAME);
}

long ModemGetNetTime(
	void)
{
	return status->localNetTime;
}

bool ModemChangeMap(	
	struct entry_point *entry)
{
	byte *wad= NULL;
	long length;
	OSErr error= noErr;
	bool success= true;

	/* If the guy that was the server died, and we are trying to change levels, we lose */
	if(status->localPlayerIndex==status->server_player_index && status->localPlayerIndex != 0)
	{
#ifdef DEBUG_NET
		dprintf("Server died, and trying to get another level. You lose;g");
#endif
		success= false;
		set_game_error(gameError, errServerDied);
	}
	else
	{
		// being the server, we must send out the map to everyone.	
		if(status->localPlayerIndex==status->server_player_index) 
		{
			wad= (unsigned char *)get_map_for_net_transfer(entry);
			if(wad)
			{
				length= get_net_map_data_length(wad);
				error= distribute_map_to_all_players(wad, length);
				if(error) success= false;
				set_game_error(systemError, error);
			} else {
//				if (!wad) alert_user(fatalError, strERRORS, badReadMap, -1);
				assert(error_pending());
			}
		} 
		else // wait for de damn map.
		{
			wad= receive_map();
			if(!wad) success= false;
			// Note that NetReceiveMap handles display of its own errors, therefore we don't
			//  assert that an error is pending.....
		}
	
		/* Now load the level.. */
		if (!error && wad)
		{
			/* Note that this frees the wad as well!! */
			process_net_map_data(wad);
		}
	}
	
	return success;
}

short ModemUpdateJoinState(
	void)
{
	OSErr error= noErr;
	short newState= modemState;

	switch (modemState)
	{
		case netJoining:
			/* This state has two valid types of packets. */
			/* 1) Request to join. */
			/* 2) Request for info. (ie NBPLookup...) */
//			if (NetStreamCheckConnectionStatus())
			{
				bool got_packet;
				byte packet_type;
	
				/* Setup the gather data. */
				got_packet= modem_read_packet(&packet_type, packet_buffers[_incoming_packet]);
				if(got_packet)
				{
					struct join_player_data *packet= (struct join_player_data *)packet_buffers[_incoming_packet];
					struct accept_join_data	*accept_packet= (struct accept_join_data *)packet_buffers[_outgoing_packet];

					switch(packet_type)
					{
						case _join_player_packet:
							/* Note that we could set accepted to false if we wanted to for some */
							/*  reason- such as bad serial numbers.... */
							{
								/* Unregister ourselves */
								ModemUnRegisterName();
							
								/* Note that we share the buffers.. */
								status->localPlayerIdentifier= packet->new_local_player_identifier;
								topology->players[status->localPlayerIndex].identifier= status->localPlayerIdentifier;
								
								/* Confirm. */
								accept_packet->accepted= true;
								obj_copy(accept_packet->player, 
									topology->players[status->localPlayerIndex]);
							}
	
							/* Note we don't retry here... */
							error= modem_send_packet(_accept_join_packet, accept_packet);
							if(!error)
							{
								/* Close and reset the connection */
//								error= NetCloseStreamConnection(false);
								if (!error)
								{
//									error= NetStreamWaitForConnection();
									if (!error)
									{
										/* start waiting for another connection */
										if(accept_packet->accepted) newState= netWaiting;
									}
								}
							}
							break;

						case _query_player_packet:
							/* Send back our player information... */
							error= modem_respond_to_lookup();
							break;
							
						default:
dprintf("Got unexpected packet: %d;g", packet_type);
							break;
					}
				}

				/* Check for errors.. */
				if(error)
				{
					newState= netJoinErrorOccurred;
//					NetCloseStreamConnection(false);
					alert_user(infoError, strNETWORK_ERRORS, netErrJoinFailed, error);
				}
			}
			break;
		
		case netWaiting:
//			if (NetStreamCheckConnectionStatus())
			{
				bool got_packet;
				byte packet_type;
			
				/* and now, the packet youÕve all been waiting for ... (the server is trying to
					hook us up with the network topology) */
//				error= NetReceiveStreamPacket(&packet_type, network_adsp_packet);
				got_packet= modem_read_packet(&packet_type, packet_buffers[_incoming_packet]);
				if(got_packet && packet_type==_topology_packet)
				{
					/* Acknowledge the topology.. */
					send_acknowledge(0l);

					/* Copy it in */
					memcpy(topology, packet_buffers[_incoming_packet], sizeof(ModemTopology));

//					if(NetGetTransportType()==kNetworkTransportType)
//					{
//						NetAddrBlock address;
//
//						NetGetStreamAddress(&address);
//						
//						/* for ARA, make stuff in an address we know is correct (donÕt believe the server) */
//						topology->players[0].dspAddress= address;
//						topology->players[0].ddpAddress.aNet= address.aNet;
//						topology->players[0].ddpAddress.aNode= address.aNode;
//						dprintf("ddp %8x, dsp %8x;g;", *((long*)&topology->players[0].ddpAddress),
//							*((long*)&topology->players[0].dspAddress));
//					}

					update_topology();
				
					switch (topology->tag)
					{
						case tagNEW_PLAYER:
							newState= netPlayerAdded;
							break;
							
						case tagCANCEL_GAME:
							newState= netCancelled;
							alert_user(infoError, strNETWORK_ERRORS, netErrServerCanceled, 0);
							break;
							
						case tagSTART_GAME:
							newState= netStartingUp;
							break;
							
						default:
							break;
					}
				
//					error= NetCloseStreamConnection(false);
					if (!error)
					{
//						error= NetStreamWaitForConnection();
						if (!error)
						{
							/* successfully got a new topology structure from the server and closed
								the connection */
						}
					}
					
					if (error)
					{
						newState= netJoinErrorOccurred;
						alert_user(infoError, strNETWORK_ERRORS, netErrJoinFailed, error);
					}
				}
				else
				{
					if(error) 
					{
						newState= netJoinErrorOccurred;
						alert_user(infoError, strNETWORK_ERRORS, netErrJoinFailed, error);
					}
				}
			}
			break;
		
		default:
			newState= NONE;
			break;
	}
	
	/* return netPlayerAdded to tell the caller to refresh his topology, but donÕt change modemState to that */
	if (newState!=netPlayerAdded && newState != NONE) modemState= newState;

	return newState;
}

/* ------------------------------------------------------------------------------------*/
/* 
	Note that just stuffing what the server gave us won't do if there is a bit error
	in the packet. We need to know & request a resend.  That sucks.

	Note also that each machine must process the same number of flags!
*/
static void update_topology(
	void)
{
	/* recalculate localPlayerIndex */					
	for (status->localPlayerIndex=0;status->localPlayerIndex<topology->player_count;++status->localPlayerIndex)
	{
		if (topology->players[status->localPlayerIndex].identifier==status->localPlayerIdentifier) break;
	}
#ifdef DEBUG
	if (status->localPlayerIndex==topology->player_count) dprintf("couldnÕt find my identifier: %p", topology);
#endif
}

static void modem_initialize_topology(
	void *game_data,
	short game_data_size,
	void *player_data,
	short player_data_size)
{
	ModemPlayerPtr net_local_player;
	
	assert(player_data_size>=0&&player_data_size<MAXIMUM_PLAYER_DATA_SIZE);
	assert(game_data_size>=0&&game_data_size<MAXIMUM_GAME_DATA_SIZE);

	/* initialize the local player (assume weÕre index zero, identifier zero) */
	status->localPlayerIndex= status->localPlayerIdentifier= 0;
	net_local_player= topology->players + status->localPlayerIndex;
	net_local_player->identifier= status->localPlayerIdentifier;

	BlockMove(player_data, net_local_player->player_data, player_data_size);
	
	/* initialize the network topology (assume weÕre the only player) */
	topology->player_count= 1;
	topology->nextIdentifier= 1;
	BlockMove(game_data, topology->game_data, game_data_size);
}

/* A maximum packet is a server packet with MAXIMUM_NUMBER_OF_PLAYERS flags */
static long max_packet_length(
	void)
{
	long max_size= -1;
	short packet_type;
	
	for(packet_type= 0; packet_type<NUMBER_OF_PACKET_TYPES; ++packet_type)
	{
		uint16 size= packet_type_size(packet_type);
		if(size>max_size) max_size= size;
	}
	
	return max_size;
}

static void client_send_action_flag(
	long action_flag,
	unsigned long sequence)
{
	struct player_packet_data packet;
	OSErr err;

	SET_CLIENT_PLAYER_IDENTIFIER(&packet, status->localPlayerIndex);
	packet.action_flag= action_flag;
	packet.sequence= sequence;
	
	err= modem_send_packet(_client_packet, &packet);
	vassert(!err, csprintf(temporary, "Acknowledge write_modem returned: %d", err));
}

/* Process the flags in the server packet.  This is called by both the server and the client */
/*  processes */
static void process_server_packet(
	struct server_packet_data *packet_data)
{
	short player_index;

	for(player_index= 0; player_index<topology->player_count; ++player_index)
	{
		uint32 *flag= &packet_data->flags[player_index];

		process_action_flags(player_index, flag, 1);	
#ifdef DEBUG_MODEM
		modem_stats.action_flags_processed++;
#endif
	}

	status->sequence++;
	status->localNetTime++;
}

static void build_server_packet(
	struct server_packet_data *packet)
{
	short player_index;
	
	packet->sequence= status->sequence;
	
	// plug in our action flags.
	for (player_index= 0; player_index<topology->player_count; player_index++)
	{
		if (player_queues[player_index].read_index != player_queues[player_index].write_index)
		{
			packet->flags[player_index]= player_queues[player_index].buffer[player_queues[player_index].read_index];
			player_queues[player_index].read_index++;
			if (player_queues[player_index].read_index >= NET_QUEUE_SIZE) player_queues[player_index].read_index= 0;
		}
		else // we unfortunately need to smear.
		{
			/* We can either set this to zero, or copy what was in there last. */
			/*  which is better? */
			/* By not changing this, we are smearing.. */
			packet->flags[player_index]= 0l;
#ifdef DEBUG_MODEM
			modem_stats.numSmears++;
#endif
		}
	}
}

static void handle_single_player(
	void)
{
	struct server_packet_data *packet_data= (struct server_packet_data *)packet_buffers[_incoming_packet];

	packet_data->sequence= status->sequence;
	packet_data->flags[status->localPlayerIndex]= parse_keymap();
	packet_data->flags[!status->localPlayerIndex]= NONE;
	process_server_packet(packet_data);

	switch(modemState)
	{
		case netStartingUp:
			modemState= netActive;
			break;
					
		case netComingDown:
			modemState= netDown;
			break;
	}
}

static short size_of_players_queue(
	short player_index)
{
	NetQueuePtr queue= &player_queues[player_index];
	short size;

	/* Calculate the size of the local queue */		
	if ((size= queue->write_index-queue->read_index) < 0) 
		size += NET_QUEUE_SIZE;

	return size;
}

#ifdef DEBUG_MODEM
static void print_modem_stats(
	void)
{
	dprintf("numSmears= %d ring sequence= %d;g", modem_stats.numSmears, 
		status->sequence);
	dprintf("localPlayerIndex= %d, server_player_index= %d;g", status->localPlayerIndex, status->server_player_index);
	dprintf("tick_count= %d, localNetTime= %d;g", dynamic_world->tick_count, status->localNetTime);
	dprintf("Aborted reads: %d Max consecutive: %d;g", modem_stats.aborted_read_attempts,
		modem_stats.max_consecutive_aborted_reads);
	dprintf("Stalls: %d Flags: %d Total: %d;g", modem_stats.number_of_stalls,
		modem_stats.flags_processed, modem_stats.number_of_stalls+modem_stats.flags_processed);
	if(status->localPlayerIndex==status->server_player_index)
	{
		dprintf("Server: Req to zero: %d;g", modem_stats.server_unsyncing);
		dprintf("Server packets resent: %d (Sent: %d);g", modem_stats.server_packets_sent-status->sequence,
			modem_stats.server_packets_sent);
		dprintf("Client packets received: %d;g", modem_stats.client_packets_received);
		dprintf("Stream: %d packets sent.  Needed: %d Early Acks: %d;g",
			modem_stats.stream_packets_sent, modem_stats.stream_packets_necessary,
			modem_stats.stream_early_acks);
	} else {
		dprintf("Old Acks: %d Server received: %d Client Packets sent: %d Missed Chances: %d;g", 
			modem_stats.old_acknowledges, modem_stats.server_packets_received,
			modem_stats.client_packets_sent, modem_stats.missed_chances);
	}
	dprintf("");
}
#endif

static short server_packet_size(
	void)
{
	short size;
	
	assert(topology);
	size= offsetof(struct server_packet_data,flags)+(topology->player_count*sizeof(long));
	
	return size;
}

/* We treat the modem like a stream, to avoid lousy transmission lines. */

/* Application specific */
static uint16 packet_type_size(
	short packet_type)
{
	uint16 size;

	switch(packet_type)
	{
		case _query_player_packet:
			size= sizeof(struct lookup_query_data);
			break;
			
		case _lookup_response_packet:
			size= sizeof(struct lookup_packet_data);
			break;
			
		case _client_packet:
			size= sizeof(struct player_packet_data);
			break;
			
		case _server_packet:
			size= server_packet_size();
			break;
			
		case _join_player_packet:
			size= sizeof(struct join_player_data);
			break;
			
		case _accept_join_packet:
			size= sizeof(struct accept_join_data);
			break;

		case _topology_packet:
			size= sizeof(struct ModemTopology);
// dprintf("Top size: %d;g", size);
			break;

		case _sync_packet:
			size= sizeof(struct sync_packet_data);
			break;

		case _acknowledge_packet:
			size= sizeof(struct acknowledge_packet_data);
			break;

			
		case _stream_size_packet:
			size= sizeof(struct stream_size_packet);
			break;
			
		case _stream_packet:
			size= sizeof(struct stream_packet_data);
			break;

		case _stream_acknowledge_packet:
			size= sizeof(struct stream_acknowledge_packet);
			break;

		default:
			size= 0;
			break;
	}
	
	return size;
}

/* This should become some sort of send_acknowledged packet function */
static OSErr distribute_topology(
	short tag)
{
	OSErr error;

	topology->tag= tag;
	error= send_acknowledged_packet(_topology_packet, topology, MODEM_JOIN_TIMEOUT);
	
	return error;
}

/* This should become some sort of send_acknowledged packet function */
static OSErr send_acknowledged_packet(
	short packet_type,
	void *data,
	long timeout)
{
	OSErr error= noErr;
	short playerIndex;
	long initial_ticks= machine_tick_count();
	
	assert(modemState==netGathering || modemState==netStartingUp);
	for (playerIndex=1; !error && playerIndex<topology->player_count; ++playerIndex)
	{
		bool done= false;
	
		while(!error && !done)
		{
			error= modem_send_packet(packet_type, data);
			if (!error)
			{
				bool got_packet;
				byte packet_type;
				
				got_packet= modem_read_packet(&packet_type, packet_buffers[_incoming_packet]);
				if(got_packet && packet_type==_acknowledge_packet)
				{
					struct acknowledge_packet *packet= (struct acknowledge_packet *)packet_buffers[_incoming_packet];
					
//					if(packet->sequence==ACKNOWLEDGE_ACTION_FLAG)
					done= true;
				}
			}

			/* Timeout error.. */			
			if(machine_tick_count()-initial_ticks>timeout) error= errTIMEOUT;
		}
	}
	
	return error;
}

/* Generic.. */
static OSErr modem_send_packet(
	byte packet_type,
	void *buffer)
{
	OSErr err;
	struct stream_header header;
	short index;
	byte *data;
	uint16 size= packet_type_size(packet_type);
	
	header.unique_byte= HEADER_START_BYTE;
	header.packet_type= packet_type;
	header.checksum= 0;
	
	data= (unsigned char *)buffer;
	for(index= 0; index<size; ++index)
	{
		header.checksum+= *data++;
	}

	err= write_modem_endpoint(&header, sizeof(struct stream_header), kNoTimeout);
	if(!err)
	{
		err= write_modem_endpoint(buffer, size, kNoTimeout);
	}

	return err;
}

static void allocate_modem_buffer(
	void)
{
	stream_read_queue.data= new byte[MODEM_QUEUE_SIZE];
	assert(stream_read_queue.data);
}

static void reset_modem_buffer(
	void)
{
	assert(stream_read_queue.data);
	stream_read_queue.read_index= stream_read_queue.write_index= 0;
}

static void modem_interrupt_idle(
	void)
{
	uint16 read_bytes_available;
	OSErr err;
	
	read_bytes_available= modem_read_bytes_available();
	if(read_bytes_available)
	{
		uint16 bytes_to_read= MIN(read_bytes_available, MODEM_QUEUE_SIZE-stream_read_queue.write_index);

		err= read_modem_endpoint(&stream_read_queue.data[stream_read_queue.write_index], 
			bytes_to_read, 0);
		vassert(!err, csprintf(temporary, "Error reading buffer: %d", err));
	
		/* oops, increment past.. */
		stream_read_queue.write_index+= bytes_to_read;
	}
}

static bool modem_read_packet(
	byte *packet_type,
	void *buffer)
{
	bool read_packet= false;
	bool done= false;
	byte *source, *dest;
	short size;

	assert(buffer);

	/* Read the port into our local buffer! */
	modem_interrupt_idle();

	while((stream_read_queue.write_index-stream_read_queue.read_index)>=sizeof(struct stream_header) && !done)
	{
		/* Got a header start byte... */
		if(stream_read_queue.data[stream_read_queue.read_index++]==HEADER_START_BYTE)
		{
			byte actual_packet_type= stream_read_queue.data[stream_read_queue.read_index];
	
			/* If this is a valid packet.. */		
			if(actual_packet_type<NUMBER_OF_PACKET_TYPES)
			{
				short size= packet_type_size(actual_packet_type);

				/* Only continue in this path if we have enough bytes in the buffer to read */
				/*  Note that the -sizeof(byte) is to take into account the header start byte */
//if(actual_packet_type==_topology_packet)
//{
//	dprintf("Expected size: %d Read: %d Write: %d", size,
//		stream_read_queue.read_index, stream_read_queue.write_index);
//}
				if((stream_read_queue.write_index-stream_read_queue.read_index)>=size+sizeof(struct stream_header)-sizeof(byte))
				{
					uint16 checksum;
					uint16 actual_checksum;
					short new_read_index= stream_read_queue.read_index;

					/* Increment past the packet type */
					new_read_index++;
			
					/* This was the correct packet type-> read the checksum... */
					checksum= *((uint16 *) &stream_read_queue.data[new_read_index]);
					new_read_index+= sizeof(uint16);

					/* Copy it in.. */
					/* Now read the bytes in, based on the packet length */
					dest= (unsigned char *)buffer;
					source= &stream_read_queue.data[new_read_index];
					actual_checksum= 0;
					while(--size>=0)
					{
						actual_checksum+= *source;
						*dest++= *source++;
					}	

					/* Go ahead an increment the index, in case it's valid.. */
					new_read_index+= size;
					vassert(new_read_index<=stream_read_queue.write_index, 
						csprintf(temporary, "New Read: %d Write: %d", new_read_index, stream_read_queue.write_index));

					if(actual_checksum==checksum)
					{
						/* We were successful in reading the packet! */
						read_packet= true;
						done= true;

// if(actual_packet_type==_topology_packet) dprintf("Read topology!;g");					
						/* Update the read_index */
						stream_read_queue.read_index= new_read_index;
						
						/* Set the packet type to what we actually read... */
						*packet_type= actual_packet_type;
					} else {
// if(actual_packet_type==_topology_packet) dprintf("Checksum failed!;g");					
						/* Well, that was all for not. The checksums didn't match, so we don't */
						/*  update our read_index, and we try for HEADER_START_BYTE again.. */
					}
				} else {
					/* Ends the if enough_data */
					/* This is possibly a valid packet, but there isn't enough data in the buffer */
					/* Decrement the read index so that we will look at it next time (again) */
					stream_read_queue.read_index--;
					assert(stream_read_queue.read_index>=0);
					done= true; /* Not enough data-> get out of here until next time.. */
				}
			} /* Ends the if(valid_packet_id..) */
		} /* Else read index is incremented... */
	} /* We either read a packet or ran out of buffer. */

#if 0
{
	static short count_times= 3;
	short index;

	if(stream_read_queue.read_index!=stream_read_queue.write_index && --count_times>=0)
	{
		dprintf("Before: Read: %d Write: %d;g", stream_read_queue.read_index, stream_read_queue.write_index);
		for(index= 0; index<stream_read_queue.write_index; ++index)
		{
			dprintf("x[%d]= %x;g", index, stream_read_queue.data[index]);
		}
		dprintf("");
	} else {
		count_times= 0;
	}
}
#endif

	/* Clear out as much of the buffer as we can.. */
	/* Now move the entire thing down in the buffer.. */
	dest= stream_read_queue.data;
	source= &stream_read_queue.data[stream_read_queue.read_index];
	size= stream_read_queue.write_index-stream_read_queue.read_index;
	while(--size>=0)
	{
		*dest++= *source++;
	}
	
	/* Move the pointers down.. */
	stream_read_queue.write_index-= stream_read_queue.read_index;
	stream_read_queue.read_index= 0;

	vassert(stream_read_queue.write_index>=0 && stream_read_queue.write_index<MODEM_QUEUE_SIZE,
		csprintf(temporary, "Write index: %d", stream_read_queue.write_index));
		
	return read_packet;
}

static OSErr distribute_map_to_all_players(
	byte *wad, 
	long wad_length)
{
	long initial_ticks;
	short playerIndex, message_id;
	OSErr error;
	struct progress_statistics progress_data;

	/* Always a max of 2 players */
	message_id= (topology->player_count==2) ? (_distribute_map_single) : (_distribute_map_multiple);
	open_progress_dialog(message_id);

	/* For updating our progress bar.. */
	initial_ticks= TickCount();
	progress_data.total= (topology->player_count-1)*wad_length;
	progress_data.sent= 0l;

	// go ahead and transfer the map to each player
	for (playerIndex= 1; playerIndex<topology->player_count; playerIndex++)
	{
		/* If the player is not dead. */
		error= send_stream_data_with_progress(wad, wad_length, MAP_TRANSFER_TIME_OUT,
			progress_function, &progress_data);
		
		if (error)
		{
			switch(error)
			{
				case errTIMEOUT:
					alert_user(infoError, strNETWORK_ERRORS, netErrWaitedTooLongForMap, error);
					break;
					
				default:
					alert_user(infoError, strNETWORK_ERRORS, netErrCouldntDistribute, error);
					break;
			} 
			break;
		}
	}
	
	/* Fill the progress bar.. */
	if(!error) 
	{
		progress_data.sent= progress_data.total;
		draw_progress_bar(progress_data.sent, progress_data.total);
	}
	close_progress_dialog();
	
	return error;
}

static byte *receive_map(
	void)
{
	byte *map_buffer= NULL;
	long map_size;

	map_size= receive_stream_size(MAP_TRANSFER_TIME_OUT/10);
	if(map_size != -1)
	{
		struct progress_statistics progress_data;
	
		open_progress_dialog(_receiving_map);		
	
		/* For updating our progress bar.. */
		progress_data.total= map_size;
		progress_data.sent= 0l;

		map_buffer = new byte[map_size]; // because my caller expects it to be portable.
// dprintf("Map size: %x;g", map_size);
		if (map_buffer)
		{
			OSErr error;
		
			error= receive_stream_data_with_progress(map_buffer, map_size, MAP_TRANSFER_TIME_OUT,
				progress_function, &progress_data);
			if(error)
			{
				alert_user(infoError, strNETWORK_ERRORS, netErrMapDistribFailed, error);
				delete []map_buffer;
				map_buffer= NULL;
			} else {
				/* Fill the progress bar. */
				progress_data.sent= progress_data.total;
				draw_progress_bar(progress_data.sent, progress_data.total);
			}
		} else {
			alert_user(infoError, strERRORS, outOfMemory, MemError());
		}

		/* Close the progress bar.... */
		close_progress_dialog();
	} else {
		alert_user(infoError, strNETWORK_ERRORS, netErrWaitedTooLongForMap, errTIMEOUT);
	}

	return map_buffer;
}



/* ------------------------ Our implementation of ADSP.. */
static OSErr send_stream_data(
	byte *data,
	long data_length,
	long timeout)
{
	return send_stream_data_with_progress(data, data_length, timeout, NULL, NULL);
}

static OSErr send_stream_data_with_progress(
	byte *data,
	long data_length,
	long timeout,
	void (*progress_function_ptr)(long bytes_written, void *data),
	void *user_data)
{
	short block_count, sequence;
	OSErr error;
	long initial_ticks;
	bool timed_out;	
	long offset;
	struct stream_size_packet first_packet;

	block_count= (data_length/DATA_TRANSFER_CHUNK_SIZE)+((data_length%DATA_TRANSFER_CHUNK_SIZE) ? (1) : (0));

	/* Send the length.. */
	first_packet.size= data_length;
	error= send_acknowledged_packet(_stream_size_packet, &first_packet, 5*MACHINE_TICKS_PER_SECOND);
	if(!error)
	{
		/* Setup the actual variables.. */
		initial_ticks= machine_tick_count();
		offset= 0l;

#ifdef DEBUG_MODEM
		modem_stats.stream_packets_necessary+= block_count;
#endif
		for(sequence= 0, error= noErr, timed_out= false; sequence<block_count && !error && !timed_out; ++sequence)
		{
			struct stream_packet_data *data_packet= (struct stream_packet_data *)packet_buffers[_outgoing_packet];
			long block_size;
			bool acknowledged;
			
			/* Write the packet.. */
			block_size= MIN(DATA_TRANSFER_CHUNK_SIZE, data_length-offset);
	
			/* Setup for the data to be sent.. */
			data_packet->sequence= sequence;
			memcpy(data_packet->data, data+offset, block_size);
	
			/* Send the packet.. */
			acknowledged= false;

			/* Timeout is based on each chunk.. */
			initial_ticks= machine_tick_count();
			do
			{
				/* Only send every other time through the loop.. */
				error= modem_send_packet(_stream_packet, data_packet);
#ifdef DEBUG_MODEM
				modem_stats.stream_packets_sent++;
#endif
				if(!error)
				{
					bool got_packet;
					byte packet_type;
				
					got_packet= modem_read_packet(&packet_type, packet_buffers[_incoming_packet]);
					if(got_packet && packet_type==_stream_acknowledge_packet)
					{
						struct stream_acknowledge_packet *packet= (struct stream_acknowledge_packet *)packet_buffers[_incoming_packet];
						
						if(packet->sequence==sequence) 
						{
							acknowledged= true;
#ifdef DEBUG_MODEM
						} else {
							assert(packet->sequence<sequence);
							modem_stats.stream_early_acks++;
#endif
						}
					}
				}

				if(machine_tick_count()-initial_ticks>timeout) timed_out= true;
			} while(!acknowledged && !timed_out && !error);
			
			/* Get the next one.. */
			offset+= DATA_TRANSFER_CHUNK_SIZE;
	
			if(progress_function_ptr)
			{
				progress_function_ptr(block_size, user_data);
			}
		}
	
		if(!error)
		{
			if(timed_out)
			{
				error= errTIMEOUT;
			} 
			else if(sequence!=block_count)
			{
				error= 1;
			}
		}
	}

	return error;
}

static long receive_stream_size(
	long timeout)
{
	bool done= false;
	bool timed_out= false;
	long size= -1l;
	long initial_ticks= machine_tick_count();

	while(!done && !timed_out)
	{
		byte packet_type;	
		bool got_packet;

		// first we'll get the map length
		got_packet= modem_read_packet(&packet_type, packet_buffers[_incoming_packet]);
		if (got_packet && packet_type==_stream_size_packet)	
		{
			struct stream_size_packet *packet= (struct stream_size_packet *)packet_buffers[_incoming_packet];	

			/* Acknowledge the first map packet.. */	
			send_acknowledge(0l);
			
			size= packet->size;
			done= true;
		}
		
		if(machine_tick_count()-initial_ticks>timeout) timed_out= true;
	}
	
	return size;
}

static OSErr receive_stream_data_with_progress(
	byte *data, 
	long data_length,
	long timeout,
	void (*progress_function_ptr)(long bytes_written, void *data),
	void *user_data)
{
	short block_count, sequence;
	OSErr error;
	long initial_ticks, offset;
	bool timed_out= false;
	
	block_count= (data_length/DATA_TRANSFER_CHUNK_SIZE)+((data_length%DATA_TRANSFER_CHUNK_SIZE) ? (1) : (0));

	initial_ticks= machine_tick_count();
	error= noErr;
	offset= 0l;
	sequence= 0;
	while(sequence<block_count && !timed_out)
	{
		bool got_packet;
		byte packet_type;
		
		got_packet= modem_read_packet(&packet_type, packet_buffers[_incoming_packet]);
		if(got_packet && packet_type==_stream_packet)
		{
			struct stream_packet_data *data_packet= (struct stream_packet_data *)packet_buffers[_incoming_packet];

			/* Acknowledge this sequence...*/
			if(data_packet->sequence==sequence || data_packet->sequence<sequence)
			{
				struct stream_acknowledge_packet ack_packet;

				ack_packet.sequence= data_packet->sequence;
				error= modem_send_packet(_stream_acknowledge_packet, &ack_packet);
			}

			/* Copy the data in */
			if(data_packet->sequence==sequence)
			{
				long block_size;
			
				block_size= MIN(DATA_TRANSFER_CHUNK_SIZE, data_length-offset);
				memcpy(data+offset, data_packet->data, block_size);
				offset+= DATA_TRANSFER_CHUNK_SIZE;
				sequence++;
				
				if(progress_function_ptr)
				{
					progress_function_ptr(block_size, user_data);
				}
				
				/* Timeout is based on each chunk */
				initial_ticks= machine_tick_count();
			}
		}
				
		if((machine_tick_count()-initial_ticks)>timeout) timed_out= true;
	}

	/* We win.. */
	if(!error)
	{
		if(timed_out)
		{
			error= errTIMEOUT;
		} 
		else if(sequence!=block_count)
		{
			error= 1;
		}
	}

	return error;
}

static void progress_function(
	long bytes_written,
	void *data)
{
	struct progress_statistics *progress= (struct progress_statistics *) data;
	
	progress->sent+= bytes_written;
	draw_progress_bar(progress->sent, progress->total);
}


/* ---------------------------------------------------------------------- */
/* ------ NBP Replacement, <sigh>... */

#ifdef DEBUG
static bool lookup_data_valid= false;
#endif
static struct lookup_packet_data lookup_packet;

static OSErr modem_respond_to_lookup(
	void)
{
	OSErr error= noErr;

#ifdef DEBUG
	if(lookup_data_valid)
#endif
	{
// dprintf("Lookup response size: %x", packet_type_size(_lookup_response_packet));
		error= modem_send_packet(_lookup_response_packet, &lookup_packet);
	}

	return error;
}

/* Maybe we setup a timer proc, that responds to querys? */
OSErr ModemRegisterName(
	unsigned char *player_name, 
	unsigned char *type, 
	short version, 
	short socket)
{
	(void) (socket);
	pstrcpy(lookup_packet.entity.objStr, player_name);
	psprintf(lookup_packet.entity.typeStr, "%.*s%d", type[0], type+1, version);
	pstrcpy(lookup_packet.entity.zoneStr, "\p");
	
#ifdef DEBUG
	lookup_data_valid= true;
#endif

	return noErr;
}

void ModemUnRegisterName(
	void)
{
#ifdef DEBUG
	lookup_data_valid= false;
#endif
}


/* -------- lookup functions */
bool ModemEntityNotInGame(
	NetEntityName *entity,
	NetAddrBlock *address)
{
	static bool first_time= true;
	bool valid= false;

	(void)(entity,address);
	
	if(first_time) 
	{
		valid= true;
		first_time= false;
	}
	
	return valid;
}

struct lookup_entity {
	NetEntityName entity;
	NetAddrBlock address;
};

struct lookup_data {
	short lookup_count;
	lookupFilterProcPtr lookupFilterProc;
	lookupUpdateProcPtr lookupUpdateProc;
	struct lookup_entity entities[MAXIMUM_MODEM_PLAYERS-1];
};

struct lookup_data *lookup_data;

OSErr ModemLookupOpen(
	char *name, 
	char *type, 
	char *zone, 
	short version, 
	lookupUpdateProcPtr updateProc, 
	lookupFilterProcPtr filterProc)
{
	OSErr err= noErr;

	(void)(name, type, zone, version,filterProc);
	
	lookup_data= (struct lookup_data *) NewPtrClear(sizeof(struct lookup_data));
	if(lookup_data)
	{
		lookup_data->lookup_count= 0;
//		lookup_data->lookupFilterProc= filterProc;
		lookup_data->lookupFilterProc= ModemEntityNotInGame;
		lookup_data->lookupUpdateProc= updateProc;
	} else {
		err= MemError();
	}
	
	return err;
}

void ModemLookupClose(
	void)
{
	assert(lookup_data);
	DisposePtr((Ptr) lookup_data);
}

void ModemLookupInformation(
	short index, 
	NetAddrBlock *address, 
	NetEntityName *entity)
{
	/* Note that the index is 1 based from the lookup.. */
	assert(lookup_data);
	vassert(index>=0 && index<lookup_data->lookup_count,
		csprintf(temporary, "Index: %d Count: %d", index, lookup_data->lookup_count));

	if(address)
	{
		obj_copy(*address, lookup_data->entities[index].address);
	}
	
	if(entity)
	{
		obj_copy(*entity, lookup_data->entities[index].entity);
	}
}


void ModemLookupUpdate(
	void)
{
	bool got_packet;
	struct lookup_query_data *query_packet= (struct lookup_query_data *)packet_buffers[_outgoing_packet];
	byte packet_type;

	assert(lookup_data);

	/* Only send the query if we haven't had all the responses yet... */
	if(lookup_data->lookup_count!=MAXIMUM_MODEM_PLAYERS-1)
	{
		OSErr error;
	
		/* Send a query packet... */
		/* Note that the query packet has no data in it, so it can be random.. */
		query_packet->unused= 0xd0;
		error= modem_send_packet(_query_player_packet, packet_buffers[_outgoing_packet]);
		vwarn(!error, csprintf(temporary, "Error sending query packet: %d", error));
	
		/* Try to read a response.. */	
		got_packet= modem_read_packet(&packet_type, packet_buffers[_incoming_packet]);
		if(got_packet && packet_type==_lookup_response_packet)
		{
			struct lookup_packet_data *response_packet= (struct lookup_packet_data *) packet_buffers[_incoming_packet];
			NetAddrBlock address; /* Unused, so doesn't ever get initialized.. */
	
	// dprintf("Got lookup response!");
			obj_clear(address);
			
			if (lookup_data->lookupFilterProc == NULL || lookup_data->lookupFilterProc(&response_packet->entity, &address))
			{
				short insertion_point= 0; /* may be different if you had more than one player. */
	
	// dprintf("Added!");
				insertion_point= lookup_data->lookup_count++;
	
				/* Stuff into our array... */
				obj_copy(lookup_data->entities[insertion_point].entity, response_packet->entity);
				obj_copy(lookup_data->entities[insertion_point].address, address);
	
				/* only tell the caller we inserted the new entry after weÕre ready to handle
					him asking us about it */
				if (lookup_data->lookupUpdateProc) lookup_data->lookupUpdateProc(insertEntity, insertion_point);
			}
		}
	}
}

void ModemLookupRemove(
	short index)
{
	assert(index>=0&&index<lookup_data->lookup_count);
	
	/* compact the entity list on top of the deleted entry, decrement lookupCount */
	BlockMove(lookup_data->entities+index+1, lookup_data->entities+index, 
		sizeof(struct lookup_entity)*(lookup_data->lookup_count-index));
	lookup_data->lookup_count-= 1;
	
	/* tell the caller to make the change */
	if (lookup_data->lookupUpdateProc) lookup_data->lookupUpdateProc(removeEntity, index);
}


static void send_acknowledge(
	unsigned long sequence)
{
	OSErr error;
	struct acknowledge_packet_data packet;

	packet.sequence= sequence;
	error= modem_send_packet(_acknowledge_packet, &packet);
	assert(!error);
}

static bool modem_read_fake_packet(
	byte *packet_type, 
	void *data)
{
	if(status->server_player_index==status->localPlayerIndex)
	{
		/* Create fake client packet.. */
		struct player_packet_data *packet;
		
		packet= (struct player_packet_data *)data;
		packet->action_flag= parse_keymap();
		packet->sequence= status->sequence;
		*packet_type= _client_packet;
	} else {
		/* Create fake server packet.. */
		struct server_packet_data *packet;
		
		packet= (struct server_packet_data *)data;
		packet->sequence= status->sequence;
		packet->flags[0]= parse_keymap();
		packet->flags[1]= parse_keymap();

		*packet_type= _server_packet;
	}

	return true;
}


static void build_and_post_async_client_packet(unsigned long sequence);
static void build_and_post_async_server_packet(void);


/* This should be fired off very frequently */
bool packet_tickler(
	void)
{
	bool got_packet;
	byte packet_type;
	
	/* Try to send the outgoing packet if there is one.. */
	if(status->outgoing_packet_valid)
	{
		if(asynchronous_write_completed())
		{
			uint16 size= packet_type_size(status->outgoing_packet_type)+sizeof(struct stream_header);

			/* Write it.. */		
			write_modem_endpoint_asynchronous(packet_buffers[_outgoing_packet], 
				size, kNoTimeout);

			/* Only send one client packet.  If they don't get it, they will request it again.. */
			if(status->outgoing_packet_type==_client_packet)
			{
				status->outgoing_packet_valid= false;
			}
#ifdef DEBUG_MODEM
			switch(status->outgoing_packet_type)
			{
				case _server_packet:
					modem_stats.server_packets_sent++;
					break;
					
				case _client_packet:
					modem_stats.client_packets_sent++;
					break;
			}
#endif
		}
	}
		
	got_packet= modem_read_packet(&packet_type, packet_buffers[_incoming_packet]);
//	got_packet= modem_read_fake_packet(&packet_type, packet_buffers[_incoming_packet]);
	if(got_packet)
	{
		switch(packet_type)
		{
			case _client_packet:
				/* If we are the server... */
				if(status->server_player_index==status->localPlayerIndex)
				{
					struct player_packet_data *player_packet= (struct player_packet_data *)packet_buffers[_incoming_packet];

					/* Process the packet.. */
					if(status->sequence==player_packet->sequence)
					{
						short player_index;

#ifdef DEBUG_MODEM
						modem_stats.client_packets_received++;
#endif
						/* Now we process the server packet that they just ack'd.... */
						process_server_packet((struct server_packet_data *)((byte *) packet_buffers[_outgoing_packet]+sizeof(struct stream_header)));

						/* Stop sending the outgoing server packet.. */
						status->outgoing_packet_valid= false;

						/* Change the modem state... */					
						switch(modemState)
						{
							case netStartingUp:
								modemState= netActive;
								status->maximum_aborted_reads= MAX_SERVER_ABORTED_READS;
								status->aborted_read_timer= MAX_SERVER_ABORTED_READS;
								break;
								
							case netComingDown:
								modemState= netDown;
								break;
						}

						/* Add the action_flags for the next packet */
						for(player_index= 0; player_index<topology->player_count; ++player_index)
						{
							/* Add this flag to the player's queues. */
							NetQueuePtr queue= &player_queues[player_index];
				
							/* Call the local net queueing proc.. */
							if (size_of_players_queue(player_index) < NET_QUEUE_SIZE)
							{
								long action_flag;
							
								if(player_index==status->server_player_index)
								{
									action_flag= parse_keymap();
								} else {
									assert(GET_CLIENT_PLAYER_IDENTIFIER(player_packet)==player_index);
									action_flag= player_packet->action_flag;
								}
	
								queue->buffer[queue->write_index++]= action_flag;
								if (queue->write_index >= NET_QUEUE_SIZE) queue->write_index= 0;
							} else {
								assert(false);
							}
						}
					}
				}
				break;
				
			case _server_packet:
				if(status->server_player_index != status->localPlayerIndex)
				{
					struct server_packet_data *packet_data= (struct server_packet_data *)packet_buffers[_incoming_packet];
					
#ifdef DEBUG_MODEM
					modem_stats.server_packets_received++;
#endif
	
					/* Acknowledge if the sequence is less than or equal to us, but only deal */
					/*  with it if it is equal to our sequence.  This is because the first time */
					/*  the server packet gets in, it updates the sequence, but the server may not */
					/*  have gotten the response.  The next time through the loop, we ack his sequence */
					/*  again, and then next time he goes through the loop, he should send us the next */
					/*  sequence, putting us back in sync.. */
					if(packet_data->sequence<status->sequence)
					{
						/* Note that we can't post a packet if the previous guy is valid.. */
						if(!status->outgoing_packet_valid)
						{
							build_and_post_async_client_packet(packet_data->sequence);
#ifdef DEBUG_MODEM
							modem_stats.old_acknowledges++;
						} else {
							modem_stats.missed_chances++;
#endif
						}
					} 
					else if(packet_data->sequence==status->sequence)
					{
						/* Note that we can't post a packet if the previous guy is valid.. */
						if(!status->outgoing_packet_valid)
						{
							/* Send the action flag.. */
							build_and_post_async_client_packet(status->sequence);
	
							/* Process the flags... (updates time, etc..) */
							process_server_packet(packet_data);
#ifdef DEBUG_MODEM
						} else {
							modem_stats.missed_chances++;
#endif
						}
						
						/* Change the modem state appropriately.. */
						switch(modemState)
						{
							case netStartingUp:
								modemState= netActive;
								status->maximum_aborted_reads= MAX_CLIENT_ABORTED_READS;
								status->aborted_read_timer= MAX_CLIENT_ABORTED_READS;
								break;
								
							case netComingDown:
								modemState= netDown;
								break;
		
							default:
								break;
						}
					}
				}
				break;
				
			case _query_player_packet:
			case _lookup_response_packet:
			case _join_player_packet:
			case _accept_join_packet:
			case _topology_packet:
			case _sync_packet:
			case _acknowledge_packet:
			case _stream_size_packet:
			case _stream_packet:
			case _stream_acknowledge_packet:
				break;
				
			default:
				break;
		}
	} 

	return true;
}


static bool network_queueing_task(
	void)
{
	bool reinstall= (modemState != netDown);

	if(reinstall)
	{
		if(status->single_player)
		{
			handle_single_player();
		} 
		else 
		{
			/* We sent the pack since we were here last.. */
			if(!status->outgoing_packet_valid)
			{
				if(status->server_player_index==status->localPlayerIndex)
				{
					/* Rebuild the server packet, using what's in our queue.. */
					build_and_post_async_server_packet();
				} else {
					/* We are a client.  We don't have to do anything because when we receive */
					/* a server packet we immediately turn it around.. */
				}
				
				/* Reset the aborted timer.. */
				status->aborted_read_timer= status->maximum_aborted_reads;
			} else {
				if(--status->aborted_read_timer<=0)
				{
					dprintf("Error! Player dissappeared!");
					status->single_player= true;
				}
#ifdef DEBUG_MODEM
				modem_stats.max_consecutive_aborted_reads= MAX(
					status->maximum_aborted_reads-status->aborted_read_timer, 
					modem_stats.max_consecutive_aborted_reads);
#endif
			}
		}
	}

	return reinstall;
}


static void build_and_post_async_client_packet(
	unsigned long sequence)
{
	struct stream_header *header= (struct stream_header *)packet_buffers[_outgoing_packet];
	struct player_packet_data *packet=
		(struct player_packet_data *)((byte *) packet_buffers[_outgoing_packet]+sizeof(struct stream_header));
	short index;
	byte *data;
	
	header->unique_byte= HEADER_START_BYTE;
	header->packet_type= _client_packet;
	header->checksum= 0;

	SET_CLIENT_PLAYER_IDENTIFIER(packet, status->localPlayerIndex);
	packet->action_flag= parse_keymap();
	packet->sequence= sequence;
	
	data= (byte *) packet;
	for(index= 0; index<sizeof(struct player_packet_data); ++index)
	{
		header->checksum+= *data++;
	}

	/* let her go.. */
	status->outgoing_packet_type= _client_packet;
	status->outgoing_packet_valid= true;
}

static void build_and_post_async_server_packet(
	void)
{
	struct server_packet_data *server_packet=
		(struct server_packet_data *)((byte *) packet_buffers[_outgoing_packet]+sizeof(struct stream_header));
	struct stream_header *header= (struct stream_header *)packet_buffers[_outgoing_packet];
	byte *data;
	short index;

	header->unique_byte= HEADER_START_BYTE;
	header->packet_type= _server_packet;
	header->checksum= 0;

	/* Build the packet.. */
	build_server_packet(server_packet);

	/* Build the checksum */
	data= (byte *) server_packet;
	for(index= 0; index<packet_type_size(_server_packet); ++index)
	{
		header->checksum+= *data++;
	}

	/* What type of packet */
	status->outgoing_packet_type= _server_packet;

	/* Start our interrupt routine on it's merry way.. */
	status->outgoing_packet_valid= true;
}

