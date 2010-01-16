#!/bin/sh
# Run this to generate all the initial makefiles, etc.
# This was lifted from the Gimp, and adapted slightly by
# Raph Levien, slightly hacked for xine by Daniel Caujolle-Bert.

DIE=0

PROG=alephone

# Check how echo works in this /bin/sh
case `echo -n` in
-n) _echo_n=   _echo_c='\c';;
*)  _echo_n=-n _echo_c=;;
esac

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
        echo
        echo "You must have autoconf installed to compile $PROG."
        echo "Download the appropriate package for your distribution,"
        echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
        DIE=1
}

(automake --version) < /dev/null > /dev/null 2>&1 || {
        echo
        echo "You must have automake installed to compile $PROG."
        echo "Get ftp://ftp.gnu.org/pub/gnu/automake-1.3.tar.gz"
        echo "(or a newer version if it is available)"
        DIE=1
}

(aclocal --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "**Error**: Missing aclocal. The version of automake"
	echo "installed doesn't appear recent enough."
	echo "Get ftp://ftp.gnu.org/pub/gnu/automake-1.3.tar.gz"
	echo "(or a newer version if it is available)"
	DIE=1
}

if [ "$DIE" -eq 1 ]; then
        exit 1
fi

rm -rf autom4te.cache

aclocalinclude="$ACLOCAL_FLAGS"; \
(echo $_echo_n " + Running aclocal: $_echo_c"; \
    aclocal $aclocalinclude; \
 echo "done.") && \
(echo $_echo_n " + Running autoheader: $_echo_c"; \
    autoheader; \
 echo "done.") && \
(echo $_echo_n " + Running automake: $_echo_c"; \
    automake --gnu --add-missing --copy; \
 echo "done.") && \
(echo $_echo_n " + Running autoconf: $_echo_c"; \
    autoconf; \
 echo "done.")

rm -f config.cache

if [ x"$NO_CONFIGURE" = "x" ]; then
    echo " + Running 'configure $@':"
    if [ -z "$*" ]; then
	echo "   ** If you wish to pass arguments to ./configure, please"
        echo "   ** specify them on the command line."
    fi
    ./configure "$@"
fi
