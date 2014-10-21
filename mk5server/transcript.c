/*========================================================================
 * transcript.c  --  Transcript generation routines for Tsunami server.
 *
 * This contains the code that logs Tsunami transfer statistics
 * available on the server to disk for later analysis.
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
 *
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

#include <tsunami-server.h>


/*------------------------------------------------------------------------
 * void xscript_close(ttp_session_t *session, u_int64_t delta);
 *
 * Closes the transcript file for the given session after writing out
 * the final transfer statistics.
 *------------------------------------------------------------------------*/
void xscript_close(ttp_session_t *session, u_int64_t delta)
{
    ttp_transfer_t  *xfer  = &session->transfer;
    ttp_parameter_t *param =  session->parameter;

    fprintf(xfer->transcript, "mb_transmitted = %0.2f\n", param->file_size / (1024.0 * 1024.0));
    fprintf(xfer->transcript, "duration = %0.2f\n", delta / 1000000.0);
    fprintf(xfer->transcript, "throughput = %0.2f\n", param->file_size * 8.0 / delta);
    fclose(xfer->transcript);
}


/*------------------------------------------------------------------------
 * void xscript_data_log(ttp_session_t *session, const char *logline);
 *------------------------------------------------------------------------*/
void xscript_data_log(ttp_session_t *session, const char *logline)
{
    fprintf(session->transfer.transcript, "%s", logline);
}


/*------------------------------------------------------------------------
 * void xscript_data_start(ttp_session_t *session,
 *                         const struct timeval *epoch);
 *
 * Begins the data section of the transcript with a "START" line
 * containing the given epoch.
 *------------------------------------------------------------------------*/
void xscript_data_start(ttp_session_t *session, const struct timeval *epoch)
{
    fprintf(session->transfer.transcript, "START %lu.%06lu\n", epoch->tv_sec, epoch->tv_usec);
}


/*------------------------------------------------------------------------
 * void xscript_data_stop(ttp_session_t *session,
 *                        const struct timeval *epoch);
 *
 * Terminates the data section of the transcript with a "STOP" line
 * containing the given epoch.
 *------------------------------------------------------------------------*/
void xscript_data_stop(ttp_session_t *session, const struct timeval *epoch)
{
    fprintf(session->transfer.transcript, "STOP %lu.%06lu\n\n", epoch->tv_sec, epoch->tv_usec);
}


/*------------------------------------------------------------------------
 * void xscript_open(ttp_session_t *session);
 *
 * Opens a new transcript file for the given session and writes the
 * initial transcript information to the file.
 *------------------------------------------------------------------------*/
void xscript_open(ttp_session_t *session)
{
    ttp_transfer_t  *xfer  = &session->transfer;
    ttp_parameter_t *param =  session->parameter;
    char             filename[64];

    /* open the transcript file */
    make_transcript_filename(filename, param->epoch, "tsus");
    xfer->transcript = fopen(filename, "w");
    if (xfer->transcript == NULL) {
	warn("Could not create transcript file");
	return;
    }

    /* write out all the header information */
    fprintf(xfer->transcript, "filename = %s\n",    xfer->filename);
    fprintf(xfer->transcript, "file_size = %llu\n", param->file_size);
    fprintf(xfer->transcript, "block_count = %u\n", param->block_count);
    fprintf(xfer->transcript, "udp_buffer = %u\n",  param->udp_buffer);
    fprintf(xfer->transcript, "block_size = %u\n",  param->block_size);
    fprintf(xfer->transcript, "target_rate = %u\n", param->target_rate);
    fprintf(xfer->transcript, "error_rate = %u\n",  param->error_rate);
    fprintf(xfer->transcript, "slower_num = %u\n",  param->slower_num);
    fprintf(xfer->transcript, "slower_den = %u\n",  param->slower_den);
    fprintf(xfer->transcript, "faster_num = %u\n",  param->faster_num);
    fprintf(xfer->transcript, "faster_den = %u\n",  param->faster_den);
    fprintf(xfer->transcript, "ipd_time = %u\n",    param->ipd_time);
    fprintf(xfer->transcript, "ipd_current = %u\n", xfer->ipd_current);
    fprintf(xfer->transcript, "version = 0x%x\n",   PROTOCOL_REVISION);
    fprintf(xfer->transcript, "ipv6 = %u\n",        param->ipv6_yn);
    fprintf(xfer->transcript, "\n");
}


/*========================================================================
 * $Log: transcript.c,v $
 * Revision 1.1  2007/05/31 09:32:07  jwagnerhki
 * removed some signedness warnings, added Mark5 server devel start code
 *
 * Revision 1.2  2006/10/24 19:14:28  jwagnerhki
 * moved server.h into common tsunami-server.h
 *
 * Revision 1.1.1.1  2006/07/20 09:21:20  jwagnerhki
 * reimport
 *
 * Revision 1.1  2006/07/10 12:37:21  jwagnerhki
 * added to trunk
 *
 */
