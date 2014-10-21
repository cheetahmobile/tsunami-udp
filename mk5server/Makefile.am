# -*- makefile -*-
#
# Copyright (c) 2001-2002 The Trustees of Indiana University.  
#                         All rights reserved.
# 
# This file is part of the Tsunami software package.  For license
# information, see the LICENSE file in the top level directory of the
# Tsunami source distribution.
#
# $Id: Makefile.am,v 1.4 2007/05/31 12:50:26 jwagnerhki Exp $
#

INCLUDES		= -I$(top_srcdir)/include -L$(top_builddir)/common

common_lib      = -lm $(top_builddir)/common/libtsunami_common.a -lm $(top_builddir)/common/libssapi.a -lpthread

bin_PROGRAMS	= mk5tsunamid

mk5tsunamid_SOURCES	= \
			config.c \
			io.c \
			log.c \
			main.c \
			network.c \
			protocol.c \
			transcript.c \
			mk5api.c \
			server.h
mk5tsunamid_LDADD	= $(common_lib)
mk5tsunamid_DEPENDENCIES	= $(common_lib)
mk5tsunamid_LDFLAGS     = -L$(top_builddir)/common
mk5tsunamid_CFLAGS = -DMK5SERVER
mk5tsunamid_CXXFLAGS = -DMK5SERVER

# libssapi.a requires g++ instead of gcc, version 3.x or 2.x
#    mk5tsunamid_CC = g++  -- this is not supported by automake
#    Instead, edit final Makefile directly and set "CC = g++" manually.
