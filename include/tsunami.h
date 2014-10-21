/*========================================================================
 * tsunami.h  --  Global header file for the Tsunami protocol suite.
 *
 * This file contains global definitions and function prototypes for
 * the Tsunami protocol suite.
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

#ifndef _TSUNAMI_H
#define _TSUNAMI_H

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64  // TODO: better 'getconf LFS_CFLAGS' in ./configure
#endif

#include <sys/types.h>  /* for u_char, u_int16_t, etc. */
#include <sys/time.h>   /* for struct timeval          */
#include <stdio.h>      /* for NULL, FILE *, etc.      */

#include "tsunami-cvs-buildnr.h"   /* for the current TSUNAMI_CVS_BUILDNR */


/*------------------------------------------------------------------------
 * Macro definitions.
 *------------------------------------------------------------------------*/

#define warn(message)   error_handler(__FILE__, __LINE__, (message), 0)
#define error(message)  error_handler(__FILE__, __LINE__, (message), 1)

#define min(a,b)  (((a) < (b)) ? (a) : (b))
#define max(a,b)  (((a) > (b)) ? (a) : (b))

#define tv_diff_usec(newer,older) ((newer.tv_sec-older.tv_sec)*1e6 + (newer.tv_usec-older.tv_usec))

typedef unsigned long long ull_t;

/*------------------------------------------------------------------------
 * Global constants.
 *------------------------------------------------------------------------*/

#define MAX_ERROR_MESSAGE  512        /* maximum length of an error message */
#define MAX_BLOCK_SIZE     65530      /* maximum size of a data block       */

extern const u_int32_t PROTOCOL_REVISION;

extern const u_int16_t REQUEST_RETRANSMIT;
extern const u_int16_t REQUEST_RESTART;
extern const u_int16_t REQUEST_STOP;
extern const u_int16_t REQUEST_ERROR_RATE;

#define  TS_TCP_PORT    46224   /* default TCP port of the remote server        */
#define  TS_UDP_PORT    46224   /* default UDP port of the client / 47221       */

#define  TS_BLOCK_ORIGINAL          'O'   /* blocktype "original block" */
#define  TS_BLOCK_TERMINATE         'X'   /* blocktype "end transmission" */
#define  TS_BLOCK_RETRANSMISSION    'R'   /* blocktype "retransmitted block" */

#define  TS_DIRLIST_HACK_CMD        "!#DIR??" /* "file name" sent by the client to request a list of the shared files */

/*------------------------------------------------------------------------
 * Data structures.
 *------------------------------------------------------------------------*/

/* retransmission request */
typedef struct {
    u_int16_t           request_type;  /* the retransmission request type           */
    u_int32_t           block;         /* the block number to retransmit {at}       */
    u_int32_t           error_rate;    /* the current error rate (in % x 1000)      */
} retransmission_t;


/*------------------------------------------------------------------------
 * Global variables.
 *------------------------------------------------------------------------*/

extern char g_error[];  /* buffer for the most recent error string    */


/*------------------------------------------------------------------------
 * Function prototypes.
 *------------------------------------------------------------------------*/

/* common.c */
int        get_random_data         (u_char *buffer, size_t bytes);
u_int64_t  get_usec_since          (struct timeval *old_time);
u_int64_t  htonll                  (u_int64_t value);
char      *make_transcript_filename(char *buffer, time_t epoch, const char *extension);
u_int64_t  ntohll                  (u_int64_t value);
u_char    *prepare_proof           (u_char *buffer, size_t bytes, const u_char *secret, u_char *digest);
int        read_line               (int fd, char *buffer, size_t buffer_length);
int        fread_line              (FILE *f, char *buffer, size_t buffer_length);
void       usleep_that_works       (u_int64_t usec);
u_int64_t  get_udp_in_errors       ();
ssize_t    full_write              (int, const void*, size_t);
ssize_t    full_read               (int, void*, size_t);

/* error.c */
int        error_handler           (const char *file, int line, const char *message, int fatal_yn);

#endif
