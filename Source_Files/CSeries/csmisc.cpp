#include <Events.h>

#include "cstypes.h"
#include "csmisc.h"

extern void initialize_debugger(Boolean);

unsigned long machine_tick_count(void)
{
	return TickCount();
}

Boolean wait_for_click_or_keypress(
	unsigned long ticks)
{
	unsigned long end;
	EventRecord event;

	end=TickCount()+ticks;
	for (;;) {
		if (GetOSEvent(mDownMask|keyDownMask,&event))
			return true;
		if (event.when>=end)
			return false;
	}
}

void kill_screen_saver(void)
{
}

void initialize_debugger(
	Boolean ignore)
{
	(void)ignore;
}

