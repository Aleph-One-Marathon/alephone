## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = libxml.a

libxml_a_SOURCES = Plugins.h		\
  QuickSave.h InfoTree.h		\
  XML_LevelScript.h XML_ParseTreeRoot.h		\
									\
  Plugins.cpp		\
  QuickSave.cpp InfoTree.cpp		\
  XML_LevelScript.cpp XML_MakeRoot.cpp

AM_CPPFLAGS = -I$(top_srcdir)/Source_Files/CSeries -I$(top_srcdir)/Source_Files/Files \
  -I$(top_srcdir)/Source_Files/GameWorld -I$(top_srcdir)/Source_Files/Input \
  -I$(top_srcdir)/Source_Files/Lua -I$(top_srcdir)/Source_Files/Misc \
  -I$(top_srcdir)/Source_Files/ModelView -I$(top_srcdir)/Source_Files/Network \
  -I$(top_srcdir)/Source_Files/Network/Metaserver \
  -I$(top_srcdir)/Source_Files/TCPMess \
  -I$(top_srcdir)/Source_Files/RenderMain \
  -I$(top_srcdir)/Source_Files/RenderOther -I$(top_srcdir)/Source_Files/Sound \
  -I$(top_srcdir)/Source_Files
