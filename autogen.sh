#! /bin/sh
aclocal
autoheader
autoconf
automake
./configure $*
