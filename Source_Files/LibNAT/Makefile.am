## Process this file with automake to produce Makefile.in

if MAKE_WINDOWS
OS_CHOICE = os_win.c
else
OS_CHOICE = os_unix.c
endif

noinst_LIBRARIES = libnat.a
libnat_a_SOURCES = error.h http_libnat.h libnat.h os.h os_common.h ssdp.h utility.h error.c http.c libnat.c os_common.c $(OS_CHOICE) ssdp.c upnp.c utility.c

EXTRA_libnat_a_SOURCES = os_win.c os_unix.c

if BUILD_EXPAT
AM_CPPFLAGS = -I$(top_srcdir)/Source_Files/Expat
endif
