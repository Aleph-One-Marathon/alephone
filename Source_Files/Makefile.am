## Process this file with automake to produce Makefile.in 

SUBDIRS = CSeries Expat Files GameWorld Input LibNAT Lua Misc ModelView Network \
   RenderMain RenderOther Sound TCPMess XML

if MAKE_WINDOWS
bin_PROGRAMS = AlephOne
else
bin_PROGRAMS = alephone
endif

alephone_SOURCES = shell.h \
  \
  shell.cpp shell_misc.cpp Misc/DefaultStringSets.cpp

alephone_LDADD = \
  CSeries/libcseries.a Expat/libexpat.a Files/libfiles.a \
  GameWorld/libgameworld.a Input/libinput.a Lua/liba1lua.a Misc/libmisc.a \
  ModelView/libmodelview.a Network/libnetwork.a Network/Metaserver/libmetaserver.a \
  RenderMain/librendermain.a RenderOther/librenderother.a Sound/libsound.a \
  XML/libxml.a \
  \
  CSeries/libcseries.a Expat/libexpat.a Files/libfiles.a \
  GameWorld/libgameworld.a LibNAT/libnat.a Input/libinput.a Lua/liba1lua.a Misc/libmisc.a \
  ModelView/libmodelview.a Network/libnetwork.a Network/Metaserver/libmetaserver.a \
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

if MAKE_WINDOWS
BUILD_YEAR = `echo $(VERSION) | cut -c 1-4`
BUILD_MONTH = `echo $(VERSION) | cut -c 5-6 | sed -e s/^0//`
BUILD_DAY = `echo $(VERSION) | cut -c 7-8 | sed -e s/^0//`
WIN_VERSION_STRING= 0, $(BUILD_YEAR), $(BUILD_MONTH), $(BUILD_DAY)
A1_DISPLAY_VERSION= `grep '^\#define A1_DISPLAY_VERSION' $(srcdir)/Misc/alephversion.h | sed -e 's/\(.*\"\)\(.*\)\(\"\)/\2/g' | tr -d '\n'`
endif

alephone-resources.o: FORCE
	sed -e s/BUILDDATE/`date +%Y-%m-%d`/g -e "s/WIN_VERSION_STRING/$(WIN_VERSION_STRING)/g" -e "s/A1_DISPLAY_VERSION/$(A1_DISPLAY_VERSION)/g" $(top_srcdir)/Resources/Windows/alephone.rc | @WINDRES@ -I$(top_srcdir)/Resources/Windows -o alephone-resources.o

FORCE:

shell.o: confpaths.h
