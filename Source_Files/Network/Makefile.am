## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = libnetwork.a

libnetwork_a_SOURCES = network.h network_data_formats.h network_dialog_widgets_sdl.h \
  network_dialogs.h network_games.h network_lookup_sdl.h network_private.h \
  network_sound.h network_stream.h SDL_netx.h SSLP_API.h SSLP_Protocol.h \
  network.cpp network_data_formats.cpp network_dialogs.cpp network_dialogs_sdl.cpp \
  network_dialog_widgets_sdl.cpp network_games.cpp network_lookup_sdl.cpp \
  network_stream.cpp network_tcp.cpp network_udp.cpp SDL_netx.cpp \
  SSLP_limited.cpp

EXTRA_libnetwork_a_SOURCES = network_dummy.cpp

INCLUDES = -I$(top_srcdir)/Source_Files/CSeries \
  -I$(top_srcdir)/Source_Files/Misc -I$(top_srcdir)/Source_Files/ModelView
