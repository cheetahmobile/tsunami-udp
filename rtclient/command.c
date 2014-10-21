/*========================================================================
 * command.c  --  CLI command routines for Tsunami client.
 *
 * This contains routines for processing the commands of the Tsunami
 * file transfer CLI client.
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
 * OR OTHER PROPRIETARY RIGHTS.  INDIANA UNIVERSITY MAKES NO
 * WARRANTIES THAT SOFTWARE IS FREE FROM "BUGS", "VIRUSES", "TROJAN
 * HORSES", "TRAP DOORS", "WORMS", OR OTHER HARMFUL CODE.  LICENSEE
 * ASSUMES THE ENTIRE RISK AS TO THE PERFORMANCE OF SOFTWARE AND/OR
 * ASSOCIATED MATERIALS, AND TO THE PERFORMANCE AND VALIDITY OF
 * INFORMATION GENERATED USING SOFTWARE.
 *========================================================================*/

#include <pthread.h>      /* for the pthreads library              */
#include <stdlib.h>       /* for *alloc() and free()               */
#include <string.h>       /* for standard string routines          */
#include <sys/socket.h>   /* for the BSD socket library            */
#include <sys/time.h>     /* for gettimeofday()                    */
#include <time.h>         /* for time()                            */
#include <unistd.h>       /* for standard Unix system calls        */
#include <ctype.h>        /* for toupper() etc                     */

#include <tsunami-client.h>

//#define DEBUG_RETX xxx // enable to show retransmit debug infos

/*------------------------------------------------------------------------
 * Prototypes for module-scope routines.
 *------------------------------------------------------------------------*/

void *disk_thread   (void *arg);
int   parse_fraction(const char *fraction, u_int16_t *num, u_int16_t *den);


/*------------------------------------------------------------------------
 * int command_close(command_t *command, ttp_session_t *session);
 *
 * Closes the given open Tsunami control session if it's active, thus
 * making it invalid for further use.  Returns 0 on success and non-zero
 * on error.
 *------------------------------------------------------------------------*/
int command_close(command_t *command, ttp_session_t *session)
{
    /* make sure we have an open connection */
    if (session == NULL || session->server == NULL)
	return warn("Tsunami session was not active");

    /* otherwise, go ahead and close it */
    fclose(session->server);
    session->server = NULL;
    if (session->parameter->verbose_yn)
	printf("Connection closed.\n\n");
    return 0;
}


/*------------------------------------------------------------------------
 * ttp_session_t *command_connect(command_t *command,
 *                                ttp_parameter_t *parameter);
 *
 * Opens a new Tsunami control session to the server specified in the
 * command or in the given set of default parameters.  This involves
 * prompting the user to enter the shared secret.  On success, we return
 * a pointer to the new TTP session object.  On failure, we return NULL.
 *
 * Note that the default host and port stored in the parameter object
 * are updated if they were specified in the command itself.
 *------------------------------------------------------------------------*/
ttp_session_t *command_connect(command_t *command, ttp_parameter_t *parameter)
{
    int            server_fd;
    ttp_session_t *session;
    char           *secret;

    /* if we were given a new host, store that information */
    if (command->count > 1) {
	if (parameter->server_name != NULL)
	    free(parameter->server_name);
	parameter->server_name = strdup(command->text[1]);
	if (parameter->server_name == NULL) {
	    warn("Could not update server name");
	    return NULL;
	}
    }

    /* if we were given a port, store that information */
    if (command->count > 2)
	parameter->server_port = atoi(command->text[2]);

    /* allocate a new session */
    session = (ttp_session_t *) calloc(1, sizeof(ttp_session_t));
    if (session == NULL)
	error("Could not allocate session object");
    session->parameter = parameter;

    /* obtain our client socket */
    server_fd = create_tcp_socket(session, parameter->server_name, parameter->server_port);
    if (server_fd < 0) {
	sprintf(g_error, "Could not connect to %s:%d.", parameter->server_name, parameter->server_port);
	warn(g_error);
	return NULL;
    }

    /* convert our server connection into a stream */
    session->server = fdopen(server_fd, "w+");
    if (session->server == NULL) {
	warn("Could not convert control channel into a stream");
	close(server_fd);
	free(session);
	return NULL;
    }

    /* negotiate the connection parameters */
    if (ttp_negotiate(session) < 0) {
	warn("Protocol negotiation failed");
	fclose(session->server);
	free(session);
	return NULL;
    }

    /* get the shared secret from the user */
    if (parameter->passphrase == NULL)
        secret = strdup(DEFAULT_SECRET);
    else 
        secret = strdup(parameter->passphrase);
    // if (NULL == (secret = getpass("Password: ")))
    //   error("Could not read shared secret");

    /* authenticate to the server */
    if (ttp_authenticate(session, (u_char*)secret) < 0) {
	warn("Authentication failed");
	fclose(session->server);
	free(secret);
	free(session);
	return NULL;
    }

    /* we succeeded */
    if (session->parameter->verbose_yn)
	printf("Connected.\n\n");
    free(secret);
    return session;
}


/*------------------------------------------------------------------------
 * int command_dir(command_t *command, ttp_session_t *session);
 *
 * Tries to request a list of server shared files and their sizes.
 * Returns 0 on a successful transfer and nonzero on an error condition.
 * Allocates and fills out session->fileslist struct, the caller needs to 
 * free it after use.
 *------------------------------------------------------------------------*/
int command_dir(command_t *command, ttp_session_t *session)
{
    u_char    result;
    char      read_str[2048];
    u_int16_t num_files, i;
    size_t    filelen;
    u_int16_t status = 0;
    
    /* make sure that we have an open session */
    if (session == NULL || session->server == NULL)
	return warn("Not connected to a Tsunami server");

    /* send request and parse the result */
    fprintf(session->server, "%s\n", TS_DIRLIST_HACK_CMD);
    
    status = fread(&result, 1, 1, session->server);
    if (status < 1)
        return warn("Could not read response to directory request");
    if (result == 8)
        return warn("Server does no support listing of shared files");
    
    read_str[0] = result;  
    fread_line(session->server, &read_str[1], sizeof(read_str)-2);
    num_files = atoi(read_str);
    
    fprintf(stderr, "Remote file list:\n");
    for (i=0; i<num_files; i++) {
        fread_line(session->server, read_str, sizeof(read_str)-1);
        fprintf(stderr, " %2d) %-64s", i+1, read_str);
        fread_line(session->server, read_str, sizeof(read_str)-1);
        filelen = atol(read_str);
        fprintf(stderr, "%8Lu bytes\n", (ull_t)filelen);
    } 
    fprintf(stderr, "\n");
    fwrite("\0", 1, 1, session->server);
    return 0;
}


/*------------------------------------------------------------------------
 * int command_get(command_t *command, ttp_session_t *session);
 *
 * Tries to initiate a file transfer for the remote file given in the
 * command.  If the user did not supply a local filename, we derive it
 * from the remote filename.  Returns 0 on a successful transfer and
 * nonzero on an error condition.
 *------------------------------------------------------------------------*/
int command_get(command_t *command, ttp_session_t *session)
{
    u_char         *datagram = NULL;            /* the buffer (in ring) for incoming blocks       */
    u_char         *local_datagram = NULL;      /* the local temp space for incoming block        */
    u_int32_t       this_block = 0;             /* the block number for the block just received   */
    u_int16_t       this_type = 0;              /* the block type for the block just received     */
    u_int64_t       delta = 0;                  /* generic holder of elapsed times                */
    u_int32_t       block = 0;                  /* generic holder of a block number               */

    double          mbit_thru, mbit_good;       /* helpers for final statistics                   */
    double          mbit_file;
    double          time_secs;

    ttp_transfer_t *xfer          = &(session->transfer);
    retransmit_t   *rexmit        = &(session->transfer.retransmit);
    int             status = 0;
    pthread_t       disk_thread_id = 0;

    /* The following variables will be used only in multiple file transfer
     * session they are used to recieve the file names and other parameters
     */
    int             multimode = 0;
    char          **file_names = NULL;
    u_int32_t       f_counter = 0, f_total = 0, f_arrsize = 0;

    /* this struct wil hold the RTT time */
    struct timeval ping_s, ping_e;
    long wait_u_sec = 1;

    /* make sure that we have a remote file name */
    if (command->count < 2)
	return warn("Invalid command syntax (use 'help get' for details)");

    /* make sure that we have an open session */
    if (session == NULL || session->server == NULL)
	return warn("Not connected to a Tsunami server");

    /* reinitialize the transfer data */
    memset(xfer, 0, sizeof(*xfer));

    /* if the client asking for multiple files to be transfered */
    if(!strcmp("*",command->text[1])) {
       char  filearray_size[10];
       char  file_count[10];

       multimode = 1;
       printf("Requesting all available files\n");

       /* Send request and try to calculate the RTT from client to server */
       gettimeofday(&(ping_s), NULL);
       status = fprintf(session->server, "%s\n", command->text[1]);
       status = fread(filearray_size, sizeof(char), 10, session->server);
       gettimeofday(&(ping_e),NULL);

       status = fread(file_count, sizeof(char), 10, session->server);
       fprintf(session->server, "got size");

       if ((status <= 0) || fflush(session->server))
          return warn("Could not request file");

       /* See if the request was successful */
       if (status < 1)
          return warn("Could not read response to file request");

       /* Calculate and convert RTT to u_sec, with +10% margin */
       wait_u_sec = (ping_e.tv_sec - ping_s.tv_sec)*1000000+(ping_e.tv_usec-ping_s.tv_usec);
       wait_u_sec = wait_u_sec + ((int)(wait_u_sec* 0.1));

       /* Parse */
       sscanf(filearray_size, "%u", &f_arrsize);
       sscanf(file_count, "%u", &f_total);

       if (f_total <= 0) {
          return warn("Server advertised no files to get");
       }
       printf("\nServer is sharing %u files\n", f_total);

       /* Read the file list */
       file_names = malloc(f_total * sizeof(char*));
       if(file_names == NULL)
          error("Could not allocate memory\n");

       printf("Multi-GET of %d files:\n", f_total);
       for(f_counter=0; f_counter<f_total; f_counter++) {
          char tmpname[1024];
          fread_line(session->server, tmpname, 1024);
          file_names[f_counter] = strdup(tmpname);
          printf("%s ", file_names[f_counter]);
       }
       fprintf(session->server, "got list");
       printf("\n");

    } else {
       f_total = 1;
    }

    f_counter = 0;
    do /*---loop for single and multi file request---*/
    {

    /* store the remote filename */
    if(!multimode)
       xfer->remote_filename = command->text[1];
    else
       xfer->remote_filename = file_names[f_counter];

    /* store the local filename */
    if(!multimode) {
       if (command->count >= 3) {
          /* command was in "GET remotefile localfile" style */
          xfer->local_filename = command->text[2];
       } else {
          /* trim into local filename without '/' */
          xfer->local_filename = strrchr(command->text[1], '/');
          if (xfer->local_filename == NULL)
             xfer->local_filename = command->text[1];
          else
             ++(xfer->local_filename);
       }
    } else {
       /* don't trim, GET* writes into remotefilename dir if exists, otherwise into CWD */
       xfer->local_filename = file_names[f_counter];
       printf("GET *: now requesting file '%s'\n", xfer->local_filename);
    }

    /* negotiate the file request with the server */
    if (ttp_open_transfer(session, xfer->remote_filename, xfer->local_filename) < 0)
	return warn("File transfer request failed");

    /* create the UDP data socket */
    if (ttp_open_port(session) < 0)
	return warn("Creation of data socket failed");

    /* allocate the retransmission table */
    rexmit->table = (u_int32_t *) calloc(DEFAULT_TABLE_SIZE, sizeof(u_int32_t));
    if (rexmit->table == NULL)
	error("Could not allocate retransmission table");

    /* allocate the received bitfield */
    xfer->received = (u_char *) calloc(xfer->block_count / 8 + 2, sizeof(u_char));
    if (xfer->received == NULL)
	error("Could not allocate received-data bitfield");

    /* allocate the ring buffer */
    xfer->ring_buffer = ring_create(session);

    /* allocate the faster local buffer */
    local_datagram = (u_char *) calloc(6 + session->parameter->block_size, sizeof(u_char));
    if (local_datagram == NULL)
        error("Could not allocate fast local datagram buffer in command_get()");

    /* start up the disk I/O thread */
    status = pthread_create(&disk_thread_id, NULL, disk_thread, session);
    if (status != 0)
	error("Could not create I/O thread");

    /* Finish initializing the retransmission object */
    rexmit->table_size = DEFAULT_TABLE_SIZE;
    rexmit->index_max  = 0;

    /* we start by expecting block #1 */
    xfer->next_block = 1;
    xfer->gapless_to_block = 0;

   /*---------------------------
   * START TIMING
   *---------------------------*/

   memset(&xfer->stats, 0, sizeof(xfer->stats));
   xfer->stats.start_udp_errors = get_udp_in_errors();
   xfer->stats.this_udp_errors = xfer->stats.start_udp_errors;
   gettimeofday(&(xfer->stats.start_time), NULL);
   gettimeofday(&(xfer->stats.this_time),  NULL);
   if (session->parameter->transcript_yn)
      xscript_data_start(session, &(xfer->stats.start_time));

   /* until we break out of the transfer */
   while (1) {

      /* try to receive a datagram */
      status = recvfrom(xfer->udp_fd, local_datagram, 6 + session->parameter->block_size, 0, NULL, 0);
      if (status < 0) {
          warn("UDP data transmission error");
          printf("Apparently frozen transfer, trying to do retransmit request\n");
          if (ttp_repeat_retransmit(session) < 0) {  /* repeat our requests */
             warn("Repeat of retransmission requests failed");
             goto abort;
          }
      }

      /* retrieve the block number and block type */
      this_block = ntohl(*((u_int32_t *) local_datagram));       // in range of 1..xfer->block_count
      this_type  = ntohs(*((u_int16_t *) (local_datagram + 4))); // TS_BLOCK_ORIGINAL etc

      /* keep statistics on received blocks */
      xfer->stats.total_blocks++;
      if (this_type != TS_BLOCK_RETRANSMISSION) {
          xfer->stats.this_flow_originals++;
      } else {
          xfer->stats.this_flow_retransmitteds++;
          xfer->stats.total_recvd_retransmits++;
      }

      /* main transfer control logic */
      if (!got_block(session, this_block) || this_type == TS_BLOCK_TERMINATE || xfer->restart_pending)
      {

          /* insert new blocks into disk write ringbuffer */
          if (!got_block(session, this_block)) {

              /* reserve ring space, copy the data in, confirm the reservation */
              datagram = ring_reserve(xfer->ring_buffer);
              memcpy(datagram, local_datagram, 6 + session->parameter->block_size);
              if (ring_confirm(xfer->ring_buffer) < 0) {
                  warn("Error in accepting block");
                  goto abort;
              }

              /* mark the block as received */
              xfer->received[this_block / 8] |= (1 << (this_block % 8));
              if (xfer->blocks_left > 0) {
                  --(xfer->blocks_left);
              } else {
                  printf("Oops! Negative-going blocks_left count at block: type=%c this=%u final=%u left=%u\n", this_type, this_block, xfer->block_count, xfer->blocks_left);
              }
          }

          /* transmit restart: avoid re-triggering on blocks still down the wire before server reacts */
          if ((xfer->restart_pending) && (this_type != TS_BLOCK_TERMINATE)) {
              if ((this_block > xfer->restart_lastidx) && (this_block <= xfer->restart_wireclearidx)) {
                  goto send_stats;
              }
          }

          /* queue any retransmits we need */
          if (this_block > xfer->next_block) {

             /* lossy transfer mode */
             if (!session->parameter->lossless) {
                if (session->parameter->losswindow_ms == 0) {
                    /* lossy transfer, no retransmits */
                    xfer->gapless_to_block = this_block;
                } else {
                    /* semi-lossy transfer, purge data past specified approximate time window */
                    double path_capability;
                    path_capability  = 0.8 * (xfer->stats.this_transmit_rate + xfer->stats.this_retransmit_rate); // reduced effective Mbit/s rate
                    path_capability *= (0.001 * session->parameter->losswindow_ms); // MBit inside window, round-trip user estimated in losswindow_ms!
                    u_int32_t earliest_block = this_block -
                       min(
                         1024 * 1024 * path_capability / (8 * session->parameter->block_size),  // # of blocks inside window
                         (this_block - xfer->gapless_to_block)                                  // # of blocks missing (tops)
                       );
                    for (block = earliest_block; block < this_block; ++block) {
                        if (ttp_request_retransmit(session, block) < 0) {
                            warn("Retransmission request failed");
                            goto abort;
                        }
                    }
                    // hop over the missing section
                    xfer->next_block = earliest_block;
                    xfer->gapless_to_block = earliest_block;
                }

             /* lossless transfer mode, request all missing data to be resent */
             } else {
                for (block = xfer->next_block; block < this_block; ++block) {
                    if (ttp_request_retransmit(session, block) < 0) {
                        warn("Retransmission request failed");
                        goto abort;
                    }
                }
             }
          }//if(missing blocks)

          /* advance the index of the gapless section going from start block to highest block  */
          while (got_block(session, xfer->gapless_to_block + 1) && (xfer->gapless_to_block < xfer->block_count)) {
              xfer->gapless_to_block++;
          }

          /* if this is an orignal, we expect to receive the successor to this block next */
          /* transmit restart note: these resent blocks are labeled original as well      */
          if (this_type == TS_BLOCK_ORIGINAL) {
              xfer->next_block = this_block + 1;
          }

          /* transmit restart: already got out of the missing blocks range? */
          if (xfer->restart_pending && (xfer->next_block >= xfer->restart_lastidx)) {
              xfer->restart_pending = 0;
          }

          /* are we at the end of the transmission? */
          if (this_type == TS_BLOCK_TERMINATE) {

              #if DEBUG_RETX
              fprintf(stderr, "Got end block: blk %u, final blk %u, left blks %u, tail %u, head %u\n",
                      this_block, xfer->block_count, xfer->blocks_left, xfer->gapless_to_block, xfer->next_block);
              #endif

              /* got all blocks by now */
              if (xfer->blocks_left == 0) {
                  break;
              } else if (!session->parameter->lossless) {
                  if ((rexmit->index_max==0) && !(xfer->restart_pending)) {
                      break;
                  }
              }

              /* add possible still missing blocks to retransmit list */
              for (block = xfer->gapless_to_block+1; block < xfer->block_count; ++block) {
                  if (ttp_request_retransmit(session, block) < 0) {
                      warn("Retransmission request failed");
                      goto abort;
                  }
              }

              /* send the retransmit request list again */
              ttp_repeat_retransmit(session);
          }

      }//if(not a duplicate block)

    send_stats:

      /* repeat our server feedback and requests if it's time */
      if (!(xfer->stats.total_blocks % 50)) {

          /* if it's been at least 350ms */
          if (get_usec_since(&(xfer->stats.this_time)) > UPDATE_PERIOD) {

            /* repeat our retransmission requests */
            if (ttp_repeat_retransmit(session) < 0) {
                warn("Repeat of retransmission requests failed");
                goto abort;
            }

            /* send and show our current statistics */
            ttp_update_stats(session);
         }
      }

    } /* Transfer of the file completes here*/

    printf("Transfer complete. Flushing to disk and signaling server to stop...\n");

    /*---------------------------
     * STOP TIMING
     *---------------------------*/

    /* tell the server to quit transmitting */
    if (ttp_request_stop(session) < 0) {
	warn("Could not request end of transfer");
	goto abort;
    }

    /* add a stop block to the ring buffer */
    datagram = ring_reserve(xfer->ring_buffer);
    *((u_int32_t *) datagram) = 0;
    if (ring_confirm(xfer->ring_buffer) < 0)
	warn("Error in terminating disk thread");

    /* wait for the disk thread to die */
    if (pthread_join(disk_thread_id, NULL) < 0)
	warn("Disk thread terminated with error");

    /*------------------------------------
     * MORE TRUE POINT TO STOP TIMING ;-)
     *-----------------------------------*/
    // time here would contain the additional delays from the
    // local disk flush and server xfer shutdown - include or omit?

    /* get finishing time */
    gettimeofday(&(xfer->stats.stop_time), NULL);
    delta = get_usec_since(&(xfer->stats.start_time));

    /* count the truly lost blocks from the 'received' bitmap table */
    xfer->stats.total_lost = 0;
    for (block=1; block <= xfer->block_count; block++) {
        if (!got_block(session, block)) xfer->stats.total_lost++;
    }

    /* display the final results */
    mbit_thru     = 8.0 * xfer->stats.total_blocks * session->parameter->block_size;
    mbit_good     = mbit_thru - 8.0 * xfer->stats.total_recvd_retransmits * session->parameter->block_size;
    mbit_file     = 8.0 * xfer->file_size;
    mbit_thru    /= (1024.0*1024.0);
    mbit_good    /= (1024.0*1024.0);
    mbit_file    /= (1024.0*1024.0);
    time_secs     = delta / 1e6;
    printf("PC performance figure : %Lu packets dropped (if high this indicates receiving PC overload)\n", 
                                         (ull_t)(xfer->stats.this_udp_errors - xfer->stats.start_udp_errors));
    printf("Transfer duration     : %0.2f seconds\n", time_secs);
    printf("Total packet data     : %0.2f Mbit\n", mbit_thru);
    printf("Goodput data          : %0.2f Mbit\n", mbit_good);
    printf("File data             : %0.2f Mbit\n", mbit_file);
    printf("Throughput            : %0.2f Mbps\n", mbit_thru / time_secs);
    printf("Goodput w/ restarts   : %0.2f Mbps\n", mbit_good / time_secs);
    printf("Final file rate       : %0.2f Mbps\n", mbit_file / time_secs);
    printf("Transfer mode         : ");
    if (session->parameter->lossless) {
        if (xfer->stats.total_lost == 0) {
           printf("lossless\n");
        } else {
           printf("lossless mode - but lost count=%u > 0, please file a bug report!!\n", xfer->stats.total_lost);
        }
    } else { 
        if (session->parameter->losswindow_ms == 0) {
            printf("lossy\n");
        } else {
            printf("semi-lossy, time window %d ms\n", session->parameter->losswindow_ms);
        }
        printf("Data blocks lost      : %Lu (%.2f%% of data) per user-specified time window constraint\n",
                  (ull_t)xfer->stats.total_lost, ( 100.0 * xfer->stats.total_lost ) / xfer->block_count );
    }
    printf("\n");

    /* update the transcript */
    if (session->parameter->transcript_yn) {
        xscript_data_stop(session, &(xfer->stats.stop_time));
        xscript_close(session, delta);
    }

    /* dump the received packet bitfield to a file, with added filename prefix ".blockmap" */
    if (session->parameter->blockdump) {
       FILE *fbits;
       u_char *dump_file;

       dump_file = calloc(strlen(xfer->local_filename) + 16, sizeof(u_char));
       strcpy((char*)dump_file, xfer->local_filename);
       strcat((char*)dump_file, ".blockmap");

       /* write: [4 bytes block_count] [map byte 0] [map byte 1] ... [map (partial) byte N] */
       fbits = fopen((char*)dump_file, "wb");
       if (fbits != NULL) {
         fwrite(&xfer->block_count, sizeof(xfer->block_count), 1, fbits);
         fwrite(xfer->received, sizeof(u_char), xfer->block_count / 8 + 1, fbits);
         fclose(fbits);
       } else {
         warn("Could not create a file for the blockmap dump");
       }
       free(dump_file);
    }

    /* close our open files */
    close(xfer->udp_fd);
    if (xfer->file     != NULL) { fclose(xfer->file);    xfer->file     = NULL; }

    /* deallocate memory */
    ring_destroy(xfer->ring_buffer);
    if (rexmit->table != NULL)  { free(rexmit->table);   rexmit->table  = NULL; }
    if (xfer->received != NULL) { free(xfer->received);  xfer->received = NULL; }
    if (local_datagram != NULL) { free(local_datagram);  local_datagram = NULL; }

    /* more files in "GET *" ? */
    } while(++f_counter<f_total);

    /* deallocate file list */
    if(multimode) {
       for(f_counter=0; f_counter<f_total; f_counter++) {
           free(file_names[f_counter]);
       }
       free(file_names);
    }

    /* we succeeded */
    return 0;

 abort:
    fprintf(stderr, "Transfer not successful.  (WARNING: You may need to reconnect.)\n\n");
    close(xfer->udp_fd);
    ring_destroy(xfer->ring_buffer);
    if (xfer->file     != NULL) { fclose(xfer->file);    xfer->file     = NULL; }
    if (rexmit->table  != NULL) { free(rexmit->table);   rexmit->table  = NULL; }
    if (xfer->received != NULL) { free(xfer->received);  xfer->received = NULL; }
    if (local_datagram != NULL) { free(local_datagram);  local_datagram = NULL; }    
    return -1;
}


/*------------------------------------------------------------------------
 * int command_help(command_t *command, ttp_session_t *session);
 *
 * Offers help on either the list of available commands or a particular
 * command.  Returns 0 on success and nonzero on failure, which is not
 * possible, but it normalizes the API.
 *------------------------------------------------------------------------*/
int command_help(command_t *command, ttp_session_t *session)
{
    /* if no command was supplied */
    if (command->count < 2) {
	printf("Help is available for the following commands:\n\n");
	printf("    close    connect    get    dir    help    quit    set\n\n");
	printf("Use 'help <command>' for help on an individual command.\n\n");

    /* handle the CLOSE command */
    } else if (!strcasecmp(command->text[1], "close")) {
	printf("Usage: close\n\n");
	printf("Closes the current connection to a remote Tsunami server.\n\n");

    /* handle the CONNECT command */
    } else if (!strcasecmp(command->text[1], "connect")) {
	printf("Usage: connect\n");
	printf("       connect <remote-host>\n");
	printf("       connect <remote-host> <remote-port>\n\n");
	printf("Opens a connection to a remote Tsunami server.  If the host and port\n");
	printf("are not specified, default values are used.  (Use the 'set' command to\n");
	printf("modify these values.)\n\n");
	printf("After connecting, you will be prompted to enter a shared secret for\n");
	printf("authentication.\n\n");

    /* handle the GET command */
    } else if (!strcasecmp(command->text[1], "get")) {
	printf("Usage: get <remote-file>\n");
	printf("       get <remote-file> <local-file>\n\n");
	printf("Attempts to retrieve the remote file with the given name using the\n");
	printf("Tsunami file transfer protocol.  If the local filename is not\n");
	printf("specified, the final part of the remote filename (after the last path\n");
	printf("separator) will be used.\n\n");

    /* handle the DIR command */
    } else if (!strcasecmp(command->text[1], "dir")) {
	printf("Usage: dir\n\n");
	printf("Attempts to list the available remote files.\n\n");

    /* handle the HELP command */
    } else if (!strcasecmp(command->text[1], "help")) {
	printf("Come on.  You know what that command does.\n\n");

    /* handle the QUIT command */
    } else if (!strcasecmp(command->text[1], "quit")) {
	printf("Usage: quit\n\n");
	printf("Closes any open connection to a remote Tsunami server and exits the\n");
	printf("Tsunami client.\n\n");

    /* handle the SET command */
    } else if (!strcasecmp(command->text[1], "set")) {
	printf("Usage: set\n");
	printf("       set <field>\n");
	printf("       set <field> <value>\n\n");
	printf("Sets one of the defaults to the given value.  If the value is omitted,\n");
	printf("the current value of the field is returned.  If the field is also\n");
	printf("omitted, the current values of all defaults are returned.\n\n");

    /* apologize for our ignorance */
    } else {
	printf("'%s' is not a recognized command.\n", command->text[1]);
	printf("Use 'help' for a list of commands.\n\n");
    }

    /* we succeeded */
    return 0;
}


/*------------------------------------------------------------------------
 * int command_quit(command_t *command, ttp_session_t *session);
 *
 * Closes the open connection (if there is one) and aborts the operation
 * of the Tsunami client.  For API uniformity, we pretend to return
 * something, but we don't.
 *------------------------------------------------------------------------*/
int command_quit(command_t *command, ttp_session_t *session)
{
    /* close the connection if there is one */
    if (session && (session->server != NULL))
	fclose(session->server);

    /* wave good-bye */
    printf("Thank you for using Tsunami.\n");
    printf("The ANML web site can be found at:    http://www.anml.iu.edu/\n");
    printf("The SourceForge site can be found at: http://tsunami-udp.sf.net/\n\n");

    /* and quit */
    exit(1);
    return 0;
}


/*------------------------------------------------------------------------
 * int command_set(command_t *command, ttp_parameter_t *parameter);
 *
 * Sets a particular parameter to the given value, or simply reports
 * on the current value of one or more fields.  Returns 0 on success
 * and nonzero on failure.
 *------------------------------------------------------------------------*/
int command_set(command_t *command, ttp_parameter_t *parameter)
{
    int do_all = (command->count == 1);

    /* handle actual set operations first */
    if (command->count == 3) {
    if (!strcasecmp(command->text[1], "server")) {
        if (parameter->server_name != NULL) free(parameter->server_name);
        parameter->server_name = strdup(command->text[2]);
        if (parameter->server_name == NULL) error("Could not update server name");
    } else if (!strcasecmp(command->text[1], "port"))       parameter->server_port   = atoi(command->text[2]);
      else if (!strcasecmp(command->text[1], "udpport"))    parameter->client_port   = atoi(command->text[2]);
      else if (!strcasecmp(command->text[1], "buffer"))     parameter->udp_buffer    = atol(command->text[2]);
      else if (!strcasecmp(command->text[1], "blocksize"))  parameter->block_size    = atol(command->text[2]);
      else if (!strcasecmp(command->text[1], "verbose"))    parameter->verbose_yn    = (strcmp(command->text[2], "yes") == 0);
      else if (!strcasecmp(command->text[1], "transcript")) parameter->transcript_yn = (strcmp(command->text[2], "yes") == 0);
      else if (!strcasecmp(command->text[1], "ip"))         parameter->ipv6_yn       = (strcmp(command->text[2], "v6")  == 0);
      else if (!strcasecmp(command->text[1], "output"))     parameter->output_mode   = (strcmp(command->text[2], "screen") ? LINE_MODE : SCREEN_MODE);
      else if (!strcasecmp(command->text[1], "rate"))       { 
        long multiplier = 1;
        char *cmd = (char*)command->text[2];
        char cpy[256];
        int l = strlen(cmd);
        strcpy(cpy, cmd);
        if(l>1 && (toupper(cpy[l-1]))=='M') { 
            multiplier = 1000000; cpy[l-1]='\0';  
        } else if(l>1 && toupper(cpy[l-1])=='G') { 
            multiplier = 1000000000; cpy[l-1]='\0';   
        }
        parameter->target_rate   = multiplier * atol(cpy); 
      }
      else if (!strcasecmp(command->text[1], "error"))        parameter->error_rate    = atof(command->text[2]) * 1000.0;
      else if (!strcasecmp(command->text[1], "slowdown"))     parse_fraction(command->text[2], &parameter->slower_num, &parameter->slower_den);
      else if (!strcasecmp(command->text[1], "speedup"))      parse_fraction(command->text[2], &parameter->faster_num, &parameter->faster_den);
      else if (!strcasecmp(command->text[1], "history"))      parameter->history       = atoi(command->text[2]);
      else if (!strcasecmp(command->text[1], "lossless"))     parameter->lossless      = (strcmp(command->text[2], "yes") == 0);
      else if (!strcasecmp(command->text[1], "losswindow"))   parameter->losswindow_ms = atol(command->text[2]);
      else if (!strcasecmp(command->text[1], "blockdump"))    parameter->blockdump     = (strcmp(command->text[2], "yes") == 0);    
      else if (!strcasecmp(command->text[1], "passphrase")) {
        if (parameter->passphrase != NULL) free(parameter->passphrase);
        parameter->passphrase = strdup(command->text[2]);
        if (parameter->passphrase == NULL) error("Could not update passphrase");
      }
    }

    /* report on current values */
    if (do_all || !strcasecmp(command->text[1], "server"))     printf("server = %s\n",      parameter->server_name);
    if (do_all || !strcasecmp(command->text[1], "port"))       printf("port = %u\n",        parameter->server_port);
    if (do_all || !strcasecmp(command->text[1], "udpport"))    printf("udpport = %u\n",     parameter->client_port);
    if (do_all || !strcasecmp(command->text[1], "buffer"))     printf("buffer = %u\n",      parameter->udp_buffer);
    if (do_all || !strcasecmp(command->text[1], "blocksize"))  printf("blocksize = %u\n",   parameter->block_size);
    if (do_all || !strcasecmp(command->text[1], "verbose"))    printf("verbose = %s\n",     parameter->verbose_yn    ? "yes" : "no");
    if (do_all || !strcasecmp(command->text[1], "transcript")) printf("transcript = %s\n",  parameter->transcript_yn ? "yes" : "no");
    if (do_all || !strcasecmp(command->text[1], "ip"))         printf("ip = %s\n",          parameter->ipv6_yn       ? "v6"  : "v4");
    if (do_all || !strcasecmp(command->text[1], "output"))     printf("output = %s\n",      (parameter->output_mode == SCREEN_MODE) ? "screen" : "line");
    if (do_all || !strcasecmp(command->text[1], "rate"))       printf("rate = %u\n",        parameter->target_rate);
    if (do_all || !strcasecmp(command->text[1], "error"))      printf("error = %0.2f%%\n",  parameter->error_rate / 1000.0);
    if (do_all || !strcasecmp(command->text[1], "slowdown"))   printf("slowdown = %d/%d\n", parameter->slower_num, parameter->slower_den);
    if (do_all || !strcasecmp(command->text[1], "speedup"))    printf("speedup = %d/%d\n",  parameter->faster_num, parameter->faster_den);
    if (do_all || !strcasecmp(command->text[1], "history"))    printf("history = %d%%\n",   parameter->history);
    if (do_all || !strcasecmp(command->text[1], "lossless"))   printf("lossless = %s\n",    parameter->lossless ? "yes" : "no");
    if (do_all || !strcasecmp(command->text[1], "losswindow")) printf("losswindow = %d msec\n", parameter->losswindow_ms);
    if (do_all || !strcasecmp(command->text[1], "blockdump"))  printf("blockdump = %s\n",   parameter->blockdump ? "yes" : "no");
    if (do_all || !strcasecmp(command->text[1], "passphrase")) printf("passphrase = %s\n",  (parameter->passphrase == NULL) ? "default" : "<user-specified>");
    printf("\n");

    /* we succeeded */
    return 0;
}


/*------------------------------------------------------------------------
 * void *disk_thread(void *arg);
 *
 * This is the thread that takes care of saved received blocks to disk.
 * It runs until the network thread sends it a datagram with a block
 * number of 0.  The return value has no meaning.
 *------------------------------------------------------------------------*/
void *disk_thread(void *arg)
{
    ttp_session_t *session = (ttp_session_t *) arg;
    u_char        *datagram;
    int            status;
    u_int32_t      block_index;
    u_int16_t      block_type;

    /* while the world is turning */
    while (1) {

	/* get another block */
	datagram    = ring_peek(session->transfer.ring_buffer);
	block_index = ntohl(*((u_int32_t *) datagram));
	block_type  = ntohs(*((u_int16_t *) (datagram + 4)));

	/* quit if we got the mythical 0 block */
	if (block_index == 0) {
	    printf("!!!!\n");
	    return NULL;
	}

	/* save it to disk */
	status = accept_block(session, block_index, datagram + 6);
	if (status < 0) {
	    warn("Block accept failed");
	    return NULL;
	}

	/* pop the block */
	ring_pop(session->transfer.ring_buffer);
    }
}


/*------------------------------------------------------------------------
 * int parse_fraction(const char *fraction,
 *                    u_int16_t *num, u_int16_t *den);
 *
 * Given a string in the form "nnn/ddd", saves the numerator and
 * denominator in the given 16-bit locations.  Returns 0 on success and
 * nonzero on error.
 *------------------------------------------------------------------------*/
int parse_fraction(const char *fraction, u_int16_t *num, u_int16_t *den)
{
    const char *slash;

    /* get the location of the '/' */
    slash = strchr(fraction, '/');
    if (slash == NULL)
	return warn("Value is not a fraction");

    /* store the two parts of the value */
    *num = atoi(fraction);
    *den = atoi(slash + 1);

    /* we succeeded */
    return 0;
}


/*------------------------------------------------------------------------
 * int got_block(ttp_session_t* session, u_int32_t blocknr)
 *
 * Returns non-0 if the block has already been received
 *------------------------------------------------------------------------*/
int got_block(ttp_session_t* session, u_int32_t blocknr)
{
    return (session->transfer.received[blocknr / 8] & (1 << (blocknr % 8)));
}


/*========================================================================
 * $Log: command.c,v $
 * Revision 1.27  2014/09/03 05:59:11  jwagnerhki
 * removed inline as OS X Clang does not like it http://clang.llvm.org/compatibility.html#inline
 *
 * Revision 1.26  2009/12/22 23:22:42  jwagnerhki
 * at end of xfer first stop server then flush
 *
 * Revision 1.25  2009/12/21 17:14:10  jwagnerhki
 * dont increment gapless_to_block infinitely
 *
 * Revision 1.24  2009/12/21 17:05:09  jwagnerhki
 * sends stats even during restart pending
 *
 * Revision 1.23  2008/07/19 20:59:15  jwagnerhki
 * use xfer->restart_wireclearidx as upper limit to ignored blocks
 *
 * Revision 1.22  2008/07/19 20:44:34  jwagnerhki
 * show Mbit File Data, transcript.c show mbyte_transmitted mbyte_usable mbyte_file
 *
 * Revision 1.21  2008/07/19 20:30:50  jwagnerhki
 * added DEBUG_RETX, rate with req restarts, actual file transfer rate, comment cleanup
 *
 * Revision 1.20  2008/07/19 20:01:25  jwagnerhki
 * gapless_to_block, ttp_repeat_retransmit changed to purge duplicates first then decide on request-restart, more DEBUG_RETX output
 *
 * Revision 1.19  2008/07/19 19:42:32  jwagnerhki
 * inline got_block
 *
 * Revision 1.18  2008/05/25 15:39:32  jwagnerhki
 * file client merge
 *
 * Revision 1.17  2008/05/22 18:30:44  jwagnerhki
 * Darwin fix LFS support fopen() not fopen64() etc
 *
 * Revision 1.16  2008/05/20 18:12:45  jwagnerhki
 * got_block and tidying
 *
 * Revision 1.15  2007/12/07 18:10:28  jwagnerhki
 * cleaned away 64-bit compile warnings, used tsunami-client.h
 *
 * Revision 1.25  2007/08/22 14:12:34  jwagnerhki
 * fix command_dir wrong result val check for backwards compatibility with server
 *
 * Revision 1.24  2007/08/22 14:07:30  jwagnerhki
 * build 27: first implementation of client dir command
 *
 * Revision 1.23  2007/08/17 10:56:31  jwagnerhki
 * added gapless_till_block client side counter
 *
 * Revision 1.22  2007/08/14 08:50:39  jwagnerhki
 * initialize server_address_length, do not have recvfrom overwrite server_address and server_address_length
 *
 * Revision 1.21  2007/07/10 08:18:05  jwagnerhki
 * rtclient merge, multiget cleaned up and improved, allow 65530 files in multiget
 *
 * Revision 1.20  2007/06/19 13:35:24  jwagnerhki
 * replaced notretransmit option with better time-limited restransmission window, reduced ringbuffer from 8192 to 4096 entries
 *
 * Revision 1.19  2007/05/31 09:32:03  jwagnerhki
 * removed some signedness warnings, added Mark5 server devel start code
 *
 * Revision 1.18  2007/05/24 10:07:21  jwagnerhki
 * client can 'set' passphrase to other than default
 *
 * Revision 1.17  2007/01/11 15:15:48  jwagnerhki
 * rtclient merge, io.c now with VSIB_REALTIME, blocks_left not allowed negative fix, overwriting file check fixed, some memset()s to keep Valgrind warnings away
 *
 * Revision 1.16  2006/12/21 13:50:33  jwagnerhki
 * added to client something that smells like a fix for non-working REQUEST_RESTART
 *
 * Revision 1.15  2006/12/15 12:57:41  jwagnerhki
 * added client 'blockdump' block bitmap dump to file feature
 *
 * Revision 1.14  2006/12/11 13:44:17  jwagnerhki
 * OS UDP err count now done in ttp_update_stats(), cleaned stats printout align, fixed CLOSE cmd segfault
 *
 * Revision 1.13  2006/12/11 11:11:55  jwagnerhki
 * show operating system UDP rx error stats in summary
 *
 * Revision 1.12  2006/12/05 15:24:49  jwagnerhki
 * now noretransmit code in client only, merged rt client code
 *
 * Revision 1.11  2006/12/01 15:25:29  jwagnerhki
 * errormsg in mget if server offers no files
 *
 * Revision 1.10  2006/11/10 11:32:42  jwagnerhki
 * indentation, transmit termination fix
 *
 * Revision 1.9  2006/11/06 07:42:44  jwagnerhki
 * changed defaults, added 144MiB bigbufsize mention
 *
 * Revision 1.8  2006/10/28 19:29:15  jwagnerhki
 * jamil GET* merge, insertionsort disabled by default again
 *
 * Revision 1.7  2006/10/28 17:00:12  jwagnerhki
 * block type defines
 *
 * Revision 1.6  2006/10/24 21:21:36  jwagnerhki
 * fixed client loosing password
 *
 * Revision 1.5  2006/10/11 08:36:50  jwagnerhki
 * added URL to SF project page for the byebye msg
 *
 * Revision 1.4  2006/09/07 13:56:57  jwagnerhki
 * udp socket reusable, udp port selectable in client
 *
 * Revision 1.3  2006/08/08 06:04:28  jwagnerhki
 * included ctype.h
 *
 * Revision 1.2  2006/07/21 07:48:56  jwagnerhki
 * set rate now also with mM gG
 *
 * Revision 1.1.1.1  2006/07/20 09:21:18  jwagnerhki
 * reimport
 *
 * Revision 1.1  2006/07/10 12:26:51  jwagnerhki
 * deleted unnecessary files
 *
 */
