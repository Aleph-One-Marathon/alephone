## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = libpfhortran.a

libpfhortran_a_SOURCES = script_instructions.h script_parser.h scripting.h \
  \
  script_instructions.cpp script_parser.cpp scripting.cpp

EXTRA_libpfhortran_a_SOURCES = language_definition.h

INCLUDES = -I$(top_srcdir)/Source_Files/CSeries -I$(top_srcdir)/Source_Files/Files \
  -I$(top_srcdir)/Source_Files/GameWorld -I$(top_srcdir)/Source_Files/Input \
  -I$(top_srcdir)/Source_Files/Misc -I$(top_srcdir)/Source_Files/ModelView \
  -I$(top_srcdir)/Source_Files/Network -I$(top_srcdir)/Source_Files/RenderMain \
  -I$(top_srcdir)/Source_Files/RenderOther -I$(top_srcdir)/Source_Files/Sound \
  -I$(top_srcdir)/Source_Files/XML -I$(top_srcdir)/Source_Files
