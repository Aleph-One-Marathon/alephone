## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = libpfhortran.a
libpfhortran_a_SOURCES = script_instructions.cpp script_instructions.h \
  script_parser.cpp script_parser.h scripting.h scripting.cpp
EXTRA_libpfhortran_a_SOURCES = language_definition.h

EXTRA_DIST = Pfhortran_Language_Definition

INCLUDES = -I$(top_srcdir)/Source_Files/CSeries \
  -I$(top_srcdir)/Source_Files/Misc -I$(top_srcdir)/Source_Files/ModelView

$(srcdir)/language_definition.h: Pfhortran_Language_Definition
	awk >$@ '!/^#/ {print "{\"" $$1 "\", 0x" $$2 "},";}' $<

script_parser.o: $(srcdir)/language_definition.h
