dnl Process this file with autoconf to produce a configure script.
dnl Written by Christian Bauer and the Aleph One developers.

AC_INIT([Aleph One],
        m4_esyscmd([ grep '^#define A1_DATE_VERSION' Source_Files/Misc/alephversion.h | sed -e 's/\(.*\"\)\(.*\)\(\"\)/\2/g' | tr -d '\n']),
        [http://sourceforge.net/p/marathon/bugs/],
        [AlephOne],
        [http://marathon.sourceforge.net/])
AC_CONFIG_SRCDIR([Source_Files/shell.cpp])
AC_CONFIG_MACRO_DIR([m4])
AC_PREREQ([2.65])

dnl Detect the canonical host and target build environment.
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

dnl Init automake.
AM_INIT_AUTOMAKE([subdir-objects])
AM_CONFIG_HEADER([config.h])

dnl Configure options.

dnl AX_ARG_ENABLE(option, desc)
AC_DEFUN([AX_ARG_ENABLE],
         [ desc_$1="$2"
           AC_ARG_ENABLE([$1], AS_HELP_STRING([--disable-$1], [do not include $2])) ])
dnl AX_ARG_WITH(option)
AC_DEFUN([AX_ARG_WITH],
         [ desc_$1="$2"
           AC_ARG_WITH([$1], AS_HELP_STRING([--without-$1], [do not use $2])) ])

AX_ARG_ENABLE([opengl], [OpenGL rendering])
AX_ARG_ENABLE([lua], [built-in Lua scripting])

AX_ARG_WITH([sdl_image], [SDL_image support])
AX_ARG_WITH([ffmpeg], [FFmpeg playback and film export])
AX_ARG_WITH([mad], [libmad MP3 playback])
AX_ARG_WITH([sndfile], [libsndfile audio decoding])
AX_ARG_WITH([vorbis], [libvorbis Ogg/Vorbis music playback])
AX_ARG_WITH([smpeg], [SMPEG movie playback])
AX_ARG_WITH([speex], [Speex net mic playback])
AX_ARG_WITH([alsa], [ALSA net mic transmission])
AX_ARG_WITH([curl], [cURL for HTTP communication])
AX_ARG_WITH([zzip], [zziplib support])
AX_ARG_WITH([png], [libpng PNG screenshot support])


dnl Check for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_OBJCXX
AC_PROG_INSTALL
AC_PROG_RANLIB

dnl Some platform specific stuff.
AS_CASE([$target],
        [*-*-mingw32*],
        [ make_windows=true
          ax_boost_user_system_lib="boost_system-mt"
          ax_boost_user_filesystem_lib="boost_filesystem-mt"
          AC_CHECK_TOOL([WINDRES], [windres], [:])
          AC_DEFINE([WIN32_DISABLE_MUSIC], [1], [Win32 music disabled])
          LIBS="$LIBS -ldsound -lwsock32" ],
        [*-*-netbsd*],
        [CPPFLAGS="$CPPFLAGS -I/usr/pkg/include"])
AM_CONDITIONAL([MAKE_WINDOWS], [test "x$make_windows" = "xtrue"])

dnl Set target system name.
AC_DEFINE_UNQUOTED([TARGET_PLATFORM], ["$target_os $target_cpu"], [Target platform name])

dnl Check for headers.
AC_CHECK_HEADERS([unistd.h pwd.h])

dnl Check for boost functions and libraries.
AX_BOOST_BASE([1.53.0],
              ,
              [AC_MSG_ERROR([You need Boost 1.53.0 or higher to build Aleph One.])])
AX_BOOST_SYSTEM
AS_IF([ test "x$BOOST_SYSTEM_LIB" != "x" ],
      [ LIBS="$BOOST_SYSTEM_LIB $LIBS" ],
      [AC_MSG_ERROR([You need the Boost::System library to run Aleph One.])])
AX_BOOST_FILESYSTEM
AS_IF([ test "x$BOOST_FILESYSTEM_LIB" != "x" ],
      [ LIBS="$BOOST_FILESYSTEM_LIB $LIBS" ],
      [AC_MSG_ERROR([You need the Boost::Filesystem library to run Aleph One.])])
AC_LANG_PUSH(C++)
AC_CHECK_HEADER([boost/bind.hpp],
                ,
                [AC_MSG_ERROR([You need boost/bind.hpp from the boost library to run Aleph One.])])
AC_CHECK_HEADER([boost/function.hpp],
                ,
                [AC_MSG_ERROR([You need boost/function.hpp from the boost library to run Aleph One.])])
AC_LANG_POP(C++)

dnl Check for library functions.
AC_CHECK_FUNCS([snprintf vsnprintf sysconf sysctlbyname])
AC_CHECK_FUNC([mkstemp],
              [AC_DEFINE([LUA_USE_MKSTEMP], [1], [mkstemp() available])])

dnl Check for net functions.
AC_SEARCH_LIBS([gethostbyname], [nsl])
AC_SEARCH_LIBS([socket], [socket],
               ,
               [AC_CHECK_LIB([nsl], [socket],
                             [ LIBS="$LIBS -lsocket -lnsl" ],
                             ,
                             [-lsocket])])

dnl Check for libraries.

dnl AX_REQUIRE_LIB(desc, define, header, lib, func)
AC_DEFUN([AX_REQUIRE_LIB],
         [ AC_CHECK_HEADERS([$3], [AC_CHECK_LIB([$4], [$5])])
           AS_IF([test "x${ac_cv_lib_$4_$5}" = "xyes"],
                 [AC_DEFINE([HAVE_$2], [1], [$1 support enabled])],
                 [AC_MSG_ERROR([You need $1 to run Aleph One.])]) ])

dnl Check for required SDL dependencies.
AM_PATH_SDL(1.2.0, , AC_MSG_ERROR([You need SDL 1.2 to run Aleph One.]))
CPPFLAGS="$CPPFLAGS $SDL_CFLAGS -DSDL"
LIBS="$LIBS $SDL_LIBS"

AX_REQUIRE_LIB([SDL_ttf], [SDL_TTF],
               [SDL_ttf.h], [SDL_ttf], [TTF_Init])
AX_REQUIRE_LIB([SDL_net], [SDL_NET],
               [SDL_net.h], [SDL_net], [SDLNet_Init])

dnl Check for zlib.
AX_REQUIRE_LIB([zlib], [ZLIB],
               [zlib.h], [z], [zlibVersion])

dnl Check for OpenGL.
AS_IF([test "x$enable_opengl" != "xno"],
      [ AC_MSG_CHECKING([for OpenGL support])
        compiled_opengl=no
        AS_CASE([$target],
                [*-*-cygwin* | *-*-mingw32*],
                [ SYS_GL_LIBS="-lGLEW -lopengl32 -lglu32"
                  LIBS="$LIBS -lstdc++" ],
                [*-*-darwin*],
                [ SYS_GL_LIBS="-F/System/Library/Frameworks -framework AGL -framework OpenGL"
                  CPPFLAGS="$CPPFLAGS -D__DARWIN__ -F/System/Library/Frameworks -I/System/Library/Frameworks/OpenGL.framework/Headers" ],
                [*-*-linux*],
                [ SYS_GL_LIBS="-lGL -lpthread" ],
                [ SYS_GL_LIBS="-lGL" ])
        AC_TRY_COMPILE([
          #ifndef __DARWIN__
          #include <GL/gl.h>
          #else
          #include <OpenGL/gl.h>
          #endif
          ],
          ,
          [ compiled_opengl=yes ])
        AC_MSG_RESULT($compiled_opengl)
        AS_IF([test "x$compiled_opengl" = "xyes"],
              [ have_opengl=true
                AC_DEFINE([HAVE_OPENGL], [1], [OpenGL support enabled])
                AC_CHECK_LIB([GLU], [gluScaleImage],
                             [SYS_GL_LIBS="$SYS_GL_LIBS -lGLU"],
                             ,
                             [$SYS_GL_LIBS])
                LIBS="$LIBS $SYS_GL_LIBS"
                AC_CHECK_HEADERS([GL/glext.h], [], [], [#include <GL/gl.h>]) ],
              [ AS_IF([test "x$enable_opengl" = "xyes"],
                      [AC_MSG_ERROR([OpenGL support requested but not available])],
                      [AC_MSG_WARN([OpenGL not available])]) ]) ])
AM_CONDITIONAL([MAKE_OPENGL], [test "x$have_opengl" = "xtrue" ])

dnl Enable Lua.
AS_IF([test "x$enable_lua" != "xno"],
      [ have_lua=true
        AC_DEFINE([HAVE_LUA], [1], [Lua support enabled]) ])


dnl Check optional packages.

dnl AX_CHECK_FEATURE_LIB(option, define, header, lib, func)
AC_DEFUN([AX_CHECK_FEATURE_LIB],
         [ AS_IF([test "x${with_$1}" != "xno"],
                 [ AC_CHECK_HEADERS([$3], [AC_CHECK_LIB([$4], [$5])])
                   AS_IF([test "x${ac_cv_lib_$4_$5}" = "xyes"],
                         [ have_$1=true
                           AC_DEFINE([HAVE_$2], [1], [${desc_$1} enabled]) ],
                         [AS_IF([test "x${with_$1}" = "xyes"],
                                [AC_MSG_ERROR([${desc_$1} requested but not found])])])
                 ]) ])
dnl AX_CHECK_FEATURE_PKG(option, define, package, lib)
AC_DEFUN([AX_CHECK_FEATURE_PKG],
         [ AS_IF([test "x${with_$1}" != "xno"],
                 [ PKG_CHECK_MODULES([$3], [$4],
                                     [ CPPFLAGS="${[$3]_CFLAGS} $CPPFLAGS"
                                       LIBS="${[$3]_LIBS} $LIBS"
                                       have_$1=true
                                       AC_DEFINE([HAVE_$2], [1], [${desc_$1} enabled]) ],
                                     [AS_IF([test "x${with_$1}" = "xyes"],
                                            [AC_MSG_ERROR([${desc_$1} requested but not available])])])
           
                 ])
            AM_CONDITIONAL([HAVE_$2], [test "x${have_$1}" = "xtrue"])
          ])

AX_CHECK_FEATURE_LIB([sdl_image], [SDL_IMAGE],
                     [SDL_image.h], [SDL_image], [IMG_Load])

AX_CHECK_FEATURE_PKG([ffmpeg], [FFMPEG],
                     [FFMPEG], [libavcodec >= 54.35.0 libavformat >= 54.20.0 libavutil >= 52.3.0 libswscale >= 2.1.1])

AX_CHECK_FEATURE_LIB([mad], [MAD],
                     [mad.h], [mad], [mad_stream_init])

AX_CHECK_FEATURE_LIB([sndfile], [SNDFILE],
                     [sndfile.h], [sndfile], [sf_open])

AX_CHECK_FEATURE_PKG([vorbis], [VORBISFILE],
                     [VORBISFILE], [vorbisfile])

AX_CHECK_FEATURE_LIB([smpeg], [SMPEG],
                     [smpeg/smpeg.h], [smpeg], [SMPEG_new])

AX_CHECK_FEATURE_LIB([speex], [SPEEX],
                     [speex/speex.h], [speex], [speex_decoder_init])
AS_IF([test "x$have_speex" = "xyes"],
      [AC_CHECK_LIB([speexdsp], [speex_preprocess_state_init])])

AX_CHECK_FEATURE_LIB([alsa], [ALSA],
                     [alsa/asoundlib.h], [asound], [snd_pcm_open])

AX_CHECK_FEATURE_PKG([curl], [CURL],
                     [CURL], [libcurl >= 7.31.0])

AX_CHECK_FEATURE_PKG([zzip], [ZZIP],
                     [ZZIP], [zziplib >= 0.10.75])
AM_CONDITIONAL([BUILD_ZZIP_RWOPS], [test "x$have_zzip" = "xtrue"])

AX_CHECK_FEATURE_PKG([png], [PNG],
                     [PNG], [libpng])


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

dnl Print summary of enabled/disabled options.

dnl AX_PRINT_SUMMARY(option)
AC_DEFUN([AX_PRINT_SUMMARY],
         [ AS_IF([test "x${have_$1}" = "xtrue"],
                 [AS_ECHO(["    Enabled: ${desc_$1}"])],
                 [test "x${enable_$1}" = "xno" ||
                  test "x${with_$1}" = "xno"],
                 [AS_ECHO(["   Disabled: ${desc_$1}"])],
                 [AS_ECHO(["  Not found: ${desc_$1}"])])
         ])

AS_ECHO([""])
AS_ECHO(["Summary of optional features:"])
AS_ECHO([""])
AX_PRINT_SUMMARY([opengl])
AX_PRINT_SUMMARY([lua])
AX_PRINT_SUMMARY([sdl_image])
AX_PRINT_SUMMARY([ffmpeg])
AX_PRINT_SUMMARY([mad])
AX_PRINT_SUMMARY([sndfile])
AX_PRINT_SUMMARY([vorbis])
AX_PRINT_SUMMARY([smpeg])
AX_PRINT_SUMMARY([speex])
AX_PRINT_SUMMARY([alsa])
AX_PRINT_SUMMARY([curl])
AX_PRINT_SUMMARY([zzip])
AX_PRINT_SUMMARY([png])
AS_ECHO([""])
AS_ECHO(["Configuration done. Now type \"make\"."])
