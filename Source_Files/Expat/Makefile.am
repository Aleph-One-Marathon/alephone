## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = libexpat.a
libexpat_a_SOURCES = asciitab.h hashtable.c hashtable.h iasciitab.h \
  latin1tab.h utf8tab.h xmldef.h xmlparse.c xmlparse.h xmlrole.c xmlrole.h \
  xmltok.c xmltok.h xmltok_impl.h
EXTRA_libexpat_a_SOURCES = xmltok_impl.c xmltok_ns.c nametab.h

noinst_PROGRAMS = gennmtab

$(srcdir)/nametab.h: gennmtab
	./gennmtab >$@

xmltok.o: $(srcdir)/nametab.h
