/*
NETWORK.C

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
        
Sept-Oct 2001 (Woody Zenfell): (roughly in order)
        Plugged in netcpy/_NET stuff and a couple #ifdef SDL byte-swappers for portable data formats.
        Changed a couple memcpy() calls to memmove() calls as they have overlapping source and dest.
        Allowed the use of the MyTM* functions, which now have SDL_thread-based implementations.
        Added optional NETWORK_FAUX_QUEUE mechanism, should work on either platform.
        It was a good idea, I think, but ultimately fairly pointless.  NETWORK_ADAPTIVE_LATENCY should be better.
        Added optional NETWORK_ADAPTIVE_LATENCY mechanism, should work on either platform.
        Changed some #ifdef mac conditionals to #ifndef NETWORK_IP to better convey what we're worried about.
        Added NETWORK_USE_RECENT_FLAGS option to discard excess flags from the head, rather than tail, of the queue.
        Added... how to say... "copious" comments (ZZZ) at various times as I browsed the source and made changes.
        Found that a basic assumption I was using in my optimizations (i.e. that the game processed action_flags
        at a constant rate) was wrong, which made the Bungie way make a lot more sense.  I now recommend using *none*
        of the three NETWORK_* options (do use NETWORK_IP though of course if appropriate).
        
Nov 13, 2001 (Woody Zenfell):
        Although things were basically OK under favorable conditions, they were IMO too "fragile" - sensitive
        to latency and jitter.  I couldn't help but try again... so NETWORK_ADAPTIVE_LATENCY_2 has been added.
        Also put in NETWORK_SMARTER_FLAG_DITCHING mechanism.

Feb 27, 2002 (Br'fin (Jeremy Parsons)):
	Rewired things to more generally key off of HAVE_SDL_NET than SDL (The Carbon build has SDL_NET, but
		understandably lacks SDL)
	Uses #if HAVE_SDL_NET in place of calls to #ifndef mac to allow SDL networking under Carbon

Mar 3-8, 2002 (Woody Zenfell):
    Changed net distribution stuff to use an STL map to associate distribution types with
    {lossy, handling procedure}.  Now different endstations can have different distribution
    types installed for handling (previously they had to all install the same handlers in the
    same order to get the same distribution type ID's).
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


#include "cseries.h"
#include "map.h"       // for TICKS_PER_SECOND and "struct entry_point"
#include "interface.h" // for transfering map
#include "mytm.h"	// ZZZ: both versions use mytm now
// for screen_printf()
#include "shell.h"

#if defined(SDL) || HAVE_SDL_NET
#include "sdl_network.h"
#include	"network_lookup_sdl.h"
#include	"SDL_thread.h"
#elif defined(mac)
#include "macintosh_network.h"
#endif

#include "game_errors.h"
#include "network_stream.h"
#include "progress.h"
#include "extensions.h"

// #define TEST_MODEM

#ifdef TEST_MODEM
#include "network_modem.h"
#include "network_modem_protocol.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <map>
#include "Logging.h"

#ifdef env68k
#pragma segment network
#endif

#define NO_PHYSICS

#ifdef DEBUG
//	#define DEBUG_NET
//#define DEBUG_NET_RECORD_PROFILE
#endif
// #define DEBUG_NET

// ZZZ: moved many struct definitions, constant #defines, etc. to header for (limited) sharing
#include "network_private.h"

// ZZZ: since network data format is now distinct from in-memory format.
// (quite similar, admittedly, in this first effort... ;) )
#include "network_data_formats.h"

// Optional features, disabled by default on Mac to preserve existing behavior (I hope ;) )
#if HAVE_SDL_NET
#undef	NETWORK_FAUX_QUEUE		// honest intentions, but perhaps ultimately useless.
#undef	NETWORK_ADAPTIVE_LATENCY	// should be better - oops, heh, or not.
#define	NETWORK_ADAPTIVE_LATENCY_2	// use this one instead; it should be good.
#define	NETWORK_IP			// needed if using IPaddress { host, port }; (as in SDL_net) rather than NetAddrBlock for addressing.
#undef	NETWORK_USE_RECENT_FLAGS	// if the game stalls, use flags at the end of the stall, not the more stale ones at the beginning
#define	NETWORK_SMARTER_FLAG_DITCHING	// this mechanism won't be quite as hasty to toss flags as Bungie's
#endif

// LP: kludge so I can get the code to compile
#if defined(mac) && !HAVE_SDL_NET
//#define NETWORK_IP // JTP: No no no, this defeats the whole purpose of NETWORK_IP
#undef NETWORK_IP
#endif

#ifdef DEBUG_NET_RECORD_PROFILE
void record_profile(int raf);
#endif

/* ---------- globals */

static short ddpSocket; /* our ddp socket number */

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

static short localPlayerIndex, localPlayerIdentifier;
static NetTopologyPtr topology;
static volatile NetStatusPtr status;
static char *network_adsp_packet;

// ZZZ note: very few folks touch the streaming data, so the data-format issues outlined above with
// datagrams (the data from which are passed around, interpreted, and touched by many functions)
// don't matter as much.  Do observe, though, that users of the "distribution" mechanism will have
// to pack and unpack their own distribution data - we can't be expected to know what they're doing.

// ZZZ note: read this externally with the NetState() function.
static short netState= netUninitialized;

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
const float	kNeedToIncreaseLatencyThreshhold = .1;
const float	kNeedToDecreaseLatencyThreshhold = -.95;

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


// ZZZ change: now using an STL 'map' to, well, _map_ distribution types to info records.
typedef map<int16, NetDistributionInfo> distribution_info_map_t;
static  distribution_info_map_t distribution_info_map;


#ifdef NETWORK_CHAT
static	NetChatMessage	incoming_chat_message_buffer;
static bool		new_incoming_chat_message = false;
#endif

/* ---------- private prototypes */
void NetPrintInfo(void);

// ZZZ: cmon, we're not fooling anyone... game_data is a game_info*; player_data is a player_info*
// Originally I guess the plan was to have a strong separation between Marathon game code and the networking code,
// such that they could be compiled independently and only know about each other at link-time, but I don't see any
// reason to try to keep that... and I suspect Jason abandoned this separation long ago anyway.
// For now, the only effect I see is a reduction in type-safety.  :)
static void NetInitializeTopology(void *game_data, short game_data_size, void *player_data, short player_data_size);
static void NetLocalAddrBlock(NetAddrBlock *address, short socketNumber);

// ZZZ: again, buffer is always going to be NetDistributionPacket_NET*
static void NetProcessLossyDistribution(void *buffer);
// ZZZ: this is used to handle an incoming ring packet; it'll always be NetPacket_NET*
static void NetProcessIncomingBuffer(void *buffer, long buffer_size, long sequence);

// ZZZ: this is the first place an incoming packet is seen by the networking subsystem.
// The job of the packetHandler is to demultiplex to NetProcessIncomingBuffer or to a
// distribution-processing function (currently, NetProcessLossyDistribution is the only one).
static void NetDDPPacketHandler(DDPPacketBufferPtr packet);

static long NetPacketSize(NetPacketPtr packet);
static void NetBuildRingPacket(DDPFramePtr frame, byte *data, short data_size, long sequence);
static void NetBuildFirstRingPacket(DDPFramePtr frame, long sequence);
static void NetRebuildRingPacket(DDPFramePtr frame, short tag, long sequence);
static void NetAddFlagsToPacket(NetPacketPtr packet);
	
static void NetSendRingPacket(DDPFramePtr frame);
static void NetSendAcknowledgement(DDPFramePtr frame, long sequence);

static bool NetCheckResendRingPacket(void);
static bool NetServerTask(void);
static bool NetQueueingTask(void);

#if defined(NETWORK_ADAPTIVE_LATENCY) || defined(NETWORK_ADAPTIVE_LATENCY_2)
// ZZZ addition, for adaptive latency business.  measurement is only used for adaptive_latency_2
static void update_adaptive_latency(int measurement = 0);
#endif//NETWORK_ADAPTIVE_LATENCY || NETWORK_ADAPTIVE_LATENCY_2

static int net_compare(void const *p1, void const *p2);

static void NetUpdateTopology(void);
static short NetAdjustUpringAddressUpwards(void);
static OSErr NetDistributeTopology(short tag);

static bool NetSetSelfSend(bool on);

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

bool NetEnter(
	void)
{
	OSErr error;
	bool success= true; /* optimism */

#ifdef TEST_MODEM
	success= ModemEnter();
#else
	
	assert(netState==netUninitialized);

	/* if this is the first time we’ve been called, add NetExit to the list of cleanup procedures */
	{
		static bool added_exit_procedure= false;
		
		if (!added_exit_procedure) atexit(NetExit);
		added_exit_procedure= true;
	}

#ifdef mac
	// Initialize SDL for Classic Mac here
	// Use a more elegant kind of fall-through than what's in the rest of the code.
	// The 900 means System 9 (Classic's version number)
	//
	// 900 = SDL not present
	// 901 = SDL not inited
	// 902 = SDL networking not inited
	//
	if (SDL_Init == NULL)
	{
		success = false;
		error = 900;
	}
	
	if (success)
	{
		if (SDL_Init(0) != 0)
		{
			success = false;
			error = 901;
		}
	}
	
	if (success)
	{
		if (SDLNet_Init() != 0)
		{
			success = false;
			error = 902;
			SDL_Quit();
		}
	}
	
	if (!success)
	{
		alert_user(infoError, strNETWORK_ERRORS, netErrCantContinue, error);
		return false;
	}
#endif

	error= NetDDPOpen();
	if (!error)
	{
		error= NetADSPOpen();
		if (!error)
		{
			topology = (NetTopologyPtr)malloc(sizeof(NetTopology));
			memset(topology, 0, sizeof(NetTopology));
			//topology= (NetTopologyPtr) NewPtrClear(sizeof(NetTopology));
			status = (NetStatusPtr)malloc(sizeof(NetStatus));
			memset(status, 0, sizeof(NetStatus));
			//status= (NetStatusPtr) NewPtrClear(sizeof(NetStatus));
			network_adsp_packet = (char *)malloc(MaxStreamPacketLength());
			memset(network_adsp_packet, 0, MaxStreamPacketLength());
			//network_adsp_packet= (char *) NewPtrClear(MaxStreamPacketLength());
			if (topology && status && network_adsp_packet)
			//error= MemError();
			//if(!error) 
			{
				status->buffer = (byte *)malloc(ddpMaxData);
				//status->buffer= (byte *) NewPtrClear(ddpMaxData);
				if (status->buffer)
				//error= MemError();
				//if(!error)
				{
					/* Set the server player identifier */
					NetSetServerIdentifier(0);
				
					ringFrame= NetDDPNewFrame();
					if (ringFrame)
					//error= MemError();
					//if(!error)
					{
						ackFrame= NetDDPNewFrame();
						if (ackFrame)
						//error= MemError();
						//if(!error)
						{
							distributionFrame= NetDDPNewFrame();
							if (distributionFrame)
							//error= MemError();
							//if (!error)
							{
								error= NetStreamEstablishConnectionEnd();
								if (error==noErr)
								{
									error= NetDDPOpenSocket(&ddpSocket, NetDDPPacketHandler);
									if (error==noErr)
									{
										status->oldSelfSendStatus= NetSetSelfSend(true);
										status->server_player_index= 0;
										status->single_player= false;
										netState= netDown;
#ifdef DEBUG_NET
										obj_clear(net_stats);
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
		success= false;
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

        // ZZZ: clean up SDL Time Manager emulation.  true says wait for any late finishers to finish
        // (but does NOT say to kill anyone not already removed.)
        myTMCleanup(true);

	if (netState!=netUninitialized)
	{
		error= NetCloseStreamConnection(false);
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
				free(topology);
				//DisposePtr((Ptr)topology);
				free(status->buffer);
				//DisposePtr((Ptr)status->buffer);
				free(status);
				//DisposePtr((Ptr)status);
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

#ifdef mac
	// Undo SDL setup in NetEnter;
	// watch out for whether SDL was weak-linked
	{
		if (SDL_Init != NULL)
		{
			SDLNet_Quit();
			SDL_Quit();
		}
	}
#endif
}

/* Add a function for a distribution type. returns the type, or NONE if it can't be
 * installed. It's safe to call this function multiple times for the same proc. */
// ZZZ: changed to take in the desired type, so a given machine can handle perhaps only some
// of the distribution types.
void
NetAddDistributionFunction(int16 inDataTypeID, NetDistributionProc inProc, bool inLossy) {
    // We don't support lossless distribution yet.
    assert(inLossy);

    // Prepare a NetDistributionInfo with the desired data.
    NetDistributionInfo theInfo;
    theInfo.lossy               = inLossy;
    theInfo.distribution_proc   = inProc;

    // Insert or update a map entry
    distribution_info_map[inDataTypeID] = theInfo;
}


/* Remove a distribution proc that has been installed. */
void
NetRemoveDistributionFunction(int16 inDataTypeID) {
    distribution_info_map.erase(inDataTypeID);
}


/* Distribute information to the whole net. */
void NetDistributeInformation(
	short type, 
	void *buffer, 
	short buffer_size, 
	bool send_to_self)
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
    distribution_info_map_t::const_iterator    theEntry = distribution_info_map.find(type);

    assert(theEntry != distribution_info_map.end());
	
	if (send_to_self)
	{
		theEntry->second.distribution_proc(buffer, buffer_size, localPlayerIndex);
	}

	distributionFrame->data_size = sizeof(NetPacketHeader_NET) + sizeof(NetDistributionPacket_NET) + buffer_size;
	{
		NetPacketHeader_NET*	header_NET	= (NetPacketHeader_NET*) distributionFrame->data;
                NetPacketHeader		header_storage;
                NetPacketHeader*	header		= &header_storage;
		
		header->tag = theEntry->second.lossy ? tagLOSSY_DISTRIBUTION : tagLOSSLESS_DISTRIBUTION;
		header->sequence = 0;
                
                netcpy(header_NET, header);
	}
	distribution_header.distribution_type = type;
	distribution_header.first_player_index = localPlayerIndex;
	distribution_header.data_size = buffer_size;
        
        // Probably could just netcpy this straight into distributionFrame->data.
        netcpy(&distribution_header_NET, &distribution_header);
        
	memcpy(distributionFrame->data + sizeof(NetPacketHeader_NET), &distribution_header_NET, sizeof(NetDistributionPacket_NET));
	//BlockMove(&distribution_header, distributionFrame->data+sizeof(NetPacketHeader), sizeof(NetDistributionPacket));
	memcpy(distributionFrame->data + sizeof(NetPacketHeader_NET) + sizeof(NetDistributionPacket_NET) /*- 2*sizeof(byte)*/, buffer, buffer_size);
	//BlockMove(buffer, 
	//	distributionFrame->data + (sizeof(NetPacketHeader) + sizeof(NetDistributionPacket) - (2*sizeof(byte))), 
	//	buffer_size);
	
	// LP: kludge to get it to compile
#ifdef NETWORK_IP
	NetDDPSendFrame(distributionFrame, &status->upringAddress, kPROTOCOL_TYPE, ddpSocket);
#endif
#endif // TEST_MODEM
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

bool NetGather(
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
	
	return true;
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
}

bool NetStart(
	void)
{
	OSErr error;
	bool success;

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
		success= false;
	} else {
		success= true;
	}
#endif

	return success;
}

static int net_compare(
	void const *p1, 
	void const *p2)
{
#ifndef NETWORK_IP
	return 0;
#else
	uint32 p1_host = ((const NetPlayer *)p1)->ddpAddress.host;
	uint32 p2_host = ((const NetPlayer *)p2)->ddpAddress.host;
	return p2_host - p1_host;
#endif
}

/*
-----------
NetGameJoin
-----------

	---> player name (to register)
	---> player type (to register)
	---> player data (no larger than MAXIMUM_PLAYER_DATA_SIZE)
	---> size of player data
	---> version number of network protocol (used with player type to construct entity name)
	---> ZZZ: SDL version only: SSLP hinting address, passed along to NetRegisterName
	
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

bool NetGameJoin(
	unsigned char *player_name,
	unsigned char *player_type,
	void *player_data,
	short player_data_size,
	short version_number
#if HAVE_SDL_NET
	, const char* hint_addr_string
#endif
	)
{
	OSErr error;
	bool success= false;

#ifdef TEST_MODEM
	success= ModemGameJoin(player_name, player_type, player_data, player_data_size, version_number);
#else
	/* initialize default topology (no game data) */
	NetInitializeTopology((void *) NULL, 0, player_data, player_data_size);
	
	/* register our downring socket with the net so gather dialogs can find us */
	error= NetRegisterName(player_name, player_type, version_number, 
		NetGetStreamSocketNumber()
#if HAVE_SDL_NET
		, hint_addr_string
#endif
		);
	
	if (error==noErr)
	{
		error= NetStreamWaitForConnection();
		if (error==noErr)
		{
			/* we’re registered and awaiting a connection request */
			netState= netJoining;
			success= true;
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
		error= NetCloseStreamConnection(true); /* this should stop the ocPassive OpenConnection */
		if (error==noErr)
		{
			/* our name has been unregistered and our connection end has been closed */
		}
	}
#endif	
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

bool NetSync(
	void)
{
	uint32 ticks;
	bool success= true;
#ifdef TEST_MODEM
	return ModemSync();
#else

	status->action_flags_per_packet= initial_updates_per_packet;
	status->update_latency= initial_update_latency;
	status->lastValidRingSequence= 0;
	status->ringPacketCount= 0;
//	status->server_player_index= 0;
	status->last_extra_flags= 0;
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

	netState= netStartingUp;
	
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
	while (success && netState != netActive) // packet handler changes this variable.
	{
		if (machine_tick_count() - ticks > NET_SYNC_TIME_OUT)
		{
			alert_user(infoError, strNETWORK_ERRORS, netErrSyncFailed, 0);

			/* How did Alain do this? */
			status->acceptPackets= false;
			status->acceptRingPackets= false;
			netState= netDown;
			success= false;
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
bool NetUnSync(
	void)
{
	bool success= true;
	uint32 ticks;

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
	netState= netDown;

#ifdef DEBUG_NET
	fdprintf("Flags processed: %d Time: %d;g", net_stats.action_flags_processed, TickCount()-ticks);
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

bool NetNumberOfPlayerIsValid(
	void)
{
	bool valid;

#ifdef TEST_MODEM
	valid= ModemNumberOfPlayerIsValid();
#else
	switch(netState)
	{
		case netUninitialized:
		case netJoining:
			valid= false;
			break;
		default:
			valid= true;
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
	
	<--- true if the entity is not in the game, false otherwise

used to filter entities which have been added to a game out of the lookup list
*/

/* if the given address is already added to our game, filter it out of the gather dialog */
bool NetEntityNotInGame(
	NetEntityName *entity,
	NetAddrBlock *address)
{
	short player_index;
	bool valid= true;
	
	(void) (entity);
	
	for (player_index=0;player_index<topology->player_count;++player_index)
	{
		NetAddrBlock *player_address= &topology->players[player_index].dspAddress;
		
#ifndef NETWORK_IP
#ifdef CLASSIC_MAC_NETWORKING
		if (address->aNode==player_address->aNode && address->aSocket==player_address->aSocket &&
			address->aNet==player_address->aNet)
#endif
#else
		if (address->host == player_address->host && address->port == player_address->port)
#endif
		{
			valid= false;
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
#ifndef NETWORK_IP
#ifdef CLASSIC_MAC_NETWORKING
					if (/*packet->sourceAddress.aNet == status->upringAddress.aNet && */
						packet->sourceAddress.aNode == status->upringAddress.aNode &&
						packet->sourceAddress.aSocket == status->upringAddress.aSocket)
#endif
#else
					if (packet->sourceAddress.host == status->upringAddress.host &&
					    packet->sourceAddress.port == status->upringAddress.port)
#endif
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
#ifndef NETWORK_IP
#ifdef CLASSIC_MAC_NETWORKING
					status->downringAddress.aNet= packet->sourceAddress.aNet;
					status->downringAddress.aNode= packet->sourceAddress.aNode;
					status->downringAddress.aSocket= packet->sourceAddress.aSocket;
#endif
#else
					status->downringAddress = packet->sourceAddress;
#endif

#ifdef DEBUG_NET
					net_stats.change_ring_packet_count++;
#endif
//					fdprintf("got change ring packet %d;g", header->sequence);

					/* fall through to tagRING_PACKET */
				
				case tagRING_PACKET:
					if(status->acceptRingPackets)
					{
#ifndef NETWORK_IP
#ifdef CLASSIC_MAC_NETWORKING
						if (/* packet->sourceAddress.aNet == status->downringAddress.aNet && */
							packet->sourceAddress.aNode == status->downringAddress.aNode &&
							packet->sourceAddress.aSocket == status->downringAddress.aSocket)
#else
						if (0)	// LP: kludge to get it to compile
#endif
#else
// LP: kludge to get it to compile
						if (packet->sourceAddress.host == status->downringAddress.host &&
						    packet->sourceAddress.port == status->downringAddress.port)
#endif
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
	#endif
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
        
//	packet_data = (NetDistributionPacketPtr)buffer;
	type = packet_data->distribution_type;

        // Act upon the data, if possible
    // ZZZ note: we use find(type) and an iterator instead of distribution_info_map[type] because
    // using operator [] on the map inserts an entry (even if it's not being used as an Lvalue).
    distribution_info_map_t::const_iterator    theEntry = distribution_info_map.find(type);

	if (theEntry != distribution_info_map.end())
	{
//		distribution_info[type].distribution_proc(&packet_data->data[0], packet_data->data_size, packet_data->first_player_index);
		theEntry->second.distribution_proc(((char*) buffer) + sizeof(NetDistributionPacket_NET), packet_data->data_size,
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
                //BlockMove(buffer, distributionFrame->data+sizeof(NetPacketHeader), sizeof(NetDistributionPacket) + packet_data->data_size);
                // LP: NetAddrBlock is the trouble here
                #ifdef NETWORK_IP
                NetDDPSendFrame(distributionFrame, &status->upringAddress, kPROTOCOL_TYPE, ddpSocket);
                #endif
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
	long buffer_size,
	long sequence)
{
        // ZZZ: convert from _NET format
        NetPacket*	packet_data		= (NetPacket*) (unpackedReceiveBuffer + sizeof(NetPacketHeader));
        NetPacket_NET*	packet_data_NET		= (NetPacket_NET*) buffer;
        
        netcpy(packet_data, packet_data_NET);
        
#ifdef DEBUG_NET_RECORD_PROFILE
        record_profile(packet_data->required_action_flags);
#endif

#ifdef NETWORK_ADAPTIVE_LATENCY_2
        // ZZZ: this is the only spot we sample/adjust our adaptive_latency_2: when we've received a valid ring packet.
        // We sample the server's required_action_flags (set to its SizeofLocalQueue before sent) as that should be a
        // good indicator of actual ring latency.
        update_adaptive_latency(packet_data->required_action_flags);
#endif
        
        // ZZZ: copy (byte-swapped) the action_flags into the unpacked buffer.
        netcpy(&packet_data->action_flags[0], (uint32*) (((char*) buffer) + sizeof(NetPacket_NET)), NetPacketSize(packet_data));
        
//	NetPacketPtr packet_data;
	short packet_tag= NONE;
	long previous_lastValidRingSequence= status->lastValidRingSequence;

//	assert(buffer_size == NetPacketSize(buffer));

//	packet_data= (NetPacketPtr) buffer;
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
			break;

		case typeUNSYNC_RING_PACKET:
			/* We sent this out to end the game, and now it has made it back to us. */
			/* This means that we are ready to exit. */
			if(netState==netComingDown)
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
			if(netState==netComingDown)
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
	short player_index, extra_flags, flags_to_remove, action_flag_index;
	static bool already_here = false;
	
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
		//	count * sizeof(long));
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
	flags_to_remove= MIN(extra_flags, status->last_extra_flags);
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
	local_player->net_dead= false;

	if(NetGetTransportType()!=kModemTransportType)
	{
		short adsp_socket_number= NetGetStreamSocketNumber();

		NetLocalAddrBlock(&local_player->dspAddress, adsp_socket_number);
		NetLocalAddrBlock(&local_player->ddpAddress, ddpSocket);
	}
	memcpy(local_player->player_data, player_data, player_data_size);
	
	/* initialize the network topology (assume we’re the only player) */
	topology->player_count= 1;
	topology->nextIdentifier= 1;
	memcpy(topology->game_data, game_data, game_data_size);
}

static void NetLocalAddrBlock(
	NetAddrBlock *address,
	short socketNumber)
{
	
#ifndef NETWORK_IP
#ifdef CLASSIC_MAC_NETWORKING
	short node, network;

	GetNodeAddress(&node, &network);
	
	address->aSocket= socketNumber;
	address->aNode= node;
	address->aNet= network;
#endif
#else
	address->host = 0x7f000001;	//!! XXX (ZZZ) yeah, that's really bad.
	address->port = socketNumber;	// OTOH, I guess others are set up to "stuff" the address they actually saw for us instead of
#endif					// this, anyway... right??  So maybe it's not that big a deal.......
}

static long NetPacketSize(
	NetPacketPtr  packet)
{
        // ZZZ: "register"... how quaint... I wonder if the compiler they used was really not smart enough on its own?
        // Welp, doesn't hurt to give hints anyway, we'll leave it.  :)
	register long   size = 0;
	register short  i;
        
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
			size += packet->action_flag_count[i] * sizeof(long);
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
	long sequence)
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
	#ifdef NETWORK_IP
	NetDDPSendFrame(frame, &status->downringAddress, kPROTOCOL_TYPE, ddpSocket);
	#endif
}

/* Only the server can call this... */
static void NetBuildFirstRingPacket(
	DDPFramePtr frame,
	long sequence)
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
	short data_size,
	long sequence)
{
        NetPacketHeader		header_storage;
        NetPacketHeader*	header		= &header_storage;
	NetPacketHeader_NET*	header_NET	= (NetPacketHeader_NET*) frame->data;

	/* build the ring packet */
        // ZZZ: note that data_size is now just the size of the variable-length part (i.e. the action_flags)
        // so we will add the sizeof both _NET format structures first.
	frame->data_size= sizeof(NetPacketHeader_NET) + sizeof(NetPacket_NET) + data_size;

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
	long sequence)
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
	#if HAVE_SDL_NET
	NetDDPSendFrame(frame, &status->upringAddress, kPROTOCOL_TYPE, ddpSocket);
	#endif
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
	bool reinstall= (netState != netDown);

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
			#ifdef NETWORK_IP
			NetDDPSendFrame(ringFrame, &status->upringAddress, kPROTOCOL_TYPE, ddpSocket);
			#endif
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
	bool reinstall= (netState != netDown);

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
				if(netState==netComingDown)
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
	
	return reinstall;
} // NetServerTask

static bool NetQueueingTask(
	void)
{
	bool reinstall= (netState != netDown);
	
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
	
	return reinstall;
} // NetQueueingTask

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
	if (localPlayerIndex==topology->player_count) fdprintf("couldn’t find my identifier: %p", topology);
#endif
	
	/* recalculate downringAddress */				
	previousPlayerIndex= localPlayerIndex ? localPlayerIndex-1 : topology->player_count-1;
	status->downringAddress= topology->players[previousPlayerIndex].ddpAddress;
	
	/* recalculate upringAddress */
	nextPlayerIndex= localPlayerIndex==topology->player_count-1 ? 0 : localPlayerIndex+1;
	status->upringAddress= topology->players[nextPlayerIndex].ddpAddress;
	status->upringPlayerIndex = nextPlayerIndex;
}


#if defined(NETWORK_ADAPTIVE_LATENCY) || defined(NETWORK_ADAPTIVE_LATENCY_2)
// ZZZ addition: adaptive latency business.  measurement used only in adaptive_latency_2.
static void
update_adaptive_latency(int measurement) {

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
#ifndef NETWORK_IP
#ifdef CLASSIC_MAC_NETWORKING
		if (address->aNet == status->upringAddress.aNet 
			&& address->aNode == status->upringAddress.aNode
			&& address->aSocket == status->upringAddress.aSocket)
#endif
#else
		if (address->host == status->upringAddress.host &&
		    address->port == status->upringAddress.port)
#endif
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

static bool NetSetSelfSend(
	bool on)
{
#ifndef NETWORK_IP
	OSErr          err;
	MPPParamBlock  pb;
	
	pb.SETSELF.newSelfFlag = on;
	err = PSetSelfSend(&pb, false);
	assert(err == noErr);
	return pb.SETSELF.oldSelfFlag;
#else
	return false;
#endif
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

        short		data_size		= NetPacketSize(packet_data);

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

/* ------ this needs to let the gatherer keep going if there was an error.. */
/* ••• Marathon Specific Code ••• */
/* Returns error code.. */
bool NetChangeMap(
	struct entry_point *entry)
{
	byte   *wad= NULL;
	long   length;
	OSErr  error= noErr;
	bool success= true;

#ifdef TEST_MODEM
	success= ModemChangeMap(entry);
#else

	/* If the guy that was the server died, and we are trying to change levels, we lose */
	if(localPlayerIndex==status->server_player_index && localPlayerIndex != 0)
	{
#ifdef DEBUG_NET
		fdprintf("Server died, and trying to get another level. You lose;g");
#endif
		success= false;
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
				if(error) success= false;
				set_game_error(systemError, error);
			} else {
//				if (!wad) alert_user(fatalError, strERRORS, badReadMap, -1);
				assert(error_pending());
			}
		} 
		else // wait for de damn map.
		{
			wad= NetReceiveGameData();
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
	uint32 initial_ticks= machine_tick_count();
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
			NetCloseStreamConnection(false);
		}
		
		if (error)
		{
			alert_user(infoError, strNETWORK_ERRORS, netErrCouldntDistribute, error);
		} 
		else if  (machine_tick_count()-initial_ticks>uint32(topology->player_count*MAP_TRANSFER_TIME_OUT))
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
	long map_length;
	uint32 ticks;
	OSErr error;
	bool timed_out= false;

	open_progress_dialog(_awaiting_map);

	// wait for our connection to start up. server will contact us.
	ticks= machine_tick_count();
	while (!NetStreamCheckConnectionStatus() && !timed_out)
	{
		if((machine_tick_count()-ticks)>MAP_TRANSFER_TIME_OUT)  timed_out= true;
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
		NetCloseStreamConnection(false);

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
#ifdef mac
				alert_user(infoError, strERRORS, outOfMemory, MemError());
#else
				alert_user(infoError, strERRORS, outOfMemory, -1);
#endif
			}
	
			if(map_buffer)
			{
				delete []map_buffer;
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
    // ZZZ: modified so as not to introduce ANY gratuitous latency.  May make play a little choppy.
    // Consider falling back to localNetTime - action_flags_per_packet...
    // (later) Took that back out.  It did make play nicely responsive, but opened us up wide to latency and jitter.
    // I hope adaptive_latency_2 will be the final meddling with this stuff.
//    return status->localNetTime;
#ifdef NETWORK_ADAPTIVE_LATENCY_2
    // This is it - this is the only place sCurrentAdaptiveLatency has any effect in adaptive_latency_2.
    // The effect is to get the game engine to drain the player_queues more smoothly than they would
    // if we returned status_localNetTime.
    return status->localNetTime - sCurrentAdaptiveLatency;
#else
	return status->localNetTime - 2*status->action_flags_per_packet - status->update_latency;
#endif
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

	// first we'll get the map length
	error= NetReceiveStreamPacket(&packet_type, network_adsp_packet);

	if (!error && packet_type==_stream_size_packet)	
	{
                // ZZZ: byte-swap if necessary
                long length_NET = *((long*) network_adsp_packet);

#if HAVE_SDL_NET
                *length = SDL_SwapBE32(length_NET);
#else
                *length = length_NET;
#endif                

		if(*length)
		{
			buffer = new byte[*length]; // because my caller expects it to be portable.
			
			if (buffer)
			{
				long offset;
			
				// we transfer the map in chunks, since ADSP can only transfer 64K at a time.
				for (offset = 0; !error && offset < *length; offset += STREAM_TRANSFER_CHUNK_SIZE)
				{
					uint16 expected_count;
										
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
} // receive_stream_data

static OSErr send_stream_data(
	void *data,
	long length)
{
	OSErr error;

	// transfer the length of the level.
        // ZZZ: byte-swap if necessary
        long	length_NET;

#if HAVE_SDL_NET
        length_NET = SDL_SwapBE32(length);
#else
        length_NET = length;
#endif

	error= NetSendStreamPacket(_stream_size_packet, &length_NET);

	if(!error)
	{
		long offset, length_written= 0;
	
		// ready or not, here it comes, in smaller chunks
		for (offset = 0; !error && offset < length; offset += STREAM_TRANSFER_CHUNK_SIZE)
		{
			uint16 adsp_count;
			
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
} // send_stream_data

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
	long buffer_size,
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
			if(netState==netComingDown)
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
	
//			fdprintf("will stuff %d flags", packet_data->required_action_flags);				
			for (index= 0; index<packet_data->required_action_flags; index++)
			{
				uint32 flag= (uint32)NET_DEAD_ACTION_FLAG;

				topology->players[player_index].net_dead= true;
				
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
	if(error) fdprintf("Err:%d", error);
	error= FSpOpenDF(&file, fsWrPerm, &stream_refnum);
	if(error || stream_refnum==NONE) fdprintf("Open Err:%d", error);
	
	action_flag_buffer= new recorded_flag[MAXIMUM_STREAM_FLAGS];
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
	if(error) fdprintf("Error: %d", error);

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
				if(error) fdprintf("Error: %d", error);
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
		
		delete []action_flag_buffer;
		action_flag_buffer= NULL;
	}
}
#endif
#endif


#ifdef NETWORK_CHAT
// Core logic brazenly ripped off NetDistributeTopology :)
OSErr NetDistributeChatMessage(
	short sender_identifier, const char* message)
{
	OSErr error = 0;
	short playerIndex;
	
	assert(netState==netGathering);
	
        NetChatMessage		theChatMessage;
        NetChatMessage_NET	theChatMessage_NET;
        
        theChatMessage.sender_identifier = sender_identifier;
        strncpy(theChatMessage.text, message, CHAT_MESSAGE_TEXT_BUFFER_SIZE);
        
        netcpy(&theChatMessage_NET, &theChatMessage);

	for (playerIndex=1; playerIndex<topology->player_count; ++playerIndex)
	{
		error= NetOpenStreamToPlayer(playerIndex);
		if (!error)
		{
			error= NetSendStreamPacket(_chat_packet, &theChatMessage_NET);
			if (!error)
			{
				error= NetCloseStreamConnection(false);
				if (!error)
				{
					/* successfully distributed topology to this player */
				}
			}
			else
			{
				NetCloseStreamConnection(true);
			}
		}
	}
	
	return error;
}

bool
NetGetMostRecentChatMessage(player_info** outPlayerData, char** outMessage) {
    if(new_incoming_chat_message) {
        player_info* thePlayer = NULL;
        new_incoming_chat_message = false;
        
        for(int i = 0; i < NetGetNumberOfPlayers(); i++)
            if(NetGetPlayerIdentifier(i) == incoming_chat_message_buffer.sender_identifier)
                thePlayer = (player_info*) NetGetPlayerData(i);

        if(thePlayer != NULL) {
            *outPlayerData = thePlayer;
            *outMessage = incoming_chat_message_buffer.text;
            return true;
        }
    }

    return false;
}
#endif // NETWORK_CHAT


/* check for messages from gather nodes; returns new state */
short NetUpdateJoinState(
	void)
{
        logContext("updating network join status");

	OSErr error;
	short newState= netState;
	short packet_type;

#ifdef TEST_MODEM
	newState= ModemUpdateJoinState();
#else
	switch (netState)
	{
		case netJoining:	// waiting to be gathered
			if (NetStreamCheckConnectionStatus())
			{
				error= NetReceiveStreamPacket(&packet_type, network_adsp_packet);

                                logTrace1("NetReceiveStreamPacket returned %d", error);

				if(!error && packet_type==_join_player_packet)
				{
					/* NOTE THESE ARE SHARED! */

                                        gather_player_data_NET* gathering_data_NET = (gather_player_data_NET*) network_adsp_packet;
                                        gather_player_data	gathering_data_storage;
                                        gather_player_data*	gathering_data = &gathering_data_storage;
                                        netcpy(gathering_data, gathering_data_NET);
                                        
                                        accept_gather_data	new_player_data;
                                        accept_gather_data_NET	new_player_data_NET;

//					struct gather_player_data *gathering_data= (struct gather_player_data *) network_adsp_packet;
//					struct accept_gather_data new_player_data;

					/* Note that we could set accepted to false if we wanted to for some */
					/*  reason- such as bad serial numbers.... */

                                        /* Unregister ourselves */
                                        error= NetUnRegisterName();

                                        logTrace1("NetUnRegisterName returned %d", error);

                                        assert(!error);
                                
                                        /* Note that we share the buffers.. */
                                        localPlayerIdentifier= gathering_data->new_local_player_identifier;
                                        topology->players[localPlayerIndex].identifier= localPlayerIdentifier;
                                        topology->players[localPlayerIndex].net_dead= false;
                                        
                                        /* Confirm. */
                                        new_player_data.accepted= true;
                                        obj_copy(new_player_data.player, topology->players[localPlayerIndex]);

                                        netcpy(&new_player_data_NET, &new_player_data);
                                        error = NetSendStreamPacket(_accept_join_packet, &new_player_data_NET);
                                        
                                        logTrace1("NetSendStreamPacket returned %d", error);

//					error= NetSendStreamPacket(_accept_join_packet, &new_player_data);

					if(!error)
					{
						/* Close and reset the connection */
						error= NetCloseStreamConnection(false);
                                                
                                                logTrace1("NetCloseStreamConnection returned %d", error);

						if (!error)
						{
							error= NetStreamWaitForConnection();
                                                        
                                                        logTrace1("NetStreamWaitForConnection returned %d", error);
                                                        
							if (!error)
							{
								/* start waiting for another connection */
//fdprintf("Accepted: %d", new_player_data.accepted);
								if(new_player_data.accepted) newState= netWaiting;
							}
						}
					}
				} 

				if (error != noErr)
				{
                                        logAnomaly1("error != noErr; error == %d", error);
                                
					newState= netJoinErrorOccurred;
					NetCloseStreamConnection(false);
					alert_user(infoError, strNETWORK_ERRORS, netErrJoinFailed, error);
				}
			}
			break;
                    // netJoining
		
		case netWaiting:	// have been gathered, waiting for other players / game start
			if (NetStreamCheckConnectionStatus())
			{
				/* and now, the packet you’ve all been waiting for ... (the server is trying to
					hook us up with the network topology) */
				error= NetReceiveStreamPacket(&packet_type, network_adsp_packet);
				if(!error)
				{	// ZZZ change to accept more kinds of packets here
                                    switch(packet_type) {
                                    
                                    case _topology_packet:

                                        netcpy(topology, (NetTopology_NET*)network_adsp_packet);

					/* Copy it in */
//					memcpy(topology, network_adsp_packet, sizeof(NetTopology));

					if(NetGetTransportType()==kNetworkTransportType)
					{
						NetAddrBlock address;
						
						// LP: NetAddrBlock is the trouble here
						#ifdef NETWORK_IP
						NetGetStreamAddress(&address);
						#endif
						
						/* for ARA, make stuff in an address we know is correct (don’t believe the server) */
						topology->players[0].dspAddress= address;
#ifndef NETWORK_IP
#ifdef CLASSIC_MAC_NETWORKING
						topology->players[0].ddpAddress.aNet= address.aNet;
						topology->players[0].ddpAddress.aNode= address.aNode;
#endif
#else
						topology->players[0].ddpAddress.host = address.host;
#endif
//						fdprintf("ddp %8x, dsp %8x;g;", *((long*)&topology->players[0].ddpAddress),
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
				
					error= NetCloseStreamConnection(false);
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
                                    break;
                                    // _topology_packet

#ifdef NETWORK_CHAT	// ZZZ addition
                                    case _chat_packet:
                                        netcpy(&incoming_chat_message_buffer, (NetChatMessage_NET*) network_adsp_packet);
                                        
					error= NetCloseStreamConnection(false);
					if (!error)
					{
						error= NetStreamWaitForConnection();
						if (!error)
						{
							/* successfully got a chat message from the server and closed
								the connection */
                                                        newState = netChatMessageReceived;
                                                        new_incoming_chat_message = true;
						}
					}
					
                                    break;
                                    // _chat_packet
#endif // NETWORK_CHAT

                                    default:	// unrecognized stream packet type
                                    // shouldn't we at least, like, close the connection and stuff?  Bungie's code didn't...
                                    break;
                                    
                                    } // switch(packet_type)
				}
				else	// error
				{
//					if(error) 
//					{
						newState= netJoinErrorOccurred;
						alert_user(infoError, strNETWORK_ERRORS, netErrJoinFailed, error);
//					}
				}
			}
			break;
                    // netWaiting
                    
		default:
			newState= NONE;
			break;
	}
	
	/* return netPlayerAdded to tell the caller to refresh his topology, but don’t change netState to that */
#ifdef NETWORK_CHAT
        // ZZZ: similar behavior for netChatMessageReceived
        if(newState != netChatMessageReceived)
#endif

	if (newState!=netPlayerAdded && newState != NONE) netState= newState;
#endif // !TEST_MODEM

	return newState;
}



bool NetGatherPlayer(
#if !HAVE_SDL_NET
        /* player_index in our lookup list */
	short player_index,
#else
        // ZZZ: in my formulation, the player information is passed along from the dialog here -
        // there's no mechanism to go back and ask for it later.
        const SSLP_ServiceInstance* player_instance,
#endif
	CheckPlayerProcPtr check_player)
{
	OSErr error;
	NetAddrBlock address;
	short packet_type;
	short stream_transport_type= NetGetTransportType();
	bool success= true;

#ifdef TEST_MODEM
	success= ModemGatherPlayer(player_index, check_player);
#else
	assert(netState==netGathering);
	assert(topology->player_count<MAXIMUM_NUMBER_OF_NETWORK_PLAYERS);
	
	/* Get the address from the dialog */
#if !HAVE_SDL_NET
	// LP: NetAddrBlock is the trouble here
	#ifndef mac
	NetLookupInformation(player_index, &address, NULL);
	#endif
#else
        // ZZZ: in my formulation, this info is passed along directly from the dialog
        // There's no "inquiry" function to go back and get it later.
        address.host = player_instance->sslps_address.host;
        address.port = player_instance->sslps_address.port;
#endif

	/* Note that the address will be garbage for modem, but that's okay.. */
	/* Force the address to be correct, so we can use our stream system.. */
	topology->players[topology->player_count].dspAddress= address;
#ifndef NETWORK_IP
#ifdef CLASSIC_MAC_NETWORKING
	topology->players[topology->player_count].ddpAddress.aNet= address.aNet;
	topology->players[topology->player_count].ddpAddress.aNode= address.aNode;
#endif
#else
	topology->players[topology->player_count].ddpAddress.host = address.host;
#endif
	error= NetOpenStreamToPlayer(topology->player_count);
	
	if (!error)
	{
		struct gather_player_data gather_data;
                gather_player_data_NET	gather_data_NET;

		/* Setup the gather data. */
		gather_data.new_local_player_identifier= topology->nextIdentifier;

                netcpy(&gather_data_NET, &gather_data);
                error = NetSendStreamPacket(_join_player_packet, &gather_data_NET);

//		error= NetSendStreamPacket(_join_player_packet, &gather_data);

		if(!error)
		{
			error= NetReceiveStreamPacket(&packet_type, network_adsp_packet);
			if(!error)
			{
				if(packet_type==_accept_join_packet)
				{
                                        accept_gather_data_NET* new_player_data_NET = (accept_gather_data_NET*) network_adsp_packet;
                                        accept_gather_data	new_player_data_storage;
                                        accept_gather_data*	new_player_data = &new_player_data_storage;
                                        netcpy(new_player_data, new_player_data_NET);

//					struct accept_gather_data *new_player_data= (struct accept_gather_data *) network_adsp_packet;

					if(new_player_data->accepted)
					{
						/* make sure everybody gets a unique identifier */
						topology->nextIdentifier+= 1;

						/* Copy in the player data */
						obj_copy(topology->players[topology->player_count], new_player_data->player);
						
						/* force in some addresses we know are correct */
						topology->players[topology->player_count].dspAddress= address;
#ifndef NETWORK_IP
#ifdef CLASSIC_MAC_NETWORKING
						topology->players[topology->player_count].ddpAddress.aNet= address.aNet;
						topology->players[topology->player_count].ddpAddress.aNode= address.aNode;
#endif
#else
						topology->players[topology->player_count].ddpAddress.host = address.host;
#endif
//						fdprintf("ddp %8x, dsp %8x;g;", *((long*)&topology->players[topology->player_count].ddpAddress),
//							*((long*)&topology->players[topology->player_count].dspAddress));
							
						error= NetCloseStreamConnection(false);
						if (!error)
						{
							/* closed connection successfully, remove this player from the list of players so
								we can’t even try to add him again */
							if(stream_transport_type!=kModemTransportType)
							{
// ZZZ: in my formulation, entry is removed from list instantly by widget when clicked
#if !HAVE_SDL_NET
								NetLookupRemove(player_index);
#endif
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
						NetCloseStreamConnection(false);
					}
				} 
				else
				{
					NetCloseStreamConnection(false);
				}
			}
			else
			{
				NetCloseStreamConnection(false);
			}
		}  
		else 
		{
			NetCloseStreamConnection(false);
		}
	}

	if(error)
	{
		alert_user(infoError, strNETWORK_ERRORS, netErrCantAddPlayer, error);
// ZZZ: in my formulation, entry is removed as soon as it's clicked, by the clicked widget.
#if !HAVE_SDL_NET
		NetLookupRemove(player_index); /* get this guy out of here, he didn’t respond */
#endif
		success= false;
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
	OSErr error = 0; //JTP: initialize to no error
	short playerIndex;
	
	assert(netState==netGathering);
	
	topology->tag= tag;
        
        NetTopology_NET	topology_NET_storage;
        NetTopology_NET* topology_NET = &topology_NET_storage;
        netcpy(topology_NET, topology);

//        NetTopology_NET* topology_NET = topology;

	for (playerIndex=1; playerIndex<topology->player_count; ++playerIndex)
	{
		error= NetOpenStreamToPlayer(playerIndex);
		if (!error)
		{
			error= NetSendStreamPacket(_topology_packet, topology_NET);
			if (!error)
			{
				error= NetCloseStreamConnection(false);
				if (!error)
				{
					/* successfully distributed topology to this player */
				}
			}
			else
			{
				NetCloseStreamConnection(true);
			}
		}
	}
	
	return error;
}

/* -------------------- application specific code */
uint16 MaxStreamPacketLength(
	void)
{
	short packet_type;
	uint16 max_length= 0;

	for(packet_type= 0; packet_type<NUMBER_OF_BUFFERED_STREAM_PACKET_TYPES; ++packet_type)
	{	
		uint16 length= NetStreamPacketLength(packet_type);
		if(length>max_length) max_length= length;
	}
	
	return max_length;
}

uint16 NetStreamPacketLength(
	short packet_type)
{
	uint16 length;

	switch(packet_type)
	{
		case _join_player_packet:
			length= sizeof(gather_player_data_NET);
			break;
			
		case _accept_join_packet:
			length= sizeof(accept_gather_data_NET);
			break;
			
		case _topology_packet:
			length= sizeof(NetTopology_NET);
			break;

		case _stream_size_packet:
			length= sizeof(long);
			break;
			
		case _stream_data_packet:
			length= STREAM_TRANSFER_CHUNK_SIZE;
			break;
                        
#ifdef NETWORK_CHAT
                case _chat_packet:
                        length = sizeof(NetChatMessage_NET);
                        break;
#endif
	
		default:
			length= 0;
			assert(false);
			break;
	}

	return length;
}

// LP: NetAddrBlock is the trouble here
#ifdef NETWORK_IP
NetAddrBlock *NetGetPlayerADSPAddress(
	short player_index)
{
	return &topology->players[player_index].dspAddress;
}
#endif


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
        net_profile[net_profile_index].timestamp	= SDL_GetTicks();
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
