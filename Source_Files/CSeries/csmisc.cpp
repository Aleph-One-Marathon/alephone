// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
#include <Events.h>

#include "cstypes.h"
#include "csmisc.h"

extern void initialize_debugger(bool);

unsigned long machine_tick_count(void)
{
	return TickCount();
}

bool wait_for_click_or_keypress(
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
	bool ignore)
{
	(void)ignore;
}

