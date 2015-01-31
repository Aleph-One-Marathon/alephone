## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = librenderother.a

if HAVE_PNG
PNG_SRCS = IMG_savepng.c
else
PNG_SRCS =
endif

librenderother_a_SOURCES = ChaseCam.h computer_interface.h \
  fades.h FontHandler.h game_window.h HUDRenderer.h \
  HUDRenderer_OGL.h HUDRenderer_SW.h HUDRenderer_Lua.h images.h IMG_savepng.h motion_sensor.h \
  Image_Blitter.h OGL_Blitter.h Shape_Blitter.h OGL_LoadScreen.h overhead_map.h OverheadMap_OGL.h OverheadMapRenderer.h OverheadMap_SDL.h \
  screen_definitions.h screen_drawing.h screen.h \
  screen_shared.h sdl_fonts.h sdl_resize.h TextLayoutHelper.h TextStrings.h ViewControl.h \
  \
  ChaseCam.cpp computer_interface.cpp fades.cpp FontHandler.cpp game_window.cpp \
  HUDRenderer.cpp HUDRenderer_OGL.cpp HUDRenderer_SW.cpp HUDRenderer_Lua.cpp \
  images.cpp motion_sensor.cpp Image_Blitter.cpp $(PNG_SRCS) OGL_Blitter.cpp Shape_Blitter.cpp OGL_LoadScreen.cpp overhead_map.cpp OverheadMap_OGL.cpp \
  OverheadMapRenderer.cpp OverheadMap_SDL.cpp screen_drawing.cpp screen.cpp \
  sdl_fonts.cpp sdl_resize.cpp TextLayoutHelper.cpp TextStrings.cpp ViewControl.cpp

AM_CPPFLAGS = -I$(top_srcdir)/Source_Files/CSeries -I$(top_srcdir)/Source_Files/Files \
  -I$(top_srcdir)/Source_Files/GameWorld -I$(top_srcdir)/Source_Files/Input \
  -I$(top_srcdir)/Source_Files/Lua -I$(top_srcdir)/Source_Files/Misc \
  -I$(top_srcdir)/Source_Files/ModelView -I$(top_srcdir)/Source_Files/Network \
  -I$(top_srcdir)/Source_Files/FFmpeg -I$(top_srcdir)/Source_Files/RenderMain \
  -I$(top_srcdir)/Source_Files/Sound -I$(top_srcdir)/Source_Files/XML \
  -I$(top_srcdir)/Source_Files
