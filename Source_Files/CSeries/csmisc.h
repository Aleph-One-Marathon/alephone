#define MACINTOSH_TICKS_PER_SECOND 60
#define MACHINE_TICKS_PER_SECOND MACINTOSH_TICKS_PER_SECOND

extern unsigned long machine_tick_count(void);
extern boolean wait_for_click_or_keypress(
	unsigned long ticks);

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
extern void initialize_debugger(Boolean);
#endif

