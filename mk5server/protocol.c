/*========================================================================
 * protocol.c  --  TTP routines for Tsunami server.
 *
 * This contains the Tsunami Transfer Protocol API for the Tsunami file
 * transfer server.
 *
 * Written by Mark Meiss (mmeiss@indiana.edu).
 * Copyright  2002 The Trustees of Indiana University.
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
 * NOWARRANTIES AS TO CAPABILITIES OR ACCURACY ARE MADE. INDIANA
 * UNIVERSITY GIVES NO WARRANTIES AND MAKES NO REPRESENTATION THAT
 * SOFTWARE IS FREE OF INFRINGEMENT OF THIRD PARTY PATENT, COPYRIGHT,
 * OR OTHER PROPRIETARY RIGHTS. INDIANA UNIVERSITY MAKES NO
 * WARRANTIES THAT SOFTWARE IS FREE FROM "BUGS", "VIRUSES", "TROJAN
 * HORSES", "TRAP DOORS", "WORMS", OR OTHER HARMFUL CODE.  LICENSEE
 * ASSUMES THE ENTIRE RISK AS TO THE PERFORMANCE OF SOFTWARE AND/OR
 * ASSOCIATED MATERIALS, AND TO THE PERFORMANCE AND VALIDITY OF
 * INFORMATION GENERATED USING SOFTWARE.
 *========================================================================*/

#include <string.h>      /* for memset(), strdup(), etc.   */
#include <sys/types.h>   /* for standard system data types */
#include <sys/socket.h>  /* for the BSD sockets library    */
#include <sys/time.h>    /* gettimeofday()                 */
#include <time.h>        /* for time()                     */
#include <unistd.h>      /* for standard Unix system calls */
#include <assert.h>
#include <math.h>        /* floor() */
#include <malloc.h>

#include <tsunami-server.h>
#include "mk5api.h"

/*------------------------------------------------------------------------
 * int ttp_accept_retransmit(ttp_session_t *session,
 *                           retransmission_t *retransmission,
 *                           u_char *datagram);
 *
 * Handles the given retransmission request.  The actions taken depend
 * on the nature of the request:
 *
 *   REQUEST_RETRANSMIT -- Retransmit the given block.
 *   REQUEST_RESTART    -- Restart the transfer at the given block.
 *   REQUEST_ERROR_RATE -- Use the given error rate to adjust the IPD.
 *
 * For REQUEST_RETRANSMIT messsages, the given buffer must be large
 * enough to hold (block_size + 6) bytes.  For other messages, the
 * datagram parameter is ignored.
 *
 * Returns 0 on success and non-zero on failure.
 *------------------------------------------------------------------------*/
int ttp_accept_retransmit(ttp_session_t *session, retransmission_t *retransmission, u_char *datagram)
{
    ttp_transfer_t  *xfer      = &session->transfer;
    ttp_parameter_t *param     = session->parameter;
    static int       iteration = 0;
    static char      stats_line[80];
    int              status;
    u_int16_t        type;
  
    /* convert the retransmission fields to host byte order */
    retransmission->block      = ntohl(retransmission->block);
    retransmission->error_rate = ntohl(retransmission->error_rate);
    type                       = ntohs(retransmission->request_type);

    /* if it's an error rate notification */
    if (type == REQUEST_ERROR_RATE) {

	/* calculate a new IPD */
	if (retransmission->error_rate > param->error_rate) {
	    double factor1 = (1.0 * param->slower_num / param->slower_den) - 1.0;
	    double factor2 = (1.0 + retransmission->error_rate - param->error_rate) / (100000.0 - param->error_rate);
	    xfer->ipd_current *= 1.0 + (factor1 * factor2);
	} else
	    xfer->ipd_current = (u_int32_t) (xfer->ipd_current * (u_int64_t) param->faster_num / param->faster_den);

	/* build the stats string */
	sprintf(stats_line, "%6u %5uus %5uus %7u %6.2f%% %3u\n",
		retransmission->error_rate, xfer->ipd_current, param->ipd_time, xfer->block,
		100.0 * xfer->block / param->block_count, session->session_id);

    /* make sure the IPD is still in range, for later calculations */
    xfer->ipd_current = max(min(xfer->ipd_current, 10000), param->ipd_time);

	/* print a status report */
	if (!(iteration++ % 23))
	    printf(" erate     ipd  target   block   %%done srvNr\n");
	printf(stats_line);

	/* print to the transcript if the user wants */
	if (param->transcript_yn)
	    xscript_data_log(session, stats_line);

    /* if it's a restart request */
    } else if (type == REQUEST_RESTART) {

	/* do range-checking first */
	if ((retransmission->block == 0) || (retransmission->block > param->block_count)) {
	    sprintf(g_error, "Attempt to restart at illegal block %u", retransmission->block);
	    return warn(g_error);
	} else
	    xfer->block = retransmission->block;

    /* if it's a retransmit request */
    } else if (type == REQUEST_RETRANSMIT) {

        /* build the retransmission */
        status = build_datagram(session, retransmission->block, TS_BLOCK_RETRANSMISSION, datagram);
        if (status < 0) {
            sprintf(g_error, "Could not build retransmission for block %u", retransmission->block);
            return warn(g_error);
        }
      
        /* try to send out the block */
        status = sendto(xfer->udp_fd, datagram, 6 + param->block_size, 0, xfer->udp_address, xfer->udp_length);
        if (status < 0) {
            sprintf(g_error, "Could not retransmit block %u", retransmission->block);
            return warn(g_error);
        }

    /* if it's another kind of request */
    } else {
	sprintf(g_error, "Received unknown retransmission request of type %u", ntohs(retransmission->request_type));
	return warn(g_error);
    }

    /* we're done */
    return 0;
}


/*------------------------------------------------------------------------
 * int ttp_authenticate(ttp_session_t *session, const u_char *secret);
 *
 * Given an active Tsunami session, returns 0 if we are able to
 * negotiate authentication successfully and a non-zero value
 * otherwise.
 *
 * The negotiation process works like this:
 *
 *     (1) The server [this process] sends 512 bits of random data
 *         to the client.
 *
 *     (2) The client XORs 512 bits of the shared secret onto this
 *         random data and responds with the MD5 hash of the result.
 *
 *     (3) The server does the same thing and compares the result.
 *         If the authentication succeeds, the server transmits a
 *         result byte of 0.  Otherwise, it transmits a non-zero
 *         result byte.
 *------------------------------------------------------------------------*/
int ttp_authenticate(ttp_session_t *session, const u_char *secret)
{
    u_char random[64];         /* the buffer of random data               */
    u_char server_digest[16];  /* the MD5 message digest (for us)         */
    u_char client_digest[16];  /* the MD5 message digest (for the client) */
    int    i;
    int    status;

    /* obtain the random data */
    status = get_random_data(random, 64);
    if (status < 0)
	return warn("Access to random data is broken");

    /* send the random data to the client */
    status = write(session->client_fd, random, 64);
    if (status < 0)
	return warn("Could not send authentication challenge to client");

    /* read the results back from the client */
    status = read(session->client_fd, client_digest, 16);
    if (status < 0)
	return warn("Could not read authentication response from client");

    /* compare the two digests */
    prepare_proof(random, 64, secret, server_digest);
    for (i = 0; i < 16; ++i)
	if (client_digest[i] != server_digest[i]) {
	    write(session->client_fd, "\001", 1);
	    return warn("Authentication failed");
	}

    /* try to tell the client it worked */
    status = write(session->client_fd, "\000", 1);
    if (status < 0)
	return warn("Could not send authentication confirmation to client");

    /* we succeeded */
    return 0;
}


/*------------------------------------------------------------------------
 * int ttp_negotiate(ttp_session_t *session);
 *
 * Performs all of the negotiation with the client that is done prior
 * to authentication.  At the moment, this consists of verifying
 * identical protocol revisions between the client and server.  Returns
 * 0 on success and non-zero on failure.
 *
 * Values are transmitted in network byte order.
 *------------------------------------------------------------------------*/
int ttp_negotiate(ttp_session_t *session)
{
    u_int32_t server_revision = htonl(PROTOCOL_REVISION);
    u_int32_t client_revision;
    int       status;

    /* send our protocol revision number to the client */
    status = write(session->client_fd, &server_revision, 4);
    if (status < 0)
	return warn("Could not send protocol revision number");

    /* read the protocol revision number from the client */
    status = read(session->client_fd, &client_revision, 4);
    if (status < 0)
	return warn("Could not read protocol revision number");

    /* compare the numbers */
    return (client_revision == server_revision) ? 0 : -1;
}


/*------------------------------------------------------------------------
 * int ttp_open_port(ttp_session_t *session);
 *
 * Creates a new UDP socket for transmitting the file data associated
 * with our pending transfer and receives the destination port number
 * from the client.  Returns 0 on success and non-zero on failure.
 *------------------------------------------------------------------------*/
int ttp_open_port(ttp_session_t *session)
{
    struct sockaddr    *address;
    int                 status;
    u_int16_t           port;
    u_char              ipv6_yn = session->parameter->ipv6_yn;

    /* create the address structure */
    session->transfer.udp_length = ipv6_yn ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in);
    address = (struct sockaddr *) malloc(session->transfer.udp_length);
    if (address == NULL)
	error("Could not allocate space for UDP socket address");

    /* prepare the UDP address structure, minus the UDP port number */
    getpeername(session->client_fd, address, &(session->transfer.udp_length));

    /* read in the port number from the client */
    status = read(session->client_fd, &port, 2);
    if (status < 0)
	return warn("Could not read UDP port number");
    if (ipv6_yn)
	((struct sockaddr_in6 *) address)->sin6_port = port;
    else
	((struct sockaddr_in *)  address)->sin_port  = port;

    /* print out the port number */
    if (session->parameter->verbose_yn)
	printf("Sending to client port %d\n", ntohs(port));

    /* open a new datagram socket */
    session->transfer.udp_fd = create_udp_socket(session->parameter);
    if (session->transfer.udp_fd < 0)
	return warn("Could not create UDP socket");

    /* we succeeded */
    session->transfer.udp_address = address;
    return 0;
}


/*------------------------------------------------------------------------
 * int ttp_open_transfer(ttp_session_t *session);
 *
 * Tries to create a new TTP file request object for the given session
 * by reading the name of a requested file from the client.  If we are
 * able to negotiate the transfer successfully, we return 0.  If we
 * can't negotiate the transfer because of I/O or file errors, we
 * return a negative vlaue.
 *
 * The client is sent a result byte of 0 if the request is accepted
 * (because the file can be read) and a non-zero result byte otherwise.
 *------------------------------------------------------------------------*/
int ttp_open_transfer(ttp_session_t *session)
{
    char             filename[MAX_FILENAME_LENGTH];  /* the name of the file to transfer     */
    u_int64_t        file_size;                      /* network-order version of file size   */
    u_int32_t        block_size;                     /* network-order version of block size  */
    u_int32_t        block_count;                    /* network-order version of block count */
    time_t           epoch;
    int              status;
    ttp_transfer_t  *xfer  = &session->transfer;
    ttp_parameter_t *param =  session->parameter;

    /*FILE *    dup_client_fd;*/
    char             size[10];
    char             message[20];
    int i,j;
    char file_no[10];
    struct timeval ping_s, ping_e;
    
    /* clear out the transfer data */
    memset(xfer, 0, sizeof(*xfer));

    /* read in the requested filename */
    status = read_line(session->client_fd, filename, MAX_FILENAME_LENGTH);
    if (status < 0)
        error("Could not read filename from client");
    filename[MAX_FILENAME_LENGTH - 1] = '\0';

   /*
     *if the client requesting for multiple file at a time
     *sent the file names and then proceed to transfer
     */
    if(!strcmp(filename,"*"))
    {  
       sprintf(size,"%d",param->file_name_size);
       printf("\nServer side file size: %s\n", size);
       
       write(session->client_fd,size,10);       
       
       sprintf(file_no,"%d",param->total_files);
       printf("\nServer side file no: %s\n", file_no);       
       
       write(session->client_fd,file_no,10);       
       
       printf("\nFile sizes sent to client\n");
       read(session->client_fd,message,8);
       
       printf("Got message back:\"%s\"\n",message);
       
       for(i=0,j=0;i<param->file_name_size;i+=strlen(param->file_names[j])+1,j++)
          write(session->client_fd,param->file_names[j],strlen(param->file_names[j])+1);
       
       
       /*write(session->client_fd,testing, sizeof(testing));*/
       read(session->client_fd,message,8);
       printf("Got back :%s\n", message);       
       
       status = read_line(session->client_fd, filename, MAX_FILENAME_LENGTH);
       printf("file name requested: %s", filename);
       /*error("I am stuck\n");*/
       if (status < 0)
          error("Could not read filename from client");
    }/*end of multimode session*/    
    
    /* store the filename in the transfer object */
    xfer->filename = strdup(filename);
    if (xfer->filename == NULL)
	return warn("Memory allocation error");

    /* make a note of the request */
    if (param->verbose_yn)
	printf("Request for file: '%s'\n", filename);

    /* try to open the file for reading */
    xfer->file = mk5_fopen64(filename, "r");
    if (xfer->file == NULL) {
        sprintf(g_error, "File '%s' does not exist or cannot be read", filename);
    	/* signal failure to the client */
    	status = write(session->client_fd, "\x008", 1);
    	if (status < 0) {
		   warn("Could not signal request failure to client");
        }
        return warn(g_error);
    }
    
    /* begin round trip time estimation */
    gettimeofday(&ping_s,NULL);
    
    /* try to signal success to the client */
    status = write(session->client_fd, "\000", 1);
    if (status < 0)
	return warn("Could not signal request approval to client");

    /* read in the block size, target bitrate, and error rate */
    if (read(session->client_fd, &param->block_size,  4) < 0) return warn("Could not read block size");            param->block_size  = ntohl(param->block_size);
    if (read(session->client_fd, &param->target_rate, 4) < 0) return warn("Could not read target bitrate");        param->target_rate = ntohl(param->target_rate);
    if (read(session->client_fd, &param->error_rate,  4) < 0) return warn("Could not read error rate");            param->error_rate  = ntohl(param->error_rate);

    /* end round trip time estimation */
    gettimeofday(&ping_e,NULL);
    
    /* read in the slowdown and speedup factors */
    if (read(session->client_fd, &param->slower_num,  2) < 0) return warn("Could not read slowdown numerator");    param->slower_num  = ntohs(param->slower_num);
    if (read(session->client_fd, &param->slower_den,  2) < 0) return warn("Could not read slowdown denominator");  param->slower_den  = ntohs(param->slower_den);
    if (read(session->client_fd, &param->faster_num,  2) < 0) return warn("Could not read speedup numerator");     param->faster_num  = ntohs(param->faster_num);
    if (read(session->client_fd, &param->faster_den,  2) < 0) return warn("Could not read speedup denominator");   param->faster_den  = ntohs(param->faster_den);

    /* try to find the file statistics */
    // fseeko64(xfer->file, 0, SEEK_END); param->file_size(xfer->file);
    param->file_size   = 60LL * 512000000LL * 4LL / 8; // ftello64(xfer->file);
    
    param->block_count = (param->file_size / param->block_size) + ((param->file_size % param->block_size) != 0);
    param->epoch       = time(NULL);

    /* reply with the length, block size, number of blocks, and run epoch */
    file_size   = htonll(param->file_size);    if (write(session->client_fd, &file_size,   8) < 0) return warn("Could not submit file size");
    block_size  = htonl (param->block_size);   if (write(session->client_fd, &block_size,  4) < 0) return warn("Could not submit block size");
    block_count = htonl (param->block_count);  if (write(session->client_fd, &block_count, 4) < 0) return warn("Could not submit block count");
    epoch       = htonl (param->epoch);        if (write(session->client_fd, &epoch,       4) < 0) return warn("Could not submit run epoch");

    /*calculate and convert RTT to u_sec*/
    session->parameter->wait_u_sec=(ping_e.tv_sec - ping_s.tv_sec)*1000000+(ping_e.tv_usec-ping_s.tv_usec);
    /*add a 10% safety margin*/
    session->parameter->wait_u_sec = session->parameter->wait_u_sec + ((int)(session->parameter->wait_u_sec* 0.1));  
    
    /* and store the inter-packet delay */
    param->ipd_time   = (u_int32_t) ((1000000LL * 8 * param->block_size) / param->target_rate);
    xfer->ipd_current = param->ipd_time * 3;

    /* if we're doing a transcript */
    if (param->transcript_yn)
	xscript_open(session);

    /* we succeeded! */
    return 0;
}


/*========================================================================
 * $Log: protocol.c,v $
 * Revision 1.4  2007/08/27 13:38:14  jwagnerhki
 * total_Files renamed total_files
 *
 * Revision 1.3  2007/05/31 12:50:26  jwagnerhki
 * now mk5server compiles with SSAPI
 *
 * Revision 1.2  2007/05/31 12:04:57  jwagnerhki
 * g++ compile warning fix, ssapi linking fix
 *
 * Revision 1.1  2007/05/31 09:32:07  jwagnerhki
 * removed some signedness warnings, added Mark5 server devel start code
 *
 * Revision 1.22  2007/02/13 13:47:31  jwagnerhki
 * UTC parse fixes and extensions, dl in addition to flen for realtime bytelength
 *
 * Revision 1.21  2007/02/12 13:41:21  jwagnerhki
 * mktime() post-fix now parsing really to utc, added 'dl' same as 'flen', added some EVN formats, added parse validity flag
 *
 * Revision 1.20  2006/12/05 15:24:50  jwagnerhki
 * now noretransmit code in client only, merged rt client code
 *
 * Revision 1.19  2006/12/05 13:38:20  jwagnerhki
 * identify concurrent server transfers by an own ID
 *
 * Revision 1.18  2006/11/26 14:12:29  jwagnerhki
 * fixed incorrect octals
 *
 * Revision 1.17  2006/11/21 09:25:02  jwagnerhki
 * cleaned up immediate vsib start code
 *
 * Revision 1.16  2006/11/21 08:13:15  jwagnerhki
 * old UTC timestamp parse moved to parse_evn_filename.c
 *
 * Revision 1.15  2006/11/21 07:28:48  jwagnerhki
 * realtime file length can be specified in filename
 *
 * Revision 1.14  2006/11/08 11:00:34  jwagnerhki
 * stats lied about real ipd
 *
 * Revision 1.13  2006/11/02 08:25:02  jwagnerhki
 * realtime file length shortened to 4min at 512mbps
 *
 * Revision 1.12  2006/10/30 08:46:58  jwagnerhki
 * removed memory leak unused ringbuf
 *
 * Revision 1.8  2006/10/28 17:00:12  jwagnerhki
 * block type defines
 *
 * Revision 1.7  2006/10/25 13:56:47  jwagnerhki
 * 'get *' mini fix, Jamil roundtrip time guess added
 *
 * Revision 1.6  2006/10/25 12:50:56  jwagnerhki
 * fallback to older datetime parser with immediate start support
 *
 * Revision 1.8  2006/10/25 12:09:08  jwagnerhki
 * multiget * support, parse_evn_filename called
 *
 * Revision 1.7  2006/10/24 21:06:00  jwagnerhki
 * file does not exist etc codes now returned to client
 *
 * Revision 1.6  2006/10/24 20:08:57  jwagnerhki
 * now realtime uses same protocol.c, VSIB_REALTIME compile flag
 *
 * Revision 1.5  2006/10/24 19:14:28  jwagnerhki
 * moved server.h into common tsunami-server.h
 *
 * Revision 1.4  2006/10/19 09:18:24  jwagnerhki
 * catch if specified UTC time is in the past
 *
 * Revision 1.3  2006/10/18 07:45:50  jwagnerhki
 * more verbose when waiting till UTC starttime
 *
 * Revision 1.2  2006/07/21 08:45:22  jwagnerhki
 * merged server and rtserver protocol.c
 *
 * Revision 1.1.1.1  2006/07/20 09:21:20  jwagnerhki
 * reimport
 *
 * Revision 1.3  2006/07/19 06:31:13  jwagnerhki
 * bool 2 char
 *
 * Revision 1.2  2006/07/17 12:18:05  jwagnerhki
 * now /dev/vsib, start immediate or at specified time
 *
 * Revision 1.1  2006/07/10 12:37:21  jwagnerhki
 * added to trunk
 *
 */
