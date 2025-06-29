## Process this file with automake to produce Makefile.in 

SUBDIRS = CSeries Files Videos GameWorld Input Lua Misc	\
   ModelView Network RenderMain RenderOther Sound TCPMess XML

if STANDALONE_HUB
bin_PROGRAMS = standalone_hub
else
if MAKE_WINDOWS
bin_PROGRAMS = AlephOne Marathon Marathon2 MarathonInfinity
else
bin_PROGRAMS = alephone
if BUILD_ALEPHONE_TESTS
bin_PROGRAMS += alephone_tests
endif
endif
endif

alephone_SOURCES = shell.h \
  \
  shell.cpp shell_misc.cpp shell_options.h shell_options.cpp main.cpp

alephone_LDADD = \
  CSeries/libcseries.a Files/libfiles.a Videos/libvideos.a \
  GameWorld/libgameworld.a Input/libinput.a Lua/liba1lua.a Misc/libmisc.a \
  ModelView/libmodelview.a Network/libnetwork.a Network/Metaserver/libmetaserver.a \
  RenderMain/librendermain.a RenderOther/librenderother.a Sound/libsound.a \
  XML/libxml.a \
  \
  CSeries/libcseries.a Files/libfiles.a Videos/libvideos.a \
  GameWorld/libgameworld.a Input/libinput.a Lua/liba1lua.a Misc/libmisc.a \
  ModelView/libmodelview.a Network/libnetwork.a Network/Metaserver/libmetaserver.a \
  RenderMain/librendermain.a RenderOther/librenderother.a Sound/libsound.a \
  TCPMess/libtcpmess.a XML/libxml.a

standalone_hub_SOURCES = shell.h shell.cpp shell_misc.cpp shell_options.h shell_options.cpp Network/StandaloneHub/standalone_hub_main.cpp

standalone_hub_LDADD = $(alephone_LDADD) Network/StandaloneHub/libstandalonehub.a

alephone_tests_SOURCES = shell.h shell.cpp shell_misc.cpp shell_options.h shell_options.cpp $(top_srcdir)/tests/replay_film_test.cpp $(top_srcdir)/tests/main.cpp
alephone_tests_LDADD = $(alephone_LDADD)

AM_CPPFLAGS = -I$(top_srcdir)/Source_Files/CSeries -I$(top_srcdir)/Source_Files/Files \
  -I$(top_srcdir)/Source_Files/GameWorld -I$(top_srcdir)/Source_Files/Input \
  -I$(top_srcdir)/Source_Files/Lua -I$(top_srcdir)/Source_Files/Misc \
  -I$(top_srcdir)/Source_Files/ModelView -I$(top_srcdir)/Source_Files/Network \
  -I$(top_srcdir)/Source_Files/Network/Metaserver \
  -I$(top_srcdir)/Source_Files/Videos -I$(top_srcdir)/Source_Files/RenderMain \
  -I$(top_srcdir)/Source_Files/RenderOther -I$(top_srcdir)/Source_Files/Sound \
  -I$(top_srcdir)/Source_Files/XML -I$(top_srcdir)/Source_Files/TCPMess

AlephOne_LDADD = $(alephone_LDADD) alephone-resources.o
AlephOne_SOURCES = $(alephone_SOURCES)

Marathon_LDADD = $(alephone_LDADD) marathon-resources.o
Marathon_SOURCES = $(alephone_SOURCES)

Marathon2_LDADD = $(alephone_LDADD) marathon2-resources.o
Marathon2_SOURCES = $(alephone_SOURCES)

MarathonInfinity_LDADD = $(alephone_LDADD) marathon-infinity-resources.o
MarathonInfinity_SOURCES = $(alephone_SOURCES)

if MAKE_WINDOWS
BUILD_YEAR = `echo $(VERSION) | cut -c 1-4`
BUILD_MONTH = `echo $(VERSION) | cut -c 5-6 | sed -e s/^0//`
BUILD_DAY = `echo $(VERSION) | cut -c 7-8 | sed -e s/^0//`
WIN_VERSION_STRING= 0, $(BUILD_YEAR), $(BUILD_MONTH), $(BUILD_DAY)
A1_DISPLAY_VERSION= `grep '^\#define A1_DISPLAY_VERSION' $(srcdir)/Misc/alephversion.h | sed -e 's/\(.*\"\)\(.*\)\(\"\)/\2/g' | tr -d '\n'`
endif

alephone-resources.o: $(top_srcdir)/Resources/Windows/alephone.rc Misc/alephversion.h
	sed -e "s/WIN_VERSION_STRING/$(WIN_VERSION_STRING)/g" -e "s/A1_DISPLAY_VERSION/$(A1_DISPLAY_VERSION)/g" $(top_srcdir)/Resources/Windows/alephone.rc | @WINDRES@ -I$(top_srcdir)/Resources/Windows -o alephone-resources.o

marathon-resources.o: $(top_srcdir)/Resources/Windows/marathon.rc Misc/alephversion.h $(top_srcdir)/Resources/Windows/marathon.ico
	sed -e "s/WIN_VERSION_STRING/$(WIN_VERSION_STRING)/g" -e "s/A1_DISPLAY_VERSION/$(A1_DISPLAY_VERSION)/g" $(top_srcdir)/Resources/Windows/marathon.rc | @WINDRES@ -I$(top_srcdir)/Resources/Windows -o marathon-resources.o

marathon2-resources.o: $(top_srcdir)/Resources/Windows/marathon2.rc Misc/alephversion.h $(top_srcdir)/Resources/Windows/marathon2.ico
	sed -e "s/WIN_VERSION_STRING/$(WIN_VERSION_STRING)/g" -e "s/A1_DISPLAY_VERSION/$(A1_DISPLAY_VERSION)/g" $(top_srcdir)/Resources/Windows/marathon2.rc | @WINDRES@ -I$(top_srcdir)/Resources/Windows -o marathon2-resources.o

marathon-infinity-resources.o: $(top_srcdir)/Resources/Windows/marathon-infinity.rc Misc/alephversion.h $(top_srcdir)/Resources/Windows/marathon-infinity.ico
	sed -e "s/WIN_VERSION_STRING/$(WIN_VERSION_STRING)/g" -e "s/A1_DISPLAY_VERSION/$(A1_DISPLAY_VERSION)/g" $(top_srcdir)/Resources/Windows/marathon-infinity.rc | @WINDRES@ -I$(top_srcdir)/Resources/Windows -o marathon-infinity-resources.o
