## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = liba1lua.a

liba1lua_a_SOURCES = lua_script.h lua_script.cpp lua_map.h lua_map.cpp lua_mnemonics.h lua_monsters.h lua_monsters.cpp lua_objects.h lua_objects.cpp lua_player.h lua_player.cpp lua_projectiles.h lua_projectiles.cpp lua_saved_objects.h lua_saved_objects.cpp lua_templates.h lapi.c lapi.h lauxlib.c lauxlib.h lbaselib.c lbitlib.c lcode.c lcode.h lctype.h lctype.c ldblib.c ldebug.c ldebug.h ldo.c ldo.h ldump.c lfunc.c lfunc.h lgc.c lgc.h linit.c liolib.c llex.c llex.h lmathlib.c lmem.c lmem.h lobject.c lobject.h lopcodes.c lopcodes.h loslib.c lparser.c lparser.h lstate.c lstate.h lstring.c lstring.h lstrlib.c ltable.c ltable.h ltablib.c ltm.c ltm.h lundump.c lundump.h lvm.c lvm.h lzio.c lzio.h llimits.h lua.h lualib.h luaconf.h language_definition.h lua_serialize.h lua_serialize.cpp lua_hud_objects.h lua_hud_objects.cpp lua_hud_script.h lua_hud_script.cpp lua_ephemera.h lua_ephemera.cpp

EXTRA_DIST = COPYRIGHT README

AM_CPPFLAGS = -I$(top_srcdir)/Source_Files/CSeries -I$(top_srcdir)/Source_Files/Files \
  -I$(top_srcdir)/Source_Files/GameWorld -I$(top_srcdir)/Source_Files/Input \
  -I$(top_srcdir)/Source_Files/Misc -I$(top_srcdir)/Source_Files/ModelView \
  -I$(top_srcdir)/Source_Files/Network \
  -I$(top_srcdir)/Source_Files/RenderMain -I$(top_srcdir)/Source_Files/RenderOther \
  -I$(top_srcdir)/Source_Files/Sound -I$(top_srcdir)/Source_Files/XML \
  -I$(top_srcdir)/Source_Files
