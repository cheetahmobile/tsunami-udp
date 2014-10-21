/*========================================================================
 * network_v6.c  --  Socket creation functions for Tsunami server.
 *
 * This contains socket creation and configuration routines for the
 * Tsunami file transfer server.
 *
 * Written by Mark Meiss (mmeiss@indiana.edu).
 * Copyright 2002 The Trustees of Indiana University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. All redistributions of source code must retain the above
 *    copyright notice, the list of authors in the original source
 *    code, this list of conditions and the disclaimer listed in this
 *    license;
 *
 * 2. All redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the disclaimer
 *    listed in this license in the documentation and/or other
 *    materials provided with the distribution;
 *
 * 3. Any documentation included with all redistributions must include
 *    the following acknowledgement:
 *
 *       "This product includes software developed by Indiana
 *        University`s Advanced Network Management Lab. For further
 *        information, contact Steven Wallace at 812-855-0960."
 *
 *    Alternatively, this acknowledgment may appear in the software
 *    itself, and wherever such third-party acknowledgments normally
 *    appear.
 *
 * 4. The name "tsunami" shall not be used to endorse or promote
 *    products derived from this software without prior written
 *    permission from Indiana University.  For written permission,
 *    please contact Steven Wallace at 812-855-0960.
 *
 * 5. Products derived from this software may not be called "tsunami",
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
#include <netdb.h>        /* for DNS resolver functions     */
#include <netinet/tcp.h>  /* for TCP_NODELAY, etc.          */
#include <string.h>       /* for standard string routines   */
#include <sys/socket.h>   /* for the BSD socket library     */
#include <unistd.h>       /* for standard Unix system calls */

#include <tsunami-server.h>


/*------------------------------------------------------------------------
 * int create_tcp_socket(ttp_parameter_t *parameter);
 *
 * Establishes a new TCP server socket, returning the file descriptor
 * of the socket on success and a negative value on error.  This will
 * be an IPv6 socket if ipv6_yn is true and an IPv4 socket otherwise.
 *------------------------------------------------------------------------*/
int create_tcp_socket(ttp_parameter_t *parameter)
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
    hints.ai_family   = parameter->ipv6_yn ? AF_INET6 : AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    /* try to get address info for ourselves */
    sprintf(buffer, "%d", parameter->tcp_port);
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
	if (status < 0)
	    continue;

	/* and try to bind it */
	status = bind(socket_fd, info->ai_addr, info->ai_addrlen);
	if (status == 0)
	    break;

    } while ((info = info->ai_next) != NULL);

    /* free the allocated memory */
    freeaddrinfo(info_save);

    /* make sure that we succeeded with at least one address */
    if (info == NULL)
	return warn("Error in creating TCP server socket");

    /* listen for new connections */
    status = listen(socket_fd, SOMAXCONN);
    if (status < 0)
	return warn("Error in listening on TCP server socket");

    /* return the file desscriptor */
    return socket_fd;
}


/*------------------------------------------------------------------------
 * int create_udp_socket(ttp_parameter_t *parameter);
 *
 * Establishes a new UDP socket for data transfer, returning the file
 * descriptor of the socket on success and a negative value on error.
 * This will be an IPv6 socket if ipv6_yn is true and an IPv4 socket
 * otherwise.
 *------------------------------------------------------------------------*/
int create_udp_socket(ttp_parameter_t *parameter)
{
    int socket_fd;
    int status;
    int yes = 1;

    /* create the socket */
    socket_fd = socket(parameter->ipv6_yn ? AF_INET6 : AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0)
	return warn("Error in creating UDP socket");

    /* make the socket reuseable */
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    if (status < 0) {
	close(socket_fd);
	return warn("Error in configuring UDP socket");
    }

    /* set the transmit buffer size */
    status = setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, &parameter->udp_buffer, sizeof(parameter->udp_buffer));
    if (status < 0) {
	warn("Error in resizing UDP transmit buffer");
    }

    /* return the file desscriptor */
    return socket_fd;
}


/*========================================================================
 * $Log: network.c,v $
 * Revision 1.3  2009/05/18 07:52:55  jwagnerhki
 * SO_SNDBUF failure non-fatal
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
