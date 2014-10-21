/*========================================================================
 * network_v6.c  --  IPv6 socket creation functions for Tsunami client.
 *
 * This contains socket creation and configuration routines for the
 * Tsunami file transfer client.
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

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>        /* for DNS resolver functions     */
#include <netinet/tcp.h>  /* for TCP_NODELAY, etc.          */
#include <string.h>       /* for standard string routines   */
#include <sys/socket.h>   /* for the BSD socket library     */
#include <sys/types.h>    /* for standard system data types */
#include <unistd.h>       /* for standard Unix system calls */

#include <tsunami-client.h>


/*------------------------------------------------------------------------
 * int create_tcp_socket_v6(ttp_session_t *session,
 *                          const char *server_name, u_int16_t server_port);
 *
 * Establishes a new TCP control session for the given session object.
 * The TCP session is connected to the given Tsunami server; we return
 * the file descriptor of the socket on success and nonzero on error.
 *------------------------------------------------------------------------*/
int create_tcp_socket_v6(ttp_session_t *session, const char *server_name, u_int16_t server_port)
{
    struct addrinfo  hints;
    struct addrinfo *info;
    struct addrinfo *info_save;
    char             buffer[10];
    int              socket_fd;
    int              status;
    int              yes = 1;

    /* set up the hints for getaddrinfo() */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET6;
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
 * int create_udp_socket_v6(ttp_parameter_t *parameter);
 *
 * Establishes a new UDP socket for data transfer, returning the file
 * descriptor of the socket on success and -1 on error.  The parameter
 * structure is used for setting the size of the UDP receive buffer.
 *------------------------------------------------------------------------*/
int create_udp_socket_v6(ttp_parameter_t *parameter)
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
    hints.ai_flags    = AI_PASSIVE;
    hints.ai_family   = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;

    /* try to get address info for ourselves */
    sprintf(buffer, "%d", parameter->server_port);
    status = getaddrinfo(NULL, buffer, &hints, &info);
    if (status)
	return warn("Error in getting address information");

    /* for each address structure returned */
    info_save = info;
    do {

	/* try to create a socket of this type */
	socket_fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	if (socket_fd < 0)
	    continue;

	/* make the socket reusable */
	status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	if (status < 0) {
	    close(socket_fd);
	    continue;
	}

	/* set the receive buffer size */
	status = setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, &parameter->udp_buffer, sizeof(parameter->udp_buffer));
	if (status < 0) {
	    close(socket_fd);
	    continue;
	}

	/* and try to bind it */
	status = bind(socket_fd, info->ai_addr, info->ai_addrlen);
	if (status == 0)
	    break;

    } while ((info = info->ai_next) != NULL);

    /* free the allocated memory */
    freeaddrinfo(info_save);

    /* make sure that we succeeded with at least one address */
    if (info == NULL)
	return warn("Error in creating UDP socket");

    /* return the file desscriptor */
    return socket_fd;
}


/*========================================================================
 * $Log: network_v6.c,v $
 * Revision 1.2  2007/12/07 18:10:28  jwagnerhki
 * cleaned away 64-bit compile warnings, used tsunami-client.h
 *
 * Revision 1.1.1.1  2006/07/20 09:21:19  jwagnerhki
 * reimport
 *
 * Revision 1.1  2006/07/10 12:26:51  jwagnerhki
 * deleted unnecessary files
 *
 */
