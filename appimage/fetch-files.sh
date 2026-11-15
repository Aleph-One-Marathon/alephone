#!/bin/sh
set -e
set -x

cd "$(dirname "$0")"

wget -q -O linuxdeploy.AppImage -c https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
chmod a+x linuxdeploy.AppImage

VERSION="6.0"

#URL=https://git.ffmpeg.org/ffmpeg.git
URL=https://github.com/FFmpeg/FFmpeg

git clone --depth 1 $URL -b release/$VERSION ffmpeg

if [ "x$(ls ../data/Scenarios/Marathon)" = "x" ]; then
    #git submodule init
    #git submodule update
    cd ../data/
    mv Scenarios Scenarios.bak
    mkdir Scenarios
    cd Scenarios
    git clone --depth 1 https://github.com/Aleph-One-Marathon/data-marathon Marathon
    git clone --depth 1 https://github.com/Aleph-One-Marathon/data-marathon-2 "Marathon 2"
    git clone --depth 1 https://github.com/Aleph-One-Marathon/data-marathon-infinity "Marathon Infinity"
fi
