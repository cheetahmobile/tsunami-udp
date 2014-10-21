/*========================================================================
 * transcript.c  --  Transcript generation routines for Tsunami client.
 *
 * This contains the code that logs Tsunami transfer statistics
 * available on the client to disk for later analysis.
 *
 * Written by Mark Meiss (mmeiss@indiana.edu).
 * Copyright (C) 2002 The Trustees of Indiana University.
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
 * NO WARRANTIES AS TO CAPABILITIES OR ACCURACY ARE MADE. INDIANA
 * UNIVERSITY GIVES NO WARRANTIES AND MAKES NO REPRESENTATION THAT
 * SOFTWARE IS FREE OF INFRINGEMENT OF THIRD PARTY PATENT, COPYRIGHT,
 * OR OTHER PROPRIETARY RIGHTS. INDIANA UNIVERSITY MAKES NO
 * WARRANTIES THAT SOFTWARE IS FREE FROM "BUGS", "VIRUSES", "TROJAN
 * HORSES", "TRAP DOORS", "WORMS", OR OTHER HARMFUL CODE.  LICENSEE
 * ASSUMES THE ENTIRE RISK AS TO THE PERFORMANCE OF SOFTWARE AND/OR
 * ASSOCIATED MATERIALS, AND TO THE PERFORMANCE AND VALIDITY OF
 * INFORMATION GENERATED USING SOFTWARE.
 *========================================================================*/

#include <tsunami-client.h>


/*------------------------------------------------------------------------
 * void xscript_close(ttp_session_t *session, u_int64_t delta);
 *
 * Closes the transcript file for the given session after writing out
 * the final transfer statistics.
 *------------------------------------------------------------------------*/
void xscript_close(ttp_session_t *session, u_int64_t delta)
{
    double mb_thru, mb_good, mb_file, secs;
    ttp_transfer_t *xfer = &session->transfer;

    mb_thru  = xfer->stats.total_blocks * session->parameter->block_size;
    mb_good  = mb_thru - xfer->stats.total_recvd_retransmits * session->parameter->block_size;
    mb_file  = xfer->file_size;
    mb_thru /= (1024.0*1024.0);
    mb_good /= (1024.0*1024.0);
    mb_file /= (1024.0*1024.0);
    secs     = delta/1e6;

    fprintf(xfer->transcript, "mbyte_transmitted = %0.2f\n", mb_thru);
    fprintf(xfer->transcript, "mbyte_usable = %0.2f\n", mb_good);
    fprintf(xfer->transcript, "mbyte_file = %0.2f\n", mb_file);
    fprintf(xfer->transcript, "duration = %0.2f\n", secs);
    fprintf(xfer->transcript, "throughput = %0.2f\n", 8.0 * mb_thru / secs);
    fprintf(xfer->transcript, "goodput_with_restarts = %0.2f\n", 8.0 * mb_good / secs);
    fprintf(xfer->transcript, "file_rate = %0.2f\n", 8.0 * mb_file / secs);
    fclose(xfer->transcript);
}


/*------------------------------------------------------------------------
 * void xscript_data_log(ttp_session_t *session, const char *logline);
 *------------------------------------------------------------------------*/
void xscript_data_log(ttp_session_t *session, const char *logline)
{
    fprintf(session->transfer.transcript, "%s", logline);
    fflush(session->transfer.transcript);
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
    fprintf(session->transfer.transcript, "START %lu.%06lu\n", 
       (unsigned long)epoch->tv_sec, (unsigned long)epoch->tv_usec);
    fflush(session->transfer.transcript);
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
    fprintf(session->transfer.transcript, "STOP %lu.%06lu\n\n", 
       (unsigned long)epoch->tv_sec, (unsigned long)epoch->tv_usec);
    fflush(session->transfer.transcript);
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
    make_transcript_filename(filename, xfer->epoch, "tsuc");
    xfer->transcript = fopen(filename, "w");
    if (xfer->transcript == NULL) {
	warn("Could not create transcript file");
	return;
    }

    /* write out all the header information */
    fprintf(xfer->transcript, "remote_filename = %s\n", xfer->remote_filename);
    fprintf(xfer->transcript, "local_filename = %s\n",  xfer->local_filename);
    fprintf(xfer->transcript, "file_size = %Lu\n",      (ull_t)xfer->file_size);
    fprintf(xfer->transcript, "block_count = %u\n",     xfer->block_count);
    fprintf(xfer->transcript, "udp_buffer = %u\n",      param->udp_buffer);
    fprintf(xfer->transcript, "block_size = %u\n",      param->block_size);
    fprintf(xfer->transcript, "target_rate = %u\n",     param->target_rate);
    fprintf(xfer->transcript, "error_rate = %u\n",      param->error_rate);
    fprintf(xfer->transcript, "slower_num = %u\n",      param->slower_num);
    fprintf(xfer->transcript, "slower_den = %u\n",      param->slower_den);
    fprintf(xfer->transcript, "faster_num = %u\n",      param->faster_num);
    fprintf(xfer->transcript, "faster_den = %u\n",      param->faster_den);
    fprintf(xfer->transcript, "history = %u\n",         param->history);
    fprintf(xfer->transcript, "lossless = %u\n",        param->lossless);
    fprintf(xfer->transcript, "losswindow = %u\n",      param->losswindow_ms);
    fprintf(xfer->transcript, "blockdump = %u\n",       param->blockdump);
    fprintf(xfer->transcript, "update_period = %llu\n", UPDATE_PERIOD);
    fprintf(xfer->transcript, "rexmit_period = %llu\n", UPDATE_PERIOD);
    fprintf(xfer->transcript, "protocol_version = 0x%x\n", PROTOCOL_REVISION);
    fprintf(xfer->transcript, "software_version = %s\n",   TSUNAMI_CVS_BUILDNR);
    fprintf(xfer->transcript, "ipv6 = %u\n",            param->ipv6_yn);
    fprintf(xfer->transcript, "\n");
    fflush(session->transfer.transcript);
}


/*========================================================================
 * $Log: transcript.c,v $
 * Revision 1.9  2008/07/19 20:44:34  jwagnerhki
 * show Mbit File Data, transcript.c show mbyte_transmitted mbyte_usable mbyte_file
 *
 * Revision 1.8  2008/07/19 20:27:00  jwagnerhki
 * rate with restarts and actual effective rate
 *
 * Revision 1.7  2008/05/25 15:43:16  jwagnerhki
 * file client merge
 *
 * Revision 1.6  2008/05/22 17:58:51  jwagnerhki
 * __darwin_suseconds_t fix
 *
 * Revision 1.5  2007/12/07 18:10:28  jwagnerhki
 * cleaned away 64-bit compile warnings, used tsunami-client.h
 *
 * Revision 1.4  2007/10/29 15:30:25  jwagnerhki
 * timeout feature for rttsunamid too, added version info to transcripts, added --hbimeout srv cmd line param
 *
 * Revision 1.3  2007/07/17 08:50:46  jwagnerhki
 * added fflush()es
 *
 * Revision 1.2  2007/07/13 18:43:56  jwagnerhki
 * added blockdump and lossless settings
 *
 * Revision 1.1.1.1  2006/07/20 09:21:19  jwagnerhki
 * reimport
 *
 * Revision 1.1  2006/07/10 12:26:51  jwagnerhki
 * deleted unnecessary files
 *
 */
