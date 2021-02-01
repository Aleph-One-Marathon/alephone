/*
 *  network_ring.cpp

	Copyright (C) 1991-2003 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.

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

 *  File created by Woody Zenfell, III on Sat May 17 2003: split out from network.cpp.
 *
 *  Implementation of the old ring-topology network game protocol, as a module in the new scheme.
 */

#if !defined(DISABLE_NETWORKING)

#include "cseries.h"

#include "RingGameProtocol.h"

#include "ActionQueues.h"
#include "player.h" // GetRealActionQueues
#include "network.h"
#include "network_private.h"
#include "network_data_formats.h"
#include "mytm.h"
#include "map.h" // TICKS_PER_SECOND
#include "vbl.h" // parse_keymap
#include "interface.h" // process_action_flags
#include "InfoTree.h"
#include "Logging.h"

// Optional features, disabled by default on Mac to preserve existing behavior (I hope ;) )
#undef	NETWORK_ADAPTIVE_LATENCY_2	// use this one instead; it should be good.  no wait...
#define NETWORK_ADAPTIVE_LATENCY_3	// there, this one ought to get it right, finally.
#undef	NETWORK_USE_RECENT_FLAGS	// if the game stalls, use flags at the end of the stall, not the more stale ones at the beginning
#define	NETWORK_SMARTER_FLAG_DITCHING	// this mechanism won't be quite as hasty to toss flags as Bungie's

// ZZZ: This structure is never placed on the wire, so you can do whatever you want with it
// without having to worry about updating additional structures.
struct NetStatus
{
	/* we receive packets from downring and send them upring */
	NetAddrBlock upringAddress, downringAddress;
	int16 upringPlayerIndex;

	int32 lastValidRingSequence; /* the sequence number of the last valid ring packet we received */
	int32 ringPacketCount; /* the number of ring packets we have received */

	bool receivedAcknowledgement; /* true if we received a valid acknowledgement for the last ring packet we sent */
	bool canForwardRing;
	bool clearToForwardRing; /* true if we are the server and we got a valid ring packet but we didn’t send it */
	bool acceptPackets; /* true if we want to get packets */
	bool acceptRingPackets; /* true if we want to get ring packets */
	bool oldSelfSendStatus;

	int16 retries;

	int16 action_flags_per_packet;
	int16 last_extra_flags;
	int16 update_latency;

	bool iAmTheServer;
	bool single_player; /* Set true if I dropped everyone else. */
	short server_player_index;
	int16 new_packet_tag; /* Valid _only_ if you are the server, and is only set when you just became the server. */

	uint8 *buffer;

	int32 localNetTime;
	bool worldUpdate;
};
typedef struct NetStatus NetStatus, *NetStatusPtr;

// ZZZ: I believe this is never placed on the wire either.  If I have not come back and changed
// this note, that's probably right.  :)
struct NetQueue
{
	int16 read_index, write_index;
	int32 buffer[NET_QUEUE_SIZE];
};
typedef struct NetQueue NetQueue, *NetQueuePtr;

static volatile NetStatusPtr status;

struct RingPreferences
{
	bool	mAcceptPacketsFromAnyone;
	bool	mAdaptToLatency;
	int32	mLatencyHoldTicks;
};

static RingPreferences sRingPreferences;

// ZZZ: note: these will always be used in _NET (packed) format.  Fortunately, the existing code only
// works directly with these in a few very specific places, so it's reasonable to hold this invariant.
// Observe that these are only used in the preparation of _outgoing_ frames.  They are read only (umm,
// those are two words, not "read-only") in
// special circumstances - when debugging code wants to see what was ACKed, or when we've retried the
// upring player too many times and want to drop him (in which case we modify the ringFrame and pass it
// to the upring's upring).
static DDPFramePtr ackFrame, ringFrame; /* ddp frames for sending ring/acknowledgement packets */
static DDPFramePtr distributionFrame;

// ZZZ: On packet reception, we copy the _NET (packed) data into the local preferred format.  This is
// the storage for the unpacked copy.  Note that few folks refer to it directly - usually a pointer
// to the unpacked data (i.e. into this buffer) is passed along as an argument.
static	char	unpackedReceiveBuffer[ddpMaxData];

// ZZZ note: originally in IPring, I had every function that dealt with network data packing and unpacking
// its own stuff.  That's probably easier to follow and more correct, but there are a few problems:
// 1)  It requires more changes to the original code.
// 2)  It requires that the original code be VERY carefully combed-through to catch all references.
// 3)  It requires a lot of extra byte-swapping and copying (to and from non-aligned structures).

// Now, as noted above, I am trying to convert an entire incoming/outgoing packet only just after it arrives
// or just before it leaves.  This should help with all three issues just mentioned.

// ZZZ random observation #3,792:
// Why use datagrams if we're not going to change the data we retransmit?  We might as well just use
// TCP and save ourselves some hassle, don'cha think??  I don't really see any other reason we would
// want to take over the task of handling ACKs and retransmissions... there's not any clever
// "implicit ACK" mechanism or updating of data between retransmissions going on here, and I don't
// think retransmission timing here is customized in any way that would be preferable over TCP's
// behavior...?

// ZZZ: observe: only one of {server, queueing} should be active at a time.
// NetServerTask is used only on, you guessed it, the server.  NetQueueingTask is used on all others.
// Everyone uses NetCheckResendRingPacket to retransmit the ring packet if no ACK has come in.
// (Distribution datagrams currently are only 'lossy', so no ACKs or retransmissions are used there.)

static myTMTaskPtr resendTMTask = (myTMTaskPtr) NULL;
static myTMTaskPtr serverTMTask = (myTMTaskPtr) NULL;
static myTMTaskPtr queueingTMTask= (myTMTaskPtr) NULL;

// ZZZ: observe: in a ring topology especially, everyone must treat the ring token as a 'hot potato' - i.e.
// deal with it and pass it on ASAP - to avoid introducing ring latency (which of course impacts all hosts).
// This is why we use separately scheduled elements (like the original) and don't depend on occasional polling
// from the main thread (e.g. between frames).
// In a star (client-server) topology, this should be less of an issue, since overall latency would be related
// to the slowest station's latency, rather than the sum of everyone's latency.
// We want these scheduling elements to have higher priority than the main thread to make sure they are
// scheduled as close as possible to the time they ought to be scheduled.  SDL_Thread provides no mechanism
// to alter thread priority currently.  I guess we'll either have to extend it somehow, or else pray that
// the OS considers these occasionally-runnable threads I/O-bound and the constantly-working main thread
// CPU-bound, and boosts these threads' priority appropriately.
// (Update: I'm now boosting thread priorities programmatically - see SDL_threadx.)


// ZZZ annotation: this holds action_flags we've captured (e.g. in NetQueueingTask) that have not been placed
// into a ring packet yet.  We don't get to process our own action_flags until they've been queued here, sent
// around the ring in the ring packet, and come back to us in NetDDPPacketHandler.
static volatile NetQueue local_queue;

// ZZZ addition: here we fake having another queue.  The idea here is that if the ring comes around and we need
// to "smear", we effectively extend the NetQueueingTask "credit" of a sort.  NQT, when it does run, will see
// that it "owes" us some action_flags, and will work with this pseudo-queue rather than with the real local_queue.
// Of course, it won't bother getting or storing real action_flags, since the packet handler already did that.
// This way we avoid pumping too many action_flags into the pipeline.
// Note that this is based on the idea that NQT will run, on average, n times in n periods - it just may be a little more
// jittery than we'd like.  In the SDL version this is true.  In the 'classic' Mac version, well, I'm not sure
// what the Time Manager does if it misses a deadline.  If it runs its task an extra time, this is the way to go.
// If it shrugs and the call for that period is lost forever, it should NOT use this mechanism.
// In the spirit of leaving things alone if they work, I'll leave this just as an IPring thing for now.
#ifdef NETWORK_FAUX_QUEUE
static volatile uint32	faux_queue_due;		// packet handler increments each time it smears
static volatile uint32	faux_queue_paid;	// NQT increments this each time it throws away action_flags instead of queueing them
#endif

// ZZZ annotation: I suspect these were used for something like what I'm about to do again.
// I'm putting in my own system (NETWORK_ADAPTIVE_LATENCY) though to make sure I understand all the interactions.
// Besides, it looks like these values never "adapt".
static short initial_updates_per_packet = UPDATES_PER_PACKET;
static short initial_update_latency = UPDATE_LATENCY;

// ZZZ addition: adaptive latency mechanism.  Here's the problem.  We want to minimize latency, but also mask
// jitter (i.e. variation in latency).  If we see too much latency, the game will feel "sluggish" in the sense
// that your player will respond late to all your commands - might feel sort of like "driving a boat".
// OTOH if we keep latency at the absolute minimum, we expose ourselves to jitter - if there's a "burp" in
// the system somewhere (either in the network or in an endsystem's scheduling), the game freezes up for the
// duration of that burp.  If small burps happen frequently, we might be better off adding an extra frame of
// latency to smooth over all those little burps.

// Before I begin my changes, let me review how things are working (at least with the way the SDL version
// schedules things).

// If a non-server has a *scheduling* burp, that means we see a too-long period with no calls to NetQueueingTask,
// followed by a bunch of calls to NQT to make up for the losses.

// If a ring packet (maybe more than one, depending on the nature of the burp) arrived and was processed during
// that dry spell, we "smeared" - the packet handler grabbed however many input_flags it needed directly from
// the input system instead of taking from the queue that NQT fills (because the queue was empty/not full enough).
// The extra calls to NQT at the end of the dry spell will go into the regular local_queue, but will probably be
// discarded the next time we feed the ring since we'll have too many flags waiting.

// Endstations upring (i.e. which receive the ring packet later) from the burper will (assuming they are not
// simultaneously burping :) ) queue up a bunch of action_flags as normal in NQT while they wait.  (Note that this also applies to
// a situation where network congestion causes a burp "between" stations - e.g. we have to retransmit due to
// lack of ACK, etc.)  When the ring packet gets around to the next endstations, they will discard the oldest
// action_flags and pass along only the newest few (if my NETWORK_USE_RECENT_FLAGS change is active - it used to
// pass along the oldest ones and throw out the newer ones.)  So far, everything looks peachy - we've had to
// freeze our animation while the burp happened, but at least we're not introducing any new latency (i.e.
// we are making sure our local queue is as empty as possible by throwing away extra flags, etc.)

// The problem comes when we look at the server (and NetServerTask).  It makes no effort to empty its queue
// currently - instead, when the ring comes back to it after a burp, it has a lot of extra flags waiting to
// go.  Instead of ditching the old ones, it sends them _all_ along, and tells the non-servers (via the packet's
// required_action_flags) that they owe a lot of flags.  So, when the ring packet gets to them, and they don't
// have enough flags, they "smear" to make up the difference.  The end result is that a lot of extra flags
// end up in the "player" queue - the one where flags sit after being received from the network but before being acted
// on by the main game code.  The main game takes from that queue only at the TICK rate, so if there are extra
// flags sitting there, any new flags will have to wait to be acted on until they make their way through the
// queue - hello latency.

// Here's my crack at a solution.  The server is responsible for making the decisions here.

// Simply ditching old flags at the server the way we do at endstations seems like the simplest way, but I
// believe it has an important drawback: what if the latency in the ring is greater than the duration of a
// TICK?  What if the average latency in the ring is less than the duration of a TICK, but we tend to jitter
// a fair amount over the TICK duration (but, we hope, not much more)?  In the former case, the game would
// proceed at half the normal TICK rate (or worse) because endstations would be constantly starved for data,
// and thus would be stalling the game all the time.  In the latter case, we wouldn't be stalling *all* the
// time, but every time we jittered past the TICK duration - too frequently to make playing pleasant.

// What we need is to adapt to the network conditions - if we seem to be starving a lot, we want to bump up
// the number of action_flags in our queues so we can use the extras to smooth over the gaps.  OTOH though
// if conditions improve, we want to reduce the number of flags so we don't have gratuitous extra latency
// between player inputs and screen updates.  I think one could argue that the code I inherited did the
// first part just fine, but never readjusted the queue size back down to remove latency when possible.
// (Perhaps this is what Jason's comment in the header means?  "(can only be adjusted upward, not downward)")

// Maybe on the cooperatively-scheduled Mac, using very-low-latency interrupts to do time-sensitive processing,
// burps were uncommon.  But on most systems, our code will probably get scheduled erraticly, so we need to
// be more jitter-tolerant.  In particular, we don't want a big burp to build a whole bunch of latency in
// to the rest of the game, if we can recover from it (exposing ourselves to another big burp later, but we
// hope that those are uncommon enough that we'd rather have a responsive game between stalls than a sluggish
// game that is unaffected by stalls).

// Actually this is all building a compelling argument for changing from a ring to a star, since endstation
// latency in a star has much less impact on the overall system (which here I use to mean all machines involved)
// that does endstation latency in a ring, especially as the number of endstations grows.

// So here's how my scheme works: we maintain a sampling window of how many TICKs it's actually taken the ring
// to go around (remember, this is all done by the server).  As this window slides along in time, we track
// (1) the number of rings that needed more time than we're currently allowing, and
// (2) the number of rings that actually needed the amount of time we're allowing (i.e. that were not 'early'
// by a tick or more).
// If (1) exceeds some threshhold, we add in another required_action_flag, so the queue grows a little longer
// to tolerate constant latency or frequent jitter.
// OTOH if (2) is less than some threshhold, we reduce the number of required_action_flags, so the queue
// shrinks and we have less overall latency.
// Update: instead of using the count of rings that were early or late, we just use the average ring latency
// and compare it to our current setting.  That way we can reuse our window data without having to go
// touch all the samples to recalculate stuff.  (Not that it would really take all *that* long, but...)

// I use these same things, slightly differently in the code, to do NETWORK_ADAPTIVE_LATENCY_2 (see below).
#if defined(NETWORK_ADAPTIVE_LATENCY) || defined(NETWORK_ADAPTIVE_LATENCY_2)
enum {
	kAdaptiveLatencyWindowSize = 3 * TICKS_PER_SECOND,	// number of rings we look at for deciding whether to change latency
	kAdaptiveLatencyInitialValue = 1,			// our first guess at a good latency value
	kAdaptiveLatencyMaximumValue = MAXIMUM_UPDATES_PER_PACKET	// cap on the latency value
};

// We adjust the ring latency when (average measured ring latency over window) - (current latency setting)
// is greater than the increase threshhold or less than the decrease threshhold.  Note that the decrease
// threshhold ought to be < (increase threshhold - 1), or we are likely to bounce back and forth between
// settings.  I have set these pretty low, which favors adding latency over allowing choppiness.  I suppose
// these could be made more dynamically settable (via MML or dialog-box) to let users decide where they want
// the tradeoff, but I doubt most really care to tweak such settings themselves.
const float	kNeedToIncreaseLatencyThreshhold = .1F;
const float	kNeedToDecreaseLatencyThreshhold = -.95F;

static int	sAdaptiveLatencySamples[kAdaptiveLatencyWindowSize];	// sample buffer, used much like a circular queue.
static int	sAdaptiveLatencyWindowEdge;	// so we know where in the sample buffer our window starts/ends
				       // between calls to update_adaptive_latency, it indicates the head of the queue.
static int	sAdaptiveLatencySampleCount;	// so we don't try to make decisions without enough samples
static int	sAdaptiveLatencyRunningTotal;	// total of latencies in the sample window
static volatile int sCurrentAdaptiveLatency;	// how many action_flags we want going around in those packets

// Hmm, now not only am I concerned about the concept of "net time", but I'm worried that the faux-queue idea I implemented
// before is now going to interfere with the adaptive latency scheme, or vice-versa.  I mean, if adaptive latency decides that
// we need another action_flag per packet, we're probably going to catch some non-servers unawares, and they'll have to smear
// to meet our requirement (which is fine) - but then when they run their NetQueueingTask, they'll try to pay back for the smear,
// which means they'll have to smear again next time... it's not the end of the world, but it defeats the purpose of calling NQT
// periodically instead of just smearing every time we need data.
// OK OK I convinced myself.  Disabling FAUX_QUEUE.  That didn't help with erratic server scheduling anyway.
#endif//NETWORK_ADAPTIVE_LATENCY || NETWORK_ADAPTIVE_LATENCY_2

// Adaptive latency take 2... heh heh...
// Here's the problem with the first effort.  (Or second, if you count FAUX_QUEUE as an effort.)
// It assumes that the engine consumes action_flags at a constant rate, and in particular that if
// the engine starves, it does not take any special steps to catch up later.  Au contraire!, it turns
// out (quite sensibly) that the engine, after starving, consumes action_flags voraciously to try to
// catch back up to real game time.  So let's try this analysis:

// Endstations produce action_flags at a constant, but perhaps jittery, rate of one per GAME_TICK.
// Endstations consume action_flags at a constant, but perhaps jittery, rate of one per GAME_TICK.

// If the ring gets held up somewhere, endstations build up flags in their local_queues.
// The next time it comes around *after reaching the server*, the ring will consume all the extras.
// Observe: this means that, ideally, we should *not* discard action_flags from the local_queue if
// we get the ring packet and we have more than it wants - it should gobble up our extras next time.
// We *could* make sure our extras get eaten - we could increment a counter for every ring that leaves
// us with flags still in our local_queue.  (When a ring leaves us empty, we reset the counter.)
// We could consider incrementing the counter by the NUMBER of flags we have left sitting around, so if
// we have somehow gotten a large number jammed in our queue, it corrects more quickly.
// If the counter exceeds a threshhold, we throw away an action_flag from the head of the queue (stale)
// and reset the counter.
// If an endstation takes too long between flag productions, it "smears" when the ring reaches it.
// The endstation thus produces too many flags.  If we use the above correction mechanism, we will
// compensate eventually by throwing out extra flags.  But if for some reason drift has caused our
// average rate to fall slightly below what's needed, the extra flag or two from smearing will bring
// our production back up to the needed rate - and that in a timely manner.

// Endstations that take too long between consumption binges build up flags in their player_queues.
// At the next consumption binge, they will eat all the extras.  Note that consumption is tied
// directly to production on an endstation - if the production element runs dry for a while, the consumer
// will not be allowed to consume, even if there are queued flags (in player_queues) available.

// A one-time dry spell and then compensating deluge in the player_queues will be correctly handled by
// the voracious-consumer approach.  The game will freeze up during the dry spell, then suddenly lurch
// ahead several frames to get back to the "present".
// It seems like there could be a problem if the consumer drifts from what's coming to it, but I think
// that having the consumer rate tied to the producer rate - and having everyone agree on that rate by
// the server dictating the net_time - gets around any such problems.

// Actually, I think we should see the following behaviors with regard to net_time:

// If the server's net_time coming in is greater than ours, we should see ourselves smear... unless we recently
// saw a server net_time less than ours, in which case we have queued flags to make up the difference.
// If the server's net_time coming in is less than ours, we should see ourselves ditch a flag.
// (Note that these may not happen at the same packet arrival, but should be nonetheless linked.)

// I have now implemented the flag ditching mechanism outlined above:
#ifdef	NETWORK_SMARTER_FLAG_DITCHING
static int	sFlagDitchingCounter;		// reset if local_queue empties; increments by local_queue leftovers otherwise.

enum {
	kFlagDitchingThreshhold = TICKS_PER_SECOND	// if counter exceeds this, ditch a flag and reset counter.
};
#endif
// We're really splitting hairs at this point.  I think this is only better than the Bungie mechanism if
// we have several late packets in a row followed by a return to normal - and even then we're just honoring
// the user's input a little more accurately.  Oh well, this was probably a poorly spent 15 minutes, but it
// can only help, I think...
// I can convince myself of some pretty odd things if I think about them long enough.  ;)

// Does the net_time mechanism alone solve everything??

// dynamic_world is not allowed to get ahead of net_time.
// Suppose we see a ring packet every three GAME_TICKs.  The ring packet should carry three action_flags with it.
// Let's watch the action starting at 0 (this may not be EXACTLY what the ring protocol does, since it has special
// startup stuff, but if this bothers you, imagine the latency suddenly jumps up to this level at some time T.
// Our "0", then, is the offset from some point in time T.):

// NQT queues a local flag.  Our local net_time increases.  Consumer tries to update dynamic_world, but there's not
// enough data.
// Same thing.  Now local net_time is 2, and there are 2 flags in local_queue.  player_queue is still empty, so
// dynamic_world cannot update.
// Same thing.  net_time is 3, there are 3 flags in local_queue, and player_queue is empty.  dynamic_world is oddly still.
// The ring packet comes in!  It has a required_flags of 1, and every player's data in the packet has 1 flag in it.
// The server's net time was 1 when the packet was sent.  *We set our local net_time to 1.*  We move a local_flag into the
// packet and pass the ring on.  net_time is 1, there are 2 flags in local_queue, and player_queue has 1.
// NQT queues a local flag.  net_time is 2.  There are 3 flags in local_queue.
// dynamic_world updates to time 1 - it could try to go to 2, but there's only 1 flag.  player_queue is now empty.
// NQT queues a local flag.  net_time is 3.  There are 4 flags in local_queue.  player_queue is empty.  dynamic_world is stopped.
// NQT queues a local flag.  net_time is 4.  There are 5 flags in local_queue.  player_queue is empty.  dynamic_world is stopped.
// The ring packet comes in!  required_flags is 3; every player has 3 flags.  The server's net_time was 4.
// Our net_time agrees with the server's.  We pack up 3 local_flags for the packet and pass it on.
// net_time = 4, |local_queue| = 2, |player_queue| = 3, DWTC = 1.
// dynamic_world wants to update to time 4, and CAN - it voraciously consumes all 3 player_flags.
// net_time = 4, |local_queue| = 2, |player_queue| = 0, DWTC = 4.
// NQT: net_time = 5, |local_queue| = 3, |player_queue| = 0, DWTC = 4.
// NQT: net_time = 6, |local_queue| = 4, |player_queue| = 0, DWTC = 4.
// NQT: net_time = 7, |local_queue| = 5, |player_queue| = 0, DWTC = 4.
// RING net_time = 7, |local_queue| = 2, |player_queue| = 3, DWTC = 4.
// DRAW net_time = 7, |local_queue| = 2, |player_queue| = 0, DWTC = 7.

// Note: if latency suddenly drops back to 1 GAME_TICK per ring, the extra flags in local_queue will drain, and DWTC
// will be able to track net_time more closely.
// If instead latency stays high, we will eventually discard the extra elements in local_queue, reducing overall
// user-input-to-screen-update latency.

// Note: if bandwidth is no problem but latency is high ("long fat pipe"?), we could have multiple updates "on the wire"
// at a time, thus letting DWTC remain in sync with net_time -> smoother updates.  Our net_time would stay a few ticks
// behind the server's net_time.

// OK, this behavior is not so good because we're only drawing the screen at 10fps (once every 3 GAME_TICKs).
// Options are to speculatively update dynamic_world, thus letting DWTC track net_time (but without "real data" to go on), (HARD),
// or to delay the engine's view of net_time by some amount (EASY), introducing even more latency but smoothing out the frames.

// How do we decide to raise or lower the amount of delay?
// Should be based on, when a packet comes in, how many player_flags there are?  How DWTC compares to (net_time - delay)?

// We would want to see
// INIT net_time = 4, |local_queue| = 2, |player_queue| = 0, DWTC = 4.
// NQT: net_time = 5, |local_queue| = 3, |player_queue| = 0, DWTC = 4.
// NQT: net_time = 6, |local_queue| = 4, |player_queue| = 0, DWTC = 4.
// NQT: net_time = 7, |local_queue| = 5, |player_queue| = 0, DWTC = 4.
// RING net_time = 7, |local_queue| = 2, |player_queue| = 3, DWTC = 4.
// DRAW net_time = 7, |local_queue| = 2, |player_queue| = 2, DWTC = 5.
// NQT: net_time = 8, |local_queue| = 3, |player_queue| = 2, DWTC = 5.
// DRAW net_time = 8, |local_queue| = 3, |player_queue| = 1, DWTC = 6.
// NQT: net_time = 9, |local_queue| = 4, |player_queue| = 1, DWTC = 6.
// DRAW net_time = 9, |local_queue| = 4, |player_queue| = 0, DWTC = 7.
// NQT: net_time = 10,|local_queue| = 5, |player_queue| = 0, DWTC = 7.
// RING net_time = 10,|local_queue| = 2, |player_queue| = 3, DWTC = 7.
// DRAW net_time = 10,|local_queue| = 2, |player_queue| = 2, DWTC = 8.
// NQT: net_time = 11,|local_queue| = 3, |player_queue| = 2, DWTC = 8.
// DRAW net_time = 11,|local_queue| = 3, |player_queue| = 1, DWTC = 9.
// NQT: net_time = 12,|local_queue| = 4, |player_queue| = 1, DWTC = 9.
// DRAW net_time = 12,|local_queue| = 4, |player_queue| = 0, DWTC = 10.
// NQT: net_time = 13,|local_queue| = 5, |player_queue| = 0, DWTC = 10.

// Looks like it should be based on (required_action_flags - delay) - try to make that 0.
// It should be harder to reduce delay than to increase it.

// This is the basis for NETWORK_ADAPTIVE_LATENCY_2, then: compute (on all stations) when packet
// arrives, based on required_action_flags - delay.  USE the delay in NetGetNetTime() to hold the
// reins on the consumer a bit.


#ifdef NETWORK_ADAPTIVE_LATENCY_3
// ZZZ: and now for something completely different...
// Instead of considering average latency as above, we're going to look at the largest
// recent latency value, and go with that for a while.  If we don't see any latency value
// that big or bigger for a while, then we'll go with the second-highest latency value we've
// seen recently for a while, etc.
// This ought to handle jittery cases more easily, I think.

static int sCurrentAdaptiveLatency;
static int sGreatestRecentLatencyMeasurement;
static int sGreatestRecentLatencyTick;
static int sSecondGreatestRecentLatencyMeasurement;
#endif


static NetTopology* topology;
static short localPlayerIndex;
static short* sNetStatePtr;

// XXX (ZZZ): this is a nasty kludge.  Originally, I think, NetDDP* interfaces
// dealt in sockets - so one could potentially have multiple open sockets.
// NetDDP* for IP use some "socket" parameters etc. for port numbers instead,
// and in other interfaces (like NetDDPSendFrame()) the socket parameter is ignored.
// Here we exploit the "ignored" property.
static short ddpSocket = 0;


// ZZZ: again, buffer is always going to be NetDistributionPacket_NET*
static void NetProcessLossyDistribution(void *buffer);
// ZZZ: this is used to handle an incoming ring packet; it'll always be NetPacket_NET*
static void NetProcessIncomingBuffer(void *buffer, int32 buffer_size, int32 sequence);

static size_t NetPacketSize(NetPacketPtr packet);
static void NetBuildRingPacket(DDPFramePtr frame, byte *data, size_t data_size, int32 sequence);
static void NetBuildFirstRingPacket(DDPFramePtr frame, int32 sequence);
static void NetRebuildRingPacket(DDPFramePtr frame, short tag, int32 sequence);
static void NetAddFlagsToPacket(NetPacketPtr packet);

static void NetSendRingPacket(DDPFramePtr frame);
static void NetSendAcknowledgement(DDPFramePtr frame, int32 sequence);

static bool NetCheckResendRingPacket(void);
static bool NetServerTask(void);
static bool NetQueueingTask(void);

#if defined(NETWORK_ADAPTIVE_LATENCY) || defined(NETWORK_ADAPTIVE_LATENCY_2) || defined(NETWORK_ADAPTIVE_LATENCY_3)
// ZZZ addition, for adaptive latency business.  measurement is only used for adaptive_latency_2
// tick is used only for adaptive_latency_3.
static void update_adaptive_latency(int measurement = 0, int tick = 0);
#endif//NETWORK_ADAPTIVE_LATENCY || NETWORK_ADAPTIVE_LATENCY_2 || NETWORK_ADAPTIVE_LATENCY_3

static short NetAdjustUpringAddressUpwards(void);
static short NetSizeofLocalQueue(void);

static void process_packet_buffer_flags(void *buffer, int32 buffer_size, short packet_tag);
static void process_flags(NetPacketPtr packet_data);

/* Note that both of these functions may result in a change of gatherer.  the first one is */
/*  called when the other guy hasn't responded after a kRETRY times to our packet, so we */
/*  drop him and if he was gatherer, we become the gatherer.  The second function is called */
/*  when the gatherer sends out an unsync packet, but we aren't ready to quit.  Therefore we */
/*  must become the gatherer. */
static void drop_upring_player(void);

#ifdef DEBUG_NET_RECORD_PROFILE
void record_profile(int raf);
#endif

#ifdef DEBUG_NET
struct network_statistics {
	int32 numSmears;
	int32 numCountChanges;

	int32 ontime_acks;
	int32 sync_ontime_acks;
	int32 time_ontime_acks;
	int32 unsync_ontime_acks;
	int32 dead_ontime_acks;

	int32 late_acks;
	int32 packets_from_the_unknown;
	int32 retry_count;
	int32 sync_retry_count;
	int32 time_retry_count;
	int32 unsync_retry_count;
	int32 dead_retry_count;

	int32 late_unsync_rings;
	int32 late_sync_rings;
	int32 late_rings;
	int32 late_time_rings;
	int32 late_dead_rings;

	int32 change_ring_packet_count;

	int32 rebuilt_server_tag;
	int32 packets_with_zero_flags;

	short spurious_unsyncs;
	short unsync_while_coming_down;
	short upring_drops;
	short server_set_required_flags_to_zero;
	short unsync_returned_to_sender;
	short server_unsyncing;
	short assuming_control;
	short assuming_control_on_retry;
	short server_bailing_early;

	uint32 action_flags_processed;
} net_stats;

#endif


bool
RingGameProtocol::Enter(short* inNetStatePtr)
{
	bool success= false;
	sNetStatePtr= inNetStatePtr;
	status = (NetStatusPtr)malloc(sizeof(NetStatus));
	if(status)
	{
		memset(status, 0, sizeof(NetStatus));
		status->buffer = (byte *)malloc(ddpMaxData);
		if (status->buffer)
		{
			ringFrame= NetDDPNewFrame();
			if (ringFrame)
			{
				ackFrame= NetDDPNewFrame();
				if (ackFrame)
				{
					distributionFrame= NetDDPNewFrame();
					if (distributionFrame)
					{
						status->single_player= false;
#ifdef DEBUG_NET
						obj_clear(net_stats);
#endif
						success= true;
					}
				}
			}
		}
	}
	return success;
}


void
RingGameProtocol::Exit1()
{
	/* These functions do the right thing for NULL pointers */
	resendTMTask= myTMRemove(resendTMTask);
	serverTMTask= myTMRemove(serverTMTask);
	queueingTMTask= myTMRemove(queueingTMTask);
}


void
RingGameProtocol::Exit2()
{
#ifdef DEBUG_NET
	NetPrintInfo();
#endif

	free(status->buffer);
	free(status);
	status= NULL;

	NetDDPDisposeFrame(ackFrame);
	NetDDPDisposeFrame(ringFrame);
	NetDDPDisposeFrame(distributionFrame);
}	


/*
 -------
 NetSync
 -------

	(no parameters)

 make sure all players are present (by waiting for the ring to come around twice).  this is what
 actually jump-starts the ring.

 returns true if we synched successfully. false otherwise.

 -------
 NetUnSync
 -------

	(no parameters)

 called at the end of a network game to ensure a clean exit from the net game.
 (we make sure that we don’t quit the game holding the ring packet.)

 */

bool
RingGameProtocol::Sync(NetTopology* inTopology, int32 inSmallestGameTick, size_t inLocalPlayerIndex, size_t inServerPlayerIndex)
{
	uint32 ticks;
	bool success= true;
#ifdef TEST_MODEM
	return ModemSync();
#else
	localPlayerIndex= inLocalPlayerIndex;
	topology= inTopology;

        // ZZZ: taking the net-time from the dynamic_world; whittling away at network/game code separation again?
        // anyway we need to do this for restoring a saved-game, since DWTC won't start at 0 in that case (but
        // without the following line, the netTime would have...)  Anyway yeah NetSync() is only called after
        // the dynamic_world is set up - I'm pretty sure ;) - so this should be ok.
        status->localNetTime= inSmallestGameTick;
	status->action_flags_per_packet= initial_updates_per_packet;
	status->update_latency= initial_update_latency;
	status->lastValidRingSequence= 0;
	status->ringPacketCount= 0;
	status->server_player_index= inServerPlayerIndex;
	status->last_extra_flags= 0;
	status->worldUpdate = false;
	status->acceptPackets= true; /* let the PacketHandler see incoming packets */
	status->acceptRingPackets= true;
	local_queue.read_index= local_queue.write_index= 0;
#ifdef NETWORK_FAUX_QUEUE
        // ZZZ addition: faux queue mechanism to handle jittery scheduling correctly
        faux_queue_due = faux_queue_paid = 0;
#endif

#if defined(NETWORK_ADAPTIVE_LATENCY) || defined(NETWORK_ADAPTIVE_LATENCY_2)
        // ZZZ addition: initialize adaptive latency mechanism
        sCurrentAdaptiveLatency		= kAdaptiveLatencyInitialValue;
        sAdaptiveLatencyRunningTotal	= 0;
        sAdaptiveLatencySampleCount	= 0;
        sAdaptiveLatencyWindowEdge	= 0;
#endif

#if defined(NETWORK_ADAPTIVE_LATENCY_3)
        // ZZZ addition: initialize adaptive latency mechanism
        sCurrentAdaptiveLatency = 0;
        sGreatestRecentLatencyTick = 0;
        sGreatestRecentLatencyMeasurement = 0;
        sSecondGreatestRecentLatencyMeasurement = 0;
#endif

	// Calculate up- and downring neighbors
	short previousPlayerIndex, nextPlayerIndex;
	
        // ZZZ: changes to these to skip players with identifier NONE, in support of generalized resume-game
        // (They Might Be Zombies)
	/* recalculate downringAddress */
        previousPlayerIndex = localPlayerIndex;
	do
        {
                previousPlayerIndex = (topology->player_count + previousPlayerIndex - 1) % topology->player_count;
        } while(topology->players[previousPlayerIndex].identifier == NONE && previousPlayerIndex != localPlayerIndex);

        status->downringAddress= topology->players[previousPlayerIndex].ddpAddress;

	/* recalculate upringAddress */
        nextPlayerIndex = localPlayerIndex;
	do
        {
                nextPlayerIndex = (topology->player_count + nextPlayerIndex + 1) % topology->player_count;
        } while(topology->players[nextPlayerIndex].identifier == NONE && nextPlayerIndex != localPlayerIndex);

	status->upringAddress= topology->players[nextPlayerIndex].ddpAddress;
	status->upringPlayerIndex = nextPlayerIndex;
	
	*sNetStatePtr= netStartingUp;

	/* if we are the server (player index zero), start the ring */
	if (localPlayerIndex==status->server_player_index)
	{
		status->iAmTheServer= true;

		/* act like somebody just sent us this packet */
		status->ringPacketCount= 1;

		NetBuildFirstRingPacket(ringFrame, status->lastValidRingSequence+1);
		NetSendRingPacket(ringFrame);
	}
	else
	{
		status->iAmTheServer = false;
	}

	/* once we get a normal packet, netState will be set, and we can cruise. */
	ticks= machine_tick_count();
	while (success && *sNetStatePtr != netActive) // packet handler changes this variable.
	{
		if (machine_tick_count() - ticks > NET_SYNC_TIME_OUT)
		{
			alert_user(infoError, strNETWORK_ERRORS, netErrSyncFailed, 0);

			/* How did Alain do this? */
			status->acceptPackets= false;
			status->acceptRingPackets= false;
			*sNetStatePtr= netDown;
			success= false;
		}
	}
#endif // ndef TEST_MODEM

	return success;
}



/*
	New unsync:
	1) Server tells everyone to give him 0 action flags.
	2) Server then waits for the packet to go all the way around the loop.
 */
bool
RingGameProtocol::UnSync(bool inGraceful, int32 inSmallestPostgameTick)
{
        bool success= true;
	uint32 ticks;

#ifdef TEST_MODEM
	success= ModemUnsync();
#else

	if (*sNetStatePtr==netStartingUp || *sNetStatePtr==netActive)
	{
		*sNetStatePtr= netComingDown;

		/* Next time the server receives a packet, and if the netState==netComingDown */
		/*  the server will send a packet with zero flags, which means process the remaining */
		/*  flags, and get ready to change level.  Once the packet with zero flags gets back */
		/*  to the server, the server sends an unsync ring packet.  This will cause all the other */
		/*  machines to unsync, and when the server gets the packet back, it turns the net off */

		ticks= machine_tick_count();
		// we wait until the packet handler changes "acceptPackets" or until we hit a serious
		// timeout, in case we are quitting and someone else is refusing to give up the ring.
		while((status->acceptRingPackets || !status->receivedAcknowledgement)
	&& (machine_tick_count()-ticks<UNSYNC_TIMEOUT))
			;
	}
	if(status->acceptRingPackets)
	{
		status->acceptRingPackets= success= false;
	}
	status->acceptPackets= false; // in case we just timed out.
	*sNetStatePtr= netDown;

#ifdef DEBUG_NET
	fdprintf("Flags processed: %d Time: %d;g", net_stats.action_flags_processed, TickCount()-ticks);
	net_stats.action_flags_processed= 0;
#endif
#endif // ndef TEST_MODEM

	return success;
}



/* Distribute information to the whole net. */
void
RingGameProtocol::DistributeInformation(
			      short type,
			      void *buffer,
			      short buffer_size,
			      bool send_to_self,
	bool)
{
	NetDistributionPacket 		distribution_header;
        NetDistributionPacket_NET	distribution_header_NET;

#ifdef TEST_MODEM
	ModemDistributeInformation(type, buffer, buffer_size, send_to_self);
#else

	// Sanity Check! Sanity Check!
	// Hand Check! Hand Check!
 // ZZZ: there ought to be some kind of check here on buffer size, but it should be based on
 // the size of the DDP (UDP) packet storage allocated and the sizes of stuff to be crammed in there.
 //	assert(buffer_size <= MAX_NET_DISTRIBUTION_BUFFER_SIZE);

	// ZZZ: I suppose one could argue that you should be able to send distribution types you don't
 // process, but currently anyway this is used to lookup lossless/lossy.  Maybe the caller should
 // specify lossless/lossy as a parameter to this function rather than associating it with a
 // distribution type anyway.  The two seem to be at least somewhat independent.
	const NetDistributionInfo* theInfo = NetGetDistributionInfoForType(type);
	assert(theInfo != NULL);
	
	if (send_to_self)
	{
		theInfo->distribution_proc(buffer, buffer_size, localPlayerIndex);
	}

	distributionFrame->data_size = sizeof(NetPacketHeader_NET) + sizeof(NetDistributionPacket_NET) + buffer_size;
	{
		NetPacketHeader_NET*	header_NET	= (NetPacketHeader_NET*) distributionFrame->data;
                NetPacketHeader		header_storage;
                NetPacketHeader*	header		= &header_storage;

		header->tag = theInfo->lossy ? tagLOSSY_DISTRIBUTION : tagLOSSLESS_DISTRIBUTION;
		header->sequence = 0;

                netcpy(header_NET, header);
	}
	distribution_header.distribution_type = type;
	distribution_header.first_player_index = localPlayerIndex;
	distribution_header.data_size = buffer_size;

        // Probably could just netcpy this straight into distributionFrame->data.
        netcpy(&distribution_header_NET, &distribution_header);

	memcpy(distributionFrame->data + sizeof(NetPacketHeader_NET), &distribution_header_NET, sizeof(NetDistributionPacket_NET));
	memcpy(distributionFrame->data + sizeof(NetPacketHeader_NET) + sizeof(NetDistributionPacket_NET) /*- 2*sizeof(byte)*/, buffer, buffer_size);

	NetDDPSendFrame(distributionFrame, &status->upringAddress, kPROTOCOL_TYPE, ddpSocket);
#endif // TEST_MODEM
}



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

// ZZZ: this is the first place an incoming packet is seen by the networking subsystem.
// The job of the packetHandler is to demultiplex to NetProcessIncomingBuffer or to a
// distribution-processing function (currently, NetProcessLossyDistribution is the only one).
void
RingGameProtocol::PacketHandler(DDPPacketBufferPtr packet)
{
	static bool already_here = false;

        // ZZZ: netcpy some of the packet's data into unpacked-storage area

        NetPacketHeaderPtr	header		= (NetPacketHeaderPtr) unpackedReceiveBuffer;
        NetPacketHeader_NET*	header_NET	= (NetPacketHeader_NET*) packet->datagramData;

        netcpy(header, header_NET);

	//	NetPacketHeaderPtr header= (NetPacketHeaderPtr) packet->datagramData;

	assert(!already_here);
	already_here = true;

	if (status->acceptPackets)
	{
		if (packet->datagramSize >= sizeof(NetPacketHeader_NET) && packet->protocolType == kPROTOCOL_TYPE)
		{
			switch (header->tag)
			{
				case tagLOSSLESS_DISTRIBUTION:
					vpause("received lossless distribution packet. not implemented.");
					break;
				case tagLOSSY_DISTRIBUTION:
					NetProcessLossyDistribution(packet->datagramData+sizeof(NetPacketHeader_NET));
					break;
				case tagACKNOWLEDGEMENT:
if (sRingPreferences.mAcceptPacketsFromAnyone ||
    (packet->sourceAddress.host == status->upringAddress.host &&
     packet->sourceAddress.port == status->upringAddress.port))
{
	if (header->sequence==status->lastValidRingSequence+1)
	{
		/* on-time acknowledgement; set a global so our time manager task doesn’t resend
		this packet when it fires */
		//								fdprintf("ontime ack;g");
#ifdef DEBUG_NET
	{
		// Figure out what was ACKed
		NetPacket	packet_data_storage;
		NetPacket*	packet_data		= &packet_data_storage;
		NetPacket_NET*	packet_data_NET	= (NetPacket_NET*)
			(ringFrame->data + sizeof(NetPacketHeader_NET));

		netcpy(packet_data, packet_data_NET);

		//NetPacketPtr packet_data= (NetPacketPtr) (ringFrame->data+sizeof(NetPacketHeader));

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
				assert(false);
				break;
		}
	}
#endif//DEBUG_NET
		status->receivedAcknowledgement= true;
	}
	else
	{
		if (header->sequence<=status->lastValidRingSequence)
		{
			/* late acknowledgement; ignore */
			//								fdprintf("late ack;g");
#ifdef DEBUG_NET
			net_stats.late_acks++;
#endif
		}
		else
		{
			/* early acknowledgement; wet our pants (this should never happen) */
			//								fdprintf("early ack (%d>%d);g", header->sequence, status->lastValidRingSequence);
			assert(false);
		}
	}
}
break;

case tagCHANGE_RING_PACKET:
	status->downringAddress = packet->sourceAddress;

#ifdef DEBUG_NET
	net_stats.change_ring_packet_count++;
#endif
	//					fdprintf("got change ring packet %d;g", header->sequence);

	/* fall through to tagRING_PACKET */

case tagRING_PACKET:
	if(status->acceptRingPackets)
	{
if (sRingPreferences.mAcceptPacketsFromAnyone ||
    (packet->sourceAddress.host == status->downringAddress.host &&
     packet->sourceAddress.port == status->downringAddress.port))
{
	if (header->sequence <= status->lastValidRingSequence)
	{
#ifdef DEBUG_NET
								{
                                                                        // Log what we saw
                                                                        NetPacket	packet_data_storage;
                                                                        NetPacket*	packet_data		= &packet_data_storage;
                                                                        NetPacket_NET*	packet_data_NET		= (NetPacket_NET*)
										(packet->datagramData + sizeof(NetPacketHeader_NET));

                                                                        netcpy(packet_data, packet_data_NET);

									//NetPacketPtr packet_data= (NetPacketPtr) (packet->datagramData+sizeof(NetPacketHeader));

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
											assert(false);
											break;
									}
								}
#endif // DEBUG_NET
		/* late ring packet; acknowledge but ignore */
		NetSendAcknowledgement(ackFrame, header->sequence);
		//							fdprintf("late ring (%d<=%d);g", header->sequence, status->lastValidRingSequence);
	} // sequence <= lastValidRingSequence
	else
	{
		/* on-time or early ring packet */
		//							fdprintf("Got ring.;g");
  //							fdprintf("on-time ring %p (%d bytes);dm #%d #%d;g", packet, packet->datagramSize, packet->datagramData, packet->datagramSize);

		/* process remote actions, add our local actions, build ringFrame for sending */
		NetProcessIncomingBuffer(packet->datagramData+sizeof(NetPacketHeader_NET),
			   packet->datagramSize-sizeof(NetPacketHeader_NET), header->sequence);
	}
} // came from expected source
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
	//						fdprintf("packet from unknown source %8x!=%8x.;g;", *((long*)&packet->sourceAddress), *((long*)&status->downringAddress));
}
	} // accept_ring_packets
break;
// (was tagRING_PACKET, or tagRING_CHANGE_PACKET)

default:
	assert(false);
	break;
			} // switch(header->tag)
		} // packet seems legitimate (correct protocolType and size)
	} // accept_packets

already_here= false;
} // NetDDPPacketHandler



// Act on an incoming lossy-distribution datagram
// i.e., call the distribution function registered earlier, and ship the datagram along
// upring (unless upring is the one who sent it).
static void NetProcessLossyDistribution(
					void *buffer)
{
	short                     	type;

        // ZZZ: convert from NET format
        NetDistributionPacket_NET*	packet_data_NET	= (NetDistributionPacket_NET*) buffer;
	NetDistributionPacketPtr  	packet_data	= (NetDistributionPacketPtr) (unpackedReceiveBuffer + sizeof(NetPacketHeader));

        netcpy(packet_data, packet_data_NET);

	type = packet_data->distribution_type;

        // Act upon the data, if possible
	const NetDistributionInfo* theInfo = NetGetDistributionInfoForType(type);
	
	if (theInfo != NULL)
	{
		theInfo->distribution_proc(((char*) buffer) + sizeof(NetDistributionPacket_NET), packet_data->data_size,
				     packet_data->first_player_index);
        }

        // Should we pass the data on around the ring?
        // ZZZ: this used to only happen if the type_in_use was set, above.  I think we have an
        // obligation to pass the data along to others: even if _we_ don't know what to do with it,
        // someone else might.
        if (packet_data->first_player_index != status->upringPlayerIndex)
        {
                // ZZZ: set up for conversion to NET format
                NetPacketHeader_NET*		header_NET;
                NetPacketHeader			header_storage;
                NetPacketHeaderPtr        	header			= &header_storage;

                // fill in data
                distributionFrame->data_size = sizeof(NetPacketHeader_NET) + sizeof(NetDistributionPacket_NET) + packet_data->data_size;

                header->tag = tagLOSSY_DISTRIBUTION;
                header->sequence = 0;

                // do the conversion
                header_NET = (NetPacketHeader_NET*) distributionFrame->data;
                netcpy(header_NET, header);

                // (conversion complete)

                // copy in distribution data (raw, since we don't know what it is)
                memcpy(distributionFrame->data + sizeof(NetPacketHeader_NET),
		       buffer,
		       sizeof(NetDistributionPacket_NET) + packet_data->data_size);

                NetDDPSendFrame(distributionFrame, &status->upringAddress, kPROTOCOL_TYPE, ddpSocket);
        }
} // NetProcessLossyDistribution

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
				     int32 buffer_size,
				     int32 sequence)
{
        // ZZZ: convert from _NET format
        NetPacket*	packet_data		= (NetPacket*) (unpackedReceiveBuffer + sizeof(NetPacketHeader));
        NetPacket_NET*	packet_data_NET		= (NetPacket_NET*) buffer;

        netcpy(packet_data, packet_data_NET);

#ifdef DEBUG_NET_RECORD_PROFILE
        record_profile(packet_data->required_action_flags);
#endif

#if defined(NETWORK_ADAPTIVE_LATENCY_2) || defined(NETWORK_ADAPTIVE_LATENCY_3)
        // ZZZ: this is the only spot we sample/adjust our adaptive_latency_2 (or 3): when we've received a valid ring packet.
        // We sample the server's required_action_flags (set to its SizeofLocalQueue before sent) as that should be a
        // good indicator of actual ring latency.
        update_adaptive_latency(packet_data->required_action_flags, packet_data->server_net_time);
#endif

        // ZZZ: copy (byte-swapped) the action_flags into the unpacked buffer.
        netcpy(&packet_data->action_flags[0], (uint32*) (((char*) buffer) + sizeof(NetPacket_NET)), NetPacketSize(packet_data));

	short packet_tag= NONE;
	int32 previous_lastValidRingSequence= status->lastValidRingSequence;

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
			*sNetStatePtr= netActive; // we are live!
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
			break;

		case typeUNSYNC_RING_PACKET:
			/* We sent this out to end the game, and now it has made it back to us. */
			/* This means that we are ready to exit. */
			if(*sNetStatePtr==netComingDown)
			{
#ifdef DEBUG_NET
				//				fdprintf("Got an unsync packet.. (%d);g", net_stats.action_flags_processed);
				net_stats.unsync_while_coming_down++;
#endif
				status->acceptRingPackets= false;
				if(status->iAmTheServer)
				{
#ifdef DEBUG_NET
					//					fdprintf("Unsync returned to sender. Going down;g");
					net_stats.unsync_returned_to_sender++;
#endif
					packet_data->ring_packet_type= typeDEAD_PACKET;
				}
			}
#ifdef DEBUG_NET
			else
			{
				//				fdprintf("Got a spurious unsync packet...;g");
				net_stats.spurious_unsyncs++;
			}
#endif
			break;

		default:
			assert(false);
			break;
	}

	switch(packet_data->ring_packet_type)
	{
		case typeSYNC_RING_PACKET:
		case typeTIME_RING_PACKET:
			NetBuildRingPacket(ringFrame, (unsigned char *)packet_data, NetPacketSize(packet_data), status->lastValidRingSequence+1);
			/* We acknowledge just before sending the ring frame.... */
			NetSendAcknowledgement(ackFrame, status->lastValidRingSequence);
			NetSendRingPacket(ringFrame);
			break;

		case typeUNSYNC_RING_PACKET:
			/* Don't ack it unless we did something with it.  They will spam us with them and then */
			/*  time out. (important if one machine is slower than the others. */
			if(*sNetStatePtr==netComingDown)
			{
				NetBuildRingPacket(ringFrame, (unsigned char *)packet_data, NetPacketSize(packet_data), status->lastValidRingSequence+1);

				NetSendAcknowledgement(ackFrame, status->lastValidRingSequence);
				NetSendRingPacket(ringFrame);
			} else {
				/* Got it but ignored it.  lastValidRingSequence should be reset to what it was before. */
				status->lastValidRingSequence= previous_lastValidRingSequence;
			}
			break;

		case typeNORMAL_RING_PACKET:
			process_packet_buffer_flags(packet_data, buffer_size, packet_tag);
			break;

		case typeDEAD_PACKET:
			/* The buck stops here (after acknowledging it). */
			NetSendAcknowledgement(ackFrame, status->lastValidRingSequence);
			break;

		default:
			assert(false);
			break;
	}
} // NetProcessIncomingBuffer


static void NetAddFlagsToPacket(
				NetPacketPtr packet)
{
	uint32 *action_flags;
	short player_index;
	short action_flag_index;
	static bool already_here = false;
#ifdef NETWORK_USE_RECENT_FLAGS
	short extra_flags;
#endif

	assert(already_here == false);
	already_here= true;

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
	  csprintf(temporary, "bad count. count = %d. packet:; dm #%p", count, ((byte*)packet)-sizeof(NetPacketHeader)));

                // ZZZ: potential very sneaky bug: memcpy is not required to correctly handle overlapping copies, and
                // I think we probably have an overlapping copy here.  memmove it is.
		//		memcpy(action_flags + packet->required_action_flags, action_flags + packet->action_flag_count[localPlayerIndex], count * sizeof(uint32));
		memmove(action_flags + packet->required_action_flags, action_flags + packet->action_flag_count[localPlayerIndex], count * sizeof(uint32));
		//BlockMove(action_flags + packet->action_flag_count[localPlayerIndex],
		//	action_flags + packet->required_action_flags,
		//	count * sizeof(int32));
	}

#ifdef DEBUG_NET
	if(packet->required_action_flags==0) net_stats.packets_with_zero_flags++;
#endif

#ifndef	NETWORK_SMARTER_FLAG_DITCHING
#ifdef	NETWORK_USE_RECENT_FLAGS
        // ZZZ change: ditch older flags first, send newer flags.  I think this will "feel" better to players.
        // Consider: if we have too many flags, that probably means there was a "burp" somewhere along the line
        // that held up the ring packet.  We have already had to freeze our animation (since we didn't have enough
        // information to proceed) - the extra flags were accumulated during that freeze.  Wouldn't you want your
        // actions following the freeze to reflect what you were doing at the end of the freeze, not at the beginning?
        // Or, look at it this way: if you keep the early flags but throw away the later ones, you've added latency
        // (at least for those few flags that do get sent along) between the player's inputs and his screen updates.
        extra_flags = NetSizeofLocalQueue() - packet->required_action_flags;
        while(extra_flags-- > 0) {
		local_queue.read_index++;
		if (local_queue.read_index >= NET_QUEUE_SIZE) local_queue.read_index= 0;
	}
        // end of that change
#endif // NETWORK_USE_RECENT_FLAGS
#endif // !NETWORK_SMARTER_FLAG_DITCHING

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
#ifdef NETWORK_FAUX_QUEUE
                        // ZZZ: faux queue mechanism to handle jittery scheduling
                        faux_queue_due++;
#endif
#ifdef DEBUG_NET
			net_stats.numSmears++;
#endif
		}
	}

#ifdef	NETWORK_SMARTER_FLAG_DITCHING
        int leftover_flags = NetSizeofLocalQueue();

        // If no leftover flags, reset the counter.
        if(leftover_flags == 0)
		sFlagDitchingCounter = 0;

        // Otherwise, increment the counter.  Inc it faster if there are more leftovers.
        else
		sFlagDitchingCounter += leftover_flags;

        // If we've crossed the threshhold, we've got some flags that are just taking up space (and time!).
        // Ditch one, and reset the counter.  Note that as long as kFlagDitchingThreshhold is positive
        // (should be!), there must be at least one flag in the queue now, or else sFlagDitchingCounter
        // would have been reset above.
        if(sFlagDitchingCounter >= kFlagDitchingThreshhold) {
		local_queue.read_index++;

		if(local_queue.read_index >= NET_QUEUE_SIZE)
			local_queue.read_index = 0;

		sFlagDitchingCounter = 0;
        }

#else // !NETWORK_SMARTER_FLAG_DITCHING

        // ZZZ: this is the code that (effectively) was moved above
#ifndef	NETWORK_USE_RECENT_FLAGS
	// if we're accumulating too many flags, just ditch some to avoid latency
	// (which we assume is worse than losing a couple of flags)
	extra_flags= NetSizeofLocalQueue();
	short flags_to_remove= MIN(extra_flags, status->last_extra_flags);
	status->last_extra_flags = extra_flags - flags_to_remove;
	while (flags_to_remove--)
	{
		local_queue.read_index++;
		if (local_queue.read_index >= NET_QUEUE_SIZE) local_queue.read_index= 0;
	}
#endif // !NETWORK_USE_RECENT_FLAGS

#endif // !NETWORK_SMARTER_FLAG_DITCHING

	/* Sync the net time... */
	if (!status->iAmTheServer)
	{
		status->localNetTime= packet->server_net_time;
	}

	// tell everyone that we’re meeting code.
	packet->action_flag_count[localPlayerIndex]= packet->required_action_flags;

	//	fdprintf("NETPACKET:;dm %x %x;g;", packet, sizeof(NetPacket)+sizeof(long)*2*8);

	/* Allow for reentrance into this function */
	already_here= false;
}



static size_t NetPacketSize(
			    NetPacketPtr  packet)
{
	size_t size = 0;
	short i;

	/*	ZZZ: should not do this now, data was already converted elsewhere and we've been passed the unpacked version.
		NetPacket	packet_storage;
        NetPacket*	packet		= &packet_storage;

        netcpy(packet, packet_NET);
	*/
	for (i = 0; i < topology->player_count; ++i)
	{
		if (packet->action_flag_count[i] != NET_DEAD_ACTION_FLAG_COUNT) // player has become net dead.
		{
			assert(packet->action_flag_count[i]>=0&&packet->action_flag_count[i]<=MAXIMUM_UPDATES_PER_PACKET);
			size += packet->action_flag_count[i] * sizeof(int32);
		}
	}

        // ZZZ: CHANGE OF SEMANTICS from Bungie version - this gives only the size of the variable part
        // of the packet (instead of the variable part + sizeof(NetPacket)).
        // Since NetPacketSize is ONLY used to compute a size value for calls to NetBuildRingPacket, and
        // NetBuildRingPacket is ONLY called with a value computed by NetPacketSize, this is safe - I will
        // alter NetBuildRingPacket to expect this different value.
	return size;
}

/*
 ----------------------
 NetSendAcknowledgement
 ----------------------

	--> DDPFramePtr (usually ackFrame)   	// ZZZ note: currently, ALWAYS ackFrame.
	--> sequence to acknowledge		// ZZZ note: always status->lastValidRingSequence EXCEPT when acking a late ring packet.

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
				   int32 sequence)
{
        NetPacketHeader_NET*	header_NET	= (NetPacketHeader_NET*) frame->data;
        NetPacketHeader		header_storage;
        NetPacketHeader*	header		= &header_storage;

	//	fdprintf("sending ack.;g");

	/* build the acknowledgement */
	frame->data_size= sizeof(NetPacketHeader_NET);
	header->tag= tagACKNOWLEDGEMENT;
	header->sequence= sequence;

        // (ZZZ) format the ack for the network
        netcpy(header_NET, header);

	/* send the acknowledgement */
	NetDDPSendFrame(frame, &status->downringAddress, kPROTOCOL_TYPE, ddpSocket);
}

/* Only the server can call this... */
static void NetBuildFirstRingPacket(
				    DDPFramePtr frame,
				    int32 sequence)
{
	short player_index;
	NetPacketPtr  data;

        // ZZZ: why doesn't he let this be automatically allocated on the stack?  It's small, and is only needed for
        // the duration of the function call... well, no changes made, just curious.
	data = (NetPacketPtr)malloc(sizeof(NetPacket));
	//data = (NetPacketPtr) NewPtr(sizeof(NetPacket));
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

	free(data);
	//DisposePtr((Ptr) data);
}

// ZZZ: now, we build a packed (_NET format) ring packet from unpacked source data.
static void NetBuildRingPacket(
			       DDPFramePtr frame,
			       byte *data,
			       size_t data_size,
			       int32 sequence)
{
        NetPacketHeader		header_storage;
        NetPacketHeader*	header		= &header_storage;
	NetPacketHeader_NET*	header_NET	= (NetPacketHeader_NET*) frame->data;

	/* build the ring packet */
        // ZZZ: note that data_size is now just the size of the variable-length part (i.e. the action_flags)
        // so we will add the sizeof both _NET format structures first.
	assert(sizeof(NetPacketHeader_NET) + sizeof(NetPacket_NET) + data_size
	== static_cast<size_t>(static_cast<short>(sizeof(NetPacketHeader_NET) + sizeof(NetPacket_NET) + data_size)));
	frame->data_size= static_cast<short>(sizeof(NetPacketHeader_NET) + sizeof(NetPacket_NET) + data_size);

        // ZZZ: set up our local header buffer's data
	header->tag= tagRING_PACKET;
	header->sequence= sequence;

        // ZZZ: changed this check to frame->data_size from just data_size, seems to be more accurate
	assert(frame->data_size<ddpMaxData);

        // ZZZ: copy in the NetPacketHeader_NET stuff from our local buffer.
        netcpy(header_NET, header);

        // ZZZ: copy in the NetPacket_NET structure from the passed-in data.
        NetPacket*	packet_data	= (NetPacket*) data;
        NetPacket_NET*	packet_data_NET	= (NetPacket_NET*) (frame->data + sizeof(NetPacketHeader_NET));

        netcpy(packet_data_NET, packet_data);

        // ZZZ: I guess this would still do the right thing if netcpy (or memcpy, in some cases) gets 0 for the
        // length, but to avoid taking that risk and to save a little work, we skip if it there aren't any flags
        // (like, if we were called from NetBuildFirstRingPacket())
        if(data_size > 0) {
		// ZZZ: copy in the action_flags from the unpacked, passed-in data.
		uint32*		action_flags		= &packet_data->action_flags[0];
		uint32*		action_flags_NET	= (uint32*) (frame->data + sizeof(NetPacketHeader_NET) + sizeof(NetPacket_NET));

		netcpy(action_flags_NET, action_flags, data_size);
        }
}

// ZZZ: fixed to deal with packed (_NET) format
static void NetRebuildRingPacket(
				 DDPFramePtr frame,
				 short tag,
				 int32 sequence)
{
        NetPacketHeader		header_storage;
        NetPacketHeader*	header		= &header_storage;
        NetPacketHeader_NET*	header_NET	= (NetPacketHeader_NET*) frame->data;
	//	NetPacketHeaderPtr header= (NetPacketHeaderPtr) frame->data;

	header->tag= tag;
	header->sequence= sequence;

        netcpy(header_NET, header);
}

static void NetSendRingPacket(
			      DDPFramePtr frame)
{
	//	fdprintf("sent frame;g");

	status->retries= 0; // needs to be here, in case retry task was canceled (’cuz it likes to set retries)
	status->receivedAcknowledgement= false; /* will not be set until we receive an acknowledgement for this packet */

	if (!resendTMTask)
	{
		resendTMTask= myTMSetup(kACK_TIMEOUT, NetCheckResendRingPacket);
	} else {
		myTMReset(resendTMTask);
	}

	status->canForwardRing= false; /* will not be set unless this task fires without a packet to forward */
	status->clearToForwardRing= false; /* will not be set until we receive the next valid ring packet but will be irrelevant if serverCanForwardRing is true */
	// LP: NetAddrBlock is the trouble here
	NetDDPSendFrame(frame, &status->upringAddress, kPROTOCOL_TYPE, ddpSocket);
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
static bool NetCheckResendRingPacket(
				     void)
{
	bool reinstall= (*sNetStatePtr != netDown);

	if(reinstall)
	{
		if (!status->receivedAcknowledgement)
		{
			if(++status->retries>=kRETRIES)
			{
				switch(*sNetStatePtr)
				{
					case netStartingUp:
						/* There might be several retries as we start up */
						break;

					case netComingDown:
#ifdef DEBUG_NET
						fdprintf("Never got confirmation on NetUnsync packet.  They don't love us.");
#endif
						reinstall= false;
						status->acceptRingPackets= false;
						break;

					default:
						/* They have been gone too long.. */
						drop_upring_player();
						break;
				}
			}

#ifdef DEBUG_NET
			// #error need to alter this to work with new (_NET) packet formats, or data will be screwy.
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
						assert(false);
						break;
				}
			}
#endif
			/* Resend it.. */
			// LP: NetAddrBlock is the trouble here
			NetDDPSendFrame(ringFrame, &status->upringAddress, kPROTOCOL_TYPE, ddpSocket);
		}
		else
		{
			status->retries = 0;
		}
	}

	return reinstall;
}

static bool NetServerTask(
			  void)
{
	short local_queue_size = NetSizeofLocalQueue();
	bool reinstall= (*sNetStatePtr != netDown);

	if(reinstall)
	{
		/* Call the local net queueing proc.. */
		if (local_queue_size < MAXIMUM_UPDATES_PER_PACKET)
		{
                        // ZZZ: did not put faux queue here since server should be right-on
			local_queue_size++; // Random voodoo...
			local_queue.buffer[local_queue.write_index++] = parse_keymap();
			if (local_queue.write_index >= NET_QUEUE_SIZE)
				local_queue.write_index = 0;
			status->localNetTime++;
		}

#ifdef NETWORK_ADAPTIVE_LATENCY
                // ZZZ change: send the packet along if we've covered the current adaptive latency.
                if(local_queue_size >= sCurrentAdaptiveLatency)
#else
			if (local_queue_size >= status->action_flags_per_packet)
#endif
			{
				// This weird voodoo with canForwardRing prevents a problem if a packet arrives at the wrong time.
				status->canForwardRing = true; /* tell the socket listener it can forward the ring if it receives it */
				if (status->clearToForwardRing) /* has the socket listener already received the ring?  and not forwarded it? */
				{	// ZZZ: this control path is taken if the ring is waiting for us.  The packet we received is in
      // status->buffer.
      // For the other case (we are ready, but ring is not), see NetDDPPacketHandler.
      // The effect is to impose a "ring speed limit" - rings will not go around faster than
      // we accumulate data to put in them.  (Makes sense...)
					NetPacketPtr packet_data= (NetPacketPtr) status->buffer;

					//				status->canForwardRing = false;
     // XXX (ZZZ): I need to investigate this net_time business; now that I am throwing away
     // server flags, I may need to throw away server time as well (?).
					packet_data->server_net_time= status->localNetTime;
					if(*sNetStatePtr==netComingDown)
					{
						if(packet_data->required_action_flags==0)
						{
#ifdef DEBUG_NET
							//						fdprintf("I Server got a normal packet, at zero.  unsyncing... (%d);g", net_stats.action_flags_processed);
							net_stats.server_unsyncing++;
#endif
							packet_data->ring_packet_type= typeUNSYNC_RING_PACKET;
						}
						else
						{
#ifdef DEBUG_NET
							//						fdprintf("I Server got a normal packet & net was coming down required flags at 0. (%d);g", net_stats.action_flags_processed);
							net_stats.server_set_required_flags_to_zero++;
#endif
							/* Change the type to an unsync ring packet... */
							packet_data->required_action_flags= 0;
						}
					} // netComingDown
					else // netState != netComingDown
					{
#ifdef NETWORK_ADAPTIVE_LATENCY
						// ZZZ change: only send out as many flags as adaptive latency suggests
						packet_data->required_action_flags = sCurrentAdaptiveLatency;
#else
						packet_data->required_action_flags= NetSizeofLocalQueue();
#endif
					}

					NetAddFlagsToPacket(packet_data);
					NetBuildRingPacket(ringFrame, (byte *) packet_data, NetPacketSize(packet_data), status->lastValidRingSequence+1);
					if(status->new_packet_tag != NONE)
					{
#ifdef DEBUG_NET
						//					fdprintf("rebuilding the server tag (%d);g", status->new_packet_tag);
						net_stats.rebuilt_server_tag++;
#endif
						NetRebuildRingPacket(ringFrame, status->new_packet_tag, status->lastValidRingSequence+1);
					}

					/* Send the Ack just before we pass the token along.. */
					NetSendAcknowledgement(ackFrame, status->lastValidRingSequence);
					NetSendRingPacket(ringFrame);
				} // clearToForwardRing (ring was already here waiting for us to accumulate enough data)
			} // we have accumulated enough data to let the ring go on
	} // reinstall (netState != netDown)

	status->worldUpdate = true;

		return reinstall;
} // NetServerTask

static bool NetQueueingTask(
			    void)
{
	bool reinstall= (*sNetStatePtr != netDown);

	if(reinstall)
	{
		if (NetSizeofLocalQueue() < MAXIMUM_UPDATES_PER_PACKET)
		{
#ifdef NETWORK_FAUX_QUEUE
                        // ZZZ: if we owe the packet handler flags (since it smeared to cover for us), pay up.
                        if(faux_queue_paid != faux_queue_due) {
				faux_queue_paid++;
                        }
                        // OTOH if we're even-steven, we will go ahead and queue some flags for the next ring packet.
                        else {
#endif
				local_queue.buffer[local_queue.write_index++] = parse_keymap();
				if (local_queue.write_index >= NET_QUEUE_SIZE)
					local_queue.write_index = 0;
#ifdef NETWORK_FAUX_QUEUE
                        }
#endif
			status->localNetTime++;
		} // room to store an action_flag
	} // reinstall (netState != netDown)

	status->worldUpdate = true;

	return reinstall;
} // NetQueueingTask



#if defined(NETWORK_ADAPTIVE_LATENCY) || defined(NETWORK_ADAPTIVE_LATENCY_2)
// ZZZ addition: adaptive latency business.  measurement used only in adaptive_latency_2.
// tick used only in adaptive_latency_3.
static void
update_adaptive_latency(int measurement, int tick) {

#ifdef NETWORK_ADAPTIVE_LATENCY_2
	// Use the provided measurement
	int theCurrentLatencyMeasurement = measurement;
#else
	// Take the current local_queue_size as a measurement of ring latency
	int theCurrentLatencyMeasurement = NetGetSizeofLocalQueue();
#endif

	// Subtract the sample leaving the window from the running total
	if(sAdaptiveLatencySampleCount >= kAdaptiveLatencyWindowSize)
		sAdaptiveLatencyRunningTotal -= sAdaptiveLatencySamples[sAdaptiveLatencyWindowEdge];

	// Put the new sample in as the newest sample in the sample buffer
	sAdaptiveLatencySamples[sAdaptiveLatencyWindowEdge] = theCurrentLatencyMeasurement;

	// Slide the window one sample
	sAdaptiveLatencyWindowEdge = (sAdaptiveLatencyWindowEdge + 1) % kAdaptiveLatencyWindowSize;

	// If we're still collecting the initial set of samples, update our count
	if(sAdaptiveLatencySampleCount < kAdaptiveLatencyWindowSize)
		sAdaptiveLatencySampleCount++;

	// Add the newest sample into our running total
	sAdaptiveLatencyRunningTotal += theCurrentLatencyMeasurement;

	// Don't adjust the latency until we have a window's worth of samples to work from
	if(sAdaptiveLatencySampleCount >= kAdaptiveLatencyWindowSize) {
		// Find the average latency for the past window's worth of samples
		float theAverageLatencyMeasurement = (float) sAdaptiveLatencyRunningTotal / (float) kAdaptiveLatencyWindowSize;

		// See if we should adjust our latency based on that average measurement.  Be careful not to adjust
  // latency below 1 or above the cap.
		if(theAverageLatencyMeasurement - sCurrentAdaptiveLatency > kNeedToIncreaseLatencyThreshhold
     && sCurrentAdaptiveLatency < kAdaptiveLatencyMaximumValue)
		{
			sCurrentAdaptiveLatency++;
			logNote1("adjusted latency upwards to %d", sCurrentAdaptiveLatency);
		}
		else if(theAverageLatencyMeasurement - sCurrentAdaptiveLatency < kNeedToDecreaseLatencyThreshhold
	  && sCurrentAdaptiveLatency > 1)
		{
			sCurrentAdaptiveLatency--;
			logNote1("adjusted latency downwards to %d", sCurrentAdaptiveLatency);
		}
	}

} // update_adaptive_latency
#endif// NETWORK_ADAPTIVE_LATENCY || NETWORK_ADAPTIVE_LATENCY_2

#ifdef NETWORK_ADAPTIVE_LATENCY_3
// ZZZ addition: adaptive latency business.
static void
update_adaptive_latency(int measurement, int tick) {
        // Ignore samples from old packets that may show up; also bail if user doesn't love us
        if(tick <= sGreatestRecentLatencyTick || !sRingPreferences.mAdaptToLatency)
                return;

        // Update our measurements etc.
        if(measurement > sGreatestRecentLatencyMeasurement)
        {
                sSecondGreatestRecentLatencyMeasurement = sGreatestRecentLatencyMeasurement;
                sGreatestRecentLatencyMeasurement = measurement;
                sGreatestRecentLatencyTick = tick;
        }
        else if(measurement == sGreatestRecentLatencyMeasurement)
        {
                sGreatestRecentLatencyTick = tick;
        }
        else if(measurement > sSecondGreatestRecentLatencyMeasurement)
        {
                sSecondGreatestRecentLatencyMeasurement = measurement;
        }

        // If it's been long enough since we've seen our current greatest, fall back to second-greatest
        if(tick - sGreatestRecentLatencyTick > sRingPreferences.mLatencyHoldTicks)
        {
                sGreatestRecentLatencyTick = tick;
                sGreatestRecentLatencyMeasurement = sSecondGreatestRecentLatencyMeasurement;
                sSecondGreatestRecentLatencyMeasurement = 0;
        }

        int theNewAdaptiveLatency = MIN(sGreatestRecentLatencyMeasurement, MAXIMUM_UPDATES_PER_PACKET);

        if(sCurrentAdaptiveLatency != theNewAdaptiveLatency)
        {
                logDump("tick %d: setting adaptive latency to %d", tick, theNewAdaptiveLatency);
                sCurrentAdaptiveLatency = theNewAdaptiveLatency;
        }
}
#endif // NETWORK_ADAPTIVE_LATENCY_3



// This function does two things. It changes the upring address to be the upring address
// of the next dude in the ring. It also returns the playerIndex of what used to be
// the next player, so that we can fiddle with things.
static short NetAdjustUpringAddressUpwards(
					   void)
{
	short nextPlayerIndex, newNextPlayerIndex;
	NetAddrBlock *address;

	// figure out where the current upring address is.
	for (nextPlayerIndex= 0; nextPlayerIndex<topology->player_count; nextPlayerIndex++)
	{
		address = &(topology->players[nextPlayerIndex].ddpAddress);
			if (address->host == status->upringAddress.host &&
       address->port == status->upringAddress.port)
			{
				break;
			}
	}
		assert(nextPlayerIndex != topology->player_count);

		// ZZZ: changed to deal with 'gaps' (players with identifier NONE), in support of generalized game-resumption
		newNextPlayerIndex = nextPlayerIndex;
		do
		{
			newNextPlayerIndex = (topology->player_count + newNextPlayerIndex + 1) % topology->player_count;
		} while(topology->players[newNextPlayerIndex].identifier == NONE && newNextPlayerIndex != localPlayerIndex);

		status->upringAddress= topology->players[newNextPlayerIndex].ddpAddress;
		status->upringPlayerIndex= newNextPlayerIndex;

		return nextPlayerIndex;
}

static void drop_upring_player(
			       void)
{
        // ZZZ: unpack existing ringFrame (from _NET format)
        byte	unpackedBuffer[ddpMaxData];

        NetPacket*	packet_data		= (NetPacket*) unpackedBuffer;
        NetPacket_NET*	packet_data_NET		= (NetPacket_NET*) (ringFrame->data + sizeof(NetPacketHeader_NET));

        netcpy(packet_data, packet_data_NET);

        uint32*		action_flags		= &packet_data->action_flags[0];
        uint32*		action_flags_NET	= (uint32*) (ringFrame->data + sizeof(NetPacketHeader_NET) + sizeof(NetPacket_NET));

        size_t		data_size		= NetPacketSize(packet_data);

        netcpy(action_flags, action_flags_NET, data_size);

        // (done unpacking)

	short flag_count, index, oldNextPlayerIndex;
	//	NetPacketPtr packet_data= (NetPacketPtr) (ringFrame->data + sizeof (NetPacketHeader));

	/* Reset the retries for the new packet. */
	status->retries= 0;

	flag_count= 0;

#ifdef DEBUG_NET
	//	fdprintf("Dropping upring- Attempting to delete upring (node %d) from ring. muhaha.;g", status->upringAddress.aNode);
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
		status->iAmTheServer= true;
#ifdef DEBUG_NET
		//		fdprintf("Trying to become the server (drop_upring);g");
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
  // ZZZ: here's that sneaky bug again, memcpy is not guaranteed to work for overlapping src and dest;
  // we use memmove instead.
		memmove(action_flags, action_flags + packet_data->action_flag_count[oldNextPlayerIndex], flag_count * sizeof(uint32));
		//memcpy(action_flags, action_flags + packet_data->action_flag_count[oldNextPlayerIndex], flag_count * sizeof(uint32));
		//BlockMove(action_flags + packet_data->action_flag_count[oldNextPlayerIndex],
		//	action_flags, sizeof(long)*flag_count);
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
	if(index==topology->player_count) status->single_player= true;

	// we have to increment the ring sequence counter in case we’re sending to ourselves
	// to prevent "late ring packets"
 // ZZZ: to take advantage of repacking, I pass our buffer into NetBuildRingPacket.
 // (original code used just Rebuild, below.)
        NetBuildRingPacket(ringFrame, unpackedBuffer, data_size, status->lastValidRingSequence+1);

        // ZZZ: still need this though to change tag.
	NetRebuildRingPacket(ringFrame, tagCHANGE_RING_PACKET, status->lastValidRingSequence+1);
}



int32
RingGameProtocol::GetNetTime(void)
{
        // ZZZ: modified so as not to introduce ANY gratuitous latency.  May make play a little choppy.
	// Consider falling back to localNetTime - action_flags_per_packet...
 // (later) Took that back out.  It did make play nicely responsive, but opened us up wide to latency and jitter.
 // I hope adaptive_latency_2 will be the final meddling with this stuff.
 //    return status->localNetTime;
#if defined(NETWORK_ADAPTIVE_LATENCY_2) || defined(NETWORK_ADAPTIVE_LATENCY_3)
	// This is it - this is the only place sCurrentAdaptiveLatency has any effect in adaptive_latency_2 (or 3).
 // The effect is to get the game engine to drain the player_queues more smoothly than they would
 // if we returned status_localNetTime.
 // Later: adding 1 in an effort to decrease lag (is this a good idea?)
 // Later than that: getting rid of that "+1", need to subtract sCurrentAdaptiveLatency and that's that.
 // Unless rings are taking less than a tick-time to get around, in which case we need not introduce
 // any latency, which is why we special-case that.
	return status->localNetTime - (sCurrentAdaptiveLatency > 1 ? sCurrentAdaptiveLatency : 0);
#else
	return status->localNetTime - 2*status->action_flags_per_packet - status->update_latency;
#endif
}

bool
RingGameProtocol::CheckWorldUpdate()
{
	if (status->worldUpdate)
	{
		status->worldUpdate = false;
		return true;
	}
	else
	{
		return false;
	}
}

int32 RingGameProtocol::GetUnconfirmedActionFlagsCount()
{
	return GetRealActionQueues()->countActionFlags(NetGetLocalPlayerIndex());
}

uint32 RingGameProtocol::PeekUnconfirmedActionFlag(int32 offset)
{
	return GetRealActionQueues()->peekActionFlags(NetGetLocalPlayerIndex(), offset);
}

void RingGameProtocol::UpdateUnconfirmedActionFlags() { }


// brazenly copied and modified from player.c (though i clearly format it much better)
static short NetSizeofLocalQueue(
				 void)
{
	short size;

	if ((size= local_queue.write_index-local_queue.read_index) < 0)
		size += NET_QUEUE_SIZE;

	return size;
}



void NetPrintInfo(
		  void)
{
#ifdef DEBUG_NET
	fdprintf("numSmears= %d numCountChanges= %d ring packet_count= %d Single: %d;g", net_stats.numSmears,
	  net_stats.numCountChanges, 	status->ringPacketCount, status->single_player);
	fdprintf("localPlayerIndex= %d, server_player_index= %d;g", localPlayerIndex, status->server_player_index);
	fdprintf("tick_count= %d, localNetTime= %d;g", dynamic_world->tick_count, status->localNetTime);
	fdprintf("Unknown packets: %d Upring Drops: %d Rebuilt server Tags: %d;g", net_stats.packets_from_the_unknown, net_stats.upring_drops, net_stats.rebuilt_server_tag);
	fdprintf("Late Rings: Sync: %d Time: %d Normal: %d Unsync: %d Dead: %d;g", net_stats.late_sync_rings, net_stats.late_time_rings, net_stats.late_rings, net_stats.late_unsync_rings, net_stats.late_dead_rings);
	fdprintf("---Retries: Sync: %d Time: %d Normal: %d Unsync: %d Dead: %d;g", net_stats.sync_retry_count, net_stats.time_retry_count, net_stats.retry_count, net_stats.unsync_retry_count, net_stats.dead_retry_count);
	fdprintf("Ontime Ack: Sync: %d Time: %d Normal: %d Unsync: %d Dead: %d Late: %d;g", net_stats.sync_ontime_acks, net_stats.time_ontime_acks, net_stats.ontime_acks, net_stats.unsync_ontime_acks, net_stats.dead_ontime_acks, net_stats.late_acks);
	if(localPlayerIndex==status->server_player_index)
	{
		fdprintf("Server: Req to zero: %d Unsyncing: %d Returned: %d;g", net_stats.server_set_required_flags_to_zero, net_stats.server_unsyncing, net_stats.unsync_returned_to_sender);
	}
	fdprintf("Packets w/zero flags: %d Spurious Unsyncs: %d;g", net_stats.packets_with_zero_flags, net_stats.spurious_unsyncs);
	fdprintf("Assumed control: Normal: %d Retry: %d (Server bailed early: %d);g", net_stats.assuming_control, net_stats.assuming_control_on_retry, net_stats.server_bailing_early);
	fdprintf("Proper unsyncs: %d", net_stats.unsync_while_coming_down);
#endif//DEBUG_NET
} // NetPrintInfo

static void process_packet_buffer_flags(
					void *buffer,
					int32 buffer_size,
					short packet_tag)
{
        // ZZZ: By now, the buffer passed to us is unpacked and contains action flags.
	/*        NetPacket	packet_data_storage;
        NetPacket*	packet_data		= &packet_data_storage;
        NetPacket_NET*	packet_data_NET		= (NetPacket_NET*) buffer;

        netcpy(packet_data, packet_data_NET);
	*/
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
		status->canForwardRing = false;
		if (status->iAmTheServer)
		{ // (ZZZ) This is the (server) control path if there is enough data available to send on the ring
			packet_data->server_player_index= localPlayerIndex;
			packet_data->server_net_time= status->localNetTime;
			if(*sNetStatePtr==netComingDown)
			{
				/* Change the type to an unsync ring packet... */
				if(packet_data->required_action_flags==0)
				{
#ifdef DEBUG_NET
					//					fdprintf("Server got a final packet & net was coming down (changed) type: %d (%d);g",  packet_data->ring_packet_type, net_stats.action_flags_processed);
					net_stats.server_unsyncing++;
#endif
					packet_data->ring_packet_type= typeUNSYNC_RING_PACKET;
				}
				else
				{
#ifdef DEBUG_NET
					//					fdprintf("Server got a packet & net was coming down (changed) (%d) current: %d;g",  net_stats.action_flags_processed, packet_data->required_action_flags);
					net_stats.server_set_required_flags_to_zero++;
#endif
					packet_data->required_action_flags= 0;
				}
			} // netState == netComingDown
			else // netState != netComingDown
			{
#ifdef NETWORK_ADAPTIVE_LATENCY
				// ZZZ addition: update the adaptive latency state
				update_adaptive_latency();
				// ZZZ change: use the number of flags the adaptive latency system tells us.
    // It's ok if this increases the number of flags needed; we'll just smear.
				packet_data->required_action_flags = sCurrentAdaptiveLatency;
#else
				packet_data->required_action_flags= NetSizeofLocalQueue();
#endif
			}
		} // status->iAmTheServer

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
	else // status->iAmTheServer && !status->canForwardRing
	{ // (ZZZ note) This is the control path if we receive the ring before we have enough data to send the next one.
   // Essentially, we package up the packet we received and wait for NetServerTask to deal with it later.
		memcpy(status->buffer, packet_data, buffer_size - sizeof(NetPacket_NET) + sizeof(NetPacket));
		//BlockMove(buffer, status->buffer, buffer_size);
		status->clearToForwardRing = true;
		status->new_packet_tag= packet_tag;
#ifdef NETWORK_ADAPTIVE_LATENCY
                // ZZZ addition: also, we update the adaptive latency now, while we know what time the packet arrived.
                // It will be used in NetServerTask next time that's scheduled.
                update_adaptive_latency();
#endif
	}
} // process_packet_buffer_flags

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
			  NetPacket* packet_data)
{
	uint32 *action_flags= packet_data->action_flags;
	short player_index;

	/* Process the action flags (including our old ones) */
	for (player_index= 0; player_index<topology->player_count; ++player_index)
	{
		short player_flag_count= packet_data->action_flag_count[player_index];

		vassert(player_flag_count >= -1 && player_flag_count <= MAXIMUM_UPDATES_PER_PACKET,
	  csprintf(temporary, "UGH! count= %d;dm #%p", player_flag_count,
	    ((byte*)packet_data)-sizeof(NetPacketHeader)));

		/* if the player is not net dead */
		if (player_flag_count != NET_DEAD_ACTION_FLAG_COUNT)
		{
			process_action_flags(player_index, action_flags, player_flag_count);
#ifdef DEBUG_NET
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

			//			fdprintf("will stuff %d flags", packet_data->required_action_flags);
			for (index= 0; index<packet_data->required_action_flags; index++)
			{
				uint32 flag= (uint32)NET_DEAD_ACTION_FLAG;

				topology->players[player_index].net_dead= true;

				process_action_flags(player_index, &flag, 1);
#ifdef DEBUG_NET
				net_stats.action_flags_processed+= 1;
#endif
			}
		}
	}
}



#ifdef DEBUG_NET_RECORD_PROFILE
// ZZZ: this was used (by me) for some debugging stuff
struct net_profile_record {
	uint32	timestamp;
	int32	local_queue_size;
	int32	required_action_flags;
	int32	player_queue_size;
	int32	other_player_queue_size;
	int32	net_time;
	int32   supposed_net_time;
	int32	world_time;
};

net_profile_record	net_profile[1000];
int	net_profile_index = 0;

#include	"player.h"

void
record_profile(int req_action_flags) {
	if(net_profile_index < 1000) {
		// capture 1000 profiling entries
		net_profile[net_profile_index].timestamp	= machine_tick_count();
		net_profile[net_profile_index].local_queue_size	= NetSizeofLocalQueue();
		net_profile[net_profile_index].required_action_flags = req_action_flags;
		net_profile[net_profile_index].player_queue_size= get_action_queue_size(local_player_index);
		net_profile[net_profile_index].other_player_queue_size= get_action_queue_size(1-local_player_index);
		net_profile[net_profile_index].net_time		= status->localNetTime;
		net_profile[net_profile_index].supposed_net_time = NetGetNetTime();
		net_profile[net_profile_index].world_time	= dynamic_world->tick_count;

		net_profile_index++;
	}
	else
		// hop into debugger to see the results
		assert(false);
}
#endif // DEBUG_NET_RECORD_PROFILE



void
RingGameProtocol::ParsePreferencesTree(InfoTree prefs, std::string version)
{
	prefs.read_attr("accept_packets_from_anyone", sRingPreferences.mAcceptPacketsFromAnyone);
	prefs.read_attr("adapt_to_latency", sRingPreferences.mAdaptToLatency);
	prefs.read_attr_bounded<int32>("latency_hold_ticks", sRingPreferences.mLatencyHoldTicks, 2, INT32_MAX);
}

InfoTree RingPreferencesTree()
{
	InfoTree root;
	root.put_attr("accept_packets_from_anyone", sRingPreferences.mAcceptPacketsFromAnyone);
	root.put_attr("adapt_to_latency", sRingPreferences.mAdaptToLatency);
	root.put_attr("latency_hold_ticks", sRingPreferences.mLatencyHoldTicks);
	
	return root;
}



void
DefaultRingPreferences()
{
	sRingPreferences.mAcceptPacketsFromAnyone = false;
	sRingPreferences.mAdaptToLatency = true;
	sRingPreferences.mLatencyHoldTicks = 2 * TICKS_PER_SECOND;
}

#endif // !defined(DISABLE_NETWORKING)
