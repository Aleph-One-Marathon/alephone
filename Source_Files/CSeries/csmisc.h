// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
#ifndef _CSERIES_MISC_
#define _CSERIES_MISC_

#ifdef mac
#define MACINTOSH_TICKS_PER_SECOND 60
#define MACHINE_TICKS_PER_SECOND MACINTOSH_TICKS_PER_SECOND
#elif defined(SDL)
#define MACHINE_TICKS_PER_SECOND 1000
#else
#error MACHINE_TICKS_PER_SECOND not defined for this platform
#endif

extern uint32 machine_tick_count(void);
extern bool wait_for_click_or_keypress(
	uint32 ticks);

#ifdef env68k

#pragma parameter __D0 get_a0

extern long get_a0(void)
	= {0x2008};

#pragma parameter __D0 get_a5

extern long get_a5(void)
	= {0x200D};

#pragma parameter __D0 set_a5(__D0)

extern long set_a5(
	long a5)
	= {0xC18D};

#endif

extern void kill_screen_saver(void);

#ifdef DEBUG
extern void initialize_debugger(bool on);
#endif

#endif
