## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = libnetwork.a

if HAVE_SDL_NET
NET_SRCS = network.cpp network_tcp.cpp network_udp.cpp network_dialogs_sdl.cpp network_lookup_sdl.cpp network_stream.cpp
else
NET_SRCS = network_dummy.cpp
endif

libnetwork_a_SOURCES = network.h network_games.cpp network_games.h \
  network_sound.h network_stream.h $(NET_SRCS)

EXTRA_libnetwork_a_SOURCES = network_dummy.cpp network.cpp network_tcp.cpp \
  network_udp.cpp network_dialogs_sdl.cpp network_lookup_sdl.cpp network_stream.cpp

INCLUDES = -I$(top_srcdir)/Source_Files/CSeries \
  -I$(top_srcdir)/Source_Files/Misc -I$(top_srcdir)/Source_Files/ModelView
