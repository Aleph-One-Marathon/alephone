/*
 *  shell.cpp - Main game loop and input handling
 */

#include "cseries.h"

#include "map.h"
#include "monsters.h"
#include "player.h"
#include "render.h"
#include "shell.h"
#include "interface.h"
#include "mysound.h"
#include "fades.h"
#include "screen.h"
#include "music.h"
#include "images.h"
#include "vbl.h"
#include "preferences.h"
#include "tags.h" /* for scenario file type.. */
#include "network_sound.h"
#include "mouse.h"
#include "screen_drawing.h"
#include "computer_interface.h"
#include "game_wad.h" /* yuck... */
#include "game_window.h" /* for draw_interface() */
#include "extensions.h"
#include "items.h"
#include "interface_menus.h"
#include "weapons.h"

#include "Crosshairs.h"
#include "OGL_Render.h"
#include "XML_ParseTreeRoot.h"
#include "FileHandler.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// LP addition: whether or not the cheats are active
static bool CheatsActive = false;

// Prototypes
static void main_event_loop(void);
static int process_keyword_key(char key);
static void handle_keyword(int type_of_cheat);

// Include platform-specific files
#if defined(mac)
#include "shell_macintosh.cpp"
#elif defined(SDL)
#include "shell_sdl.cpp"
#endif

// LP: the rest of the code has been moved to Jeremy's shell_misc.file.
