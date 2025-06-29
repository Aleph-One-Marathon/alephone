## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = libvideos.a

libvideos_a_SOURCES = Movie.cpp Movie.h pl_mpeg.h

AM_CPPFLAGS = -I$(top_srcdir)/Source_Files/CSeries -I$(top_srcdir)/Source_Files/Files \
  -I$(top_srcdir)/Source_Files/GameWorld -I$(top_srcdir)/Source_Files/Input \
  -I$(top_srcdir)/Source_Files/Misc -I$(top_srcdir)/Source_Files/ModelView \
  -I$(top_srcdir)/Source_Files/Network -I$(top_srcdir)/Source_Files/Sound \
  -I$(top_srcdir)/Source_Files/RenderMain -I$(top_srcdir)/Source_Files/RenderOther \
  -I$(top_srcdir)/Source_Files/XML -I$(top_srcdir)/Source_Files

AM_CPPFLAGS += -D__STDC_CONSTANT_MACROS
