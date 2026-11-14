#!/bin/sh
set -e
set -x

cd "$(dirname "$0")"

rm -f linuxdeploy.AppImage
rm -rf build ffmpeg out

if [ -d ../data/Scenarios.bak ]; then
    rm -rf ../data/Scenarios
    mv ../data/Scenarios.bak ../data/Scenarios
fi
