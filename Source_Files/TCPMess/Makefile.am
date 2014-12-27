## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = libtcpmess.a
libtcpmess_a_SOURCES = CommunicationsChannel.h Message.h MessageDispatcher.h \
  MessageHandler.h MessageInflater.h \
  CommunicationsChannel.cpp Message.cpp MessageDispatcher.cpp MessageHandler.cpp\
  MessageInflater.cpp

AM_CPPFLAGS = -I$(top_srcdir)/Source_Files/CSeries -I$(top_srcdir)/Source_Files/Files -I$(top_srcdir)/Source_Files/Misc
