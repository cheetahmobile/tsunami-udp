# -*- shell-script -*-
#
# Copyright (c) 2001-2002 The Trustees of Indiana University.  
#                         All rights reserved.
# 
# This file is part of the tsunami software package.  For license
# information, see the LICENSE file in the top level directory of the
# tsunami source distribution.
#
# $Id: configure.ac,v 1.3 2013/08/15 16:02:03 jwagnerhki Exp $
#

#
# Init autoconf
#


AC_INIT([tsunami], [1.1b43])

echo "Configuring Tsunami version AC_PACKAGE_VERSION"

#
# Announce
#

#
# Init automake
#
AM_INIT_AUTOMAKE([foreign])
AM_CONFIG_HEADER(config.h:config.in)

#
# Make automake clean emacs ~ files for "make clean"
#

CLEANFILES="*~"
AC_SUBST(CLEANFILES)

#
# This is useful later
#

AC_CANONICAL_HOST

# Determine what the compiler, linker, and library tools are.  Issue a
# big warning if it's not gcc.  Workaround for autoconf annoyance --
# save the CFLAGS so that AC_PROG_CC doesn't overwrite them.

CFLAGS_save="$CFLAGS"
AC_PROG_CC
if test "$GCC" != "yes"; then
    AC_MSG_WARN([*** WARNING! *** You are not using the GNU C compiler.])
    AC_MSG_WARN([*** WARNING! *** Tsunami may fail to compile due to])
    AC_MSG_WARN([*** WARNING! *** extensive use of the "long long" type.])
    AC_MSG_WARN([*** WARNING! *** You have been warned.])
fi
AC_PROG_RANLIB
CFLAGS="$CFLAGS_save"
unset CFLAGS_save

#
# Some CFLAGS that we need
#

EXTRA_CFLAGS="-O3 -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64"

#
# Check for command line flags
#

#AC_ARG_ENABLE([debug], 
#    [  --enable-debug          Enable compile-time debugging])
#if test "$enable_debug" = "yes"; then
#    AC_MSG_WARN(["-g -Wall" has been added to the C compiler flags])
EXTRA_CFLAGS="-g -Wall $EXTRA_CFLAGS"
#fi
CFLAGS="$CFLAGS $EXTRA_CFLAGS"

#
# Look for required libraries
#

AC_CHECK_LIB(pthread, main, [HAPPY=1], [HAPPY=0])
if test "$HAPPY" = "0"; then
    AC_MSG_WARN([Could not find the pthread library])
    AC_MSG_ERROR([Cannot continue])
fi

# version info needed for .spec file generation
version=AC_PACKAGE_VERSION
AC_SUBST(version)

#
# Party on
#

AC_OUTPUT([
    Makefile
    tsunami.spec
    client/Makefile
    rtclient/Makefile
    common/Makefile
    include/Makefile
    server/Makefile
    rtserver/Makefile
    util/Makefile
])
