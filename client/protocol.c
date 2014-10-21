/*========================================================================
 * protocol.c  --  TTP routines for Tsunami client.
 *
 * This contains the Tsunami Transfer Protocol API for the Tsunami
 * file transfer client.
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

#include <stdlib.h>       /* for *alloc() and free()               */
#include <string.h>       /* for standard string routines          */
#include <sys/socket.h>   /* for the BSD socket library            */
#include <sys/time.h>     /* for gettimeofday()                    */
#include <time.h>         /* for time()                            */
#include <unistd.h>       /* for standard Unix system calls        */

#include <tsunami-client.h>
//#define DEBUG_RETX xxx // enable to show retransmit debug infos

/*------------------------------------------------------------------------
 * int ttp_authenticate(ttp_session_t *session, u_char *secret);
 *
 * Given an active Tsunami session, returns 0 if we are able to
 * negotiate authentication successfully and a non-zero value
 * otherwise.
 *
 * The negotiation process works like this:
 *
 *     (1) The server sends 512 bits of random data to the client
 *         [this process].
 *
 *     (2) The client XORs 512 bits of the shared secret onto this
 *         random data and responds with the MD5 hash of the result.
 *
 *     (3) The server does the same thing and compares the result.
 *         If the authentication succeeds, the server transmits a
 *         result byte of 0.  Otherwise, it transmits a non-zero
 *         result byte.
 *------------------------------------------------------------------------*/
int ttp_authenticate(ttp_session_t *session, u_char *secret)
{
    u_char  random[64];  /* the buffer of random data               */
    u_char  digest[16];  /* the MD5 message digest (for the server) */
    u_char  result;      /* the result byte from the server         */
    int     status;      /* return status from function calls       */

    /* read in the shared secret and the challenge */
    status = fread(random, 1, 64, session->server);
    if (status < 64)
	return warn("Could not read authentication challenge from server");

    /* prepare the proof of the shared secret and destroy the password */
    prepare_proof(random, 64, secret, digest);
    while (*secret)
	*(secret++) = '\0';

    /* send the response to the server */
    status = fwrite(digest, 1, 16, session->server);
    if ((status < 16) || fflush(session->server))
	return warn("Could not send authentication response");

    /* read the results back from the server */
    status = fread(&result, 1, 1, session->server);
    if (status < 1)
	return warn("Could not read authentication status");

    /* check the result byte */
    return (result == 0) ? 0 : -1;
}


/*------------------------------------------------------------------------
 * int ttp_negotiate(ttp_session_t *session);
 *
 * Performs all of the negotiation with the remote server that is done
 * prior to authentication.  At the moment, this consists of verifying
 * identical protocol revisions between the client and server.  Returns
 * 0 on success and non-zero on failure.
 *
 * Values are transmitted in network byte order.
 *------------------------------------------------------------------------*/
int ttp_negotiate(ttp_session_t *session)
{
    u_int32_t server_revision;
    u_int32_t client_revision = htonl(PROTOCOL_REVISION);
    int       status;

    /* send our protocol revision number to the server */
    status = fwrite(&client_revision, 4, 1, session->server);
    if ((status < 1) || fflush(session->server))
	return warn("Could not send protocol revision number");

    /* read the protocol revision number from the server */
    status = fread(&server_revision, 4, 1, session->server);
    if (status < 1)
	return warn("Could not read protocol revision number");

    /* compare the numbers */
    return (client_revision == server_revision) ? 0 : -1;
}


/*------------------------------------------------------------------------
 * int ttp_open_transfer(ttp_session_t *session,
 *                       const char *remote_filename,
 *                       const char *local_filename);
 *
 * Tries to create a new TTP file request object for the given session
 * by submitting a file request to the server (which is waiting for
 * the name of a file to transfer).  If the request is accepted, we
 * retrieve the file parameters, open the file for writing, and return
 * 0 for success.  If anything goes wrong, we return a non-zero value.
 *------------------------------------------------------------------------*/
int ttp_open_transfer(ttp_session_t *session, const char *remote_filename, const char *local_filename)
{
    u_char           result;    /* the result byte from the server     */
    u_int32_t        temp;      /* used for transmitting 32-bit values */
    u_int16_t        temp16;    /* used for transmitting 16-bit values */
    int              status;
    ttp_transfer_t  *xfer  = &session->transfer;
    ttp_parameter_t *param =  session->parameter;

    /* submit the transfer request */
    status = fprintf(session->server, "%s\n", remote_filename);
    if ((status <= 0) || fflush(session->server))
	return warn("Could not request file");

    /* see if the request was successful */
    status = fread(&result, 1, 1, session->server);
    if (status < 1)
	return warn("Could not read response to file request");

    /* make sure the result was a good one */
    if (result != 0)
	return warn("Server: File does not exist or cannot be transmitted");

    /* Submit the block size, target bitrate, and maximum error rate */
    temp = htonl(param->block_size);   if (fwrite(&temp, 4, 1, session->server) < 1) return warn("Could not submit block size");
    temp = htonl(param->target_rate);  if (fwrite(&temp, 4, 1, session->server) < 1) return warn("Could not submit target rate");
    temp = htonl(param->error_rate);   if (fwrite(&temp, 4, 1, session->server) < 1) return warn("Could not submit error rate");
    if (fflush(session->server))
	return warn("Could not flush control channel");

    /* submit the slower and faster factors */
    temp16 = htons(param->slower_num);  if (fwrite(&temp16, 2, 1, session->server) < 1) return warn("Could not submit slowdown numerator");
    temp16 = htons(param->slower_den);  if (fwrite(&temp16, 2, 1, session->server) < 1) return warn("Could not submit slowdown denominator");
    temp16 = htons(param->faster_num);  if (fwrite(&temp16, 2, 1, session->server) < 1) return warn("Could not submit speedup numerator");
    temp16 = htons(param->faster_den);  if (fwrite(&temp16, 2, 1, session->server) < 1) return warn("Could not submit speedup denominator");
    if (fflush(session->server))
	return warn("Could not flush control channel");

    /* populate the fields of the transfer object */
    memset(xfer, 0, sizeof(*xfer));
    xfer->remote_filename = remote_filename;
    xfer->local_filename  = local_filename;

    /* read in the file length, block size, block count, and run epoch */
    if (fread(&xfer->file_size,   8, 1, session->server) < 1) return warn("Could not read file size");         xfer->file_size   = ntohll(xfer->file_size);
    if (fread(&temp,              4, 1, session->server) < 1) return warn("Could not read block size");        if (htonl(temp) != param->block_size) return warn("Block size disagreement");
    if (fread(&xfer->block_count, 4, 1, session->server) < 1) return warn("Could not read number of blocks");  xfer->block_count = ntohl (xfer->block_count);
    if (fread(&xfer->epoch,       4, 1, session->server) < 1) return warn("Could not read run epoch");         xfer->epoch       = ntohl (xfer->epoch);

    /* we start out with every block yet to transfer */
    xfer->blocks_left = xfer->block_count;

    /* try to open the local file for writing */
    if (!access(xfer->local_filename, F_OK))
        printf("Warning: overwriting existing file '%s'\n", local_filename);     
    xfer->file = fopen(xfer->local_filename, "wb");
    if (xfer->file == NULL) {
        char * trimmed = rindex(xfer->local_filename, '/');
        if ((trimmed != NULL) && (strlen(trimmed)>1)) {
           printf("Warning: could not open file %s for writing, trying local directory instead.\n", xfer->local_filename);
           xfer->local_filename = trimmed + 1;
           if (!access(xfer->local_filename, F_OK))
              printf("Warning: overwriting existing file '%s'\n", xfer->local_filename);     
           xfer->file = fopen(xfer->local_filename, "wb");
        }
        if(xfer->file == NULL) {
           return warn("Could not open local file for writing");
        }
    }

    #ifdef VSIB_REALTIME
    /* try to open the vsib for output */
    xfer->vsib = fopen("/dev/vsib", "wb");
    if (xfer->vsib == NULL)
    return warn("VSIB board does not exist in /dev/vsib or it cannot be read");
    
    /* pre-reserve the ring buffer */  
    param->ringbuf = malloc(param->block_size * RINGBUF_BLOCKS); 
    if (param->ringbuf == NULL)
    return warn("Could not reserve space for ring buffer");
    #endif

    /* make crude estimate of blocks on the wire if RTT delay is 500ms */
    xfer->on_wire_estimate = (u_int32_t)(0.5 * param->target_rate/(8*param->block_size));
    xfer->on_wire_estimate = min(xfer->block_count, xfer->on_wire_estimate);

    /* if we're doing a transcript */
    if (param->transcript_yn)
	xscript_open(session);

    /* indicate success */
    return 0;
}


/*------------------------------------------------------------------------
 * int ttp_open_port(ttp_session_t *session);
 *
 * Creates a new UDP socket for receiving the file data associated with
 * our pending transfer and communicates the port number back to the
 * server.  Returns 0 on success and non-zero on failure.
 *------------------------------------------------------------------------*/
int ttp_open_port(ttp_session_t *session)
{
    struct sockaddr udp_address;
    unsigned int    udp_length = sizeof(udp_address);
    int             status;
    u_int16_t      *port;

    /* open a new datagram socket */
    session->transfer.udp_fd = create_udp_socket(session->parameter);
    if (session->transfer.udp_fd < 0)
	return warn("Could not create UDP socket");

    /* find out the port number we're using */
    memset(&udp_address, 0, sizeof(udp_address));
    getsockname(session->transfer.udp_fd, (struct sockaddr *) &udp_address, &udp_length);

    /* get a hold of the port number */
    port = (session->parameter->ipv6_yn ? &((struct sockaddr_in6 *) &udp_address)->sin6_port : &((struct sockaddr_in *) &udp_address)->sin_port);

    /* send that port number to the server */
    status = fwrite(port, 2, 1, session->server);
    if ((status < 1) || fflush(session->server)) {
	close(session->transfer.udp_fd);
	return warn("Could not send UDP port number");
    }

    /* we succeeded */
    return 0;
}


/*------------------------------------------------------------------------
 * int ttp_repeat_retransmit(ttp_session_t *session);
 *
 * Tries to repeat all of the outstanding retransmit requests for the
 * current transfer on the given session.  Returns 0 on success and
 * non-zero on error.  This also takes care of maintanence operations
 * on the transmission table, such as relocating the entries toward the
 * bottom of the array.
 *------------------------------------------------------------------------*/
int ttp_repeat_retransmit(ttp_session_t *session)
{
    retransmission_t  retransmission[MAX_RETRANSMISSION_BUFFER];  /* the retransmission request object        */
    int               entry;                                      /* an index into the retransmission table   */
    int               status;
    int               block;
    int               count = 0;
    retransmit_t     *rexmit = &(session->transfer.retransmit);
    ttp_transfer_t   *xfer = &session->transfer;

    #ifdef DEBUG_RETX
    fprintf(stderr, "ttp_repeat_retransmit: index_max=%u\n", rexmit->index_max);
    #endif

    /* reset */
    memset(retransmission, 0, sizeof(retransmission));
    xfer->stats.this_retransmits = 0;
    count = 0;

    /* discard received blocks from the list and prepare retransmit requests */
    for (entry = 0; (entry<rexmit->index_max) && (count<MAX_RETRANSMISSION_BUFFER); ++entry) {

        /* get the block number */
        block = rexmit->table[entry];

        /* if we want the block */
        if (block && !got_block(session, block)) {

            /* save it */
            rexmit->table[count] = block;

            /* insert retransmit request */
            retransmission[count].request_type = htons(REQUEST_RETRANSMIT);
            retransmission[count].block        = htonl(block);
            ++count;

            #ifdef DEBUG_RETX
            // printf("retx %lu\n", block);
            #endif
        }
    }

    /* if there are too many entries, restart transfer from earlier point */
    if (count >= MAX_RETRANSMISSION_BUFFER) {

        /* restart from first missing block */
        block                          = min(xfer->block_count, xfer->gapless_to_block + 1);
        retransmission[0].request_type = htons(REQUEST_RESTART);
        retransmission[0].block        = htonl(block);

        /* send out the request */
        status = fwrite(&retransmission[0], sizeof(retransmission[0]), 1, session->server);
        if (status <= 0) {
            return warn("Could not send restart-at request");
        }

        /* remember the request so we can then ignore blocks that are still on the wire */
        xfer->restart_pending        = 1;
        xfer->restart_lastidx        = rexmit->table[rexmit->index_max - 1];
        xfer->restart_wireclearidx   = min(xfer->block_count, xfer->restart_lastidx + xfer->on_wire_estimate);

        #ifdef DEBUG_RETX
        printf("ttp_repeat_restransmit: restart_pending=1, range %u to %u, clear at %u, gapless to %u, old head %u\n",
               block, xfer->restart_lastidx, xfer->restart_wireclearidx, xfer->gapless_to_block, xfer->next_block);
        #endif

        /* reset the retransmission table and head block */
        rexmit->index_max = 0;
        xfer->next_block  = block;

       xfer->stats.this_retransmits = MAX_RETRANSMISSION_BUFFER;

    /* queue is small enough */
    } else {

        /* update to shrunken size */
        rexmit->index_max = count;

        /* update the statistics */
        xfer->stats.this_retransmits   = count;
        xfer->stats.total_retransmits += count;

        /* send out the requests */
        if (count > 0) {
            status = fwrite(retransmission, sizeof(retransmission_t), count, session->server);
            if (status <= 0) {
                return warn("Could not send retransmit requests");
            }
        }

    }//if(num entries)

    /* flush the server connection */
    if (fflush(session->server)) {
        return warn("Could not flush retransmit requests");
    }

    /* we succeeded */
    #ifdef DEBUG_RETX
    fprintf(stderr, "ttp_repeat_retransmit: post-index_max=%u\n", rexmit->index_max);
    #endif
    return 0;
}


/*------------------------------------------------------------------------
 * int ttp_request_retransmit(ttp_session_t *session, u_int32_t block);
 *
 * Requests a retransmission of the given block in the current transfer.
 * Returns 0 on success and non-zero otherwise.
 *------------------------------------------------------------------------*/
int ttp_request_retransmit(ttp_session_t *session, u_int32_t block)
{
   #ifdef RETX_REQBLOCK_SORTING
   u_int32_t     tmp32_ins = 0, tmp32_up;
   u_int32_t     idx = 0;
   #endif

   u_int32_t    *ptr;
   retransmit_t *rexmit = &(session->transfer.retransmit);

   /* double checking: if we already got the block, don't add it */
   if (got_block(session, block)) {
      return 0;
   }

   /* if we don't have space for the request */
   if (rexmit->index_max >= rexmit->table_size) {

      /* don't overgrow the table */
      if (rexmit->index_max >= 32*MAX_RETRANSMISSION_BUFFER)
         return 0;

      /* try to reallocate the table twice the size*/
      ptr = (u_int32_t *) realloc(rexmit->table, 2 * sizeof(u_int32_t)*rexmit->table_size);
      if (ptr == NULL)
         return warn("Could not grow retransmission table");

      /* prepare the new table space */
      rexmit->table = ptr;
      memset(rexmit->table + rexmit->table_size, 0, sizeof(u_int32_t) * rexmit->table_size);
      rexmit->table_size *= 2;

      #if DEBUG_RETX
      fprintf(stderr, "ttp_request_retransmit: new table size is %u entries\n", rexmit->table_size);
      #endif
   }

   #ifndef RETX_REQBLOCK_SORTING

   /* store the request */
   rexmit->table[rexmit->index_max] = block;
   rexmit->index_max++;

   #else

   /* 
    * Store the request via "insertion sort"
    * this maintains a sequentially sorted table and discards duplicate requests,
    * and does not flood the net with so many unnecessary retransmissions like old Tsunami did
    * -- however, this can be very slow on high loss transfers! with slow CPU this causes
    *    even more loss : consider well if you want to enable the feature or not
    */

   /* seek to insertion point or end - could use binary search here... */
   while ((idx < rexmit->index_max) && (rexmit->table[idx] < block)) {
     idx++; 
   }

   /* insert the entry */
   if (idx == rexmit->index_max) { 
      rexmit->table[rexmit->index_max] = block;
      rexmit->index_max++;
   } else if (rexmit->table[idx] == block) { 
      // fprintf(stderr, "duplicate retransmit req for block %d discarded\n", block);
   } else { 
      /* insert and shift remaining table upwards - linked list could be nice... */
      tmp32_ins = block;
      do {
         tmp32_up = rexmit->table[idx];
         rexmit->table[idx++] = tmp32_ins;
         tmp32_ins = tmp32_up;
      } while(idx <= rexmit->index_max);
      rexmit->index_max++;
   }
   #endif

   /* we succeeded */
   return 0;
}


/*------------------------------------------------------------------------
 * int ttp_request_stop(ttp_session_t *session);
 *
 * Requests that the server stop transmitting data for the current
 * file transfer in the given session.  This is done by sending a
 * retransmission request with a type of REQUEST_STOP.  Returns 0 on
 * success and non-zero otherwise.  Success means that we successfully
 * requested, not that we successfully halted.
 *------------------------------------------------------------------------*/
int ttp_request_stop(ttp_session_t *session)
{
    retransmission_t retransmission = { 0, 0, 0 };
    int              status;

    /* initialize the retransmission structure */
    retransmission.request_type = htons(REQUEST_STOP);

    /* send out the request */
    status = fwrite(&retransmission, sizeof(retransmission), 1, session->server);
    if ((status <= 0) || fflush(session->server))
       return warn("Could not request end of transmission");
    
    #ifdef VSIB_REALTIME
    /* Wait until ring buffer is empty, then stop vsib and free buffer memory*/
    stop_vsib(session);
    free(session->parameter->ringbuf);
    session->parameter->ringbuf = NULL; /* it is no more... */ 
    #endif

    /* we succeeded */
    return 0;
}


/*------------------------------------------------------------------------
 * int ttp_update_stats(ttp_session_t *session);
 *
 * This routine must be called every interval to update the statistics
 * for the progress of the ongoing file transfer.  Returns 0 on success
 * and non-zero on failure.  (There is not currently any way to fail.)
 *------------------------------------------------------------------------*/
int ttp_update_stats(ttp_session_t *session)
{
    time_t            now_epoch = time(NULL);                 /* the current Unix epoch                         */
    u_int64_t         delta;                                  /* time delta since last statistics update (usec) */
    double            d_seconds;
    u_int64_t         delta_total;                            /* time delta since start of transmission (usec)  */
    double            d_seconds_total;
    u_int64_t         temp;                                   /* temporary value for building the elapsed time  */
    int               hours, minutes, seconds, milliseconds;  /* parts of the elapsed time                      */
    double            data_total;                             /* the total amount of data transferred (bytes)   */
    double            data_total_rate;
    double            data_this;                              /* the amount of data since last stat time        */
    double            data_this_rexmit;                       /* the amount of data in received retransmissions */ 
    double            data_this_goodpt;                       /* the amount of data as non-lost packets         */
    double            retransmits_fraction;                   /* how many retransmit requests there were vs received blocks */
    double            total_retransmits_fraction;
    double            ringfill_fraction;
    statistics_t     *stats = &(session->transfer.stats);
    retransmission_t  retransmission;
    int               status;
    static u_int32_t  iteration = 0;
    static char       stats_line[128];
    static char       stats_flags[8];

    double ff, fb;

    const double u_mega = 1024*1024;
    const double u_giga = 1024*1024*1024;

    /* find the total time elapsed */
    delta        =        get_usec_since(&stats->this_time);
    delta_total  = temp = get_usec_since(&stats->start_time);
    milliseconds = (temp % 1000000) / 1000;  temp /= 1000000;
    seconds      = temp % 60;                temp /= 60;
    minutes      = temp % 60;                temp /= 60;
    hours        = temp;

    d_seconds       = delta / 1e6;
    d_seconds_total = delta_total / 1e6;

    /* find the amount of data transferred (bytes) */
    data_total  = ((double) session->parameter->block_size) * stats->total_blocks;
    data_this   = ((double) session->parameter->block_size) * (stats->total_blocks - stats->this_blocks);
    data_this_rexmit = ((double) session->parameter->block_size) * stats->this_flow_retransmitteds;
    data_this_goodpt = ((double) session->parameter->block_size) * stats->this_flow_originals;
    // <=> data_this == data_this_rexmit + data_this_goodpt

    /* get the current UDP receive error count reported by the operating system */
    stats->this_udp_errors = get_udp_in_errors();

    /* precalculate some fractions */
    retransmits_fraction = stats->this_retransmits / (1.0 + stats->this_retransmits + stats->total_blocks - stats->this_blocks);
    ringfill_fraction    = session->transfer.ring_buffer->count_data / MAX_BLOCKS_QUEUED;
    total_retransmits_fraction = stats->total_retransmits / (stats->total_retransmits + stats->total_blocks);

    /* update the rate statistics */
    // incoming transmit rate R = goodput R (Mbit/s) + retransmit R (Mbit/s)
    stats->this_transmit_rate   = 8.0 * data_this        / (d_seconds * u_mega); 
    stats->this_retransmit_rate = 8.0 * data_this_rexmit / (d_seconds * u_mega);
    data_total_rate             = 8.0 * data_total       / (d_seconds_total * u_mega);

    fb = session->parameter->history / 100.0;  // feedback
    ff = 1.0 - fb;                             // feedforward

    // IIR filter rate R
    stats->transmit_rate = fb * stats->transmit_rate + ff * stats->this_transmit_rate;

    // IIR filtered composite error and loss, some sort of knee function
    stats->error_rate = fb * stats->error_rate + ff * 500*100 * (retransmits_fraction + ringfill_fraction);
        
    /* send the current error rate information to the server */
    memset(&retransmission, 0, sizeof(retransmission));
    retransmission.request_type = htons(REQUEST_ERROR_RATE);
    retransmission.error_rate   = htonl((u_int64_t) session->transfer.stats.error_rate);
    status = fwrite(&retransmission, sizeof(retransmission), 1, session->server);
    if ((status <= 0) || fflush(session->server))
        return warn("Could not send error rate information");

    /* build the stats string */    
    sprintf(stats_flags, "%c%c",
               ((session->transfer.restart_pending) ? 'R' : '-'),
               (!(session->transfer.ring_buffer->space_ready) ? 'F' : '-')
    );
    #ifdef STATS_MATLABFORMAT
    sprintf(stats_line, "%02d\t%02d\t%02d\t%03d\t%4u\t%6.2f\t%6.1f\t%5.1f\t%7u\t%6.1f\t%6.1f\t%5.1f\t%5d\t%5d\t%7u\t%8u\t%8Lu\t%s\n",
    #else
    sprintf(stats_line, "%02d:%02d:%02d.%03d %4u %6.2fM %6.1fMbps %5.1f%% %7u %6.1fG %6.1fMbps %5.1f%% %5d %5d %7u %8u %8Lu %s\n",
    #endif
        hours, minutes, seconds, milliseconds,
        stats->total_blocks - stats->this_blocks,
        stats->this_retransmit_rate,
        stats->this_transmit_rate,
        100.0 * retransmits_fraction,
        session->transfer.stats.total_blocks,
        data_total / u_giga,
        data_total_rate,
        100.0 * total_retransmits_fraction,
        session->transfer.retransmit.index_max,
        session->transfer.ring_buffer->count_data,
        session->transfer.blocks_left, 
        stats->this_retransmits,
        (ull_t)(stats->this_udp_errors - stats->start_udp_errors),
        stats_flags
        );

    /* give the user a show if they want it */
    if (session->parameter->verbose_yn) {

        /* screen mode */
        if (session->parameter->output_mode == SCREEN_MODE) {
            printf("\033[2J\033[H");
            printf("Current time:   %s\n", ctime(&now_epoch));
            printf("Elapsed time:   %02d:%02d:%02d.%03d\n\n", hours, minutes, seconds, milliseconds);
            printf("Last interval\n--------------------------------------------------\n");
            printf("Blocks count:     %u\n",             stats->total_blocks - stats->this_blocks);
            printf("Data transferred: %0.2f GB\n",       data_this  / u_giga);
            printf("Transfer rate:    %0.2f Mbps\n",     stats->this_transmit_rate);
            printf("Retransmissions:  %u (%0.2f%%)\n\n", stats->this_retransmits, 100.0*retransmits_fraction);
            printf("Cumulative\n--------------------------------------------------\n");
            printf("Blocks count:     %u\n",             session->transfer.stats.total_blocks);
            printf("Data transferred: %0.2f GB\n",       data_total / u_giga);
            printf("Transfer rate:    %0.2f Mbps\n",     data_total_rate);
            printf("Retransmissions:  %u (%0.2f%%)\n",   stats->total_retransmits, 100.0*total_retransmits_fraction);
            printf("Flags          :  %s\n\n",           stats_flags);
            printf("OS UDP rx errors: %llu\n",           (ull_t)(stats->this_udp_errors - stats->start_udp_errors));

        /* line mode */
        } else {

            /* print a header if necessary */
            #ifndef STATS_NOHEADER
            if (!(iteration++ % 23)) {
                printf("             last_interval                   transfer_total                   buffers      transfer_remaining  OS UDP\n");
                printf("time          blk    data       rate rexmit     blk    data       rate rexmit queue  ring     blk   rt_len      err \n");
            }
            #endif
            printf("%s", stats_line);
        }

        /* and flush the output */
        fflush(stdout);
    }

    /* print to the transcript if the user wants */
    if (session->parameter->transcript_yn)
        xscript_data_log(session, stats_line);

    /* reset the statistics for the next interval */
    stats->this_blocks              = stats->total_blocks;
    stats->this_retransmits         = 0;
    stats->this_flow_originals      = 0;
    stats->this_flow_retransmitteds = 0;
    gettimeofday(&(stats->this_time), NULL);

    /* indicate success */
    return 0;
}


/*========================================================================
 * $Log: protocol.c,v $
 * Revision 1.30  2009/12/22 23:01:21  jwagnerhki
 * don't allow the retransmit list size to grow without bounds
 *
 * Revision 1.29  2009/12/21 17:09:08  jwagnerhki
 * print flags
 *
 * Revision 1.28  2009/05/18 08:40:31  jwagnerhki
 * Lu formatting to llu
 *
 * Revision 1.27  2008/07/19 20:01:25  jwagnerhki
 * gapless_to_block, ttp_repeat_retransmit changed to purge duplicates first then decide on request-restart, more DEBUG_RETX output
 *
 * Revision 1.26  2008/07/19 14:59:53  jwagnerhki
 * added restart_wireclearidx variable
 *
 * Revision 1.25  2008/05/22 23:36:33  jwagnerhki
 * much better statistics counters, cleaned up statistics math, restart-req doesnt change total_blocks and this_blocks
 *
 * Revision 1.24  2008/05/22 18:30:44  jwagnerhki
 * Darwin fix LFS support fopen() not fopen64() etc
 *
 * Revision 1.23  2008/05/20 18:12:45  jwagnerhki
 * got_block and tidying
 *
 * Revision 1.22  2007/12/07 18:10:28  jwagnerhki
 * cleaned away 64-bit compile warnings, used tsunami-client.h
 *
 * Revision 1.21  2007/08/27 15:12:30  jwagnerhki
 * fixed 32-bit write as 16-bit endianness problem
 *
 * Revision 1.20  2007/07/16 09:51:09  jwagnerhki
 * rt-server now ipd-throttled again
 *
 * Revision 1.19  2007/06/19 13:35:24  jwagnerhki
 * replaced notretransmit option with better time-limited restransmission window, reduced ringbuffer from 8192 to 4096 entries
 *
 * Revision 1.18  2007/05/23 11:58:33  jwagnerhki
 * slightly better filename path trim
 *
 * Revision 1.17  2007/05/21 13:51:15  jwagnerhki
 * client side path slash removal if dir not existing
 *
 * Revision 1.16  2007/01/11 15:15:48  jwagnerhki
 * rtclient merge, io.c now with VSIB_REALTIME, blocks_left not allowed negative fix, overwriting file check fixed, some memset()s to keep Valgrind warnings away
 *
 * Revision 1.15  2006/12/22 12:06:21  jwagnerhki
 * warn about file overwrite, truncate could take long time
 *
 * Revision 1.14  2006/12/21 13:50:33  jwagnerhki
 * added to client something that smells like a fix for non-working REQUEST_RESTART
 *
 * Revision 1.13  2006/12/19 12:12:41  jwagnerhki
 * corrected bad reallocs
 *
 * Revision 1.12  2006/12/11 13:44:17  jwagnerhki
 * OS UDP err count now done in ttp_update_stats(), cleaned stats printout align, fixed CLOSE cmd segfault
 *
 * Revision 1.11  2006/12/05 15:24:50  jwagnerhki
 * now noretransmit code in client only, merged rt client code
 *
 * Revision 1.10  2006/11/10 11:29:45  jwagnerhki
 * updated stats display
 *
 * Revision 1.9  2006/10/28 19:29:15  jwagnerhki
 * jamil GET* merge, insertionsort disabled by default again
 *
 * Revision 1.8  2006/10/27 20:12:36  jwagnerhki
 * fix for very bad original retransmit req assembly
 *
 * Revision 1.7  2006/10/25 14:53:16  jwagnerhki
 * removed superfluous ACK on GET*
 *
 * Revision 1.6  2006/10/25 14:20:31  jwagnerhki
 * attempt to support multimode get already implemented in Jamil server
 *
 * Revision 1.5  2006/10/19 08:06:31  jwagnerhki
 * fix STATS_MATLABFORMAT ifndef to ifdef
 *
 * Revision 1.4  2006/10/17 12:39:50  jwagnerhki
 * disabled retransmit debug infos on default
 *
 * Revision 1.3  2006/08/08 08:38:20  jwagnerhki
 * added some debug output trying to catch file corruption issues
 *
 * Revision 1.2  2006/07/21 08:50:41  jwagnerhki
 * merged client and rtclient protocol.c
 *
 * Revision 1.1.1.1  2006/07/20 09:21:19  jwagnerhki
 * reimport
 *
 * Revision 1.2  2006/07/11 07:38:32  jwagnerhki
 * new debug defines
 *
 * Revision 1.1  2006/07/10 12:26:51  jwagnerhki
 * deleted unnecessary files
 *
 */
