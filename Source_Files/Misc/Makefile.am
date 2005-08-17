## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = libmisc.a

if MAKE_WINDOWS
THREAD_PRIORITY = thread_priority_sdl_win32.cpp
else
THREAD_PRIORITY = thread_priority_sdl_posix.cpp
endif

libmisc_a_SOURCES = ActionQueues.h alephversion.h CircularByteBuffer.h \
  CircularQueue.h Console.h game_errors.h \
  interface.h interface_menus.h key_definitions.h Logging.h Logging_gruntwork.h \
  PlayerImage_sdl.h \
  PlayerName.h preferences.h preferences_private.h preferences_sdl.h \
  preferences_widgets_sdl.h progress.h Random.h sdl_dialogs.h sdl_network.h \
  sdl_widgets.h shared_widgets.h thread_priority_sdl.h vbl_definitions.h vbl.h VecOps.h \
  WindowedNthElementFinder.h \
  \
  ActionQueues.cpp CircularByteBuffer.cpp Console.cpp game_errors.cpp \
  interface.cpp interface_sdl.cpp \
  Logging.cpp PlayerImage_sdl.cpp PlayerName.cpp preferences.cpp \
  preferences_widgets_sdl.cpp sdl_dialogs.cpp $(THREAD_PRIORITY) \
  sdl_widgets.cpp shared_widgets.cpp vbl.cpp vbl_sdl.cpp

EXTRA_libmisc_a_SOURCES = thread_priority_sdl_posix.cpp thread_priority_sdl_dummy.cpp thread_priority_sdl_win32.cpp

INCLUDES = -I$(top_srcdir)/Source_Files/CSeries -I$(top_srcdir)/Source_Files/Files \
  -I$(top_srcdir)/Source_Files/GameWorld -I$(top_srcdir)/Source_Files/Input \
  -I$(top_srcdir)/Source_Files/ModelView -I$(top_srcdir)/Source_Files/Network \
  -I$(top_srcdir)/Source_Files/Network/Metaserver \
  -I$(top_srcdir)/Source_Files/Pfhortran -I$(top_srcdir)/Source_Files/RenderMain \
  -I$(top_srcdir)/Source_Files/RenderOther -I$(top_srcdir)/Source_Files/Sound \
  -I$(top_srcdir)/Source_Files/TCPMess \
  -I$(top_srcdir)/Source_Files/XML -I$(top_srcdir)/Source_Files/Expat \
  -I$(top_srcdir)/Source_Files
