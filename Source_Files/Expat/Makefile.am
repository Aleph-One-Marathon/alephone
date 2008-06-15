## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = libexpat.a
libexpat_a_SOURCES = ascii.h asciitab.h expat.h expat_external.h internal.h iasciitab.h \
  latin1tab.h nametab.h utf8tab.h xmlparse.c xmlrole.c xmlrole.h \
  xmltok.c xmltok.h xmltok_impl.h
EXTRA_libexpat_a_SOURCES = xmltok_impl.c xmltok_ns.c

AM_CPPFLAGS = -DHAVE_MEMMOVE

