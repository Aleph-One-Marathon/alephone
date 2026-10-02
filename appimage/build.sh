#!/bin/bash
set -e
set -x

cd "$(dirname "$0")"

export LD_LIBRARY_PATH="$PWD/ffmpeg/usr/lib:$LD_LIBRARY_PATH"
export PKG_CONFIG_PATH="$PWD/ffmpeg/usr/lib/pkgconfig"

# build FFmpeg from source to avoid unneeded dependencies
# https://github.com/Aleph-One-Marathon/mac-frameworks/blob/master/ffmpeg/ffmpeg.mk

if [ ! -f ffmpeg/usr/lib/pkgconfig/libavcodec.pc ]; then
    cd ffmpeg
    ./configure --prefix="$PWD/usr" \
        --disable-programs \
        --disable-doc \
        --enable-gpl \
        --enable-version3 \
        --disable-static \
        --enable-shared \
        --enable-postproc \
        --enable-libvorbis \
        --enable-libvpx \
        --disable-everything \
        --enable-muxer=webm \
        --enable-encoder=libvorbis \
        --enable-encoder=libvpx_vp8 \
        --enable-demuxer=aiff \
        --enable-demuxer=mp3 \
        --enable-demuxer=mpegps \
        --enable-demuxer=mpegts \
        --enable-demuxer=mpegtsraw \
        --enable-demuxer=mpegvideo \
        --enable-demuxer=ogg \
        --enable-demuxer=wav \
        --enable-parser=mpegaudio \
        --enable-parser=mpegvideo \
        --enable-decoder=adpcm_ima_wav \
        --enable-decoder=adpcm_ms \
        --enable-decoder=gsm \
        --enable-decoder=gsm_ms \
        --enable-decoder=mp1 \
        --enable-decoder=mp1float \
        --enable-decoder=mp2 \
        --enable-decoder=mp2float \
        --enable-decoder=mp3 \
        --enable-decoder=mp3float \
        --enable-decoder=mpeg1video \
        --enable-decoder=pcm_alaw \
        --enable-decoder=pcm_f32be \
        --enable-decoder=pcm_f32le \
        --enable-decoder=pcm_f64be \
        --enable-decoder=pcm_f64le \
        --enable-decoder=pcm_mulaw \
        --enable-decoder=pcm_s8 \
        --enable-decoder=pcm_s8_planar \
        --enable-decoder=pcm_s16be \
        --enable-decoder=pcm_s16le \
        --enable-decoder=pcm_s16le_planar \
        --enable-decoder=pcm_s24be \
        --enable-decoder=pcm_s24le \
        --enable-decoder=pcm_s32be \
        --enable-decoder=pcm_s32le \
        --enable-decoder=pcm_u8 \
        --enable-decoder=theora \
        --enable-decoder=vorbis \
        --enable-decoder=vp8 \
        --enable-protocol=file \
    | tee config.log
    make -j$(nproc)
    make install
    cd ..
fi

# build AlephOne
if [ ! -f build/Source_Files/alephone ]; then
    rm -rf build
    mkdir build
    cd build
    test -f ../../configure || autoreconf -if ../..
    CFLAGS=-O2 CXXFLAGS=-O2 LDFLAGS=-s ../../configure --without-smpeg
    make -j$(nproc) V=0
    cd ..
fi

# create AppDirs
rm -rf out
mkdir -p out/appdir/usr/bin out/appdir/usr/share/doc/{alephone,ffmpeg}
cp build/Source_Files/alephone out/appdir/usr/bin
cp ../{AUTHORS,COPYING,THANKS} out/appdir/usr/share/doc/alephone
cp ffmpeg/{*.md,MAINTAINERS,Changelog} out/appdir/usr/share/doc/ffmpeg
cp -r out/appdir out/alephone.appdir
cp -r out/appdir out/marathon.appdir
cp -r out/appdir out/marathon2.appdir
cp -r out/appdir out/marathon-infinity.appdir
cp -r ../data/Scenarios/Marathon out/marathon.appdir/usr/share
cp -r ../data/Scenarios/"Marathon 2" out/marathon2.appdir/usr/share
cp -r ../data/Scenarios/"Marathon Infinity" out/marathon-infinity.appdir/usr/share

# create AppImages
cd out

../linuxdeploy.AppImage -oappimage --appdir=alephone.appdir \
    --desktop-file=../data/alephone.desktop \
    --icon-file=../data/alephone.png

../linuxdeploy.AppImage -oappimage --appdir=marathon.appdir \
    --desktop-file=../data/marathon.desktop \
    --icon-file=../data/marathon.png \
    --custom-apprun=../data/apprun-marathon.sh

../linuxdeploy.AppImage -oappimage --appdir=marathon2.appdir \
    --desktop-file=../data/marathon2.desktop \
    --icon-file=../data/marathon2.png \
    --custom-apprun=../data/apprun-marathon2.sh

../linuxdeploy.AppImage -oappimage --appdir=marathon-infinity.appdir \
    --desktop-file=../data/marathon-infinity.desktop \
    --icon-file=../data/marathon-infinity.png \
    --custom-apprun=../data/apprun-marathon-infinity.sh
