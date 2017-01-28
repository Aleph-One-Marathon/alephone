#!/bin/sh
PREFIX=/home/local/mxe/usr
TARGET=i686-w64-mingw32.static
LIBS="-lFLAC -liphlpapi" ./configure --host=$TARGET

