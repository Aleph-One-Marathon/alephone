## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = libcseries.a
libcseries_a_SOURCES = byte_swapping.h BStream.h csalerts.h		\
  csdialogs.h cscluts.h cseries.h csfonts.h csmacros.h	\
  csmisc.h cspaths.h cspixels.h csstrings.h cstypes.h FilmProfile.h mytm.h	\
									\
  byte_swapping.cpp BStream.cpp csalerts_sdl.cpp cscluts_sdl.cpp	\
  csdialogs_sdl.cpp csmisc_sdl.cpp cspaths_sdl.cpp csstrings.cpp FilmProfile.cpp	\
  mytm_sdl.cpp

EXTRA_libcseries_a_SOURCES = csalerts.mm cspaths.mm

AM_CPPFLAGS = -I$(top_srcdir)/Source_Files/Files -I$(top_srcdir)/Source_Files/GameWorld \
  -I$(top_srcdir)/Source_Files/Input -I$(top_srcdir)/Source_Files/Misc \
  -I$(top_srcdir)/Source_Files/ModelView -I$(top_srcdir)/Source_Files/Network \
  -I$(top_srcdir)/Source_Files/RenderMain \
  -I$(top_srcdir)/Source_Files/RenderOther -I$(top_srcdir)/Source_Files/Sound \
  -I$(top_srcdir)/Source_Files/XML -I$(top_srcdir)/Source_Files/Network/Metaserver \
  -I$(top_srcdir)/Source_Files/TCPMess \
  -I$(top_srcdir)/Source_Files

confpaths.h: Makefile
	echo "#define PKGDATADIR \"$(pkgdatadir)\"" > $@

DISTCLEANFILES = confpaths.h

cspaths_sdl.o: confpaths.h
