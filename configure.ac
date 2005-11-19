dnl Process this file with autoconf to produce a configure script.
dnl Written in 2000 by Christian Bauer <Christian.Bauer@uni-mainz.de>

AC_INIT([Aleph One/SDL], 20051119, [http://sourceforge.net/bugs/?group_id=1997], [AlephOne])
AC_CONFIG_SRCDIR([Source_Files/shell.cpp])
AC_PREREQ(2.52)

dnl Detect the canonical host and target build environment.
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

dnl Init automake.
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

dnl Configure options.
AC_ARG_ENABLE(opengl, [  --enable-opengl         use OpenGL for rendering [default=yes]], , enable_opengl=yes)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_INSTALL
AC_PROG_RANLIB

case $target in
*-*-mingw32*)
	AC_CHECK_TOOL(WINDRES, windres, :)
	;;
*)
	;;
esac

dnl Checks for headers.
AC_CHECK_HEADERS(unistd.h)

dnl Checks for library functions.
AC_CHECK_FUNC(snprintf, [
  AC_DEFINE(HAVE_SNPRINTF, 1, [snprintf() is available])
])
AC_CHECK_FUNC(vsnprintf, [
  AC_DEFINE(HAVE_VSNPRINTF, 1, [vsnprintf() is available])
])

dnl Check for SDL.
AM_PATH_SDL(1.2.0, , AC_MSG_ERROR([You need SDL 1.2 to run Aleph One.]))
CPPFLAGS="$CPPFLAGS $SDL_CFLAGS -DSDL"
LIBS="$LIBS $SDL_LIBS"

dnl Check for SDL_image.
AC_CHECK_HEADERS(SDL_image.h, [
  AC_CHECK_LIB(SDL_image, IMG_Load, [
    LIBS="$LIBS -lSDL_image"
    AC_DEFINE(HAVE_SDL_IMAGE, 1, [SDL_image support is enabled])
  ])
])

dnl Check for SDL_net.
AC_CHECK_HEADERS(SDL_net.h, [
  AC_CHECK_LIB(SDL_net, SDLNet_Init, [
    LIBS="$LIBS -lSDL_net"
    AC_DEFINE(HAVE_SDL_NET, 1, [SDL_net support is enabled])
  ])
])
if [[ "x$ac_cv_lib_SDL_net_SDLNet_Init" != xyes ]]; then
  AC_MSG_ERROR([You need SDL_net to run Aleph One.])
fi
AM_CONDITIONAL(HAVE_SDL_NET, test x$ac_cv_lib_SDL_net_SDLNet_Init = xyes)

dnl Check for boost/bind
AC_LANG_PUSH(C++)
AC_CHECK_HEADER([boost/bind.hpp], , [AC_MSG_ERROR([You need boost/bind.hpp from the boost library to run Aleph One])])
AC_LANG_POP(C++)

dnl Check for boost/function
AC_LANG_PUSH(C++)
AC_CHECK_HEADER([boost/function.hpp], , [AC_MSG_ERROR([You need boost/function.hpp from the boost library to run Aleph One])])
AC_LANG_POP(C++)

dnl Check for SDL_sound
AC_CHECK_HEADERS(SDL_sound.h, [
  AC_CHECK_LIB(SDL_sound, Sound_Init, [
    LIBS="$LIBS -lSDL_sound"
    AC_DEFINE(HAVE_SDL_SOUND, 1, [SDL_sound support is enabled])
  ])
])

dnl Check for Lua.
AC_CHECK_HEADERS(lua.h, [
  AC_CHECK_HEADERS(lualib.h, [
    AC_CHECK_LIB(lua, lua_open, [
      AC_CHECK_LIB(lualib, luaL_loadbuffer, [
        LIBS="$LIBS -llua -llualib"
        AC_DEFINE(HAVE_LUA, 1, [Lua support is enabled])
      ], [], [-llua])
    ])
  ])
])

dnl Check for Speex
AC_CHECK_HEADERS(speex.h, [
  AC_CHECK_LIB(speex, speex_decoder_init, [
    LIBS="$LIBS -lspeex"
    AC_DEFINE(SPEEX, 1, [Speex support is enabled])
  ])
])

dnl Figure out which OpenGL library to use
case "$target" in
  *-*-cygwin* | *-*-mingw32*)
    SYS_GL_LIBS="-lopengl32 -lglu32"
    LIBS="$LIBS -lstdc++"
    ;;
  *-*-beos*)
    SYS_GL_LIBS="-lGL"
    ;;
  *-*-darwin*)
    SYS_GL_LIBS="-F/System/Library/Frameworks -framework AGL -framework OpenGL"
    CPPFLAGS="$CPPFLAGS -D__DARWIN__ -F/System/Library/Frameworks  -I/System/Library/Frameworks/OpenGL.framework/Headers"
    ;;
  *-*-linux*)
    SYS_GL_LIBS="-lGL -lpthread"
    CPPFLAGS="$CPPFLAGS -I/usr/X11R6/include -L/usr/X11R6/lib"
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
  else
    AC_MSG_WARN([Could not find OpenGL, ignoring --enable-opengl.])
  fi
fi

dnl Set target system name.
AC_DEFINE_UNQUOTED(TARGET_PLATFORM, "$target_os $target_cpu", [Target platform name])

dnl add some windows goodies
case $target in
*-*-mingw32*)
	make_windows=true
	AC_DEFINE(WIN32_DISABLE_MUSIC, 1, [Win32 music is disabled])
	LIBS="$LIBS -ldsound -lwsock32"
	LDFLAGS="$LDFLAGS -s"
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
Source_Files/Files/Makefile
Source_Files/GameWorld/Makefile
Source_Files/Input/Makefile
Source_Files/Lua/Makefile
Source_Files/Misc/Makefile
Source_Files/ModelView/Makefile
Source_Files/Network/Makefile
Source_Files/Network/Metaserver/Makefile
Source_Files/Pfhortran/Makefile
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
