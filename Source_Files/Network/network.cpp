/*
NETWORK.C
Monday, June 20, 1994 12:22:03 PM
Wednesday, June 29, 1994 9:14:21 PM
	made ddp ring work with more than 2 players (upring and downring were confused)
Saturday, July 2, 1994 3:54:12 PM
	simple distribution of map
Friday, July 15, 1994 10:51:38 AM
	gracefully handling players dropping from the game. don't allow quiting from the game while
	we have the ring packet. changed distribution of the map now that we transfer a level at a time.
Sunday, July 17, 1994 4:01:18 PM
	multiple updates per packet
Monday, July 18, 1994 11:51:51 AM
	transfering map in chunks now, since ADSP can only write 64K at a time.
Tuesday, July 19, 1994 7:14:30 PM
	fixed one player ring bug yesterday.
Wednesday, July 20, 1994 12:34:06 AM
	variable number of updates per packet. (can only be adjusted upward, not downward).
Monday, July 25, 1994 9:04:24 PM
	Jason's new algorithm. dropping players and slowing down the ring doesn't work now.
	but performance is much smoother, and better understood, to boot. 
Sunday, August 21, 1994 3:58:23 PM
	about a week ago, added stuff to use the ring to distribute other information, like
	sound or text for the game.

Jan 30, 2000 (Loren Petrich):
	Added some typecasts

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging
*/

/*
I would really like to be able to let the Unysnc packet go around the loop, but it is difficult
	because all the code is currently setup to handle only one packet at a time.
Currently 1 player games (when others are dropped) always have lots of late packets (never get
	acknowledged properly, since they are the only player.)
Note that the unregister isn't fast enough, and that the registration code is stupid.  Also should
	setup the dialog such that it doesn't allow for network play when a player is added.
*/

/*
NetADSPRead() should time out by calling PBControl(dspStatus, ...) to see if there are enough bytes
clearly this is all broken until we have packet types
*/

#include "macintosh_cseries.h"
#include "mytm.h"
#include "map.h"       // for TICKS_PER_SECOND and "struct entry_point"
#include "interface.h" // for transfering map
#include "macintosh_network.h"
#include "game_errors.h"
#include "network_stream.h"
#include "network_modem.h"
#include "progress.h"
#include "portable_files.h"
#include "extensions.h"

// #define TEST_MODEM

#ifdef TEST_MODEM
#include "network_modem_protocol.h"
#endif

#include <stdlib.h>
#include <string.h>

#ifdef env68k
#pragma segment network
#endif

#define NO_PHYSICS

#ifdef DEBUG
//	#define DEBUG_NET
#endif

/* ---------- constants */

#define NET_DEAD_ACTION_FLAG_COUNT (-1)
#define NET_DEAD_ACTION_FLAG (NONE)

#define MAXIMUM_GAME_DATA_SIZE       256
#define MAXIMUM_PLAYER_DATA_SIZE     128
#define MAXIMUM_UPDATES_PER_PACKET    16 // how many action flags per player can be sent in each ring packet
#define UPDATES_PER_PACKET             1  // defines action flags per packet and period of the ring
#define UPDATE_LATENCY                 1

#define NET_QUEUE_SIZE (MAXIMUM_UPDATES_PER_PACKET+1)

#define UNSYNC_TIMEOUT (3*MACINTOSH_TICKS_PER_SECOND) // 3 seconds

#define STREAM_TRANSFER_CHUNK_SIZE (10000)
#define MAP_TRANSFER_TIME_OUT   (60*70) // 70 seconds to wait for map.
#define NET_SYNC_TIME_OUT       (60*50) // 50 seconds to time out of syncing. 

#define kACK_TIMEOUT 40
#define kRETRIES     50  // how many timeouts allowed before dropping the next player
                         // kRETRIES * kACK_TIMEOUT / 1000 = timeout in seconds

#define NUM_DISTRIBUTION_TYPES    3

#define kPROTOCOL_TYPE           69

enum /* tag */
{
	tagRING_PACKET,
	tagACKNOWLEDGEMENT,
	tagCHANGE_RING_PACKET,  // to tell a player to change his downring address. also has action flags.
	
	tagNEW_PLAYER,
	tagCANCEL_GAME,
	tagSTART_GAME,
	
	tagLOSSY_DISTRIBUTION,     // for transfer data other than action flags
	tagLOSSLESS_DISTRIBUTION   // ditto, but currently unimplemented
};

enum
{
	typeSYNC_RING_PACKET,    // first packet of the game, gets everyone in the game
	typeTIME_RING_PACKET,    // second packet of the game, sets everyone's clock
	typeNORMAL_RING_PACKET,   // all the other packets of the game
	
	typeUNSYNC_RING_PACKET,	// last packet of the game, get everyone unsynced properly.
	typeDEAD_PACKET	// This is simply a convenience for a switch. This packet never leaves the server.
};

/* ---------- structures */

struct NetPacketHeader
{
	short  tag;
	long   sequence;
	
	/* data */
};
typedef struct NetPacketHeader NetPacketHeader, *NetPacketHeaderPtr;

struct NetPacket
{
	byte ring_packet_type;         // typeSYNC_RING_PACKET, etc...
	byte server_player_index;
	long server_net_time;
	short required_action_flags;                         // handed down from on high (the server)
	short action_flag_count[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];  // how many each player actually has.
	long action_flags[1];
};
typedef struct NetPacket NetPacket, *NetPacketPtr;

struct NetDistributionPacket
{
	short distribution_type;  // type of information
	short first_player_index; // who sent the information
	short data_size;          // how much they're sending.
	byte  data[2];            // the chunk ’o shit to send
};
typedef struct NetDistributionPacket NetDistributionPacket, *NetDistributionPacketPtr;

struct NetPlayer
{
	AddrBlock dspAddress, ddpAddress;
	
	short identifier;

	boolean net_dead; // only valid if you are the server.

	byte player_data[MAXIMUM_PLAYER_DATA_SIZE];
};
typedef struct NetPlayer NetPlayer, *NetPlayerPtr;

struct NetTopology
{
	short tag;
	short player_count;
	
	short nextIdentifier;
	
	byte game_data[MAXIMUM_GAME_DATA_SIZE];
	
	struct NetPlayer players[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];
};
typedef struct NetTopology NetTopology, *NetTopologyPtr;

struct NetStatus
{
	/* we receive packets from downring and send them upring */
	AddrBlock upringAddress, downringAddress;
	short upringPlayerIndex;
	
	long lastValidRingSequence; /* the sequence number of the last valid ring packet we received */
	long ringPacketCount; /* the number of ring packets we have received */
	
	boolean receivedAcknowledgement; /* TRUE if we received a valid acknowledgement for the last ring packet we sent */
	boolean canForwardRing; 
	boolean clearToForwardRing; /* TRUE if we are the server and we got a valid ring packet but we didn’t send it */
	boolean acceptPackets; /* TRUE if we want to get packets */
	boolean acceptRingPackets; /* TRUE if we want to get ring packets */
	boolean oldSelfSendStatus;

	short retries;
	
	short action_flags_per_packet;
	short last_extra_flags;
	short update_latency;
	
	boolean iAmTheServer;
	boolean single_player; /* Set true if I dropped everyone else. */
	short server_player_index;
	short new_packet_tag; /* Valid _only_ if you are the server, and is only set when you just became the server. */

	byte *buffer;
	
	long localNetTime;
};
typedef struct NetStatus NetStatus, *NetStatusPtr;

struct NetQueue
{
	short read_index, write_index;
	long buffer[NET_QUEUE_SIZE];
};
typedef struct NetQueue NetQueue, *NetQueuePtr;

struct NetDistributionInfo
{
	boolean              type_in_use;
	boolean              lossy;
	NetDistributionProc  distribution_proc;
};

typedef struct NetDistributionInfo NetDistributionInfo, *NetDistributionInfoPtr;

#define errInvalidMapPacket (42)

/* ===== application specific data structures/enums */
struct gather_player_data {
	short new_local_player_identifier;
};

struct accept_gather_data {
	boolean accepted;
	NetPlayer player;
};

enum {
	_join_player_packet,
	_accept_join_packet,
	_topology_packet,
	_stream_size_packet,
	_stream_data_packet,
	NUMBER_OF_BUFFERED_STREAM_PACKET_TYPES,
	NUMBER_OF_STREAM_PACKET_TYPES= 	NUMBER_OF_BUFFERED_STREAM_PACKET_TYPES
};
/* ===== end of application specific data structures/enums */

/* ---------- globals */

static short ddpSocket; /* our ddp socket number */
static DDPFramePtr ackFrame, ringFrame; /* ddp frames for sending ring/acknowledgement packets */
static DDPFramePtr distributionFrame;

static short localPlayerIndex, localPlayerIdentifier;
static NetTopologyPtr topology;
static volatile NetStatusPtr status;
static char *network_adsp_packet;

static short netState= netUninitialized;

static myTMTaskPtr resendTMTask = (myTMTaskPtr) NULL;
static myTMTaskPtr serverTMTask = (myTMTaskPtr) NULL;
static myTMTaskPtr queueingTMTask= (myTMTaskPtr) NULL;

static volatile NetQueue local_queue;

static short initial_updates_per_packet = UPDATES_PER_PACKET;
static short initial_update_latency = UPDATE_LATENCY;

static NetDistributionInfo distribution_info[NUM_DISTRIBUTION_TYPES];

/* ---------- private prototypes */
void NetPrintInfo(void);

static void NetInitializeTopology(void *game_data, short game_data_size, void *player_data, short player_data_size);
static void NetLocalAddrBlock(AddrBlock *address, short socketNumber);

static void NetProcessLossyDistribution(void *buffer);
static void NetProcessIncomingBuffer(void *buffer, long buffer_size, long sequence);

static void NetDDPPacketHandler(DDPPacketBufferPtr packet);

static long NetPacketSize(NetPacketPtr packet);
static void NetBuildRingPacket(DDPFramePtr frame, byte *data, short data_size, long sequence);
static void NetBuildFirstRingPacket(DDPFramePtr frame, long sequence);
static void NetRebuildRingPacket(DDPFramePtr frame, short tag, long sequence);
static void NetAddFlagsToPacket(NetPacketPtr packet);
	
static void NetSendRingPacket(DDPFramePtr frame);
static void NetSendAcknowledgement(DDPFramePtr frame, long sequence);
static boolean NetDelayedSendRingFrame(void);

static boolean NetCheckResendRingPacket(void);
static boolean NetServerTask(void);
static boolean NetQueueingTask(void);
static boolean NetCheckForwardRingPacket(void);

static int net_compare(void const *p1, void const *p2);

static void NetUpdateTopology(void);
static short NetAdjustUpringAddressUpwards(void);
static OSErr NetDistributeTopology(short tag);

static boolean NetSetSelfSend(boolean on);

static short NetSizeofLocalQueue(void);

static void process_packet_buffer_flags(void *buffer, long buffer_size, short packet_tag);
static void process_flags(NetPacketPtr packet_data);

static OSErr NetDistributeGameDataToAllPlayers(byte *wad_buffer, long wad_length);
static byte *NetReceiveGameData(void);

/* Note that both of these functions may result in a change of gatherer.  the first one is */
/*  called when the other guy hasn't responded after a kRETRY times to our packet, so we */
/*  drop him and if he was gatherer, we become the gatherer.  The second function is called */
/*  when the gatherer sends out an unsync packet, but we aren't ready to quit.  Therefore we */
/*  must become the gatherer. */
static void drop_upring_player(void);
static void become_the_gatherer(NetPacketPtr packet_data);

static void *receive_stream_data(long *length, OSErr *receive_error);
static OSErr send_stream_data(void *data, long length);

/* ADSP Packets.. */

#ifdef DEBUG_NET
struct network_statistics {
	long numSmears;
	long numCountChanges;
	
	long ontime_acks;
	long sync_ontime_acks;
	long time_ontime_acks;
	long unsync_ontime_acks;
	long dead_ontime_acks;
	
	long late_acks;
	long packets_from_the_unknown;
	long retry_count;
	long sync_retry_count;
	long time_retry_count;
	long unsync_retry_count;
	long dead_retry_count;

	long late_unsync_rings;
	long late_sync_rings;
	long late_rings;
	long late_time_rings;
	long late_dead_rings;

	long change_ring_packet_count;

	long rebuilt_server_tag;
	long packets_with_zero_flags;

	short spurious_unsyncs;
	short unsync_while_coming_down;
	short upring_drops;
	short server_set_required_flags_to_zero;
	short unsync_returned_to_sender;
	short server_unsyncing;
	short assuming_control;
	short assuming_control_on_retry;
	short server_bailing_early;

	unsigned long action_flags_processed;
} net_stats;

#ifdef STREAM_NET
static void open_stream_file(void);
static void debug_stream_of_flags(long action_flag, short player_index);
static void close_stream_file(void);
#endif
#endif

/* ---------- code */

/*
--------
NetEnter
--------

	(no parameters)

make sure the MPP and DSP drivers are open and remembers the DSP’s reference number in dspRefNum.
opens two connection end (upring and downring ring connections).

-------
NetExit
-------

	(no parameters)

frees memory, disposes our three connection ends, etc.

--------
NetState
--------

	<--- state of the network
*/

boolean NetEnter(
	void)
{
	OSErr error;
	short i;
	boolean success= TRUE; /* optimism */

#ifdef TEST_MODEM
	success= ModemEnter();
#else
	
	assert(netState==netUninitialized);

	/* if this is the first time we’ve been called, add NetExit to the list of cleanup procedures */
	{
		static boolean added_exit_procedure= FALSE;
		
		if (!added_exit_procedure) atexit(NetExit);
		added_exit_procedure= TRUE;
	}

	for (i = 0; i < NUM_DISTRIBUTION_TYPES; i++)
	{
		distribution_info[i].type_in_use = FALSE;
	}
	
	error= NetDDPOpen();	
	if (!error)
	{
		error= NetADSPOpen();
		if (!error)
		{
			topology= (NetTopologyPtr) NewPtrClear(sizeof(NetTopology));
			status= (NetStatusPtr) NewPtrClear(sizeof(NetStatus));
			network_adsp_packet= (char *) NewPtrClear(MaxStreamPacketLength());
			error= MemError();
			if(!error) 
			{
				status->buffer= (byte *) NewPtrClear(ddpMaxData);
				error= MemError();
				if(!error)
				{
					/* Set the server player identifier */
					NetSetServerIdentifier(0);
				
					ringFrame= NetDDPNewFrame();
					error= MemError();
					if(!error)
					{
						ackFrame= NetDDPNewFrame();
						error= MemError();
						if(!error)
						{
							distributionFrame= NetDDPNewFrame();
							error= MemError();
							if (!error)
							{
								error= NetStreamEstablishConnectionEnd();
								if (error==noErr)
								{
									error= NetDDPOpenSocket(&ddpSocket, NetDDPPacketHandler);
									if (error==noErr)
									{
										status->oldSelfSendStatus= NetSetSelfSend(TRUE);
										status->server_player_index= 0;
										status->single_player= FALSE;
										netState= netDown;
#ifdef DEBUG_NET
										memset(&net_stats, 0, sizeof(net_stats));
#ifdef STREAM_NET
										open_stream_file();
#endif
#endif
									}
								}
							}
						}
					}
				}
			} 
		}
	}

	/* Handle our own errors.. */
	if(error)
	{
		alert_user(infoError, strNETWORK_ERRORS, netErrCantContinue, error);
		NetExit();
		success= FALSE;
	}
#endif
	
	return success;
}

void NetExit(
	void)
{
	OSErr error;

#ifdef TEST_MODEM
	ModemExit();
#else

	/* These functions do the right thing for NULL pointers */
	resendTMTask= myTMRemove(resendTMTask);
	serverTMTask= myTMRemove(serverTMTask);
	queueingTMTask= myTMRemove(queueingTMTask);

	if (netState!=netUninitialized)
	{
		error= NetCloseStreamConnection(FALSE);
		vwarn(!error, csprintf(temporary, "NetADSPCloseConnection returned %d", error));
		error= NetStreamDisposeConnectionEnd();
		vwarn(!error, csprintf(temporary, "NetADSPDisposeConnectionEnd returned %d", error));
		if (!error)
		{
			error= NetDDPCloseSocket(ddpSocket);
			vwarn(!error, csprintf(temporary, "NetDDPCloseSocket returned %d", error));
			if (!error)
			{
				NetSetSelfSend(status->oldSelfSendStatus);

#ifdef DEBUG_NET
				NetPrintInfo();
#ifdef STREAM_NET
				close_stream_file();
#endif
#endif
				DisposePtr((Ptr)topology);
				DisposePtr((Ptr)status->buffer);
				DisposePtr((Ptr)status);
				status= NULL;
				topology= NULL;
				
				NetDDPDisposeFrame(ackFrame);
				NetDDPDisposeFrame(ringFrame);
				NetDDPDisposeFrame(distributionFrame);
								
				netState= netUninitialized;
			}
		}
	}

	NetUnRegisterName();
	NetLookupClose();
	NetDDPClose();
	NetADSPClose();
#endif
		
	return;
}

/* Add a function for a distribution type. returns the type, or NONE if it can't be
 * installed. It's safe to call this function multiple times for the same proc. */
short NetAddDistributionFunction(
	NetDistributionProc proc, 
	boolean lossy)
{
	short    i;
	boolean  found_slot = FALSE;

#ifdef TEST_MODEM
	return ModemAddDistributionFunction(proc, lossy);
#else

	assert(lossy == TRUE); // until we decide to support it.
	
	// see if it's already installed.
	for (i = 0; i < NUM_DISTRIBUTION_TYPES; i++)
	{
		if (distribution_info[i].type_in_use && distribution_info[i].distribution_proc == proc)
		{
			distribution_info[i].lossy = lossy; // maybe they want to change it. who am i to argue?
			found_slot = TRUE;
			break;
		}
	}
	
	if (!found_slot) // not installed, install it.
	{
		for (i = 0; i < NUM_DISTRIBUTION_TYPES; i++)
		{
			if (!distribution_info[i].type_in_use)
			{
				distribution_info[i].type_in_use = TRUE;
				distribution_info[i].lossy = lossy;
				distribution_info[i].distribution_proc = proc;
				found_slot = TRUE;
				break;
			}
		}
	}
	
	return found_slot ? i : NONE;
#endif
}

/* Remove a distribution proc that has been installed. */
void NetRemoveDistributionFunction(
	short type)
{
#ifdef TEST_MODEM
	ModemRemoveDistributionFunction(type);
#else
	distribution_info[type].type_in_use = FALSE;
#endif
	return;
}

/* Distribute information to the whole net. */
void NetDistributeInformation(
	short type, 
	void *buffer, 
	short buffer_size, 
	boolean send_to_self)
{
	NetDistributionPacket distribution_header;

#ifdef TEST_MODEM
	ModemDistributeInformation(type, buffer, buffer_size, send_to_self);
#else

	// Sanity Check! Sanity Check!
	// Hand Check! Hand Check!
	assert(buffer_size <= MAX_NET_DISTRIBUTION_BUFFER_SIZE);
	assert(type >= 0 && type < NUM_DISTRIBUTION_TYPES);
	assert(distribution_info[type].type_in_use);
	
	if (send_to_self)
	{
		distribution_info[type].distribution_proc(buffer, buffer_size, localPlayerIndex);
	}

	distributionFrame->data_size = sizeof(NetPacketHeader) + sizeof(NetDistributionPacket) + buffer_size;
	{
		NetPacketHeaderPtr header = (NetPacketHeaderPtr) distributionFrame->data;
		
		header->tag = distribution_info[type].lossy ? tagLOSSY_DISTRIBUTION : tagLOSSLESS_DISTRIBUTION;
		header->sequence = 0;
	}
	distribution_header.distribution_type = type;
	distribution_header.first_player_index = localPlayerIndex;
	distribution_header.data_size = buffer_size;
	BlockMove(&distribution_header, distributionFrame->data+sizeof(NetPacketHeader), sizeof(NetDistributionPacket));
	BlockMove(buffer, 
		distributionFrame->data + (sizeof(NetPacketHeader) + sizeof(NetDistributionPacket) - (2*sizeof(byte))), 
		buffer_size);
	
	NetDDPSendFrame(distributionFrame, &status->upringAddress, kPROTOCOL_TYPE, ddpSocket);
#endif
	
	return;
}

short NetState(
	void)
{
	return netState;
}

/*
---------
NetGather
---------

	---> game data (pointer to typeless data no bigger than MAXIMUM_GAME_DATA_SIZE)
	---> size of game data
	---> player data of gathering player (no bigger than MAXIMUM_PLAYER_DATA_SIZE)
	---> size of player data
	---> lookupUpdateProc (we call NetLookupOpen() and NetLookupClose())
	
	<--- error

start gathering players.

---------------
NetGatherPlayer
---------------

	---> player index (into the array of looked up names)
	
	<--- error

bring the given player into our game.

---------------
NetCancelGather
---------------

	<--- error

tells all players in the game that the game has been cancelled.

--------
NetStart
--------

	<--- error

start the game with the existing topology (which all players should have)
*/

boolean NetGather(
	void *game_data,
	short game_data_size,
	void *player_data,
	short player_data_size)
{
#ifdef TEST_MODEM
	return ModemGather(game_data, game_data_size, player_data, player_data_size);
#else

	NetInitializeTopology(game_data, game_data_size, player_data, player_data_size);
	netState= netGathering;
#endif
	
	return TRUE;
}

void NetCancelGather(
	void)
{
#ifdef TEST_MODEM
	ModemCancelGather();
#else
	assert(netState==netGathering);

	NetDistributeTopology(tagCANCEL_GAME);
#endif

	return;
}

boolean NetStart(
	void)
{
	OSErr error;
	boolean success;

#ifdef TEST_MODEM
	success= ModemStart();
#else
	assert(netState==netGathering);

	// how about we sort the players before we pass them out to everyone?
	// This is an attempt to have a slightly more efficent ring in a multi-zone network.
	// we should really do some sort of pinging to determine an optimal order (or perhaps
	// sort on hop counts) but we'll just order them by their network numbers.
	// however, we need to leave the server player index 0 because we assume that the person
	// that starts out at index 0 is the server.
	
	if (topology->player_count > 2)
	{
		qsort(topology->players+1, topology->player_count-1, sizeof(struct NetPlayer), net_compare);
	}

	NetUpdateTopology();
	error= NetDistributeTopology(tagSTART_GAME);

	if(error)
	{
		alert_user(infoError, strNETWORK_ERRORS, netErrCouldntDistribute, error);
		success= FALSE;
	} else {
		success= TRUE;
	}
#endif

	return success;
}

static int net_compare(
	void const *p1, 
	void const *p2)
{
	word base_network_number;
	word p1_network_number, p2_network_number;
	
	base_network_number = topology->players[0].ddpAddress.aNet; // get server's addres.
	p1_network_number = ((struct NetPlayer const *)p1)->ddpAddress.aNet;
	p2_network_number = ((struct NetPlayer const *)p2)->ddpAddress.aNet;
	if (p1_network_number == p2_network_number)
		return 0;
	if (p1_network_number >= base_network_number && p2_network_number >= base_network_number)
		return p1_network_number - p2_network_number;
	else if (p1_network_number < base_network_number && p2_network_number < base_network_number)
		return p1_network_number - p2_network_number;
	else if (p1_network_number >= base_network_number)
		return -1;
	else // p2_network_number >= base_network_number
		return 1;
}

/*
-----------
NetGameJoin
-----------

	---> player name (to register)
	---> player type (to register)
	---> player data (no larger than MAXIMUM_PLAYER_DATA_SIZE)
	---> size of player data
	
	<--- error

------------------
NetUpdateJoinState
------------------

	<--- new state (==netJoined,netWaiting,netStartingUp)

-------------
NetCancelJoin
-------------

	<--- error

can’t be called after the player has been gathered
*/

boolean NetGameJoin(
	unsigned char *player_name,
	unsigned char *player_type,
	void *player_data,
	short player_data_size,
	short version_number)
{
	OSErr error;
	boolean success= FALSE;

#ifdef TEST_MODEM
	success= ModemGameJoin(player_name, player_type, player_data, player_data_size, version_number);
#else
	/* initialize default topology (no game data) */
	NetInitializeTopology((void *) NULL, 0, player_data, player_data_size);
	
	/* register our downring socket with the net so gather dialogs can find us */
	error= NetRegisterName(player_name, player_type, version_number, 
		NetGetStreamSocketNumber());
	
	if (error==noErr)
	{
		error= NetStreamWaitForConnection();
		if (error==noErr)
		{
			/* we’re registered and awaiting a connection request */
			netState= netJoining;
			success= TRUE;
		}
	}
	
	if(error)
	{
		alert_user(infoError, strNETWORK_ERRORS, netErrCouldntJoin, error);
	}
#endif
	
	return success;
}

void NetCancelJoin(
	void)
{
	OSErr error;

#ifdef TEST_MODEM
	ModemCancelJoin();
#else
	
	assert(netState==netJoining||netState==netWaiting||netState==netCancelled||netState==netJoinErrorOccurred);
	
	error= NetUnRegisterName();
	if (error==noErr)
	{
		error= NetCloseStreamConnection(TRUE); /* this should stop the ocPassive OpenConnection */
		if (error==noErr)
		{
			/* our name has been unregistered and our connection end has been closed */
		}
	}
#endif	
	return;
}

/*
-------
NetSync
-------

	(no parameters)

make sure all players are present (by waiting for the ring to come around twice).  this is what
actually jump-starts the ring.

returns TRUE if we synched successfully. FALSE otherwise.

-------
NetUnSync
-------

	(no parameters)

called at the end of a network game to ensure a clean exit from the net game.
(we make sure that we don’t quit the game holding the ring packet.)

*/

/* 
	Externally, this is only called before a new game.  It removes the reliance that 
	localPlayerIndex of zero is the server, which is not necessarily true if we are
	resyncing for another cooperative level.
*/
void NetSetServerIdentifier(
	short identifier)
{
	assert(status);
	status->server_player_index= identifier;
}

boolean NetSync(
	void)
{
	long ticks;
	boolean success= TRUE;
#ifdef TEST_MODEM
	return ModemSync();
#else

	status->action_flags_per_packet= initial_updates_per_packet;
	status->update_latency= initial_update_latency;
	status->lastValidRingSequence= 0;
	status->ringPacketCount= 0;
//	status->server_player_index= 0;
	status->last_extra_flags= 0;
	status->acceptPackets= TRUE; /* let the PacketHandler see incoming packets */
	status->acceptRingPackets= TRUE;
	local_queue.read_index= local_queue.write_index= 0;

	netState= netStartingUp;
	
	/* if we are the server (player index zero), start the ring */
	if (localPlayerIndex==status->server_player_index)
	{
		status->iAmTheServer= TRUE;

		/* act like somebody just sent us this packet */
		status->ringPacketCount= 1;
		
		NetBuildFirstRingPacket(ringFrame, status->lastValidRingSequence+1);
		NetSendRingPacket(ringFrame);
	}
	else
	{
		status->iAmTheServer = FALSE;
	}

	/* once we get a normal packet, netState will be set, and we can cruise. */
	ticks= TickCount();
	while (success && netState != netActive) // packet handler changes this variable.
	{
		if (TickCount() - ticks > NET_SYNC_TIME_OUT)
		{
			alert_user(infoError, strNETWORK_ERRORS, netErrSyncFailed, 0);

			/* How did Alain do this? */
			status->acceptPackets= FALSE;
			status->acceptRingPackets= FALSE;
			netState= netDown;
			success= FALSE;
		}
	}
#endif
	
	return success;
}

/*
	New unsync:
	1) Server tells everyone to give him 0 action flags.
	2) Server then waits for the packet to go all the way around the loop.
*/
boolean NetUnSync(
	void)
{
	boolean success= TRUE;
	long ticks;

#ifdef TEST_MODEM
	success= ModemUnsync();
#else
	
	if (netState==netStartingUp || netState==netActive)
	{
		netState= netComingDown;
		
		/* Next time the server receives a packet, and if the netState==netComingDown */
		/*  the server will send a packet with zero flags, which means process the remaining */
		/*  flags, and get ready to change level.  Once the packet with zero flags gets back */
		/*  to the server, the server sends an unsync ring packet.  This will cause all the other */
		/*  machines to unsync, and when the server gets the packet back, it turns the net off */

		ticks= TickCount();
		// we wait until the packet handler changes "acceptPackets" or until we hit a serious
		// timeout, in case we are quitting and someone else is refusing to give up the ring.
		while((status->acceptRingPackets || !status->receivedAcknowledgement)
				&& (TickCount()-ticks<UNSYNC_TIMEOUT))
			;
	}
	if(status->acceptRingPackets) 
	{
		status->acceptRingPackets= success= FALSE;
	}
	status->acceptPackets= FALSE; // in case we just timed out.
	netState= netDown;

#ifdef DEBUG_NET
	dprintf("Flags processed: %d Time: %d;g", net_stats.action_flags_processed, TickCount()-ticks);
	net_stats.action_flags_processed= 0;
#endif
#endif	

	return success;
}

/*
net accessor functions
*/

short NetGetLocalPlayerIndex(
	void)
{
#ifdef TEST_MODEM
	return ModemGetLocalPlayerIndex();
#else
	assert(netState!=netUninitialized&&netState!=netDown&&netState!=netJoining);

	return localPlayerIndex;
#endif
}

short NetGetPlayerIdentifier(
	short player_index)
{
#ifdef TEST_MODEM
	return ModemGetPlayerIdentifier(player_index);
#else
	assert(netState!=netUninitialized&&netState!=netDown&&netState!=netJoining);
	assert(player_index>=0&&player_index<topology->player_count);
	
	return topology->players[player_index].identifier;
#endif
}

boolean NetNumberOfPlayerIsValid(
	void)
{
	boolean valid;

#ifdef TEST_MODEM
	valid= ModemNumberOfPlayerIsValid();
#else
	switch(netState)
	{
		case netUninitialized:
		case netJoining:
			valid= FALSE;
			break;
		default:
			valid= TRUE;
			break;
	}
#endif
	
	return valid;
}

short NetGetNumberOfPlayers(
	void)
{
#ifdef TEST_MODEM
	return ModemGetNumberOfPlayers();
#else
	assert(netState!=netUninitialized /* &&netState!=netDown*/ &&netState!=netJoining);
	
	return topology->player_count;
#endif
}

void *NetGetPlayerData(
	short player_index)
{
#ifdef TEST_MODEM
	return ModemGetPlayerData(player_index);
#else
	assert(netState!=netUninitialized/* && netState!=netDown */ &&netState!=netJoining);
	assert(player_index>=0&&player_index<topology->player_count);
	
	return topology->players[player_index].player_data;
#endif
}

static void NetSetPlayerData(
	short player_index,
	void *data,
	long data_size)
{
	assert(data_size>=0&&data_size<=MAXIMUM_PLAYER_DATA_SIZE);
	BlockMove(data, topology->players[player_index].player_data, data_size);
	
	return;
}

void *NetGetGameData(
	void)
{
#ifdef TEST_MODEM
	return ModemGetGameData();
#else
	assert(netState!=netUninitialized&&netState!=netDown&&netState!=netJoining);
	
	return topology->game_data;
#endif
}

/*
------------------
NetEntityNotInGame
------------------

	---> entity
	---> address
	
	<--- TRUE if the entity is not in the game, FALSE otherwise

used to filter entities which have been added to a game out of the lookup list
*/

/* if the given address is already added to our game, filter it out of the gather dialog */
boolean NetEntityNotInGame(
	EntityName *entity,
	AddrBlock *address)
{
	short player_index;
	boolean valid= TRUE;
	
	(void) (entity);
	
	for (player_index=0;player_index<topology->player_count;++player_index)
	{
		AddrBlock *player_address= &topology->players[player_index].dspAddress;
		
		if (address->aNode==player_address->aNode && address->aSocket==player_address->aSocket &&
			address->aNet==player_address->aNet)
		{
			valid= FALSE;
			break;
		}
	}
	
	return valid;
}

/* ---------- private code */

/*
-------------------
NetDDPPacketHandler
-------------------

	---> DDPPacketBufferPtr

called at interrupt time; will send an acknowledgement and (if not the server node) forward the
ring packet and spawn a time manager task to verify that it was acknowledged.  because these all
work off global data structures, we can only have one ring packet ‘in the air’ (i.e., waiting to
be acknowledged) at a time.
*/

void NetDDPPacketHandler(
	DDPPacketBufferPtr packet)
{
	static already_here= FALSE;
	NetPacketHeaderPtr header= (NetPacketHeaderPtr) packet->datagramData;
	
	assert(!already_here);
	already_here = TRUE;
	
	if (status->acceptPackets)
	{
		if (packet->datagramSize >= sizeof(NetPacketHeader) && packet->protocolType == kPROTOCOL_TYPE)
		{
			switch (header->tag)
			{
				case tagLOSSLESS_DISTRIBUTION:
					vpause("received lossless distribution packet. not implemented.");
					break;
				case tagLOSSY_DISTRIBUTION:
					NetProcessLossyDistribution(packet->datagramData+sizeof(NetPacketHeader));
					break;
				case tagACKNOWLEDGEMENT:
					if (/*packet->sourceAddress.aNet == status->upringAddress.aNet && */
						packet->sourceAddress.aNode == status->upringAddress.aNode &&
						packet->sourceAddress.aSocket == status->upringAddress.aSocket)
					{
						if (header->sequence==status->lastValidRingSequence+1)
						{
							/* on-time acknowledgement; set a global so our time manager task doesn’t resend
								this packet when it fires */
//								dprintf("ontime ack;g");
#ifdef DEBUG_NET
							{
								NetPacketPtr packet_data= (NetPacketPtr) (ringFrame->data+sizeof(NetPacketHeader));
								switch(packet_data->ring_packet_type)
								{
									case typeSYNC_RING_PACKET:
										net_stats.sync_ontime_acks++;
										break;
										
									case typeTIME_RING_PACKET:
										net_stats.time_ontime_acks++;
										break;
							
									case typeUNSYNC_RING_PACKET:
										net_stats.unsync_ontime_acks++;
										break;
							
									case typeNORMAL_RING_PACKET:
										net_stats.ontime_acks++;
										break;
							
									case typeDEAD_PACKET:
										net_stats.dead_ontime_acks++;
										break;
										
									default:
										// LP change:
										assert(false);
										// halt();
										break;
								}
							}
#endif
							status->receivedAcknowledgement= TRUE;
						}
						else
						{
							if (header->sequence<=status->lastValidRingSequence)
							{
								/* late acknowledgement; ignore */
//								dprintf("late ack;g");
#ifdef DEBUG_NET
								net_stats.late_acks++;
#endif
							}
							else
							{
								/* early acknowledgement; wet our pants (this should never happen) */
//								dprintf("early ack (%d>%d);g", header->sequence, status->lastValidRingSequence);
								// LP change:
								assert(false);
								// halt();
							}
						}
					}
					break;
					
				case tagCHANGE_RING_PACKET:
					status->downringAddress.aNet= packet->sourceAddress.aNet;
					status->downringAddress.aNode= packet->sourceAddress.aNode;
					status->downringAddress.aSocket= packet->sourceAddress.aSocket;

#ifdef DEBUG_NET
					net_stats.change_ring_packet_count++;
#endif
//					dprintf("got change ring packet %d;g", header->sequence);
					/* fall through to tagRING_PACKET */
				
				case tagRING_PACKET:
					if(status->acceptRingPackets)
					{
						if (/* packet->sourceAddress.aNet == status->downringAddress.aNet && */
							packet->sourceAddress.aNode == status->downringAddress.aNode &&
							packet->sourceAddress.aSocket == status->downringAddress.aSocket)
						{
							if (header->sequence <= status->lastValidRingSequence)
							{
	#ifdef DEBUG_NET
								{
									NetPacketPtr packet_data= (NetPacketPtr) (packet->datagramData+sizeof(NetPacketHeader));
	
									switch(packet_data->ring_packet_type)
									{
										case typeUNSYNC_RING_PACKET:
											net_stats.late_unsync_rings++;
											break;
											
										case typeSYNC_RING_PACKET:
											net_stats.late_sync_rings++;
											break;
	
										case typeTIME_RING_PACKET:
											net_stats.late_time_rings++;
											break;
											
										case typeNORMAL_RING_PACKET:
											net_stats.late_rings++;
											break;
	
										case typeDEAD_PACKET:
											net_stats.late_dead_rings++;
											break;
										
										default:
											// LP change:
											assert(false);
											// halt();
											break;
									}
								}
	#endif
								/* late ring packet; acknowledge but ignore */
								NetSendAcknowledgement(ackFrame, header->sequence);
	//							dprintf("late ring (%d<=%d);g", header->sequence, status->lastValidRingSequence);
							}
							else
							{
								/* on-time or early ring packet */
	//							dprintf("Got ring.;g");
	//							dprintf("on-time ring %p (%d bytes);dm #%d #%d;g", packet, packet->datagramSize, packet->datagramData, packet->datagramSize);
	
								/* process remote actions, add our local actions, build ringFrame for sending */
								NetProcessIncomingBuffer(packet->datagramData+sizeof(NetPacketHeader),
									packet->datagramSize-sizeof(NetPacketHeader), header->sequence);
							}
						}
	/* Note that I ignore packets from an unknown source. There's a valid reason that we could get
	 * them. Imagine a ring with 3+ players. A->B->C->...->A. Player B sends to C, and crashes before
	 * getting the ack. but it's not a fatal crash. (macsbug warning...) C sends an ack though and
	 * forwards the packet. within 2 seconds, A drops B from the game. B recovers from the crash
	 * and says "Whoa! I didn't get an ack from C, let's resend". Then C gets a packet from the
	 * wrong person. (Note that time didn't pass for B while in macsbug, that's why he didn't just drop
	 * c from the game) */
	/* Come to think of it, B could have had a fatal crash. He goes into macsbug with an assert, then
	 * when he comes out, the network code hasn't been halted quite yet, so everything still happens
	 * for a moment. */
						else
						{
#ifdef DEBUG_NET
							net_stats.packets_from_the_unknown++;
#endif
	//						dprintf("packet from unknown source %8x!=%8x.;g;", *((long*)&packet->sourceAddress), *((long*)&status->downringAddress));
						}
					}
					break;
				
				default:
					// LP change:
					assert(false);
					// halt();
			}
		}
	}
	
	already_here= FALSE;
	
	return;
}

static void NetProcessLossyDistribution(
	void *buffer)
{
	short                     type;
	NetPacketHeaderPtr        header;
	NetDistributionPacketPtr  packet_data;
	
	packet_data = (NetDistributionPacketPtr)buffer;
	type = packet_data->distribution_type;

	if (distribution_info[type].type_in_use)
	{
		distribution_info[type].distribution_proc(&packet_data->data[0], packet_data->data_size, packet_data->first_player_index);
		if (packet_data->first_player_index != status->upringPlayerIndex)
		{
			distributionFrame->data_size = sizeof(NetPacketHeader) + sizeof(NetDistributionPacket) + packet_data->data_size;

			header = (NetPacketHeaderPtr) distributionFrame->data;
			header->tag = tagLOSSY_DISTRIBUTION;
			header->sequence = 0;

			BlockMove(buffer, distributionFrame->data+sizeof(NetPacketHeader), sizeof(NetDistributionPacket) + packet_data->data_size);
			NetDDPSendFrame(distributionFrame, &status->upringAddress, kPROTOCOL_TYPE, ddpSocket);
		}
	}

	return;
}

/*
------------------------
NetProcessIncomingBuffer
------------------------

this function queues flags from remote players, adds the local player’s latest command (thus
modifying the buffer in place), calls NetBuildRingPacket to set up ringFrame based on this new
data and then returns.
*/

/* •••• Marathon Specific Code (some of it, anyway) •••• */
static void NetProcessIncomingBuffer(
	void *buffer,
	long buffer_size,
	long sequence)
{
	NetPacketPtr packet_data;
	short packet_tag= NONE;
	long previous_lastValidRingSequence= status->lastValidRingSequence;

//	assert(buffer_size == NetPacketSize(buffer));

	packet_data= (NetPacketPtr) buffer;
	status->server_player_index= packet_data->server_player_index;

	/* remember this as the last valid ring sequence we received (set it now so we can send sequence+1) */
	status->lastValidRingSequence= sequence;
	status->ringPacketCount+= 1;

	switch(packet_data->ring_packet_type)
	{
		case typeSYNC_RING_PACKET:
			/* We sent this out to start the game, and now it has made it back to us. */
			/* This means that we are ready to start. */
			if (status->iAmTheServer)
			{
				packet_data->ring_packet_type= typeTIME_RING_PACKET;
				// I hearby declare that time starts now! Let There Be Light!
//				packet_data->server_net_time= 0;
//				status->localNetTime= 0;
//				packet_data->server_net_time= dynamic_world->tick_count;
//				status->localNetTime= dynamic_world->tick_count;

				if(serverTMTask)
				{
					/* This can only happen if we are resyncing for a changed level */
					myTMReset(serverTMTask);
				} else {
					serverTMTask= myXTMSetup(1000/TICKS_PER_SECOND, NetServerTask);
				}
			}
			/* else forward immediately. */
			break;
			
		case typeTIME_RING_PACKET:
			netState= netActive; // we are live!
			if (status->iAmTheServer)
			{
				/* We have completed the sequence, and got our time packet back */
				packet_data->ring_packet_type= typeNORMAL_RING_PACKET;
			}
			else // the server tells us that now is the beginning of time.
			{
//				status->localNetTime= 0;
//				status->localNetTime= packet_data->server_net_time;

				if(queueingTMTask)
				{
					/* This can only happen if we are resyncing for a changed level */
					myTMReset(queueingTMTask);
				} else {
					queueingTMTask= myXTMSetup(1000/TICKS_PER_SECOND, NetQueueingTask);
				}
			}
			break;
			
		case typeNORMAL_RING_PACKET:
/* This needs to be done differently, because the required action flags at zero need to be */
/*  read so that I know that the netState==netComingDown. */
/* For now, I remove it, so that we aren't able to recover nicely, only through the resend */
/*  task */
#ifdef OBSOLETE
			if(packet_data->required_action_flags==0 && netState!=netComingDown)
			{
#ifdef DEBUG_NET
				net_stats.server_bailing_early++;
				if(status->upringPlayerIndex!=status->server_player_index)
				{
					dprintf("Upring PLayer: %d Server player: %d", status->upringPlayerIndex,
				}
#endif
				warn(status->upringPlayerIndex==status->server_player_index);
				if(status->upringPlayerIndex==status->server_player_index)
				{
					become_the_gatherer(packet_data);
					packet_tag= tagCHANGE_RING_PACKET;
				} 
			}
#endif
			break;

		case typeUNSYNC_RING_PACKET:
			/* We sent this out to end the game, and now it has made it back to us. */
			/* This means that we are ready to exit. */
			if(netState==netComingDown)
			{
#ifdef DEBUG_NET
//				dprintf("Got an unsync packet.. (%d);g", net_stats.action_flags_processed);
				net_stats.unsync_while_coming_down++;
#endif
				status->acceptRingPackets= FALSE;
				if(status->iAmTheServer)
				{
#ifdef DEBUG_NET
//					dprintf("Unsync returned to sender. Going down;g");
					net_stats.unsync_returned_to_sender++;
#endif
					packet_data->ring_packet_type= typeDEAD_PACKET;
				}
			} 
#ifdef DEBUG_NET
			else 
			{
//				dprintf("Got a spurious unsync packet...;g");
				net_stats.spurious_unsyncs++;
			}
#endif
			break;
			
		default:
			// LP change:
			assert(false);
			// halt();
			break;
	}

	switch(packet_data->ring_packet_type)
	{
		case typeSYNC_RING_PACKET:
		case typeTIME_RING_PACKET:
			NetBuildRingPacket(ringFrame, (unsigned char *)buffer, NetPacketSize((NetPacketPtr) buffer), status->lastValidRingSequence+1);
			/* We acknowledge just before sending the ring frame.... */
			NetSendAcknowledgement(ackFrame, status->lastValidRingSequence);
			NetSendRingPacket(ringFrame);
			break;

		case typeUNSYNC_RING_PACKET:
			/* Don't ack it unless we did something with it.  They will spam us with them and then */
			/*  time out. (important if one machine is slower than the others. */
			if(netState==netComingDown)
			{
				NetBuildRingPacket(ringFrame, (unsigned char *)buffer, NetPacketSize((NetPacketPtr) buffer), status->lastValidRingSequence+1);

				NetSendAcknowledgement(ackFrame, status->lastValidRingSequence);
				NetSendRingPacket(ringFrame);
			} else {
				/* Got it but ignored it.  lastValidRingSequence should be reset to what it was before. */
				status->lastValidRingSequence= previous_lastValidRingSequence;
			}
			break;

		case typeNORMAL_RING_PACKET:
			process_packet_buffer_flags(buffer, buffer_size, packet_tag);
			break;

		case typeDEAD_PACKET:
			/* The buck stops here (after acknowledging it). */
			NetSendAcknowledgement(ackFrame, status->lastValidRingSequence);
			break;
			
		default:
			// LP change:
			assert(false);
			// halt();
			break;
	}
	
	return;
}

static void NetAddFlagsToPacket(
	NetPacketPtr packet)
{
	long *action_flags;
	short player_index, extra_flags, flags_to_remove, action_flag_index;
	static boolean already_here = FALSE;
	
	assert(already_here == FALSE);
	already_here= TRUE;

	vwarn(packet->required_action_flags >= 0 && packet->required_action_flags <= MAXIMUM_UPDATES_PER_PACKET,
		csprintf(temporary, "the server asked for %d flags.  bastard.  fucking ram doubler.", packet->required_action_flags));
	
	// figure out where our action flags are.
	action_flags= packet->action_flags;
	for (player_index= 0; player_index<localPlayerIndex; player_index++)
	{
		vwarn(packet->action_flag_count[player_index] >= -1 && packet->action_flag_count[player_index] <= MAXIMUM_UPDATES_PER_PACKET,
			csprintf(temporary, "action_flag_count[%d] = %d", player_index, packet->action_flag_count[player_index]));

		if (packet->action_flag_count[player_index] != NET_DEAD_ACTION_FLAG_COUNT) // player is net dead
		{
			action_flags += packet->action_flag_count[player_index];
		}
	}

	/* readjust the packet if the required action flag doesn't equal the action flag count */
	/*  for me and I am not the last player (if I am the last, I can just overflow.. */
	if (packet->required_action_flags != packet->action_flag_count[localPlayerIndex]
		&& localPlayerIndex != topology->player_count - 1)
	{
		short count= 0;

#ifdef DEBUG_NET
		net_stats.numCountChanges++;
#endif
		for (player_index= localPlayerIndex+1; player_index<topology->player_count; player_index++)
		{
			if (packet->action_flag_count[player_index] != NET_DEAD_ACTION_FLAG_COUNT) // player is net dead.
			{
				count+= packet->action_flag_count[player_index];
			}
		}
		
		vassert(count>=0 && count<=(MAXIMUM_UPDATES_PER_PACKET * MAXIMUM_NUMBER_OF_NETWORK_PLAYERS),
			csprintf(temporary, "bad count. count = %d. packet:; dm #%d", count, ((byte*)packet)-sizeof(NetPacketHeader)));

		BlockMove(action_flags + packet->action_flag_count[localPlayerIndex],
			action_flags + packet->required_action_flags,
			count * sizeof(long));
	}

#ifdef DEBUG_NET
	if(packet->required_action_flags==0) net_stats.packets_with_zero_flags++;
#endif

	// plug in our action flags.
	for (action_flag_index= 0; action_flag_index<packet->required_action_flags; action_flag_index++)
	{
		if (local_queue.read_index != local_queue.write_index)
		{
			action_flags[action_flag_index] = local_queue.buffer[local_queue.read_index];
			local_queue.read_index++;
			if (local_queue.read_index >= NET_QUEUE_SIZE) local_queue.read_index = 0;
		}
		else // we unfortunately need to smear.
		{
			action_flags[action_flag_index]= parse_keymap();
#ifdef DEBUG_NET
			net_stats.numSmears++;
#endif
		}
	}

	// if we're accumulating too many flags, just ditch some to avoid latency
	// (which we assume is worse than losing a couple of flags)
	extra_flags= NetSizeofLocalQueue();
	flags_to_remove= MIN(extra_flags, status->last_extra_flags);
	status->last_extra_flags = extra_flags - flags_to_remove;
	while (flags_to_remove--)
	{
		local_queue.read_index++;
		if (local_queue.read_index >= NET_QUEUE_SIZE) local_queue.read_index= 0;
	}

	/* Sync the net time... */
	if (!status->iAmTheServer)
	{
		status->localNetTime= packet->server_net_time;
	}

	// tell everyone that we’re meeting code.
	packet->action_flag_count[localPlayerIndex]= packet->required_action_flags;
	
//	dprintf("NETPACKET:;dm %x %x;g;", packet, sizeof(NetPacket)+sizeof(long)*2*8);
	
	/* Allow for reentrance into this function */
	already_here= FALSE;
}

/*
local player initializers
*/

static void NetInitializeTopology(
	void *game_data,
	short game_data_size,
	void *player_data,
	short player_data_size)
{
	NetPlayerPtr local_player;
	
	assert(player_data_size>=0&&player_data_size<MAXIMUM_PLAYER_DATA_SIZE);
	assert(game_data_size>=0&&game_data_size<MAXIMUM_GAME_DATA_SIZE);

	/* initialize the local player (assume we’re index zero, identifier zero) */
	localPlayerIndex= localPlayerIdentifier= 0;
	local_player= topology->players + localPlayerIndex;
	local_player->identifier= localPlayerIdentifier;
	local_player->net_dead= FALSE;

	if(NetGetTransportType()!=kModemTransportType)
	{
		short adsp_socket_number= NetGetStreamSocketNumber();

		NetLocalAddrBlock(&local_player->dspAddress, adsp_socket_number);
		NetLocalAddrBlock(&local_player->ddpAddress, ddpSocket);
	}
	BlockMove(player_data, local_player->player_data, player_data_size);
	
	/* initialize the network topology (assume we’re the only player) */
	topology->player_count= 1;
	topology->nextIdentifier= 1;
	BlockMove(game_data, topology->game_data, game_data_size);

	return;
}

static void NetLocalAddrBlock(
	AddrBlock *address,
	short socketNumber)
{
	short node, network;
	
	GetNodeAddress(&node, &network);
	
	address->aSocket= socketNumber;
	address->aNode= node;
	address->aNet= network;
	
	return;
}

static long NetPacketSize(
	NetPacketPtr  packet)
{
	register long   size = 0;
	register short  i;
	
	for (i = 0; i < topology->player_count; ++i)
	{
		if (packet->action_flag_count[i] != NET_DEAD_ACTION_FLAG_COUNT) // player has become net dead.
		{
			assert(packet->action_flag_count[i]>=0&&packet->action_flag_count[i]<=MAXIMUM_UPDATES_PER_PACKET);
			size += packet->action_flag_count[i] * sizeof(long);
		}
	}

	return size + sizeof(NetPacket); // add the other stuff in the packet.
}

/*
----------------------
NetSendAcknowledgement
----------------------

	--> DDPFramePtr (usually ackFrame)
	--> sequence to acknowledge

always sends the acknowledgement to downringAddress

------------------
NetBuildRingPacket
------------------

-----------------
NetSendRingPacket
-----------------
*/

static void NetSendAcknowledgement(
	DDPFramePtr frame,
	long sequence)
{
	NetPacketHeaderPtr header= (NetPacketHeaderPtr) frame->data;

//	dprintf("sending ack.;g");

	/* build the acknowledgement */
	frame->data_size= sizeof(NetPacketHeader);
	header->tag= tagACKNOWLEDGEMENT;
	header->sequence= sequence;
	
	/* send the acknowledgement */
	NetDDPSendFrame(frame, &status->downringAddress, kPROTOCOL_TYPE, ddpSocket);
	
	return;
}

/* Only the server can call this... */
static void NetBuildFirstRingPacket(
	DDPFramePtr frame,
	long sequence)
{
	short player_index;
	NetPacketPtr  data;
	
	data = (NetPacketPtr) NewPtr(sizeof(NetPacket));
	assert(data);
	
	data->server_player_index= localPlayerIndex;
	data->ring_packet_type= typeSYNC_RING_PACKET;
	data->required_action_flags= UPDATES_PER_PACKET;

	/* This is a very important step- the first time the server gets the packet back */
	/*  it strips flags.  It should not find any... */
	for (player_index= 0; player_index<topology->player_count; player_index++)
	{
		data->action_flag_count[player_index]= 0;
	}

	NetBuildRingPacket(frame, (byte *)data, NetPacketSize(data), sequence);
	
	DisposePtr((Ptr) data);
	
	return;
}

static void NetBuildRingPacket(
	DDPFramePtr frame,
	byte *data,
	short data_size,
	long sequence)
{
	NetPacketHeaderPtr header= (NetPacketHeaderPtr) frame->data;

	/* build the ring packet */
	frame->data_size= sizeof(NetPacketHeader) + data_size;
	header->tag= tagRING_PACKET;
	header->sequence= sequence;

	assert(data_size>=0&&data_size<ddpMaxData);
	BlockMove(data, frame->data+sizeof(NetPacketHeader), data_size);

	return;
}

static void NetRebuildRingPacket(
	DDPFramePtr frame,
	short tag,
	long sequence)
{
	NetPacketHeaderPtr header= (NetPacketHeaderPtr) frame->data;
	
	header->tag= tag;
	header->sequence= sequence;
}

static void NetSendRingPacket(
	DDPFramePtr frame)
{
//	dprintf("sent frame;g");
	
	status->retries= 0; // needs to be here, in case retry task was canceled (’cuz it likes to set retries)
	status->receivedAcknowledgement= FALSE; /* will not be set until we receive an acknowledgement for this packet */

	if (!resendTMTask) 
	{
		resendTMTask= myTMSetup(kACK_TIMEOUT, NetCheckResendRingPacket);
	} else {
		myTMReset(resendTMTask);
	}

	status->canForwardRing= FALSE; /* will not be set unless this task fires without a packet to forward */
	status->clearToForwardRing= FALSE; /* will not be set until we receive the next valid ring packet but will be irrelevant if serverCanForwardRing is TRUE */
	NetDDPSendFrame(frame, &status->upringAddress, kPROTOCOL_TYPE, ddpSocket);	
	
	return;
}

/*
------------------------
NetCheckResendRingPacket
------------------------

	(no parameters)

this function is called kACK_TIMEOUT after a ring packet has been sent.  if the ring
packet has not been acknowledged during this time, it will be resent from within this timer
task and the task will be requeued to check again in kACK_TIMEOUT.

*/
/* Possibly this should check for status->receivedAcknowledgement before !reinstalling.. */
static boolean NetCheckResendRingPacket(
	void)
{
	boolean reinstall= (netState != netDown);

	if(reinstall)
	{
		if (!status->receivedAcknowledgement)
		{
			if(++status->retries>=kRETRIES)
			{
				switch(netState)
				{
					case netStartingUp:
						/* There might be several retries as we start up */
						break;
						
					case netComingDown:
#ifdef DEBUG_NET
dprintf("Never got confirmation on NetUnsync packet.  They don't love us.");
#endif
						reinstall= FALSE;
						status->acceptRingPackets= FALSE;
						break;
						
					default:
						/* They have been gone too long.. */
						drop_upring_player();
						break;
				}
			}

#ifdef DEBUG_NET
			{
				NetPacketPtr packet_data= (NetPacketPtr) (ringFrame->data+sizeof(NetPacketHeader));
				switch(packet_data->ring_packet_type)
				{
					case typeSYNC_RING_PACKET:
						net_stats.sync_retry_count++;
						break;
						
					case typeTIME_RING_PACKET:
						net_stats.time_retry_count++;
						break;
			
					case typeUNSYNC_RING_PACKET:
						net_stats.unsync_retry_count++;
						break;
			
					case typeNORMAL_RING_PACKET:
						net_stats.retry_count++;
						break;
			
					case typeDEAD_PACKET:
						net_stats.dead_retry_count++;
						break;
						
					default:
						// LP change:
						assert(false);
						// halt();
						break;
				}
			}
#endif
			/* Resend it.. */
			NetDDPSendFrame(ringFrame, &status->upringAddress, kPROTOCOL_TYPE, ddpSocket);
		}
		else
		{
			status->retries = 0;
		}
	}

	return reinstall;
}

static boolean NetServerTask(
	void)
{
	short local_queue_size = NetSizeofLocalQueue();
	boolean reinstall= (netState != netDown);

	if(reinstall)
	{
		/* Call the local net queueing proc.. */
		if (local_queue_size < MAXIMUM_UPDATES_PER_PACKET)
		{
			local_queue_size++; // Random voodoo...
			local_queue.buffer[local_queue.write_index++] = parse_keymap();
			if (local_queue.write_index >= NET_QUEUE_SIZE)
				local_queue.write_index = 0;
			status->localNetTime++;
		}
		
		if (local_queue_size >= status->action_flags_per_packet)
		{
			// This weird voodoo with canForwardRing prevents a problem if a packet arrives at the wrong time.
			status->canForwardRing = TRUE; /* tell the socket listener it can forward the ring if it receives it */
			if (status->clearToForwardRing) /* has the socket listener already received the ring?  and not forwarded it? */
			{
				NetPacketPtr packet_data= (NetPacketPtr) status->buffer;
				
//				status->canForwardRing = FALSE;
				packet_data->server_net_time= status->localNetTime;
				if(netState==netComingDown)
				{
					if(packet_data->required_action_flags==0)
					{
#ifdef DEBUG_NET
//						dprintf("I Server got a normal packet, at zero.  unsyncing... (%d);g", net_stats.action_flags_processed);
						net_stats.server_unsyncing++;
#endif
						packet_data->ring_packet_type= typeUNSYNC_RING_PACKET;
					} 
					else 
					{
#ifdef DEBUG_NET
//						dprintf("I Server got a normal packet & net was coming down required flags at 0. (%d);g", net_stats.action_flags_processed);
						net_stats.server_set_required_flags_to_zero++;
#endif
						/* Change the type to an unsync ring packet... */
						packet_data->required_action_flags= 0;
					}
				} 
				else 
				{
					packet_data->required_action_flags= NetSizeofLocalQueue();
				}
				
				NetAddFlagsToPacket(packet_data);
				NetBuildRingPacket(ringFrame, (byte *) packet_data, NetPacketSize(packet_data), status->lastValidRingSequence+1);
				if(status->new_packet_tag != NONE) 
				{
#ifdef DEBUG_NET
//					dprintf("rebuilding the server tag (%d);g", status->new_packet_tag);
					net_stats.rebuilt_server_tag++;
#endif
					NetRebuildRingPacket(ringFrame, status->new_packet_tag, status->lastValidRingSequence+1);
				}

				/* Send the Ack just before we pass the token along.. */
				NetSendAcknowledgement(ackFrame, status->lastValidRingSequence);
				NetSendRingPacket(ringFrame);
			}
		}
	}
	
	return reinstall;
}

static boolean NetQueueingTask(
	void)
{
	boolean reinstall= (netState != netDown);
	
	if(reinstall)
	{
		if (NetSizeofLocalQueue() < MAXIMUM_UPDATES_PER_PACKET)
		{
			local_queue.buffer[local_queue.write_index++] = parse_keymap();
			if (local_queue.write_index >= NET_QUEUE_SIZE)
				local_queue.write_index = 0;
			status->localNetTime++;
		}
	}
	
	return reinstall;
}

static void NetUpdateTopology(
	void)
{
	short previousPlayerIndex, nextPlayerIndex;
	
	/* recalculate localPlayerIndex */					
	for (localPlayerIndex=0;localPlayerIndex<topology->player_count;++localPlayerIndex)
	{
		if (topology->players[localPlayerIndex].identifier==localPlayerIdentifier) break;
	}
#ifdef DEBUG
	if (localPlayerIndex==topology->player_count) dprintf("couldn’t find my identifier: %p", topology);
#endif
	
	/* recalculate downringAddress */				
	previousPlayerIndex= localPlayerIndex ? localPlayerIndex-1 : topology->player_count-1;
	status->downringAddress= topology->players[previousPlayerIndex].ddpAddress;
	
	/* recalculate upringAddress */
	nextPlayerIndex= localPlayerIndex==topology->player_count-1 ? 0 : localPlayerIndex+1;
	status->upringAddress= topology->players[nextPlayerIndex].ddpAddress;
	status->upringPlayerIndex = nextPlayerIndex;
	
	return;
}

// This function does two things. It changes the upring address to be the upring address
// of the next dude in the ring. It also returns the playerIndex of what used to be
// the next player, so that we can fiddle with things.
static short NetAdjustUpringAddressUpwards(
	void)
{
	short nextPlayerIndex, newNextPlayerIndex;
	AddrBlock *address;
	
	// figure out where the current upring address is.
	for (nextPlayerIndex= 0; nextPlayerIndex<topology->player_count; nextPlayerIndex++)
	{
		address = &(topology->players[nextPlayerIndex].ddpAddress);
		if (address->aNet == status->upringAddress.aNet 
			&& address->aNode == status->upringAddress.aNode
			&& address->aSocket == status->upringAddress.aSocket)
		{
			break;
		}
	}
	assert(nextPlayerIndex != topology->player_count);

	newNextPlayerIndex= nextPlayerIndex==topology->player_count-1 ? 0 : nextPlayerIndex+1;
	status->upringAddress= topology->players[newNextPlayerIndex].ddpAddress;
	status->upringPlayerIndex= newNextPlayerIndex;
	
	return nextPlayerIndex;
	
}

static boolean NetSetSelfSend(
	boolean on)
{
	OSErr          err;
	MPPParamBlock  pb;
	
	pb.SETSELF.newSelfFlag = on;
	err = PSetSelfSend(&pb, FALSE);
	assert(err == noErr);
	return pb.SETSELF.oldSelfFlag;
}

static void drop_upring_player(
	void)
{
	short flag_count, index, oldNextPlayerIndex;
	long *action_flags;
	NetPacketPtr packet_data= (NetPacketPtr) (ringFrame->data + sizeof (NetPacketHeader));

	/* Reset the retries for the new packet. */
	status->retries= 0;

	flag_count= 0;

#ifdef DEBUG_NET
//	dprintf("Dropping upring- Attempting to delete upring (node %d) from ring. muhaha.;g", status->upringAddress.aNode);
	net_stats.upring_drops++;
#endif

	// uh-oh. looks like the upring address has gone down.
	// modify the ring packet to zero out the players action flags
	// and find a new downring address.
	oldNextPlayerIndex= NetAdjustUpringAddressUpwards();

	/* If the next player upring was the server, and the next player upring wasn't us.. */
	if (oldNextPlayerIndex==status->server_player_index && !status->iAmTheServer)
	{				
		// let us crown ourselves!
		status->server_player_index= localPlayerIndex;
		status->iAmTheServer= TRUE;
#ifdef DEBUG_NET
//		dprintf("Trying to become the server (drop_upring);g");				
		net_stats.assuming_control_on_retry++;
#endif

		// now down to work. gotta switch tasks. Take a deep breath...
		queueingTMTask = myTMRemove(queueingTMTask);
		assert(!serverTMTask);
		serverTMTask = myXTMSetup(1000/TICKS_PER_SECOND, NetServerTask);
		packet_data->server_net_time= status->localNetTime;
	}
	
	//  adjust the packet to indicate that our fellow player has become deceased.
	// (is this an obituary?)
	action_flags = packet_data->action_flags;
	for (index= 0; index<oldNextPlayerIndex; index++)
	{
		if (packet_data->action_flag_count[index] != NET_DEAD_ACTION_FLAG_COUNT)
		{
			action_flags += packet_data->action_flag_count[index];
		}
	}
	
	for (index= oldNextPlayerIndex+1; index<topology->player_count; index++)
	{
		if (packet_data->action_flag_count[index] != NET_DEAD_ACTION_FLAG_COUNT)
		{
			flag_count += packet_data->action_flag_count[index];
		}
	}
	
	/* Remove the servers flags.. */
	if (flag_count > 0) 
	{
		// changed "flag_count" to "sizeof(long)*flag_count"
		BlockMove(action_flags + packet_data->action_flag_count[oldNextPlayerIndex], 
			action_flags, sizeof(long)*flag_count);
	}
	/* Mark the server as net dead */
	packet_data->action_flag_count[oldNextPlayerIndex]= NET_DEAD_ACTION_FLAG_COUNT;

	/* If everyone else is netdead, set the single player flag. */
	for(index= 0; index<topology->player_count; ++index)
	{
		if(index!=localPlayerIndex && packet_data->action_flag_count[index]!=NET_DEAD_ACTION_FLAG_COUNT)
		{
			break;
		}
	}
	if(index==topology->player_count) status->single_player= TRUE;

	// we have to increment the ring sequence counter in case we’re sending to ourselves
	// to prevent "late ring packets"
	NetRebuildRingPacket(ringFrame, tagCHANGE_RING_PACKET, status->lastValidRingSequence+1);
}

/* ------ this needs to let the gatherer keep going if there was an error.. */
/* ••• Marathon Specific Code ••• */
/* Returns error code.. */
boolean NetChangeMap(
	struct entry_point *entry)
{
	byte   *wad= NULL;
	long   length;
	OSErr  error= noErr;
	boolean success= TRUE;

#ifdef TEST_MODEM
	success= ModemChangeMap(entry);
#else

	/* If the guy that was the server died, and we are trying to change levels, we lose */
	if(localPlayerIndex==status->server_player_index && localPlayerIndex != 0)
	{
#ifdef DEBUG_NET
		dprintf("Server died, and trying to get another level. You lose;g");
#endif
		success= FALSE;
		set_game_error(gameError, errServerDied);
	}
	else
	{
		// being the server, we must send out the map to everyone.	
		if(localPlayerIndex==status->server_player_index) 
		{
			wad= (unsigned char *)get_map_for_net_transfer(entry);
			if(wad)
			{
				length= get_net_map_data_length(wad);
				error= NetDistributeGameDataToAllPlayers(wad, length);
				if(error) success= FALSE;
				set_game_error(systemError, error);
			} else {
//				if (!wad) alert_user(fatalError, strERRORS, badReadMap, -1);
				assert(error_pending());
			}
		} 
		else // wait for de damn map.
		{
			wad= NetReceiveGameData();
			if(!wad) success= FALSE;
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
#endif
	
	return success;
}

static OSErr NetDistributeGameDataToAllPlayers(
	byte *wad_buffer, 
	long wad_length)
{
	short playerIndex, message_id;
	OSErr error= noErr;
	long total_length, length_written;
	long initial_ticks= TickCount();
	short physics_message_id;
	byte *physics_buffer;
	long physics_length;
	
	message_id= (topology->player_count==2) ? (_distribute_map_single) : (_distribute_map_multiple);
	physics_message_id= (topology->player_count==2) ? (_distribute_physics_single) : (_distribute_physics_multiple);
	open_progress_dialog(physics_message_id);

	/* For updating our progress bar.. */
	total_length= (topology->player_count-1)*wad_length;
	length_written= 0l;

	/* Get the physics crap. */
	physics_buffer= (unsigned char *)get_network_physics_buffer(&physics_length);

	// go ahead and transfer the map to each player
	for (playerIndex= 1; !error && playerIndex<topology->player_count; playerIndex++)
	{
		/* If the player is not net dead. */
		if(!topology->players[playerIndex].net_dead)
		{

			/* Send the physics.. */
			set_progress_dialog_message(physics_message_id);

			error= NetOpenStreamToPlayer(playerIndex);
			if (!error)
			{
#ifdef NO_PHYSICS
				error= send_stream_data(physics_buffer, physics_length);
#endif

				if(!error)
				{
					set_progress_dialog_message(message_id);
					reset_progress_bar(); /* Reset the progress bar */
					error= send_stream_data(wad_buffer, wad_length);
				}
			}

			/* Note that we try to close regardless of error. */
			NetCloseStreamConnection(FALSE);
		}
		
		if (error)
		{
			alert_user(infoError, strNETWORK_ERRORS, netErrCouldntDistribute, error);
		} 
		else if  (TickCount()-initial_ticks>(topology->player_count*MAP_TRANSFER_TIME_OUT))
		{
			alert_user(infoError, strNETWORK_ERRORS, netErrWaitedTooLongForMap, error);
			error= 1;
		}
	}
	
	/* Fill the progress bar.. */
	if (!error) 
	{
		/* Process the physics file & frees it!.. */
		process_network_physics_model(physics_buffer);
		draw_progress_bar(total_length, total_length);
	}

	close_progress_dialog();
	
	return error;
}

static byte *NetReceiveGameData(
	void)
{
	byte *map_buffer= NULL;
	long map_length, ticks;
	OSErr error;
	boolean timed_out= FALSE;

	open_progress_dialog(_awaiting_map);

	// wait for our connection to start up. server will contact us.
	ticks= TickCount();
	while (!NetStreamCheckConnectionStatus() && !timed_out)
	{
		if((TickCount()-ticks)>MAP_TRANSFER_TIME_OUT)  timed_out= TRUE;
	}
	
	if (timed_out)
	{
			alert_user(infoError, strNETWORK_ERRORS, netErrWaitedTooLongForMap, 0);
	} 
	else
	{
		byte *physics_buffer;
		long physics_length;

		/* Receiving map.. */
		set_progress_dialog_message(_receiving_physics);

#ifdef NO_PHYSICS
		physics_buffer= (unsigned char *)receive_stream_data(&physics_length, &error);
#else
		physics_buffer= NULL;
#endif

		if(!error)
		{
			/* Process the physics file & frees it!.. */
			process_network_physics_model(physics_buffer);

			/* receiving the map.. */
			set_progress_dialog_message(_receiving_map);
			reset_progress_bar(); /* Reset the progress bar */
			map_buffer= (unsigned char *)receive_stream_data(&map_length, &error);
		}

		// close everything up.
		NetCloseStreamConnection(FALSE);

		/* And close our dialog.. */
		draw_progress_bar(10, 10);
		close_progress_dialog();
	
		/* Await for the next connection attempt. */
		error= NetStreamWaitForConnection();
		if (error != noErr || map_buffer == NULL)
		{
			if(error)
			{
				alert_user(infoError, strNETWORK_ERRORS, netErrMapDistribFailed, error);
			} else {
				alert_user(infoError, strERRORS, outOfMemory, MemError());
			}
	
			if(map_buffer)
			{
				free(map_buffer);
				map_buffer= NULL;
			}
		}
	}
	
	return map_buffer;
}

void NetSetInitialParameters(
	short updates_per_packet, 
	short update_latency)
{
	initial_updates_per_packet= updates_per_packet;
	initial_update_latency= update_latency;
}

long NetGetNetTime(
	void)
{
	return status->localNetTime - 2*status->action_flags_per_packet - status->update_latency;
}

// brazenly copied and modified from player.c (though i clearly format it much better)
static short NetSizeofLocalQueue(
	void)
{
	short size;
	
	if ((size= local_queue.write_index-local_queue.read_index) < 0) 
		size += NET_QUEUE_SIZE;

	return size;
}

static void *receive_stream_data(
	long *length,
	OSErr *receive_error)
{
	OSErr error;
	short packet_type;
	void *buffer= NULL;
	long initial_ticks= machine_tick_count();

	// first we'll get the map length
	error= NetReceiveStreamPacket(&packet_type, network_adsp_packet);

	if (!error && packet_type==_stream_size_packet)	
	{
		*length= *((long *) network_adsp_packet);
		if(*length)
		{
			buffer = (byte *) malloc(*length); // because my caller expects it to be portable.
			
			if (buffer)
			{
				long offset;
			
				// we transfer the map in chunks, since ADSP can only transfer 64K at a time.
				for (offset = 0; !error && offset < *length; offset += STREAM_TRANSFER_CHUNK_SIZE)
				{
					word expected_count;
										
					expected_count = MIN(STREAM_TRANSFER_CHUNK_SIZE, *length - offset);
					
					error= NetReceiveStreamPacket(&packet_type, network_adsp_packet);
					if(packet_type != _stream_data_packet)
					{
						error= errInvalidMapPacket;
					} else {
						/* Copy the data in.  This is done in two steps because the final */
						/*  packet would overrite the map buffer, unless it was perfectly */
						/*  a multiple of the STREAM_TRANSFER_CHUNK_SIZE */
						memcpy(((byte *)buffer)+offset, network_adsp_packet, expected_count);
					}
					draw_progress_bar(offset, *length);
				}
			}
		}
	}
	*receive_error= error;
		
	return buffer;
}

static OSErr send_stream_data(
	void *data,
	long length)
{
	OSErr error;

	// transfer the length of the level.
	error= NetSendStreamPacket(_stream_size_packet, &length);

	if(!error)
	{
		long offset, length_written= 0;
	
		// ready or not, here it comes, in smaller chunks
		for (offset = 0; !error && offset < length; offset += STREAM_TRANSFER_CHUNK_SIZE)
		{
			word adsp_count;
			
			adsp_count = MIN(STREAM_TRANSFER_CHUNK_SIZE, length - offset);

			/* Note that the last time through this we will have garbage at the */
			/*  end of the packet, but it doesn't matter since we know the length */
			/*  of the map. */
			memcpy(network_adsp_packet, ((byte *) data)+offset, adsp_count);
			error= NetSendStreamPacket(_stream_data_packet, network_adsp_packet);
			
			length_written+= adsp_count;
			draw_progress_bar(length_written, length);
		}
	}

	return error;
}

void NetPrintInfo(
	void)
{
#ifdef DEBUG_NET
	dprintf("numSmears= %d numCountChanges= %d ring packet_count= %d Single: %d;g", net_stats.numSmears, 
		net_stats.numCountChanges, 	status->ringPacketCount, status->single_player);
	dprintf("localPlayerIndex= %d, server_player_index= %d;g", localPlayerIndex, status->server_player_index);
	dprintf("tick_count= %d, localNetTime= %d;g", dynamic_world->tick_count, status->localNetTime);
	dprintf("Unknown packets: %d Upring Drops: %d Rebuilt server Tags: %d;g", net_stats.packets_from_the_unknown, net_stats.upring_drops, net_stats.rebuilt_server_tag);
	dprintf("Late Rings: Sync: %d Time: %d Normal: %d Unsync: %d Dead: %d;g", net_stats.late_sync_rings, net_stats.late_time_rings, net_stats.late_rings, net_stats.late_unsync_rings, net_stats.late_dead_rings);
	dprintf("---Retries: Sync: %d Time: %d Normal: %d Unsync: %d Dead: %d;g", net_stats.sync_retry_count, net_stats.time_retry_count, net_stats.retry_count, net_stats.unsync_retry_count, net_stats.dead_retry_count);
	dprintf("Ontime Ack: Sync: %d Time: %d Normal: %d Unsync: %d Dead: %d Late: %d;g", net_stats.sync_ontime_acks, net_stats.time_ontime_acks, net_stats.ontime_acks, net_stats.unsync_ontime_acks, net_stats.dead_ontime_acks, net_stats.late_acks);
	if(localPlayerIndex==status->server_player_index)
	{
		dprintf("Server: Req to zero: %d Unsyncing: %d Returned: %d;g", net_stats.server_set_required_flags_to_zero, net_stats.server_unsyncing, net_stats.unsync_returned_to_sender);
	}
	dprintf("Packets w/zero flags: %d Spurious Unsyncs: %d;g", net_stats.packets_with_zero_flags, net_stats.spurious_unsyncs);
	dprintf("Assumed control: Normal: %d Retry: %d (Server bailed early: %d);g", net_stats.assuming_control, net_stats.assuming_control_on_retry, net_stats.server_bailing_early);
	dprintf("Proper unsyncs: %d", net_stats.unsync_while_coming_down);
#endif
}

static void process_packet_buffer_flags(
	void *buffer,
	long buffer_size,
	short packet_tag)
{
	NetPacketPtr packet_data= (NetPacketPtr) buffer;

	/* The only time we don't process all flags is on network Unsyncing.. */
	process_flags(packet_data);

#ifdef DEBUG_NET
	if(packet_data->required_action_flags==0)
	{
		short index;

		for(index= status->server_player_index; index<localPlayerIndex; ++index)
		{
			warn(packet_data->action_flag_count[index]<=0);
		}
		
		for(index= 0; index<status->server_player_index; index++)
		{
			warn(packet_data->action_flag_count[index]<=0);
		}
	}
#endif

	// can we send on the packet?
	if (!status->iAmTheServer || status->canForwardRing)
	{
		status->canForwardRing = FALSE;
		if (status->iAmTheServer)
		{
			packet_data->server_player_index= localPlayerIndex;
			packet_data->server_net_time= status->localNetTime;
			if(netState==netComingDown)
			{
				/* Change the type to an unsync ring packet... */
				if(packet_data->required_action_flags==0)
				{
#ifdef DEBUG_NET
//					dprintf("Server got a final packet & net was coming down (changed) type: %d (%d);g",  packet_data->ring_packet_type, net_stats.action_flags_processed);
					net_stats.server_unsyncing++;
#endif
					packet_data->ring_packet_type= typeUNSYNC_RING_PACKET;
				} 
				else 
				{
#ifdef DEBUG_NET
//					dprintf("Server got a packet & net was coming down (changed) (%d) current: %d;g",  net_stats.action_flags_processed, packet_data->required_action_flags);
					net_stats.server_set_required_flags_to_zero++;
#endif
					packet_data->required_action_flags= 0;
				}
			} 
			else 
			{
				packet_data->required_action_flags= NetSizeofLocalQueue();
			}
		} 
		NetAddFlagsToPacket(packet_data);
		NetBuildRingPacket(ringFrame, (byte *) packet_data, NetPacketSize(packet_data), status->lastValidRingSequence+1);

		/* We just became the server.. */
		if(packet_tag != NONE) 
		{
			NetRebuildRingPacket(ringFrame, packet_tag, status->lastValidRingSequence+1);
		}
		
		/* Send the Ack just after we pass the token along.. */
		NetSendAcknowledgement(ackFrame, status->lastValidRingSequence);
		NetSendRingPacket(ringFrame);
	}
	// tell the server task to send on the packet
	else
	{
		BlockMove(buffer, status->buffer, buffer_size);
		status->clearToForwardRing = TRUE;
		status->new_packet_tag= packet_tag;
	}
}

/*
	0	tick0	tick0				tick0						tick1 Pulls (0, 1, 2)
	1	-----	tick0 (Pulls 0)		tick0						tick0
	2	-----	-----				tick0 (Pulls 0, 1, 2(none))	tick0
	
	must pull yourself and everything above you to complete the ring.
*/

/* On friday, the counts were different.  I don't think that should have been the case- the */
/*  flags should have been different though.. */
/* Got rid of the redundant action_flag_index, by using the more convenient and faster */
/*  pointer arithmetic. */
static void process_flags(
	NetPacketPtr packet_data)
{
	long *action_flags= packet_data->action_flags;
	short player_index;

	/* Process the action flags (including our old ones) */
	for (player_index= 0; player_index<topology->player_count; ++player_index)
	{
		short player_flag_count= packet_data->action_flag_count[player_index];
	
		vassert(player_flag_count >= -1 && player_flag_count <= MAXIMUM_UPDATES_PER_PACKET,
			csprintf(temporary, "UGH! count= %d;dm #%d", player_flag_count, 
			((byte*)packet_data)-sizeof(NetPacketHeader)));
	
		/* if the player is not net dead */
		if (player_flag_count != NET_DEAD_ACTION_FLAG_COUNT) 
		{
			process_action_flags(player_index, action_flags, player_flag_count);
#ifdef DEBUG_NET
#ifdef STREAM_NET
			{
				short ii;
				
				for(ii= 0; ii<player_flag_count; ++ii)
				{
					debug_stream_of_flags(action_flags[ii], player_index);
				}
			}
#endif
			net_stats.action_flags_processed+= player_flag_count;
#endif
			/* Regardless of whether you process this player, you need to increment past */
			/*  this player's flags */
			action_flags+= player_flag_count;
		}
		else // stuff zeroes, for the good of the recording, and everyone’s sanity.
		{
			/* Only process if this is in our range of flags. */
			short index;
	
//			dprintf("will stuff %d flags", packet_data->required_action_flags);				
			for (index= 0; index<packet_data->required_action_flags; index++)
			{
				long flag= NET_DEAD_ACTION_FLAG;

				topology->players[player_index].net_dead= TRUE;
				
				process_action_flags(player_index, &flag, 1);	
#ifdef DEBUG_NET
#ifdef STREAM_NET
				debug_stream_of_flags(flag, player_index);
#endif
				net_stats.action_flags_processed+= 1;
#endif
			}
		}
	}

	return;
}

static void become_the_gatherer(
	NetPacketPtr packet_data)
{
	short flag_count, index, oldNextPlayerIndex;
	long *action_flags;
	
	flag_count= 0;

#ifdef DEBUG_NET
//	dprintf("Attempting to delete upring (node %d) from ring. muhaha.;g", status->upringAddress.aNode);
	net_stats.upring_drops++;
#endif

	/* uh-oh. looks like the upring address has gone down. modify the ring packet to zero */
	/* out the players action flags and find a new downring address. */
	oldNextPlayerIndex= NetAdjustUpringAddressUpwards();

	/* If the next player upring was the server, and the next player upring wasn't us.. */
	assert(oldNextPlayerIndex==status->server_player_index);
	if (oldNextPlayerIndex==status->server_player_index && !status->iAmTheServer)
	{				
		// let us crown ourselves!
		status->server_player_index= localPlayerIndex;
		status->iAmTheServer= TRUE;
#ifdef DEBUG_NET
//		dprintf("Trying to become the server...;g");				
		net_stats.assuming_control++;
#endif

		// now down to work. gotta switch tasks. Take a deep breath...
		queueingTMTask = myTMRemove(queueingTMTask);
		assert(!serverTMTask);
		serverTMTask = myXTMSetup(1000/TICKS_PER_SECOND, NetServerTask);
		packet_data->server_net_time= status->localNetTime;
	}
	
	//  adjust the packet to indicate that our fellow player has become deceased.
	// (is this an obituary?)
	action_flags= packet_data->action_flags;
	for (index= 0; index<oldNextPlayerIndex; index++)
	{
		if (packet_data->action_flag_count[index] != NET_DEAD_ACTION_FLAG_COUNT)
		{
			action_flags += packet_data->action_flag_count[index];
		}
	}
	
	for (index= oldNextPlayerIndex+1; index<topology->player_count; index++)
	{
		if (packet_data->action_flag_count[index] != NET_DEAD_ACTION_FLAG_COUNT)
		{
			flag_count += packet_data->action_flag_count[index];
		}
	}
	
	/* Remove the servers flags.. */
	if (flag_count > 0) 
	{
		BlockMove(action_flags + packet_data->action_flag_count[oldNextPlayerIndex], 
			action_flags, sizeof(long)*flag_count);
	}
	/* Mark the server as net dead */
	packet_data->action_flag_count[oldNextPlayerIndex]= NET_DEAD_ACTION_FLAG_COUNT;
	packet_data->ring_packet_type= typeNORMAL_RING_PACKET;

	/* If everyone else is netdead, set the single player flag. */
	for(index= 0; index<topology->player_count; ++index)
	{
		if(index!=localPlayerIndex && packet_data->action_flag_count[index]!=NET_DEAD_ACTION_FLAG_COUNT)
		{
			break;
		}
	}
	if(index==topology->player_count) status->single_player= TRUE;
}

#ifdef DEBUG_NET
#ifdef STREAM_NET
#define MAXIMUM_STREAM_FLAGS (8192) // 48k

struct recorded_flag {
	long flag;
	short player_index;
};

static short stream_refnum= NONE;
static struct recorded_flag *action_flag_buffer= NULL;
static long action_flag_index= 0;

static void open_stream_file(
	void)
{
	FSSpec file;
	char name[]= "\pStream";
	OSErr error;

	get_my_fsspec(&file);
	memcpy(file.name, name, name[0]+1);
	
	FSpDelete(&file);
	error= FSpCreate(&file, 'ttxt', 'TEXT', smSystemScript);
	if(error) dprintf("Err:%d", error);
	error= FSpOpenDF(&file, fsWrPerm, &stream_refnum);
	if(error || stream_refnum==NONE) dprintf("Open Err:%d", error);
	
	action_flag_buffer= (struct recorded_flag *) malloc(MAXIMUM_STREAM_FLAGS*sizeof(struct recorded_flag));
	assert(action_flag_buffer);
	action_flag_index= 0;
}

static void write_flags(
	void)
{
	long index, size;
	short player_index;
	OSErr error;

	sprintf(temporary, "%d Total Flags\n", action_flag_index-1);
	size= strlen(temporary);
	error= FSWrite(stream_refnum, &size, temporary);
	if(error) dprintf("Error: %d", error);

	for(player_index= 0; player_index<topology->player_count; ++player_index)
	{
		long player_action_flag_count= 0;
	
		for(index= 0; index<action_flag_index-1; ++index)
		{
			if(action_flag_buffer[index].player_index==player_index)
			{
				if(!(player_action_flag_count%TICKS_PER_SECOND))
				{
					sprintf(temporary, "%d 0x%08x (%d secs)\n", action_flag_buffer[index].player_index, 
						action_flag_buffer[index].flag, player_action_flag_count/TICKS_PER_SECOND);
				} else {
					sprintf(temporary, "%d 0x%08x\n", action_flag_buffer[index].player_index, 
						action_flag_buffer[index].flag);
				}
				size= strlen(temporary);
				error= FSWrite(stream_refnum, &size, temporary);
				if(error) dprintf("Error: %d", error);
				player_action_flag_count++;
			}
		}
	}
}

static void debug_stream_of_flags(
	long action_flag,
	short player_index)
{
	if(stream_refnum != NONE)
	{
		assert(action_flag_buffer);
		if(action_flag_index<MAXIMUM_STREAM_FLAGS)
		{
			action_flag_buffer[action_flag_index].player_index= player_index;
			action_flag_buffer[action_flag_index++].flag= action_flag;
		}
	}
}

static void close_stream_file(
	void)
{
	if(stream_refnum != NONE)
	{
		assert(action_flag_buffer);

		write_flags();
		FSClose(stream_refnum);
		
		free(action_flag_buffer);
		action_flag_buffer= NULL;
	}
}
#endif
#endif

/* check for messages from gather nodes; returns new state */
short NetUpdateJoinState(
	void)
{
	OSErr error;
	short newState= netState;
	short packet_type;

#ifdef TEST_MODEM
	newState= ModemUpdateJoinState();
#else
	switch (netState)
	{
		case netJoining:
			if (NetStreamCheckConnectionStatus())
			{
				error= NetReceiveStreamPacket(&packet_type, network_adsp_packet);
				if(!error && packet_type==_join_player_packet)
				{
					/* NOTE THESE ARE SHARED! */
					struct gather_player_data *gathering_data= (struct gather_player_data *) network_adsp_packet;
					struct accept_gather_data new_player_data;

					/* Note that we could set accepted to FALSE if we wanted to for some */
					/*  reason- such as bad serial numbers.... */
					{
						/* Unregister ourselves */
						error= NetUnRegisterName();
						assert(!error);
					
						/* Note that we share the buffers.. */
						localPlayerIdentifier= gathering_data->new_local_player_identifier;
						topology->players[localPlayerIndex].identifier= localPlayerIdentifier;
						topology->players[localPlayerIndex].net_dead= FALSE;
						
						/* Confirm. */
						new_player_data.accepted= TRUE;
						memcpy(&new_player_data.player, topology->players+localPlayerIndex, sizeof(NetPlayer));
					}

					error= NetSendStreamPacket(_accept_join_packet, &new_player_data);
					if(!error)
					{
						/* Close and reset the connection */
						error= NetCloseStreamConnection(FALSE);
						if (!error)
						{
							error= NetStreamWaitForConnection();
							if (!error)
							{
								/* start waiting for another connection */
//dprintf("Accepted: %d", new_player_data.accepted);
								if(new_player_data.accepted) newState= netWaiting;
							}
						}
					}
				} 

				if (error != noErr)
				{
					newState= netJoinErrorOccurred;
					NetCloseStreamConnection(FALSE);
					alert_user(infoError, strNETWORK_ERRORS, netErrJoinFailed, error);
				}
			}
			break;
		
		case netWaiting:
			if (NetStreamCheckConnectionStatus())
			{
				/* and now, the packet you’ve all been waiting for ... (the server is trying to
					hook us up with the network topology) */
				error= NetReceiveStreamPacket(&packet_type, network_adsp_packet);
				if(!error && packet_type==_topology_packet)
				{
				
					/* Copy it in */
					memcpy(topology, network_adsp_packet, sizeof(NetTopology));

					if(NetGetTransportType()==kNetworkTransportType)
					{
						AddrBlock address;

						NetGetStreamAddress(&address);
						
						/* for ARA, make stuff in an address we know is correct (don’t believe the server) */
						topology->players[0].dspAddress= address;
						topology->players[0].ddpAddress.aNet= address.aNet;
						topology->players[0].ddpAddress.aNode= address.aNode;
//						dprintf("ddp %8x, dsp %8x;g;", *((long*)&topology->players[0].ddpAddress),
//							*((long*)&topology->players[0].dspAddress));
					}

					NetUpdateTopology();
				
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
				
					error= NetCloseStreamConnection(FALSE);
					if (!error)
					{
						error= NetStreamWaitForConnection();
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
	
	/* return netPlayerAdded to tell the caller to refresh his topology, but don’t change netState to that */
	if (newState!=netPlayerAdded && newState != NONE) netState= newState;
#endif

	return newState;
}

/* player_index in our lookup list */
boolean NetGatherPlayer(
	short player_index,
	CheckPlayerProcPtr check_player)
{
	OSErr error;
	AddrBlock address;
	short packet_type;
	short stream_transport_type= NetGetTransportType();
	boolean success= TRUE;

#ifdef TEST_MODEM
	success= ModemGatherPlayer(player_index, check_player);
#else
	assert(netState==netGathering);
	assert(topology->player_count<MAXIMUM_NUMBER_OF_NETWORK_PLAYERS);
	
	/* Get the address from the dialog */
	NetLookupInformation(player_index, &address, NULL);

	/* Note that the address will be garbage for modem, but that's okay.. */
	/* Force the address to be correct, so we can use our stream system.. */
	topology->players[topology->player_count].dspAddress= address;
	topology->players[topology->player_count].ddpAddress.aNet= address.aNet;
	topology->players[topology->player_count].ddpAddress.aNode= address.aNode;
	error= NetOpenStreamToPlayer(topology->player_count);
	
	if (!error)
	{
		struct gather_player_data gather_data;

		/* Setup the gather data. */
		gather_data.new_local_player_identifier= topology->nextIdentifier;
		error= NetSendStreamPacket(_join_player_packet, &gather_data);
		if(!error)
		{
			error= NetReceiveStreamPacket(&packet_type, network_adsp_packet);
			if(!error)
			{
				if(packet_type==_accept_join_packet)
				{
					struct accept_gather_data *new_player_data= (struct accept_gather_data *) network_adsp_packet;

					if(new_player_data->accepted)
					{
						/* make sure everybody gets a unique identifier */
						topology->nextIdentifier+= 1;

						/* Copy in the player data */
						memcpy(topology->players+topology->player_count, &new_player_data->player, sizeof(NetPlayer));
						
						/* force in some addresses we know are correct */
						topology->players[topology->player_count].dspAddress= address;
						topology->players[topology->player_count].ddpAddress.aNet= address.aNet;
						topology->players[topology->player_count].ddpAddress.aNode= address.aNode;
//						dprintf("ddp %8x, dsp %8x;g;", *((long*)&topology->players[topology->player_count].ddpAddress),
//							*((long*)&topology->players[topology->player_count].dspAddress));
							
						error= NetCloseStreamConnection(FALSE);
						if (!error)
						{
							/* closed connection successfully, remove this player from the list of players so
								we can’t even try to add him again */
							if(stream_transport_type!=kModemTransportType)
							{
								NetLookupRemove(player_index);
							}
						
							/* successfully added a player */
							topology->player_count+= 1;
						
							check_player(topology->player_count-1, topology->player_count);
	
							/* recalculate our local information */
							NetUpdateTopology();
						
							/* distribute this new topology with the new player tag */
							/* There is no reason to check the error here, because we can't do */
							/* anything about it.. */
							NetDistributeTopology(tagNEW_PLAYER);
						}
					}
					else 
					{
						NetCloseStreamConnection(FALSE);
					}
				} 
				else
				{
					NetCloseStreamConnection(FALSE);
				}
			}
			else
			{
				NetCloseStreamConnection(FALSE);
			}
		}  
		else 
		{
			NetCloseStreamConnection(FALSE);
		}
	}

	if(error)
	{
		alert_user(infoError, strNETWORK_ERRORS, netErrCantAddPlayer, error);
		NetLookupRemove(player_index); /* get this guy out of here, he didn’t respond */
		success= FALSE;
	}
#endif
	
	return success;
}

/*
---------------------
NetDistributeTopology
---------------------

	<--- error

connect to everyone’s dspAddress and give them the latest copy of the network topology.  this
used to be NetStart() and it used to connect all upring and downring ADSP connections.
*/
static OSErr NetDistributeTopology(
	short tag)
{
	OSErr error;
	short playerIndex;
	
	assert(netState==netGathering);
	
	topology->tag= tag;
	for (playerIndex=1; playerIndex<topology->player_count; ++playerIndex)
	{
		error= NetOpenStreamToPlayer(playerIndex);
		if (!error)
		{
			error= NetSendStreamPacket(_topology_packet, topology);
			if (!error)
			{
				error= NetCloseStreamConnection(FALSE);
				if (!error)
				{
					/* successfully distributed topology to this player */
				}
			}
			else
			{
				NetCloseStreamConnection(TRUE);
			}
		}
	}
	
	return error;
}

/* -------------------- application specific code */
word MaxStreamPacketLength(
	void)
{
	short packet_type;
	word max_length= 0;

	for(packet_type= 0; packet_type<NUMBER_OF_BUFFERED_STREAM_PACKET_TYPES; ++packet_type)
	{	
		word length= NetStreamPacketLength(packet_type);
		if(length>max_length) max_length= length;
	}
	
	return max_length;
}

word NetStreamPacketLength(
	short packet_type)
{
	word length;

	switch(packet_type)
	{
		case _join_player_packet:
			length= sizeof(struct gather_player_data);
			break;
			
		case _accept_join_packet:
			length= sizeof(struct accept_gather_data);
			break;
			
		case _topology_packet:
			length= sizeof(NetTopology);
			break;

		case _stream_size_packet:
			length= sizeof(long);
			break;
			
		case _stream_data_packet:
			length= STREAM_TRANSFER_CHUNK_SIZE;
			break;
			
		default:
			length= 0;
			// LP change:
			assert(false);
			// halt();
			break;
	}

	return length;
}

AddrBlock *NetGetPlayerADSPAddress(
	short player_index)
{
	return &topology->players[player_index].dspAddress;
}