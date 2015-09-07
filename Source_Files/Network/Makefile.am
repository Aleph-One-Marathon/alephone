## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = libnetwork.a

SUBDIRS = Metaserver

if MAKE_WINDOWS
NETWORK_MIC = network_microphone_sdl_win32.cpp
else
NETWORK_MIC = network_microphone_sdl_alsa.cpp
endif

libnetwork_a_SOURCES = ConnectPool.h network.h network_audio_shared.h network_capabilities.h \
  network_data_formats.h \
  network_dialog_widgets_sdl.h network_dialogs.h network_distribution_types.h \
  network_games.h network_microphone_shared.h network_lookup_sdl.h network_messages.h network_private.h \
  network_sound.h network_speaker_sdl.h network_speex.h network_star.h \
  NetworkGameProtocol.h RingGameProtocol.h SDL_netx.h \
  SSLP_API.h SSLP_Protocol.h StarGameProtocol.h Update.h \
  HTTP.h \
  \
  ConnectPool.cpp network.cpp network_capabilities.cpp network_data_formats.cpp \
  network_dialogs.cpp \
  network_dialog_widgets_sdl.cpp network_games.cpp \
  network_lookup_sdl.cpp network_messages.cpp $(NETWORK_MIC) \
  network_microphone_shared.cpp network_speex.cpp network_speaker_sdl.cpp \
  network_speaker_shared.cpp network_star_hub.cpp network_star_spoke.cpp \
  network_udp.cpp RingGameProtocol.cpp \
  SDL_netx.cpp SSLP_limited.cpp StarGameProtocol.cpp Update.cpp \
  HTTP.cpp

EXTRA_libnetwork_a_SOURCES = network_dummy.cpp network_microphone_sdl_win32.cpp network_microphone_coreaudio.cpp network_microphone_sdl_dummy.cpp

AM_CPPFLAGS = -I$(top_srcdir)/Source_Files/CSeries -I$(top_srcdir)/Source_Files/Files \
  -I$(top_srcdir)/Source_Files/GameWorld -I$(top_srcdir)/Source_Files/Input \
  -I$(top_srcdir)/Source_Files/LibNAT -I$(top_srcdir)/Source_Files/Network/Metaserver \
  -I$(top_srcdir)/Source_Files/Misc -I$(top_srcdir)/Source_Files/ModelView \
  -I$(top_srcdir)/Source_Files/RenderMain \
  -I$(top_srcdir)/Source_Files/RenderOther -I$(top_srcdir)/Source_Files/Sound \
  -I$(top_srcdir)/Source_Files/TCPMess -I$(top_srcdir)/Source_Files/XML \
  -I$(top_srcdir)/Source_Files/Lua -I$(top_srcdir)/Source_Files
