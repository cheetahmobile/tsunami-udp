/* registration.h -- helps in registering RCS ids of each file */
/* $Id: registration.h,v 1.1 2007/05/31 09:32:07 jwagnerhki Exp $ */
/* Copyright (C) 1995 Ari Mujunen. (amn@nfra.nl, Ari.Mujunen@hut.fi) */

/* This is free software; you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License. */
/* See the file 'COPYING' for details. */

/*
 * $Log: registration.h,v $
 * Revision 1.1  2007/05/31 09:32:07  jwagnerhki
 * removed some signedness warnings, added Mark5 server devel start code
 *
 * Revision 1.1.1.1  2006/07/20 09:21:20  jwagnerhki
 * reimport
 *
 * Revision 1.1  2006/07/10 12:37:21  jwagnerhki
 * added to trunk
 *
 * Revision 1.6  2002/12/18 07:31:03  amn
 * Upped revision.
 *
 * Revision 1.2  2002/12/18 07:31:02  amn
 * intermediate
 *
 * Revision 1.1  2002/12/18 07:23:46  amn
 * Recovered from /home disk crash by re-creating 'common' module in CVS.
 *
 * Revision 1.5  2002/11/03 10:42:22  amn
 * Removing "unused" warnings for gcc-295 and gcc-300.
 *
 * Revision 1.4  1998/05/29 05:51:14  amn
 * Comment leader of log lines changed.
 *
 * Revision 1.3  1997/03/10 08:59:31  amn
 * Had to make use of 'rcsid_##file' more complicated as GCC 2.7.2
 * had acquired new talents in recognizing dummy uses of variables.
 *
 * Revision 1.2  1995/03/20 15:01:04  amn
 * Changed from C++-style comments to ordinary C comments
 * to make this file usable from both C and C++.
 *
 * Revision 1.1  1995/02/11  19:54:27  amn
 * Initial revision
 *
 */


#ifndef _registration_h
#define _registration_h 1

/* By embedding static variables containing the RCS special strings */
/* in the format "$Id xxx$", the resulting object code and executables */
/* can be examined with the command 'ident'.  This macro takes care */
/* of both defining the static variable and using it, so we don't */
/* get extra warnings of unused variables in GCC. */

#define RCSID(file, id) \
static char rcsid_##file[] __attribute__ ((unused)) = id;
/* static void *use_rcsid_##file = (&use_rcsid_##file, (void *)&rcsid_##file) */

RCSID(registration_h, "$Id: registration.h,v 1.1 2007/05/31 09:32:07 jwagnerhki Exp $");

#endif
