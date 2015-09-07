## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = libmodelview.a

libmodelview_a_SOURCES = Model3D.h ModelRenderer.h Dim3_Loader.h \
  StudioLoader.h WavefrontLoader.h \
  \
  Model3D.cpp ModelRenderer.cpp Dim3_Loader.cpp StudioLoader.cpp \
  WavefrontLoader.cpp

AM_CPPFLAGS = -I$(top_srcdir)/Source_Files/CSeries \
  -I$(top_srcdir)/Source_Files/Files -I$(top_srcdir)/Source_Files/GameWorld \
  -I$(top_srcdir)/Source_Files/Input -I$(top_srcdir)/Source_Files/Misc \
  -I$(top_srcdir)/Source_Files/Network \
  -I$(top_srcdir)/Source_Files/RenderMain -I$(top_srcdir)/Source_Files/RenderOther \
  -I$(top_srcdir)/Source_Files/Sound -I$(top_srcdir)/Source_Files/XML
