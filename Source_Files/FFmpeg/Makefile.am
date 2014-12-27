## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = libffmpeg.a

if HAVE_FFMPEG
SDL_FFMPEG = SDL_ffmpeg.c SDL_ffmpeg.h
else
SDL_FFMPEG = 
endif

libffmpeg_a_SOURCES = $(SDL_FFMPEG) Movie.cpp Movie.h

EXTRA_libffmpeg_a_SOURCES = SDL_ffmpeg.c SDL_ffmpeg.h

AM_CPPFLAGS = -I$(top_srcdir)/Source_Files/CSeries -I$(top_srcdir)/Source_Files/Files \
  -I$(top_srcdir)/Source_Files/GameWorld -I$(top_srcdir)/Source_Files/Input \
  -I$(top_srcdir)/Source_Files/Misc -I$(top_srcdir)/Source_Files/ModelView \
  -I$(top_srcdir)/Source_Files/Network -I$(top_srcdir)/Source_Files/Sound \
  -I$(top_srcdir)/Source_Files/RenderMain -I$(top_srcdir)/Source_Files/RenderOther \
  -I$(top_srcdir)/Source_Files/XML -I$(top_srcdir)/Source_Files

AM_CFLAGS=-std=c99
AM_CPPFLAGS += -D__STDC_CONSTANT_MACROS
