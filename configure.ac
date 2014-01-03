dnl Process this file with autoconf to produce a configure script.
dnl Written in 2000 by Christian Bauer <Christian.Bauer@uni-mainz.de>

AC_INIT([Aleph One/SDL], m4_esyscmd([ grep '^#define A1_DATE_VERSION' Source_Files/Misc/alephversion.h | sed -e 's/\(.*\"\)\(.*\)\(\"\)/\2/g' | tr -d '\n']), [http://sourceforge.net/bugs/?group_id=1997], [AlephOne])
AC_CONFIG_SRCDIR([Source_Files/shell.cpp])
AC_PREREQ(2.52)

dnl Detect the canonical host and target build environment.
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

dnl Init automake.
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

dnl Configure options.
AC_ARG_ENABLE(opengl, 
[  --enable-opengl         use OpenGL for rendering [default=yes]], , enable_opengl=yes)
AC_ARG_ENABLE(mad, 
[  --enable-mad            use libmad for mp3 playback [default=yes]], , enable_mad=yes)
AC_ARG_ENABLE(ffmpeg,
[  --enable-ffmpeg         use ffmpeg for audio/video playback and film export [default=yes]], , enable_ffmpeg=yes)
AC_ARG_ENABLE(sndfile, 
[  --enable-sndfile        use libsndfile for decoding audio files [default=yes]], , enable_sndfile=yes)
AC_ARG_ENABLE(vorbis, 
[  --enable-vorbis         enable ogg/vorbis music playback [default=yes]], , enable_vorbis=yes)
AC_ARG_ENABLE(lua, 
[  --enable-lua            use built-in Lua scripting [default=yes]], , enable_lua=yes)
AC_ARG_ENABLE(smpeg, 
[  --enable-smpeg          use SMPEG for movie playback [default=yes]], , enable_smpeg=yes)
AC_ARG_ENABLE(speex, 
[  --enable-speex          enable Speex net mic playback [default=yes]], , enable_speex=yes)
AC_ARG_ENABLE(alsa,
[  --enable-alsa           enable ALSA net mic transmission [default=yes]], , enable_alsa=yes)
AC_ARG_ENABLE(zzip,
[  --enable-zzip           enable zziplib support [default=yes]], , enable_zzip=yes)
AC_ARG_ENABLE(png,
[  --enable-png            enable PNG screenshot support [default=yes]], , enable_png=yes)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_INSTALL
AC_PROG_RANLIB

dnl some platform specific stuff
case $target in
*-*-mingw32*)
	AC_CHECK_TOOL(WINDRES, windres, :)
	;;
*-*-netbsd*)
	CPPFLAGS="$CPPFLAGS -I/usr/pkg/include"	
	;;
*)
	;;
esac

dnl Checks for headers.
AC_CHECK_HEADERS(unistd.h)
AC_CHECK_HEADERS(pwd.h)

dnl Checks for library functions.
AC_CHECK_FUNC(snprintf, [
  AC_DEFINE(HAVE_SNPRINTF, 1, [snprintf() is available])
])
AC_CHECK_FUNC(vsnprintf, [
  AC_DEFINE(HAVE_VSNPRINTF, 1, [vsnprintf() is available])
])

AC_CHECK_FUNC(mkstemp, [
  AC_DEFINE(LUA_USE_MKSTEMP, 1, [mkstemp() is available])
])
AC_CHECK_FUNC(sysconf, [
  AC_DEFINE(HAVE_SYSCONF, 1, [sysconf() is available])
])
AC_CHECK_FUNC(sysctlbyname, [
  AC_DEFINE(HAVE_SYSCTLBYNAME, 1, [sysctlbyname() is available])
])

dnl Check for SDL.
AM_PATH_SDL(1.2.0, , AC_MSG_ERROR([You need SDL 1.2 to run Aleph One.]))
CPPFLAGS="$CPPFLAGS $SDL_CFLAGS -DSDL"
LIBS="$LIBS $SDL_LIBS"

dnl Check for SDL_image.
AC_CHECK_HEADERS(SDL_image.h, [
  AC_CHECK_LIB(SDL_image, IMG_Load, [
    LIBS="-lSDL_image $LIBS"
    AC_DEFINE(HAVE_SDL_IMAGE, 1, [SDL_image support is enabled])
  ])
])

AC_CHECK_HEADERS(SDL_ttf.h, [
 AC_CHECK_LIB(SDL_ttf, TTF_Init, [
   LIBS="-lSDL_ttf $LIBS"
   AC_DEFINE(HAVE_SDL_TTF, 1, [SDL_TTF support is enabled])
 ])],
 AC_MSG_ERROR([You need SDL_ttf to run Aleph One.])
)

dnl Check for SDL_net.
AC_CHECK_HEADERS(SDL_net.h, [
  AC_CHECK_LIB(SDL_net, SDLNet_Init, [
    LIBS="-lSDL_net $LIBS"
    AC_DEFINE(HAVE_SDL_NET, 1, [SDL_net support is enabled])
  ])
])
if [[ "x$ac_cv_lib_SDL_net_SDLNet_Init" != xyes ]]; then
  AC_MSG_ERROR([You need SDL_net to run Aleph One.])
fi
AM_CONDITIONAL(HAVE_SDL_NET, test x$ac_cv_lib_SDL_net_SDLNet_Init = xyes)

AC_SEARCH_LIBS(gethostbyname, nsl)
AC_SEARCH_LIBS(socket, socket, ,
    [AC_CHECK_LIB(nsl, socket, LIBS="$LIBS -lsocket -lnsl", , -lsocket)])


dnl Check for zlib
AC_CHECK_HEADER(zlib.h, , AC_ERROR([Aleph One requires zlib]))
AC_CHECK_LIB(z, zlibVersion, LIBS="-lz $LIBS", AC_ERROR([Aleph One requires zlib]))

dnl Check for zziplib
have_zzip=false
if [[ "x$enable_zzip" = "xyes" ]]; then
PKG_CHECK_MODULES([ZZIP], [zziplib >= 0.10.75], [
			  CPPFLAGS="$ZZIP_CFLAGS $CPPFLAGS"
			  LIBS="$ZZIP_LIBS $LIBS"
			  have_zzip=true
			  AC_DEFINE(HAVE_ZZIP, 1, [ZZIP support enabled])
])
fi
AM_CONDITIONAL(BUILD_ZZIP_RWOPS, test x$have_zzip = xtrue)

have_png=false
if [[ "x$enable_png" = "xyes" ]]; then
PKG_CHECK_MODULES([PNG], [libpng], [
		  CPPFLAGS="$PNG_CFLAGS $CPPFLAGS"
		  LIBS="$PNG_LIBS $LIBS"
		  have_png=true
		  AC_DEFINE(HAVE_PNG, 1, [PNG support enabled])]
)
fi
AM_CONDITIONAL(HAVE_PNG, test x$have_png = xtrue)

dnl Check for boost/bind
AC_LANG_PUSH(C++)
AC_CHECK_HEADER([boost/bind.hpp], , [AC_MSG_ERROR([You need boost/bind.hpp from the boost library to run Aleph One])])
AC_LANG_POP(C++)

dnl Check for boost/function
AC_LANG_PUSH(C++)
AC_CHECK_HEADER([boost/function.hpp], , [AC_MSG_ERROR([You need boost/function.hpp from the boost library to run Aleph One])])
AC_LANG_POP(C++)

if [[ "x$enable_smpeg" = "xyes" ]]; then
dnl Check for SMPEG
AC_CHECK_HEADERS([smpeg/smpeg.h], [
  AC_CHECK_LIB(smpeg, SMPEG_new, [
    LIBS="-lsmpeg $LIBS"
    AC_DEFINE(HAVE_SMPEG, 1, [SMPEG movie playback is enabled])
  ])
])
fi	    

if [[ "x$enable_mad" = "xyes" ]]; then
dnl Check for libmad
AC_CHECK_HEADERS([mad.h], [
			  AC_CHECK_LIB(mad, mad_stream_init, [
			  LIBS="-lmad $LIBS"
			  AC_DEFINE(HAVE_MAD, 1, [MAD mp3 playback is enabled])
])
])
fi

if [[ "x$enable_sndfile" = "xyes" ]]; then
dnl Check for libsndfile
AC_CHECK_HEADERS([sndfile.h], [
			      AC_CHECK_LIB(sndfile, sf_open, [
			      LIBS="-lsndfile $LIBS"
			      AC_DEFINE(HAVE_SNDFILE, 1, [libsndfile support is enabled])
])
])
fi

if [[ "x$enable_vorbis" = "xyes" ]]; then
dnl Check for vorbisfile
PKG_CHECK_MODULES([VORBISFILE], [vorbisfile], [
			  CPPFLAGS="$VORBISFILE_CFLAGS $CPPFLAGS"
			  LIBS="$VORBISFILE_LIBS $LIBS"
			  AC_DEFINE(HAVE_VORBISFILE, 1, [vorbis support enabled])
])
fi

if [[ "x$enable_ffmpeg" = "xyes" ]]; then
dnl Check for libavcodec
PKG_CHECK_MODULES([FFMPEG], [libavcodec >= 53.35.0 libavformat >= 53.21.0 libavutil >= 51.22.0 libswscale >= 2.1.0], [
			    CPPFLAGS="$FFMPEG_CFLAGS $CPPFLAGS"
			    LIBS="$FFMPEG_LIBS $LIBS"
			    AC_DEFINE(HAVE_FFMPEG, 1, [ffmpeg support enabled])
			    have_ffmpeg=true
])
fi
AM_CONDITIONAL(HAVE_FFMPEG, test x$have_ffmpeg = xtrue)

if [[ "x$enable_lua" = "xyes" ]]; then
AC_DEFINE(HAVE_LUA, 1, [Lua support is enabled])
fi

if [[ "x$enable_speex" = "xyes" ]]; then
dnl Check for Speex
AC_CHECK_HEADERS(speex/speex.h, [
  AC_CHECK_LIB(speex, speex_decoder_init, [
    LIBS="-lspeex $LIBS"
    AC_DEFINE(SPEEX, 1, [Speex support is enabled])
  ])
  AC_CHECK_LIB(speexdsp, speex_preprocess_state_init, [
  LIBS="-lspeexdsp $LIBS"
])
])
fi

if [[ "x$enable_alsa" = "xyes" ]]; then
dnl Check for ALSA
AC_CHECK_HEADERS([alsa/asoundlib.h], [
  AC_CHECK_LIB(asound, snd_pcm_open, [
    LIBS="-lasound $LIBS"
    AC_DEFINE(HAVE_ALSA, 1, [ALSA netmic is enabled])
  ])
])
fi

dnl Figure out which OpenGL library to use
case "$target" in
  *-*-cygwin* | *-*-mingw32*)
    SYS_GL_LIBS="-lGLEW -lopengl32 -lglu32"
    LIBS="$LIBS -lstdc++"
    ;;
  *-*-darwin*)
    SYS_GL_LIBS="-F/System/Library/Frameworks -framework AGL -framework OpenGL"
    CPPFLAGS="$CPPFLAGS -D__DARWIN__ -F/System/Library/Frameworks  -I/System/Library/Frameworks/OpenGL.framework/Headers"
    ;;
  *-*-linux*)
    SYS_GL_LIBS="-lGL -lpthread"	
    ;;	
  *)
    SYS_GL_LIBS="-lGL"
    ;;
esac

dnl Check for OpenGL.
if [[ "x$enable_opengl" = "xyes" ]]; then
  AC_MSG_CHECKING(for OpenGL support)
  have_opengl=no
  AC_TRY_COMPILE([
   #ifndef __DARWIN__
   #include <GL/gl.h>
   #else
   #include <OpenGL/gl.h>
   #endif
  ],[
  ],[
  have_opengl=yes
  ])
  AC_MSG_RESULT($have_opengl)
  if test x$have_opengl = xyes; then
    AC_DEFINE(HAVE_OPENGL, 1, [OpenGL support is enabled])
    AC_CHECK_LIB(GLU, gluScaleImage, [SYS_GL_LIBS="$SYS_GL_LIBS -lGLU"], , $SYS_GL_LIBS)
    LIBS="$LIBS $SYS_GL_LIBS"
    AC_CHECK_HEADERS(GL/glext.h, [], [], [#include <GL/gl.h>])
    make_opengl=true
  else
    AC_MSG_WARN([Could not find OpenGL, ignoring --enable-opengl.])
    make_opengl=false
  fi
fi
AM_CONDITIONAL(MAKE_OPENGL, test x$make_opengl = xtrue)

dnl Set target system name.
AC_DEFINE_UNQUOTED(TARGET_PLATFORM, "$target_os $target_cpu", [Target platform name])

dnl add some windows goodies
case $target in
*-*-mingw32*)
	make_windows=true
	AC_DEFINE(WIN32_DISABLE_MUSIC, 1, [Win32 music is disabled])
	LIBS="$LIBS -ldsound -lwsock32"
	;;
*)
	make_windows=false ;;
esac
AM_CONDITIONAL(MAKE_WINDOWS, test x$make_windows = xtrue)


dnl Generate Makefiles.
AC_CONFIG_FILES([
Makefile
AlephOne.spec
Source_Files/Makefile
Source_Files/CSeries/Makefile
Source_Files/Expat/Makefile
Source_Files/FFmpeg/Makefile
Source_Files/Files/Makefile
Source_Files/GameWorld/Makefile
Source_Files/Input/Makefile
Source_Files/LibNAT/Makefile
Source_Files/Lua/Makefile
Source_Files/Misc/Makefile
Source_Files/ModelView/Makefile
Source_Files/Network/Makefile
Source_Files/Network/Metaserver/Makefile
Source_Files/RenderMain/Makefile
Source_Files/RenderOther/Makefile
Source_Files/Sound/Makefile
Source_Files/TCPMess/Makefile
Source_Files/XML/Makefile
tools/Makefile
data/Makefile
data/default_theme/Makefile
])
AC_OUTPUT

dnl Print summary.
echo "Configuration done. Now type \"make\"."
