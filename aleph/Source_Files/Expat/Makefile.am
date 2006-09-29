## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = libexpat.a
libexpat_a_SOURCES = asciitab.h hashtable.c hashtable.h iasciitab.h \
  latin1tab.h utf8tab.h xmldef.h xmlparse.c xmlparse.h xmlrole.c xmlrole.h \
  xmltok.c xmltok.h xmltok_impl.h
EXTRA_libexpat_a_SOURCES = xmltok_impl.c xmltok_ns.c

noinst_PROGRAMS = gennmtab
CLEANFILES = nametab.h

nametab.h: gennmtab
	./gennmtab >$@

xmltok.o: nametab.h
