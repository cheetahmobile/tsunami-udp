# -*- makefile -*-
#
# Copyright (c) 2001-2002 The Trustees of Indiana University.  
#                         All rights reserved.
# 
# This file is part of the Tsunami software package.  For license
# information, see the LICENSE file in the top level directory of the
# Tsunami source distribution.
#
# $Id: Makefile.am,v 1.6 2013/08/15 14:56:53 jwagnerhki Exp $
#

AM_CPPFLAGS		= -I$(top_srcdir)/include

common_lib		= $(top_builddir)/common/libtsunami_common.a

bin_PROGRAMS		= rttsunami

rttsunami_SOURCES		= \
			command.c \
			config.c \
			io.c \
			main.c \
			network.c \
			protocol.c \
			ring.c \
			vsibctl.c \
			transcript.c \
			vsibctl.h \
			vsib_ioctl.h

rttsunami_LDADD		= $(common_lib) -lpthread
rttsunami_DEPENDENCIES	= $(common_lib)
AM_CFLAGS               = -DVSIB_REALTIME # -DRETX_REQBLOCK_SORTING too slow
