/*========================================================================
 * main.c  --  Main routine and supporting function for Tsunami server.
 *
 * This is the persistent process that sends out files upon request.
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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <errno.h>       /* for the errno variable and perror()   */
#include <fcntl.h>       /* for the fcntl() function              */
#include <getopt.h>      /* for getopt_long()                     */
#include <signal.h>      /* for standard POSIX signal handling    */
#include <stdlib.h>      /* for memory allocation, exit(), etc.   */
#include <string.h>      /* for memset(), sprintf(), etc.         */
#include <sys/types.h>   /* for standard system data types        */
#include <sys/socket.h>  /* for the BSD sockets library           */
#include <sys/stat.h>
#include <arpa/inet.h>   /* for inet_ntoa()                       */
#include <sys/wait.h>    /* for waitpid()                         */
#include <unistd.h>      /* for Unix system calls                 */

#include <tsunami-server.h>
#ifdef VSIB_REALTIME
#include "vsibctl.h"
#endif

/*------------------------------------------------------------------------
 * Function prototypes (module scope).
 *------------------------------------------------------------------------*/

void client_handler (ttp_session_t *session);
void process_options(int argc, char *argv[], ttp_parameter_t *parameter);
void reap           (int signum);


/*------------------------------------------------------------------------
 * MAIN PROGRAM
 *------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    int                server_fd, client_fd;
    struct sockaddr_in remote_address;
    socklen_t          remote_length = sizeof(struct sockaddr_in);
    ttp_parameter_t    parameter;
    ttp_session_t      session;
    pid_t              child_pid;

    /* initialize our parameters */
    memset(&session, 0, sizeof(session));
    reset_server(&parameter);

    /* process our command-line options */
    process_options(argc, argv, &parameter);

    /* obtain our server socket */
    server_fd = create_tcp_socket(&parameter);
    if (server_fd < 0) {
        sprintf(g_error, "Could not create server socket on port %d", parameter.tcp_port);
        return error(g_error);
    }

    /* install a signal handler for our children */
    #ifndef VSIB_REALTIME
    signal(SIGCHLD, reap);
    #endif

    /* now show version / build information */
    #ifdef VSIB_REALTIME
    fprintf(stderr, "Tsunami Realtime Server for protocol rev %X\nRevision: %s\nCompiled: %s %s\n"
                    "   /dev/vsib VSIB accesses mode=%d, sample skip=%d, gigabit=%d, 1pps embed=%d\n"
                    "Waiting for clients to connect.\n",
            PROTOCOL_REVISION, TSUNAMI_CVS_BUILDNR, __DATE__ , __TIME__,
            vsib_mode, vsib_mode_skip_samples, vsib_mode_gigabit, vsib_mode_embed_1pps_markers);
    #else
    fprintf(stderr, "Tsunami Server for protocol rev %X\nRevision: %s\nCompiled: %s %s\n"
                    "Waiting for clients to connect.\n",
            PROTOCOL_REVISION, TSUNAMI_CVS_BUILDNR, __DATE__ , __TIME__);
    #endif

    /* while our little world keeps turning */
    while (1) {

        /* accept a new client connection */
        client_fd = accept(server_fd, (struct sockaddr *) &remote_address, &remote_length);
        if (client_fd < 0) {
            warn("Could not accept client connection");
            continue;
        } else {
            fprintf(stderr, "New client connecting from %s...\n", inet_ntoa(remote_address.sin_addr));
        }

        session.session_id++;

        #ifndef VSIB_REALTIME
        /* fork a new child process to handle the client */
        child_pid = fork();
        if (child_pid < 0) {
            warn("Could not create child process");
            continue;
        }

        /* if we're the child */
        if (child_pid == 0) {

            /* close the server socket */
            close(server_fd);

            /* set up the session structure */
            session.client_fd = client_fd;
            session.parameter = &parameter;
            memset(&session.transfer, 0, sizeof(session.transfer));
            session.transfer.ipd_current = 0.0;

            /* and run the client handler */
            client_handler(&session);
            return 0;

        /* if we're the parent */
        } else {

            /* close the client socket */
            close(client_fd);
        }
        #else

        /* The VSIB supports only single user access:  *
         * do not fork a client handler processes      */

        /* set up the session structure */
        session.client_fd = client_fd;
        session.parameter = &parameter;
        memset(&session.transfer, 0, sizeof(session.transfer));
        session.transfer.ipd_current = 0.0;

        /* run the client handler */
        client_handler(&session);

        /* close the client */
        close(client_fd);
        #endif
    }
}


/*------------------------------------------------------------------------
 * void client_handler(ttp_session_t *session);
 *
 * This routine is run by the client processes that are created in
 * response to incoming connections.
 *------------------------------------------------------------------------*/
void client_handler(ttp_session_t *session)
{
    retransmission_t  retransmission;                /* the retransmission data object                 */
    struct timeval    start, stop;                   /* the start and stop times for the transfer      */
    struct timeval    prevpacketT;                   /* the send time of the previous packet           */
    struct timeval    currpacketT;                   /* the interpacket delay value                    */
    struct timeval    lastfeedback;                  /* the time since last client feedback            */
    struct timeval    lasthblostreport;              /* the time since last 'heartbeat lost' report    */
    u_int32_t         deadconnection_counter;        /* the counter for checking dead conn timeout     */
    int               retransmitlen = 0;             /* number of bytes read from retransmission queue */
    u_char            datagram[MAX_BLOCK_SIZE + 6];  /* the datagram containing the file block         */
    int64_t           ipd_time;                      /* the time to delay/sleep after packet, signed   */
    int64_t           ipd_usleep_diff;               /* the time correction to ipd_time, signed        */
    int64_t           ipd_time_max;
    int               status;
    ttp_transfer_t   *xfer  = &session->transfer;
    ttp_parameter_t  *param =  session->parameter;
    u_int64_t         delta;
    u_char            block_type;

    /* negotiate the connection parameters */
    status = ttp_negotiate(session);
    if (status < 0)
        error("Protocol revision number mismatch");

    /* have the client try to authenticate to us */
    status = ttp_authenticate(session, session->parameter->secret);
    if (status < 0)
        error("Client authentication failure");

    if (1==param->verbose_yn) {
        fprintf(stderr,"Client authenticated. Negotiated parameters are:\n");
        fprintf(stderr,"Block size: %d\n", param->block_size);
        fprintf(stderr,"Buffer size: %d\n", param->udp_buffer); 
        fprintf(stderr,"Port: %d\n", param->tcp_port);    
    }

    /* while we haven't been told to stop */
    while (1) {

    /* make the client descriptor blocking */
    status = fcntl(session->client_fd, F_SETFL, 0);
    if (status < 0)
        error("Could not make client socket blocking");

    /* negotiate another transfer */
    status = ttp_open_transfer(session);
    if (status < 0) {
        warn("Invalid file request");
        #ifndef VSIB_REALTIME
        continue;
        #else
        return;
        #endif
    }

    /* negotiate a data transfer port */
    status = ttp_open_port(session);
    if (status < 0) {
        warn("UDP socket creation failed");
        continue;
    }

    /* make the client descriptor non-blocking again */
    status = fcntl(session->client_fd, F_SETFL, O_NONBLOCK);
    if (status < 0)
        error("Could not make client socket non-blocking");

    /*---------------------------
     * START TIMING
     *---------------------------*/
    gettimeofday(&start, NULL);
    if (param->transcript_yn)
        xscript_data_start(session, &start);

    lasthblostreport       = start;
    lastfeedback           = start;
    prevpacketT            = start;
    deadconnection_counter = 0;
    ipd_time               = 0;
    ipd_time_max           = 0;
    ipd_usleep_diff        = 0;
    retransmitlen          = 0;

    /* start by blasting out every block */
    xfer->block = 0;
    while (xfer->block <= param->block_count) {

        /* default: flag as retransmitted block */
        block_type = TS_BLOCK_RETRANSMISSION;

        /* precalculate time to wait after sending the next packet */
        gettimeofday(&currpacketT, NULL);
        ipd_usleep_diff = xfer->ipd_current + tv_diff_usec(prevpacketT, currpacketT);
        prevpacketT = currpacketT;
        if (ipd_usleep_diff > 0 || ipd_time > 0) {
            ipd_time += ipd_usleep_diff;
        }
        ipd_time_max = (ipd_time > ipd_time_max) ? ipd_time : ipd_time_max;

        /* see if transmit requests are available */
        status = read(session->client_fd, ((char*)&retransmission)+retransmitlen, sizeof(retransmission)-retransmitlen);
        #ifndef VSIB_REALTIME
        if ((status <= 0) && (errno != EAGAIN))
            error("Retransmission read failed");
        #else
        if ((status <= 0) && (errno != EAGAIN) && (!session->parameter->fileout))
            error("Retransmission read failed and not writing local backup file");
        #endif
        if (status > 0)
            retransmitlen += status;


        /* if we have a retransmission */
        if (retransmitlen == sizeof(retransmission_t)) {

            /* store current time */
            lastfeedback           = currpacketT;
            lasthblostreport       = currpacketT;
            deadconnection_counter = 0;

            /* if it's a stop request, go back to waiting for a filename */
            if (ntohs(retransmission.request_type) == REQUEST_STOP) {
                fprintf(stderr, "Transmission complete.\n");
                break;
            }

            /* otherwise, handle the retransmission */
            status = ttp_accept_retransmit(session, &retransmission, datagram);
            if (status < 0)
                warn("Retransmission error");
            retransmitlen = 0;

        /* if we have no retransmission */
        } else if (retransmitlen < sizeof(retransmission_t)) {

            /* build the block */
            xfer->block = min(xfer->block + 1, param->block_count);
            block_type = (xfer->block == param->block_count) ? TS_BLOCK_TERMINATE : TS_BLOCK_ORIGINAL;
            status = build_datagram(session, xfer->block, block_type, datagram);
            if (status < 0) {
                sprintf(g_error, "Could not read block #%u", xfer->block);
                error(g_error);
            }

            /* transmit the block */
            status = sendto(xfer->udp_fd, datagram, 6 + param->block_size, 0, xfer->udp_address, xfer->udp_length);
            if (status < 0) {
                sprintf(g_error, "Could not transmit block #%u", xfer->block);
                warn(g_error);
                continue;
            }

        /* if we have a partial retransmission message */
        } else if (retransmitlen > sizeof(retransmission_t)) {

            fprintf(stderr, "warn: retransmitlen > %d\n", (int)sizeof(retransmission_t));
            retransmitlen = 0;

        }

        /* monitor client heartbeat and disconnect dead client */
        if ((deadconnection_counter++) > 2048) {
            char stats_line[160];

            deadconnection_counter = 0;

            /* limit 'heartbeat lost' reports to 500ms intervals */
            if (get_usec_since(&lasthblostreport) < 500000.0) continue;
            gettimeofday(&lasthblostreport, NULL);

            /* throttle IPD with fake 100% loss report */
            #ifndef VSIB_REALTIME
            retransmission.request_type = htons(REQUEST_ERROR_RATE);
            retransmission.error_rate   = htonl(100000);
            retransmission.block = 0;
            ttp_accept_retransmit(session, &retransmission, datagram);
            #endif

            delta = get_usec_since(&lastfeedback);

            /* show an (additional) statistics line */
            snprintf(stats_line, sizeof(stats_line)-1,
                                "   n/a     n/a     n/a %7u %6.2f %3u -- no heartbeat since %3.2fs\n",
                                xfer->block, 100.0 * xfer->block / param->block_count, session->session_id,
                                1e-6*delta);
            if (param->transcript_yn)
               xscript_data_log(session, stats_line);
            fprintf(stderr, "%s", stats_line);

            /* handle timeout for normal file transfers */
            #ifndef VSIB_REALTIME
            if ((1e-6 * delta) > param->hb_timeout) {
                fprintf(stderr, "Heartbeat timeout of %d seconds reached, terminating transfer.\n", param->hb_timeout);
                break;
            }
            #else
            /* handle timeout condition for : realtime with local backup, simple realtime */
            if ((1e-6 * delta) > param->hb_timeout) {
                if ((session->parameter->fileout) && (block_type == TS_BLOCK_TERMINATE)) {
                    fprintf(stderr, "Reached the Terminate block and timed out, terminating transfer.\n");
                    break;
                } else if(!session->parameter->fileout) {
                    fprintf(stderr, "Heartbeat timeout of %d seconds reached and not doing local backup, terminating transfer now.\n", param->hb_timeout);
                    break;
                } else {
                    lastfeedback = currpacketT;
                }
            }
            #endif
        }

         /* wait before handling the next packet */
         if (block_type == TS_BLOCK_TERMINATE) {
             usleep_that_works(10*ipd_time_max);
         }
         if (ipd_time > 0) {
             usleep_that_works(ipd_time);
         }

    }

    /*---------------------------
     * STOP TIMING
     *---------------------------*/
    gettimeofday(&stop, NULL);
    if (param->transcript_yn)
        xscript_data_stop(session, &stop);
    delta = 1000000LL * (stop.tv_sec - start.tv_sec) + stop.tv_usec - start.tv_usec;

    /* report on the transfer */
    if (param->verbose_yn)
        fprintf(stderr, "Server %d transferred %llu bytes in %0.2f seconds (%0.1f Mbps)\n",
                session->session_id, (ull_t)param->file_size, delta / 1000000.0,
                8.0 * param->file_size / (delta * 1e-6 * 1024*1024) );

    /* close the transcript */
    if (param->transcript_yn)
        xscript_close(session, delta);

    #ifndef VSIB_REALTIME

    /* close the file */
    fclose(xfer->file);

    #else

    /* VSIB local disk copy: close file only if file output was requested */
    if (param->fileout) {
        fclose(xfer->file);
    }

    /* stop the VSIB */
    stop_vsib(session);
    fclose(xfer->vsib);

    #endif

    /* close the UDP socket */
    close(xfer->udp_fd);
    memset(xfer, 0, sizeof(*xfer));

    } //while(1)

}


/*------------------------------------------------------------------------
 * void process_options(int argc, char *argv[],
 *                      ttp_parameter_t *parameter);
 *
 * Processes the command-line options and sets the protocol parameters
 * as appropriate.
 *------------------------------------------------------------------------*/
void process_options(int argc, char *argv[], ttp_parameter_t *parameter)
{
    struct option long_options[] = { { "verbose",    0, NULL, 'v' },
                     { "transcript", 0, NULL, 't' },
                     { "v6",         0, NULL, '6' },
                     { "port",       1, NULL, 'p' },
                     { "secret",     1, NULL, 's' },
                     { "buffer",     1, NULL, 'b' },
                     { "hbtimeout",  1, NULL, 'h' },
                     { "v",          0, NULL, 'v' },
                     #ifdef VSIB_REALTIME
                     { "vsibmode",   1, NULL, 'M' },
                     { "vsibskip",   1, NULL, 'S' },
                     #endif
                     { NULL,         0, NULL, 0 } };
    struct stat   filestat;
    int           which;

    /* for each option found */
    while ((which = getopt_long(argc, argv, "+", long_options, NULL)) > 0) {

    /* depending on which option we got */
    switch (which) {

        /* --verbose    : enter verbose mode for debugging */
        case 'v':  parameter->verbose_yn = 1;
             break;

        /* --transcript : enter transcript mode for recording stats */
        case 't':  parameter->transcript_yn = 1;
                 break;

        /* --v6         : enter IPv6 mode */
        case '6':  parameter->ipv6_yn = 1;
                 break;

        /* --port=i     : port number for the server */
        case 'p':  parameter->tcp_port   = atoi(optarg);
             break;

        /* --secret=s   : shared secret for the client and server */
        case 's':  parameter->secret     = (unsigned char*)optarg;
             break;

        /* --buffer=i   : size of socket buffer */
        case 'b':  parameter->udp_buffer = atoi(optarg);
             break;

        /* --hbtimeout=i : client heartbeat timeout in seconds */
        case 'h': parameter->hb_timeout = atoi(optarg);
            break;

        #ifdef VSIB_REALTIME
        /* --vsibmode=i   : size of socket buffer */
        case 'M':  vsib_mode = atoi(optarg);
             break;

        /* --vsibskip=i   : size of socket buffer */
        case 'S':  vsib_mode_skip_samples = atoi(optarg);
             break;
        #endif

        /* otherwise    : display usage information */
        default: 
             fprintf(stderr, "Usage: tsunamid [--verbose] [--transcript] [--v6] [--port=n] [--buffer=bytes]\n");
             fprintf(stderr, "                [--hbtimeout=seconds] ");
             #ifdef VSIB_REALTIME
             fprintf(stderr, "[--vsibmode=mode] [--vsibskip=skip] [filename1 filename2 ...]\n\n");
             #else
             fprintf(stderr, "[filename1 filename2 ...]\n\n");
             #endif
             fprintf(stderr, "verbose or v : turns on verbose output mode\n");
             fprintf(stderr, "transcript   : turns on transcript mode for statistics recording\n");
             fprintf(stderr, "v6           : operates using IPv6 instead of (not in addition to!) IPv4\n");
             fprintf(stderr, "port         : specifies which TCP port on which to listen to incoming connections\n");
             fprintf(stderr, "secret       : specifies the shared secret for the client and server\n");
             fprintf(stderr, "buffer       : specifies the desired size for UDP socket send buffer (in bytes)\n");
             fprintf(stderr, "hbtimeout    : specifies the timeout in seconds for disconnect after client heartbeat lost\n");
             #ifdef VSIB_REALTIME
             fprintf(stderr, "vsibmode     : specifies the VSIB mode to use (see VSIB documentation for modes)\n");
             fprintf(stderr, "vsibskip     : a value N other than 0 will skip N samples after every 1 sample\n");
             #endif
             fprintf(stderr, "filenames    : list of files to share for downloaded via a client 'GET *'\n");
             fprintf(stderr, "\n");
             fprintf(stderr, "Defaults: verbose    = %d\n",   DEFAULT_VERBOSE_YN);
             fprintf(stderr, "          transcript = %d\n",   DEFAULT_TRANSCRIPT_YN);
             fprintf(stderr, "          v6         = %d\n",   DEFAULT_IPV6_YN);
             fprintf(stderr, "          port       = %d\n",   DEFAULT_TCP_PORT);
             fprintf(stderr, "          buffer     = %d bytes\n",   DEFAULT_UDP_BUFFER);
             fprintf(stderr, "          hbtimeout  = %d seconds\n",   DEFAULT_HEARTBEAT_TIMEOUT);
             #ifdef VSIB_REALTIME
             fprintf(stderr, "          vsibmode   = %d\n",   0);
             fprintf(stderr, "          vsibskip   = %d\n",   0);
             #endif
             fprintf(stderr, "\n");
             exit(1);
    }
    }

    if (argc>optind) {
        int counter;
        parameter->file_names = argv+optind;
        parameter->file_name_size = 0;
        parameter->total_files = argc-optind;
        parameter->file_sizes = (size_t*)malloc(sizeof(size_t) * parameter->total_files);
        fprintf(stderr, "\nThe specified %d files will be listed on GET *:\n", parameter->total_files);
        for (counter=0; counter < argc-optind; counter++) {
            stat(parameter->file_names[counter], &filestat);
            parameter->file_sizes[counter] = filestat.st_size;
            parameter->file_name_size += strlen(parameter->file_names[counter])+1;
            fprintf(stderr, " %3d)   %-20s  %llu bytes\n", counter+1, parameter->file_names[counter], (ull_t)parameter->file_sizes[counter]);
        }
        fprintf(stderr, "total characters %d\n", parameter->file_name_size);
    }

    if (1==parameter->verbose_yn) {
       fprintf(stderr,"Block size: %d\n", parameter->block_size);
       fprintf(stderr,"Buffer size: %d\n", parameter->udp_buffer);
       fprintf(stderr,"Port: %d\n", parameter->tcp_port);
    }
}


/*------------------------------------------------------------------------
 * void reap(int signum);
 *
 * A signal handler to take care of our children's deaths (SIGCHLD).
 *------------------------------------------------------------------------*/
void reap(int signum)
{
    int status;

    /* accept as many deaths as we can */
    while (waitpid(-1, &status, WNOHANG) > 0) {
        fprintf(stderr,"Child server process terminated with status code 0x%X\n", status);
    }

    /* reenable the handler */
    signal(SIGCHLD, reap);
}


/*========================================================================
 * $Log: main.c,v $
 * Revision 1.46  2009/12/22 23:13:22  jwagnerhki
 * merge with non-rt server, throttle the blast speed of terminate-block
 *
 * Revision 1.45  2009/12/21 14:44:16  jwagnerhki
 * fix Ubuntu Karmic compile warning, clear str message before read
 *
 * Revision 1.44  2009/05/18 09:46:14  jwagnerhki
 * removed %% from stats line
 *
 * Revision 1.43  2009/05/18 08:40:31  jwagnerhki
 * Lu formatting to llu
 *
 * Revision 1.42  2008/07/19 14:29:35  jwagnerhki
 * Mbps rate reported with 2-pow-20
 *
 * Revision 1.41  2008/07/18 06:27:06  jwagnerhki
 * build 37 with iperf-style server send rate control
 *
 * Revision 1.40  2008/07/10 11:17:37  jwagnerhki
 * close VSIB after transfer
 *
 * Revision 1.39  2008/05/29 09:15:59  jwagnerhki
 * single-user realtime, don't exit on client disconnect
 *
 * Revision 1.38  2008/05/29 07:07:22  jwagnerhki
 * first try at singleuser rtserver
 *
 * Revision 1.37  2008/04/25 10:37:15  jwagnerhki
 * build35 changed 'ipd_current' from int32 to double for much smoother rate changes
 *
 * Revision 1.36  2008/01/16 11:14:24  jwagnerhki
 * removed server --datagram option as this is client-side specified
 *
 * Revision 1.35  2008/01/11 08:35:21  jwagnerhki
 * tighter IPD control like in v1.2 petabit Tsunami
 *
 * Revision 1.34  2007/12/07 18:10:28  jwagnerhki
 * cleaned away 64-bit compile warnings, used tsunami-client.h
 *
 * Revision 1.33  2007/11/29 10:58:46  jwagnerhki
 * data skip fixed with vsib fread() not read(), heartbeat lost messages now in at most 350ms intervals
 *
 * Revision 1.30  2007/10/30 09:17:05  jwagnerhki
 * backupping rtserver don't disconnect at tcp EOF yet
 *
 * Revision 1.29  2007/10/29 15:30:25  jwagnerhki
 * timeout feature for rttsunamid too, added version info to transcripts, added --hbimeout srv cmd line param
 *
 * Revision 1.28  2007/10/26 08:00:08  jwagnerhki
 * corrected help-text on vsibskip
 *
 * Revision 1.27  2007/10/05 06:13:52  jwagnerhki
 * tabs to spaces
 *
 * Revision 1.26  2007/10/05 06:07:16  jwagnerhki
 * tabs to spaces
 *
 * Revision 1.25  2007/09/05 08:42:49  jwagnerhki
 * printf filelen now unsigned, realtime server no 15s timeout
 *
 * Revision 1.24  2007/09/04 15:39:24  jwagnerhki
 * different timeout code for realtime
 *
 * Revision 1.23  2007/08/22 12:34:12  jwagnerhki
 * read in file length of commandline shared files
 *
 * Revision 1.22  2007/08/10 09:19:35  jwagnerhki
 * server closes connection if no client feedback in 15s
 *
 * Revision 1.21  2007/08/10 09:05:06  jwagnerhki
 * 5 second timeout on no client feedback
 *
 * Revision 1.20  2007/07/16 09:51:10  jwagnerhki
 * rt-server now ipd-throttled again
 *
 * Revision 1.19  2007/07/16 08:55:54  jwagnerhki
 * build 21, upped 16 to 256 clients, reduced end block blast speed, enabled RETX_REQBLOCK_SORTING compile flag
 *
 * Revision 1.18  2007/07/14 17:06:24  jwagnerhki
 * show client IP prior to auth
 *
 * Revision 1.17  2007/07/10 08:18:07  jwagnerhki
 * rtclient merge, multiget cleaned up and improved, allow 65530 files in multiget
 *
 * Revision 1.16  2007/05/31 09:16:12  jwagnerhki
 * removed 2 compiler warnings
 *
 * Revision 1.15  2007/05/25 08:30:22  jwagnerhki
 * realtime server enabled no extra delay on new VSIB blocks
 *
 * Revision 1.14  2007/05/18 12:49:32  jwagnerhki
 * added Realtime to printed name
 *
 * Revision 1.13  2007/03/23 07:23:15  jwagnerhki
 * added rttsunamid vsib mode and skip CLI options
 *
 * Revision 1.12  2006/12/05 15:24:50  jwagnerhki
 * now noretransmit code in client only, merged rt client code
 *
 * Revision 1.11  2006/12/05 13:38:20  jwagnerhki
 * identify concurrent server transfers by an own ID
 *
 * Revision 1.10  2006/12/04 14:45:34  jwagnerhki
 * added more proper TSUNAMI_CVS_BUILDNR, added exit and bye commands to client
 *
 * Revision 1.9  2006/11/08 11:45:04  jwagnerhki
 * vsib read without IPD throttling
 *
 * Revision 1.8  2006/10/30 08:46:58  jwagnerhki
 * removed memory leak unused ringbuf
 *
 * Revision 1.7  2006/10/28 17:00:12  jwagnerhki
 * block type defines
 *
 * Revision 1.6  2006/10/25 14:27:09  jwagnerhki
 * typo fix
 *
 * Revision 1.5  2006/10/25 13:32:08  jwagnerhki
 * build cmd line args filelist for 'get *'
 *
 * Revision 1.4  2006/10/24 19:41:12  jwagnerhki
 * realtime and normal server.c now identical, define VSIB_REALTIME for mode
 *
 * Revision 1.3  2006/10/24 19:14:28  jwagnerhki
 * moved server.h into common tsunami-server.h
 *
 * Revision 1.2  2006/10/19 07:19:40  jwagnerhki
 * show proto rev and build number, rttsunamid show startup settings like tsunamid
 *
 * Revision 1.1.1.1  2006/07/20 09:21:20  jwagnerhki
 * reimport
 *
 * Revision 1.1  2006/07/10 12:39:52  jwagnerhki
 * added to trunk
 *
 */
