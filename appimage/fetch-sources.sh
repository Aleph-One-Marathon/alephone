#!/bin/sh
set -e
set -x

wget -q -O linuxdeploy.AppImage -c https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
chmod a+x linuxdeploy.AppImage

#URL=https://git.ffmpeg.org/ffmpeg.git
URL=https://github.com/FFmpeg/FFmpeg
git clone --depth 1 $URL ffmpeg

set +x
set +e

if [ "x$(ls ../data/Scenarios/Marathon)" = "x" ]; then
    echo "Warning: no game files in \`../data/Scenarios'!"
fi