## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = libsound.a

libsound_a_SOURCES = music.h mysound.h song_definitions.h sound_definitions.h \
  sound_sdl.h \
  \
  mysound.cpp

INCLUDES = -I$(top_srcdir)/Source_Files/CSeries -I$(top_srcdir)/Source_Files/Files \
  -I$(top_srcdir)/Source_Files/GameWorld -I$(top_srcdir)/Source_Files/Input \
  -I$(top_srcdir)/Source_Files/Misc -I$(top_srcdir)/Source_Files/ModelView \
  -I$(top_srcdir)/Source_Files/Network -I$(top_srcdir)/Source_Files/Pfhortran \
  -I$(top_srcdir)/Source_Files/RenderMain -I$(top_srcdir)/Source_Files/RenderOther \
  -I$(top_srcdir)/Source_Files/XML -I$(top_srcdir)/Source_Files
