## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = libsound.a

libsound_a_SOURCES = BasicIFFDecoder.h BasicIFFDecoder.cpp Decoder.h Decoder.cpp MADDecoder.h MADDecoder.cpp Mixer.h Music.h song_definitions.h sound_definitions.h Mixer.cpp Music.cpp ReplacementSounds.h ReplacementSounds.cpp SndfileDecoder.h SndfileDecoder.cpp SoundFile.h SoundFile.cpp SoundManager.h SoundManagerEnums.h SoundManager.cpp VorbisDecoder.h VorbisDecoder.cpp FFmpegDecoder.h FFmpegDecoder.cpp

AM_CPPFLAGS = -I$(top_srcdir)/Source_Files/CSeries -I$(top_srcdir)/Source_Files/Files \
  -I$(top_srcdir)/Source_Files/GameWorld -I$(top_srcdir)/Source_Files/Input \
  -I$(top_srcdir)/Source_Files/Misc -I$(top_srcdir)/Source_Files/ModelView \
  -I$(top_srcdir)/Source_Files/Network -I$(top_srcdir)/Source_Files/FFmpeg \
  -I$(top_srcdir)/Source_Files/RenderMain -I$(top_srcdir)/Source_Files/RenderOther \
  -I$(top_srcdir)/Source_Files/XML -I$(top_srcdir)/Source_Files
