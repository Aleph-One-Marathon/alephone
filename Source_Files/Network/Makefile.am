## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = libnetwork.a

libnetwork_a_SOURCES = network.h network_data_formats.h \
  network_dialog_widgets_sdl.h network_dialogs.h network_distribution_types.h \
  network_games.h network_lookup_sdl.h network_microphone_sdl.h \
  network_modem.h network_private.h network_sound.h network_speaker_sdl.h \
  network_stream.h SDL_netx.h SSLP_API.h SSLP_Protocol.h \
  \
  network.cpp network_data_formats.cpp network_dialogs.cpp \
  network_dialogs_sdl.cpp network_dialog_widgets_sdl.cpp network_games.cpp \
  network_lookup_sdl.cpp network_microphone_sdl_dummy.cpp \
  network_speaker_sdl.cpp network_stream.cpp network_tcp.cpp network_udp.cpp \
  SDL_netx.cpp SSLP_limited.cpp

EXTRA_libnetwork_a_SOURCES = network_dummy.cpp network_microphone_sdl_win32.cpp

INCLUDES = -I$(top_srcdir)/Source_Files/CSeries -I$(top_srcdir)/Source_Files/Files \
  -I$(top_srcdir)/Source_Files/GameWorld -I$(top_srcdir)/Source_Files/Input \
  -I$(top_srcdir)/Source_Files/Misc -I$(top_srcdir)/Source_Files/ModelView \
  -I$(top_srcdir)/Source_Files/Pfhortran -I$(top_srcdir)/Source_Files/RenderMain \
  -I$(top_srcdir)/Source_Files/RenderOther -I$(top_srcdir)/Source_Files/Sound \
  -I$(top_srcdir)/Source_Files/XML -I$(top_srcdir)/Source_Files
