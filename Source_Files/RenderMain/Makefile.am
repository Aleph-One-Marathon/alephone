## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = librendermain.a

if MAKE_WINDOWS
EXTRA_GL_H = OGL_Win32.h
EXTRA_GL = OGL_Win32.cpp
else
EXTRA_GL_H =
EXTRA_GL = 
endif

librendermain_a_SOURCES = AnimatedTextures.h collection_definition.h \
  Crosshairs.h ImageLoader.h low_level_textures.h OGL_Faders.h OGL_Model_Def.h \
  OGL_Render.h OGL_Setup.h OGL_Subst_Texture_Def.h OGL_Texture_Def.h \
  OGL_Textures.h $(EXTRA_GL_H) Rasterizer.h Rasterizer_OGL.h Rasterizer_SW.h render.h \
  RenderPlaceObjs.h RenderRasterize.h RenderSortPoly.h RenderVisTree.h \
  scottish_textures.h shape_definitions.h shape_descriptors.h shapes_sdl.h \
  textures.h \
  \
  AnimatedTextures.cpp Crosshairs_SDL.cpp ImageLoader_SDL.cpp OGL_Faders.cpp \
  OGL_Model_Def.cpp OGL_Render.cpp OGL_Setup.cpp OGL_Subst_Texture_Def.cpp \
  OGL_Textures.cpp $(EXTRA_GL) render.cpp RenderPlaceObjs.cpp RenderRasterize.cpp \
  RenderSortPoly.cpp RenderVisTree.cpp scottish_textures.cpp shapes.cpp \
  textures.cpp

INCLUDES = -I$(top_srcdir)/Source_Files/CSeries -I$(top_srcdir)/Source_Files/Files \
  -I$(top_srcdir)/Source_Files/GameWorld -I$(top_srcdir)/Source_Files/Input \
  -I$(top_srcdir)/Source_Files/Misc -I$(top_srcdir)/Source_Files/ModelView \
  -I$(top_srcdir)/Source_Files/Network -I$(top_srcdir)/Source_Files/Pfhortran \
  -I$(top_srcdir)/Source_Files/RenderOther -I$(top_srcdir)/Source_Files/Sound \
  -I$(top_srcdir)/Source_Files/XML -I$(top_srcdir)/Source_Files
