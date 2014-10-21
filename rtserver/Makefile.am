# -*- makefile -*-
#
# Copyright (c) 2001-2002 The Trustees of Indiana University.  
#                         All rights reserved.
# 
# This file is part of the Tsunami software package.  For license
# information, see the LICENSE file in the top level directory of the
# Tsunami source distribution.
#
# $Id: Makefile.am,v 1.7 2013/08/15 14:56:53 jwagnerhki Exp $
#

AM_CPPFLAGS		= -I$(top_srcdir)/include

common_lib		= $(top_builddir)/common/libtsunami_common.a

bin_PROGRAMS		= rttsunamid

rttsunamid_SOURCES	= \
			config.c \
			io.c \
			log.c \
			main.c \
			network.c \
			protocol.c \
			transcript.c \
			vsibctl.c \
			parse_evn_filename.c \
			parse_evn_filename.h \
			registration.h \
			vsibctl.h \
			vsib_ioctl.h

rttsunamid_LDADD	= $(common_lib) -lm
rttsunamid_DEPENDENCIES	= $(common_lib)
AM_CFLAGS		= -DVSIB_REALTIME
