## Process this file with automake to produce Makefile.in 

SUBDIRS = CSeries Expat Files GameWorld Input Misc ModelView Network Pfhortran \
  RenderMain RenderOther Sound XML

bin_PROGRAMS = alephone

alephone_SOURCES = shell.h shell_sdl.h \
  \
  shell.cpp shell_misc.cpp

alephone_LDADD = \
  CSeries/libcseries.a Expat/libexpat.a Files/libfiles.a \
  GameWorld/libgameworld.a Input/libinput.a Misc/libmisc.a \
  ModelView/libmodelview.a Network/libnetwork.a Pfhortran/libpfhortran.a \
  RenderMain/librendermain.a RenderOther/librenderother.a Sound/libsound.a \
  XML/libxml.a \
  \
  CSeries/libcseries.a Expat/libexpat.a Files/libfiles.a \
  GameWorld/libgameworld.a Input/libinput.a Misc/libmisc.a \
  ModelView/libmodelview.a Network/libnetwork.a Pfhortran/libpfhortran.a \
  RenderMain/librendermain.a RenderOther/librenderother.a Sound/libsound.a \
  XML/libxml.a \
  @SYS_GL_LIBS@

INCLUDES = -I$(top_srcdir)/Source_Files/CSeries -I$(top_srcdir)/Source_Files/Files \
  -I$(top_srcdir)/Source_Files/GameWorld -I$(top_srcdir)/Source_Files/Input \
  -I$(top_srcdir)/Source_Files/Misc -I$(top_srcdir)/Source_Files/ModelView \
  -I$(top_srcdir)/Source_Files/Network -I$(top_srcdir)/Source_Files/Pfhortran \
  -I$(top_srcdir)/Source_Files/RenderMain -I$(top_srcdir)/Source_Files/RenderOther \
  -I$(top_srcdir)/Source_Files/Sound -I$(top_srcdir)/Source_Files/XML \
  -I$(top_srcdir)/Source_Files/Expat

# Data directories
confpaths.h: Makefile
	echo "#define PKGDATADIR \"$(pkgdatadir)\"" > $@

DISTCLEANFILES = confpaths.h

shell.o: confpaths.h
