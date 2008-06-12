/*

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

*/
/*
 *  csalerts_sdl.cpp - Game alerts and debugging support, SDL implementation
 *
 *  Written in 2000 by Christian Bauer

April 22, 2003 (Woody Zenfell):
        Now dumping alert text etc. with Logging as well
 */

#include "cseries.h"

#include <stdio.h>

#include "Logging.h"

#include "sdl_dialogs.h"
#include "sdl_widgets.h"

/*
 *  Display alert message
 */

const int MAX_ALERT_WIDTH = 320;

extern void update_game_window(void);

void alert_user(const char *message, short severity) 
{
  if (SDL_GetVideoSurface() == NULL) {
    fprintf(stderr, "%s: %s\n", severity == infoError ? "INFO" : "FATAL", message);
  } else {
    dialog d;
    vertical_placer *placer = new vertical_placer;
    placer->dual_add(new w_title(severity == infoError ? "WARNING" : "ERROR"), d);
    placer->add(new w_spacer, true);
    
    // Wrap lines
    uint16 style;
    font_info *font = get_theme_font(MESSAGE_WIDGET, style);
    char *t = strdup(message);
    char *p = t;

    while (strlen(t)) {
      unsigned i = 0, last = 0;
      int width = 0;
      while (i < strlen(t) && width < MAX_ALERT_WIDTH) {
	width = text_width(t, i, font, style);
	if (t[i] == ' ')
	  last = i;
	i++;
      }
      if (i != strlen(t))
	t[last] = 0;
      placer->dual_add(new w_static_text(t), d);
      if (i != strlen(t))
	t += last + 1;
      else
	t += i;
    }
    free(p);
    placer->add(new w_spacer, true);
    w_button *button = new w_button(severity == infoError ? "OK" : "QUIT", dialog_ok, &d);
    placer->dual_add (button, d);
    d.set_widget_placer(placer);

    d.activate_widget(button);

    d.run();
    if (severity == infoError && top_dialog == NULL)
      update_game_window();
  }
  if (severity != infoError) exit(1);
}

void alert_user(short severity, short resid, short item, OSErr error)
{
  char str[256];
  getcstr(str, resid, item);
  char msg[300];
  sprintf(msg, "%s (error %d)", str, error);
  if (severity == infoError) {
    logError2("alert (ID=%hd): %s", error, str);
  } else if (severity == fatalError) {
    logFatal2("fatal alert (ID=%hd): %s", error, str);
  }
  alert_user(msg, severity);
}

extern "C" void debugger(const char *message);

/*
 *  Jump into debugger (and return)
 */

void pause_debug(void)
{
        logNote("pause_debug called");
	fprintf(stderr, "pause\n");
}


/*
 *  Display message
 */

void vpause(const char *message)
{
        logWarning1("vpause: %s", message);
	fprintf(stderr, "vpause %s\n", message);
}


/*
 *  Jump into debugger (and don't return)
 */

void halt(void)
{
        logFatal("halt called");
	fprintf(stderr, "halt\n");
	abort();
}


/*
 *  Display message and halt
 */

extern void stop_recording();

void vhalt(const char *message)
{
	stop_recording();
        logFatal1("vhalt: %s", message);
	fprintf(stderr, "vhalt %s\n", message);
#if defined(__APPLE__) && defined(__MACH__)
	* (char *) 0x0 = 0;
#else
	abort();
#endif
}


/*
 *  Assertion failed
 */

static char assert_text[256];

void _alephone_assert(const char *file, long line, const char *what)
{
	vhalt(csprintf(assert_text, "%s:%ld: %s", file, line, what));
}

void _alephone_warn(const char *file, long line, const char *what)
{
	vpause(csprintf(assert_text, "%s:%ld: %s", file, line, what));
}
