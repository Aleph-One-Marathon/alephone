## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = libnetwork.a

SUBDIRS = Metaserver StandaloneHub

libnetwork_a_SOURCES = NetworkInterface.h ConnectPool.h network.h network_capabilities.h \
  network_dialog_widgets_sdl.h network_dialogs.h network_games.h \
  network_messages.h network_private.h network_star.h NetworkGameProtocol.h	  \
  SSLP_API.h SSLP_Protocol.h StarGameProtocol.h \
  Update.h HTTP.h PortForward.h Pinger.h \
  \
  NetworkInterface.cpp ConnectPool.cpp network.cpp network_capabilities.cpp \
  network_dialogs.cpp network_dialog_widgets_sdl.cpp \
  network_games.cpp network_messages.cpp				  \
  network_star_hub.cpp network_star_spoke.cpp network_udp.cpp \
  SSLP_limited.cpp StarGameProtocol.cpp	  \
  Update.cpp HTTP.cpp PortForward.cpp Pinger.cpp

EXTRA_libnetwork_a_SOURCES = network_dummy.cpp

AM_CPPFLAGS = -I$(top_srcdir)/Source_Files/CSeries -I$(top_srcdir)/Source_Files/Files \
  -I$(top_srcdir)/Source_Files/GameWorld -I$(top_srcdir)/Source_Files/Input \
  -I$(top_srcdir)/Source_Files/Network/Metaserver \
  -I$(top_srcdir)/Source_Files/Network/StandaloneHub \
  -I$(top_srcdir)/Source_Files/Misc -I$(top_srcdir)/Source_Files/ModelView \
  -I$(top_srcdir)/Source_Files/RenderMain \
  -I$(top_srcdir)/Source_Files/RenderOther -I$(top_srcdir)/Source_Files/Sound \
  -I$(top_srcdir)/Source_Files/TCPMess -I$(top_srcdir)/Source_Files/XML \
  -I$(top_srcdir)/Source_Files/Lua -I$(top_srcdir)/Source_Files
