## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = libxml.a

libxml_a_SOURCES = ColorParser.h DamageParser.h Plugins.h		\
  QuickSave.h InfoTree.h		\
  ShapesParser.h XML_Configure.h XML_DataBlock.h XML_ElementParser.h	\
  XML_LevelScript.h XML_Loader_SDL.h XML_ParseTreeRoot.h		\
									\
  ColorParser.cpp DamageParser.cpp ShapesParser.cpp Plugins.cpp		\
  QuickSave.cpp InfoTree.cpp		\
  XML_Configure.cpp XML_DataBlock.cpp XML_ElementParser.cpp		\
  XML_LevelScript.cpp XML_Loader_SDL.cpp XML_MakeRoot.cpp

AM_CPPFLAGS = -I$(top_srcdir)/Source_Files/CSeries -I$(top_srcdir)/Source_Files/Files \
  -I$(top_srcdir)/Source_Files/GameWorld -I$(top_srcdir)/Source_Files/Input \
  -I$(top_srcdir)/Source_Files/Lua -I$(top_srcdir)/Source_Files/Misc \
  -I$(top_srcdir)/Source_Files/ModelView -I$(top_srcdir)/Source_Files/Network \
  -I$(top_srcdir)/Source_Files/Network/Metaserver \
  -I$(top_srcdir)/Source_Files/TCPMess \
  -I$(top_srcdir)/Source_Files/RenderMain \
  -I$(top_srcdir)/Source_Files/RenderOther -I$(top_srcdir)/Source_Files/Sound \
  -I$(top_srcdir)/Source_Files/Expat -I$(top_srcdir)/Source_Files
