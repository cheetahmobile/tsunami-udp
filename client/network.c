/*========================================================================
 * network.c  --  Socket creation functions for Tsunami client.
 *
 * This contains socket creation and configuration routines for the
 * Tsunami file transfer client.
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

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>        /* for DNS resolver functions     */
#include <netinet/tcp.h>  /* for TCP_NODELAY, etc.          */
#include <string.h>       /* for standard string routines   */
#include <sys/socket.h>   /* for the BSD socket library     */
#include <sys/types.h>    /* for standard system data types */
#include <unistd.h>       /* for standard Unix system calls */
#include <stdlib.h>       /* for *alloc() and free()        */

#include <tsunami-client.h>


/*------------------------------------------------------------------------
 * int create_tcp_socket(ttp_session_t *session,
 *                       const char *server_name, u_int16_t server_port);
 *
 * Establishes a new TCP control session for the given session object.
 * The TCP session is connected to the given Tsunami server; we return
 * the file descriptor of the socket on success and nonzero on error.
 * This will be an IPv6 socket if ipv6_yn is true and an IPv4 socket
 * otherwise.
 *------------------------------------------------------------------------*/
int create_tcp_socket(ttp_session_t *session, const char *server_name, u_int16_t server_port)
{
    struct addrinfo  hints;
    struct addrinfo *info;
    struct addrinfo *info_save;
    char             buffer[10];
    int              socket_fd;
    int              yes = 1;
    int              status;

    /* set up the hints for getaddrinfo() */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = session->parameter->ipv6_yn ? AF_INET6 : AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    /* try to get address info for the server */
    sprintf(buffer, "%d", server_port);
    status = getaddrinfo(server_name, buffer, &hints, &info);
    if (status)
	return warn("Error in getting address information for server");

    /* for each address structure returned */
    info_save = info;
    do {

	/* try to create a socket of this type */
	socket_fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	if (socket_fd < 0) {
	    warn("Could not create socket");
	    continue;
	}

	/* make the socket reusable */
	status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	if (status < 0) {
	    warn("Could not make socket reusable");
	    close(socket_fd);
	    continue;
	}

	/* disable Nagle's algorithm */
	status = setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes));
	if (status < 0) {
	    warn("Could not disable Nagle's algorithm");
	    close(socket_fd);
	    continue;
	}

	/* try to connect to the server */
	status = connect(socket_fd, info->ai_addr, info->ai_addrlen);
	if (status == 0) {

	    /* copy the address */
	    session->server_address = (struct sockaddr *) malloc(info->ai_addrlen);
            session->server_address_length = info->ai_addrlen;
	    if (session->server_address == NULL)
		error("Could not allocate space for server address");
	    memcpy(session->server_address, info->ai_addr, info->ai_addrlen);
	    break;
	}

    } while ((info = info->ai_next) != NULL);

    /* free the allocated memory */
    freeaddrinfo(info_save);

    /* make sure that we succeeded with at least one address */
    if (info == NULL)
	return warn("Error in connecting to Tsunami server");

    /* return the file desscriptor */
    return socket_fd;
}


/*------------------------------------------------------------------------
 * int create_udp_socket(ttp_parameter_t *parameter);
 *
 * Establishes a new UDP socket for data transfer, returning the file
 * descriptor of the socket on success and -1 on error.  The parameter
 * structure is used for setting the size of the UDP receive buffer.
 * This will be an IPv6 socket if ipv6_yn is true and an IPv4 socket
 * otherwise. The next available port starting from parameter->client_port 
 * will be taken, and the value of client_port is updated.
 *------------------------------------------------------------------------*/
int create_udp_socket(ttp_parameter_t *parameter)
{
    struct addrinfo  hints;
    struct addrinfo *info;
    struct addrinfo *info_save;
    char             buffer[10];
    int              socket_fd;
    // int              yes = 1;
    int              status;
    int              higher_port_attempt = 0;
    
    /* set up the hints for getaddrinfo() */
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags    = AI_PASSIVE;
    hints.ai_family   = parameter->ipv6_yn ? AF_INET6 : AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    do {
  
        /* try to get address info for ourselves */
        sprintf(buffer, "%d", parameter->client_port + higher_port_attempt);
        status = getaddrinfo(NULL, buffer, &hints, &info);
        if (status) {
            return warn("Error in getting address information");
        }
        
        /* for each address structure returned */
        info_save = info;
        do {
   
            /* try to create a socket of this type */
            socket_fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
            if (socket_fd < 0) {
                continue;
            }
            
            /* set the receive buffer size */
            status = setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, &parameter->udp_buffer, sizeof(parameter->udp_buffer));
            if (status < 0) {
                warn("Error in resizing UDP receive buffer");
            }
            
            /* and try to bind it */
            status = bind(socket_fd, info->ai_addr, info->ai_addrlen);
            if (status == 0) {
                parameter->client_port = ntohs(((struct sockaddr_in*)info->ai_addr)->sin_port);
                fprintf(stderr, "Receiving data on UDP port %d\n", parameter->client_port);
                break;
            }
   
        } while ((info = info->ai_next) != NULL);
        
        /* free the allocated memory */
        freeaddrinfo(info_save);

    } while ((++higher_port_attempt < 256) && (info == NULL));

    /* warn about other transfers running concurrently */
    if(higher_port_attempt>1)
    fprintf(stderr, "Warning: there are %d other Tsunami clients running\n", higher_port_attempt-1);
    
    /* make sure that we succeeded with at least one address */
    if (info == NULL)
    return warn("Error in creating UDP socket");
    
    /* return the file desscriptor */
    return socket_fd;
}


/*========================================================================
 * $Log: network.c,v $
 * Revision 1.10  2009/05/18 07:51:31  jwagnerhki
 * SO_RCVBUF failure non-fatal
 *
 * Revision 1.9  2007/12/07 18:10:28  jwagnerhki
 * cleaned away 64-bit compile warnings, used tsunami-client.h
 *
 * Revision 1.8  2007/08/14 08:50:39  jwagnerhki
 * initialize server_address_length, do not have recvfrom overwrite server_address and server_address_length
 *
 * Revision 1.7  2007/07/16 09:09:45  jwagnerhki
 * printf into fprintf stderr
 *
 * Revision 1.6  2007/07/16 08:55:53  jwagnerhki
 * build 21, upped 16 to 256 clients, reduced end block blast speed, enabled RETX_REQBLOCK_SORTING compile flag
 *
 * Revision 1.5  2007/01/11 15:15:48  jwagnerhki
 * rtclient merge, io.c now with VSIB_REALTIME, blocks_left not allowed negative fix, overwriting file check fixed, some memset()s to keep Valgrind warnings away
 *
 * Revision 1.4  2006/12/05 14:24:13  jwagnerhki
 * disabled client UDP socket reuse, multi client per PC now ok
 *
 * Revision 1.3  2006/09/07 13:56:57  jwagnerhki
 * udp socket reusable, udp port selectable in client
 *
 * Revision 1.2  2006/07/21 07:58:01  jwagnerhki
 * concurrent clients by scannig for next available UDP port
 *
 * Revision 1.1.1.1  2006/07/20 09:21:18  jwagnerhki
 * reimport
 *
 * Revision 1.1  2006/07/10 12:26:51  jwagnerhki
 * deleted unnecessary files
 *
 */
