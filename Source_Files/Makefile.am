## Process this file with automake to produce Makefile.in 

SUBDIRS = CSeries Expat Files GameWorld Input Lua Misc ModelView Network \
  Pfhortran RenderMain RenderOther Sound TCPMess XML

if MAKE_WINDOWS
bin_PROGRAMS = AlephOne
else
bin_PROGRAMS = alephone
endif

alephone_SOURCES = shell.h shell_sdl.h \
  \
  shell.cpp shell_misc.cpp Misc/DefaultStringSets.cpp

alephone_LDADD = \
  CSeries/libcseries.a Expat/libexpat.a Files/libfiles.a \
  GameWorld/libgameworld.a Input/libinput.a Lua/liba1lua.a Misc/libmisc.a \
  ModelView/libmodelview.a Network/libnetwork.a Network/Metaserver/libmetaserver.a Pfhortran/libpfhortran.a \
  RenderMain/librendermain.a RenderOther/librenderother.a Sound/libsound.a \
  XML/libxml.a \
  \
  CSeries/libcseries.a Expat/libexpat.a Files/libfiles.a \
  GameWorld/libgameworld.a Input/libinput.a Lua/liba1lua.a Misc/libmisc.a \
  ModelView/libmodelview.a Network/libnetwork.a Network/Metaserver/libmetaserver.a Pfhortran/libpfhortran.a \
  RenderMain/librendermain.a RenderOther/librenderother.a Sound/libsound.a \
  TCPMess/libtcpmess.a XML/libxml.a

INCLUDES = -I$(top_srcdir)/Source_Files/CSeries -I$(top_srcdir)/Source_Files/Files \
  -I$(top_srcdir)/Source_Files/GameWorld -I$(top_srcdir)/Source_Files/Input \
  -I$(top_srcdir)/Source_Files/Lua -I$(top_srcdir)/Source_Files/Misc \
  -I$(top_srcdir)/Source_Files/ModelView -I$(top_srcdir)/Source_Files/Network \
  -I$(top_srcdir)/Source_Files/Network/Metaserver \
  -I$(top_srcdir)/Source_Files/Pfhortran -I$(top_srcdir)/Source_Files/RenderMain \
  -I$(top_srcdir)/Source_Files/RenderOther -I$(top_srcdir)/Source_Files/Sound \
  -I$(top_srcdir)/Source_Files/XML -I$(top_srcdir)/Source_Files/Expat \
  -I$(top_srcdir)/Source_Files/TCPMess

AlephOne_LDADD = $(alephone_LDADD) alephone-resources.o
AlephOne_SOURCES = $(alephone_SOURCES)

# Data directories
confpaths.h: Makefile
	echo "#define PKGDATADIR \"$(pkgdatadir)\"" > $@

DISTCLEANFILES = confpaths.h

alephone-resources.o: FORCE
	sed -e s/BUILDDATE/`date +%Y-%m-%d`/g ../Resources/Windows/alephone.rc | @WINDRES@ -o alephone-resources.o

FORCE:

shell.o: confpaths.h
