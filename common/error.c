/*========================================================================
 * error.c  --  Error-handling routines for Tsunami file transfer.
 *
 * This is the transient process that requests files.
 *
 * Written by Mark Meiss (mmeiss@indiana.edu).
 * Copyright © 2002 The Trustees of Indiana University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1) All redistributions of source code must retain the above
 *    copyright notice, the list of authors in the original source
 *    code, this list of conditions and the disclaimer listed in this
 *    license;
 *
 * 2) All redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the disclaimer
 *    listed in this license in the documentation and/or other
 *    materials provided with the distribution;
 *
 * 3) Any documentation included with all redistributions must include
 *    the following acknowledgement:
 *
 *      "This product includes software developed by Indiana
 *      University`s Advanced Network Management Lab. For further
 *      information, contact Steven Wallace at 812-855-0960."
 *
 *    Alternatively, this acknowledgment may appear in the software
 *    itself, and wherever such third-party acknowledgments normally
 *    appear.
 *
 * 4) The name "tsunami" shall not be used to endorse or promote
 *    products derived from this software without prior written
 *    permission from Indiana University.  For written permission,
 *    please contact Steven Wallace at 812-855-0960.

 * 5) Products derived from this software may not be called "tsunami",
 *    nor may "tsunami" appear in their name, without prior written
 *    permission of Indiana University.
 *
 * Indiana University provides no reassurances that the source code
 * provided does not infringe the patent or any other intellectual
 * property rights of any other entity.  Indiana University disclaims
 * any liability to any recipient for claims brought by any other
 * entity based on infringement of intellectual property rights or
 * otherwise.
 *
 * LICENSEE UNDERSTANDS THAT SOFTWARE IS PROVIDED "AS IS" FOR WHICH
 * NO WARRANTIES AS TO CAPABILITIES OR ACCURACY ARE MADE. INDIANA
 * UNIVERSITY GIVES NO WARRANTIES AND MAKES NO REPRESENTATION THAT
 * SOFTWARE IS FREE OF INFRINGEMENT OF THIRD PARTY PATENT, COPYRIGHT,
 * OR OTHER PROPRIETARY RIGHTS.  INDIANA UNIVERSITY MAKES NO
 * WARRANTIES THAT SOFTWARE IS FREE FROM "BUGS", "VIRUSES", "TROJAN
 * HORSES", "TRAP DOORS", "WORMS", OR OTHER HARMFUL CODE.  LICENSEE
 * ASSUMES THE ENTIRE RISK AS TO THE PERFORMANCE OF SOFTWARE AND/OR
 * ASSOCIATED MATERIALS, AND TO THE PERFORMANCE AND VALIDITY OF
 * INFORMATION GENERATED USING SOFTWARE.
 *========================================================================*/

#include <errno.h>   /* for perror()                */
#include <stdarg.h>  /* for variable argument lists */
#include <stdio.h>   /* for basic I/O routines      */
#include <stdlib.h>  /* for exit()                  */

#include "tsunami.h"


/*------------------------------------------------------------------------
 * Global variables.
 *------------------------------------------------------------------------*/

char g_error[MAX_ERROR_MESSAGE];


/*------------------------------------------------------------------------
 * int error_handler(const char *file, int line, const char *message,
 *                   int fatal_yn);
 *
 * Prints an error message (possibly with file and line number
 * information included).  If fatal_yn is true, also aborts the client.
 * The return value is always non-zero to facilitate brevity in code
 * that propagates error conditions upwards.
 *------------------------------------------------------------------------*/
int error_handler(const char *file, int line, const char *message, int fatal_yn)
{
#ifdef DEBUG
    fprintf(stderr, "Error condition detected in %s, line %d:\n", file, line);
    perror("    ");
#endif

    /* print out the message */
    fprintf(stderr, "%s: %s\n", (fatal_yn ? "Error" : "Warning"), message);
    if (fatal_yn)
	exit(1);
    return -1;
}


/*========================================================================
 * $Log: error.c,v $
 * Revision 1.1.1.1  2006/07/20 09:21:19  jwagnerhki
 * reimport
 *
 * Revision 1.1  2006/07/10 12:27:29  jwagnerhki
 * added to trunk
 *
 */
