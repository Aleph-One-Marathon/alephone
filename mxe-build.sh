#!/bin/sh
PREFIX=/home/local/mxe/usr
TARGET=i686-w64-mingw32.static
PATH="$PREFIX/bin:$PREFIX/$TARGET/bin:$PATH"

if [ -f "$PREFIX/$TARGET/bin/$TARGET-sdl-config" ]; then
    SDL_CONFIG="$PREFIX/$TARGET/bin/$TARGET-sdl-config"
    export SDL_CONFIG
fi
PKG_CONFIG_LIBDIR=$PREFIX/$TARGET/lib/pkgconfig
export PKG_CONFIG_LIBDIR
export PATH
LIBS="-lvorbis -logg -lFLAC -lvorbisenc  -lSDL2 -lfreetype -ljpeg -lpng -lwebp -lz -lbz2 -lstdc++ -ltiff -lharfbuzz -lglib-2.0 -lole32 -lintl -liconv -lwinmm -liphlpapi -lws2_32 -lwsock32" LDFLAGS="-L$PREFIX/i686-pc/mingw32/lib -Wl,-S" ./configure --host=$TARGET --build=i386-linux CPPFLAGS="-I$PREFIX/i686-pc-mingw32/include -I$PREFIX/include"

