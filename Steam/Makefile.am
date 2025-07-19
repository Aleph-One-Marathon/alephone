## Process this file with automake to produce Makefile.in

if STEAM
bin_PROGRAMS = classic_marathon_launcher
else
bin_PROGRAMS =
endif

classic_marathon_launcher_SOURCES = steamshim_parent.cpp

classic_marathon_launcher_LDADD = -lsteam_api -ldl

classic_marathon_launcher_LDFLAGS = -Wl,-rpath=.


