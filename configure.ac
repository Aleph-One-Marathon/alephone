dnl Process this file with autoconf to produce a configure script.
dnl Written by Christian Bauer and the Aleph One developers.

AC_INIT([Aleph One],
        m4_esyscmd([ grep '^#define A1_DATE_VERSION' Source_Files/Misc/alephversion.h | sed -e 's/\(.*\"\)\(.*\)\(\"\)/\2/g' | tr -d '\n']),
        [https://github.com/Aleph-One-Marathon/alephone/issues],
        [AlephOne],
        [https://alephone.lhowon.org/])
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

AX_ARG_WITH([sdl_image], [SDL2_image support])
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

dnl Require C++11 or newer and enable C++11 if needed.
AX_CXX_COMPILE_STDCXX([11], [], [mandatory])

dnl Some platform specific stuff.
AS_CASE([$target],
        [*-*-mingw32*],
        [ make_windows=true
          ax_boost_user_system_lib="boost_system-mt"
          ax_boost_user_filesystem_lib="boost_filesystem-mt"
          AC_CHECK_TOOL([WINDRES], [windres], [:])
          AC_DEFINE([WIN32_DISABLE_MUSIC], [1], [Win32 music disabled])
          CPPFLAGS="-D__USE_MINGW_ANSI_STDIO $CPPFLAGS"
          LIBS="$LIBS -ldsound -lwsock32"
          LDFLAGS="$LDFLAGS -Wl,-subsystem,windows" ],
        [*-*-netbsd*],
        [ CPPFLAGS="$CPPFLAGS -I/usr/pkg/include"
          install_mime=true ],
        [*linux*|*bsd*],
        [ install_mime=true ])
AM_CONDITIONAL([MAKE_WINDOWS], [test "x$make_windows" = "xtrue"])
AM_CONDITIONAL([INSTALL_MIME], [test "x$install_mime" = "xtrue"])

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
      
AC_DEFUN([AX_CHECK_BOOST_HEADER],
         [ AC_LANG_PUSH(C++)
           AC_CHECK_HEADER([$1], , [AC_MSG_ERROR([You need $1 from the Boost library to build Aleph One.])])
           AC_LANG_POP(C++) ])

dnl Some checks suppressed to speed configuration.
AX_CHECK_BOOST_HEADER([boost/bind.hpp])
AX_CHECK_BOOST_HEADER([boost/function.hpp])
AX_CHECK_BOOST_HEADER([boost/foreach.hpp])
AX_CHECK_BOOST_HEADER([boost/lexical_cast.hpp])
dnl AX_CHECK_BOOST_HEADER([boost/make_shared.hpp])
dnl AX_CHECK_BOOST_HEADER([boost/scoped_ptr.hpp])
dnl AX_CHECK_BOOST_HEADER([boost/shared_array.hpp])
AX_CHECK_BOOST_HEADER([boost/shared_ptr.hpp])
AX_CHECK_BOOST_HEADER([boost/tokenizer.hpp])
AX_CHECK_BOOST_HEADER([boost/unordered_map.hpp])
dnl AX_CHECK_BOOST_HEADER([boost/utility.hpp])
dnl AX_CHECK_BOOST_HEADER([boost/algorithm/hex.hpp])
dnl AX_CHECK_BOOST_HEADER([boost/algorithm/string/case_conv.hpp])
AX_CHECK_BOOST_HEADER([boost/algorithm/string/predicate.hpp])
dnl AX_CHECK_BOOST_HEADER([boost/algorithm/string/replace.hpp])
dnl AX_CHECK_BOOST_HEADER([boost/iostreams/categories.hpp])
dnl AX_CHECK_BOOST_HEADER([boost/iostreams/device/array.hpp])
dnl AX_CHECK_BOOST_HEADER([boost/iostreams/positioning.hpp])
AX_CHECK_BOOST_HEADER([boost/iostreams/stream_buffer.hpp])
dnl AX_CHECK_BOOST_HEADER([boost/iterator/filter_iterator.hpp])
dnl AX_CHECK_BOOST_HEADER([boost/iterator/transform_iterator.hpp])
dnl AX_CHECK_BOOST_HEADER([boost/property_tree/ini_parser.hpp])
dnl AX_CHECK_BOOST_HEADER([boost/property_tree/ptree.hpp])
AX_CHECK_BOOST_HEADER([boost/property_tree/xml_parser.hpp])
AX_CHECK_BOOST_HEADER([boost/ptr_container/ptr_map.hpp])
dnl AX_CHECK_BOOST_HEADER([boost/range/any_range.hpp])
AX_CHECK_BOOST_HEADER([boost/tuple/tuple.hpp])
dnl AX_CHECK_BOOST_HEADER([boost/tuple/tuple_comparison.hpp])
AX_CHECK_BOOST_HEADER([boost/uuid/uuid.hpp])
dnl AX_CHECK_BOOST_HEADER([boost/uuid/uuid_generators.hpp])
dnl AX_CHECK_BOOST_HEADER([boost/uuid/uuid_io.hpp])

dnl Check for library functions.
AC_CHECK_FUNCS([snprintf vsnprintf], , AC_MSG_ERROR([You need snprintf and vsnprintf to run Aleph One.]))     
AC_CHECK_FUNCS([sysconf sysctlbyname])
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

dnl AX_REQUIRE_PKG(desc, define, modules)
AC_DEFUN([AX_REQUIRE_PKG],
         [ PKG_CHECK_MODULES([$2], [$3],
                             [ CPPFLAGS="${[$2]_CFLAGS} $CPPFLAGS"
                               LIBS="${[$2]_LIBS} $LIBS"
                               have_$1=true
                               AC_DEFINE([HAVE_$2], [1], [$1 support enabled]) ],
                             [ AC_MSG_ERROR([You need $1 to run Aleph One.]) ]) ])

dnl Check for required SDL dependencies.
AM_PATH_SDL2(2.0.2, , AC_MSG_ERROR([You need SDL 2 to run Aleph One.]))
CPPFLAGS="$CPPFLAGS $SDL_CFLAGS -DSDL"
LIBS="$LIBS $SDL_LIBS"

AX_REQUIRE_PKG([SDL2_ttf], [SDL_TTF], [SDL2_ttf])
AX_REQUIRE_PKG([SDL2_net], [SDL_NET], [SDL2_net])

dnl Check for zlib.
AX_REQUIRE_PKG([zlib], [ZLIB], [zlib])

dnl Check for Expat.
have_expat_h=no
have_libexpat=no
AC_CHECK_HEADER([expat.h], [have_expat_h=yes])
AC_CHECK_LIB([expat], [XML_ParserCreate], [have_libexpat=yes])
AM_CONDITIONAL([BUILD_EXPAT], [test "x$have_expat_h" != "xyes" -o "x$have_libexpat" != "xyes"])

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

AX_CHECK_FEATURE_PKG([sdl_image], [SDL_IMAGE],
                     [SDL_IMAGE], [SDL2_image])

AX_CHECK_FEATURE_PKG([ffmpeg], [FFMPEG],
                     [FFMPEG], [libavcodec >= 54.35.0 libavformat >= 54.20.0 libavutil >= 52.3.0 libswscale >= 2.1.1])
dnl Remove SDL 1 inclusions from ffmpeg (https://trac.ffmpeg.org/ticket/2009)
FFMPEG_LIBS=`echo "$FFMPEG_LIBS" | sed 's|-lSDL ||' | sed 's|-lSDLmain ||'`
LIBS=`echo "$LIBS" | sed 's|-lSDL ||' | sed 's|-lSDLmain ||'`

AX_CHECK_FEATURE_LIB([mad], [MAD],
                     [mad.h], [mad], [mad_stream_init])

AX_CHECK_FEATURE_PKG([sndfile], [SNDFILE],
                     [SNDFILE], [sndfile])

AX_CHECK_FEATURE_PKG([vorbis], [VORBISFILE],
                     [VORBISFILE], [vorbisfile])

AX_CHECK_FEATURE_LIB([smpeg], [SMPEG],
                     [smpeg/smpeg.h], [smpeg], [SMPEG_new])

AX_CHECK_FEATURE_PKG([speex], [SPEEX], [speex], [speex])
AS_IF([test "x$have_speex" = "xtrue"],
      [ PKG_CHECK_MODULES([speexdsp], [speexdsp],
              [ CPPFLAGS="-DSPEEX $speexdsp_CFLAGS $CPPFLAGS"
                LIBS="$speexdsp_LIBS $LIBS" ],
              [ have_speex=false
                AS_IF([test "x$with_speex" = "xyes"],
                      [AC_MSG_ERROR([$desc_speex requested but not available])])])])

AX_CHECK_FEATURE_LIB([alsa], [ALSA],
                     [alsa/asoundlib.h], [asound], [snd_pcm_open])

AX_CHECK_FEATURE_PKG([curl], [CURL],
                     [CURL], [libcurl >= 7.31.0])

AX_CHECK_FEATURE_PKG([zzip], [ZZIP],
                     [ZZIP], [zziplib >= 0.10.75])
AM_CONDITIONAL([BUILD_ZZIP_RWOPS], [test "x$have_zzip" = "xtrue"])
dnl Remove hardened flags on Fedora (disrupts static linking)
ZZIP_LIBS=`echo "$ZZIP_LIBS" | sed -E 's|-specs=@<:@^ @:>@+||'`
LIBS=`echo "$LIBS" | sed -E 's|-specs=@<:@^ @:>@+||'`

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
tools/headertest/GNUmakefile
data/Makefile
data/default_theme/Makefile
data/icons/Makefile
])
AC_OUTPUT

dnl Print summary of key compilation variables.

AS_ECHO([""])
AS_ECHO(["Compilation settings:"])
AS_ECHO([""])
AS_ECHO(["         CC: ${CC}"])
AS_ECHO(["        CXX: ${CXX}"])
AS_ECHO(["        CPP: ${CPP}"])
AS_ECHO(["     CFLAGS: ${CFLAGS}"])
AS_ECHO(["   CXXFLAGS: ${CXXFLAGS}"])
AS_ECHO(["   CPPFLAGS: ${CPPFLAGS}"])
AS_ECHO(["    LDFLAGS: ${LDFLAGS}"])
AS_ECHO(["       LIBS: ${LIBS}"])

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
AS_IF([test "x$have_expat_h" != "xyes" -o "x$have_libexpat" != "xyes"],
      AS_ECHO(["    Enabled: internal libexpat"]),
      AS_ECHO(["    Enabled: system libexpat"]) )
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
