# -*- makefile -*-
#
# Copyright (c) 2001-2002 The Trustees of Indiana University.  
#                         All rights reserved.
# 
# This file is part of the Tsunami software package.  For license
# information, see the LICENSE file in the top level directory of the
# Tsunami source distribution.
#
# $Id: Makefile.am,v 1.3 2013/08/15 14:56:53 jwagnerhki Exp $
#

AM_CPPFLAGS		= -I$(top_srcdir)/include

common_lib		= $(top_builddir)/util/libtsunami_common.a

noinst_PROGRAMS		= readtest writetest fusereadtest
readtest_SOURCES	= readtest.c
#readtest_LDADD		= $(common_lib)
#readtest_DEPENDENCIES	= $(common_lib)

writetest_SOURCES	= writetest.c
#writetest_LDADD	= $(common_lib)
#writetest_DEPENDENCIES	= $(common_lib)

fusereadtest_SOURCES	= fusereadtest.c
