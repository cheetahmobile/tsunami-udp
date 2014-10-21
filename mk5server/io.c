/*========================================================================
 * io.c  --  Disk I/O routines for Tsunami server with VSIB access.
 *
 * This contains disk I/O routines for the realtime Tsunami file transfer server.
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
 * int build_datagram(ttp_session_t *session, u_int32_t block_index,
 *                    u_int16_t block_type, u_char *datagram);
 *
 * Constructs to hold the given block of data, with the given type
 * stored in it.  The format of the datagram is:
 *
 *     32                    0
 *     +---------------------+
 *     |     block_number    |
 *     +----------+----------+
 *     |   type   |   data   |
 *     +----------+     :    |
 *     :     :          :    :
 *     +---------------------+
 *
 * The datagram is stored in the given buffer, which must be at least
 * six bytes longer than the block size for the transfer.  Returns 0 on
 * success and non-zero on failure.
 *------------------------------------------------------------------------*/
int build_datagram(ttp_session_t *session, u_int32_t block_index,
           u_int16_t block_type, u_char *datagram)
{
    static u_int32_t last_block = 0;
    int              status;

    /* move the file pointer to the appropriate location */
    if (block_index != (last_block + 1))
    mk5_fseek(session->transfer.file, ((u_int64_t) session->parameter->block_size) * (block_index - 1), SEEK_SET);

    /* try to read in the block */
    status = mk5_fread(datagram + 6, 1, session->parameter->block_size, session->transfer.file);
    if (status < 0) {
    sprintf(g_error, "Could not read block #%u", block_index);
    return warn(g_error);
    }

    /* build the datagram header */
    *((u_int32_t *) (datagram + 0)) = htonl(block_index);
    *((u_int16_t *) (datagram + 4)) = htons(block_type);

    /* return success */
    last_block = block_index;
    return 0;
}



/*========================================================================
 * $Log: io.c,v $
 * Revision 1.3  2007/05/31 12:50:26  jwagnerhki
 * now mk5server compiles with SSAPI
 *
 * Revision 1.2  2007/05/31 09:48:48  jwagnerhki
 * basic API
 *
 * Revision 1.1  2007/05/31 09:32:04  jwagnerhki
 * removed some signedness warnings, added Mark5 server devel start code
 *
 * Revision 1.7  2006/10/27 08:52:34  jwagnerhki
 * guifres bugfix
 *
 * Revision 1.6  2006/10/27 07:05:59  jwagnerhki
 * now MODE_34TH not defined by default, added some commented out debug code
 *
 * Revision 1.5  2006/10/25 12:22:02  jwagnerhki
 * renamed last_vsib_block, optinal no seek on contiguos reads
 *
 * Revision 1.4  2006/10/25 12:07:08  jwagnerhki
 * hacked for two channel discard mode
 *
 * Revision 1.3  2006/10/24 19:14:28  jwagnerhki
 * moved server.h into common tsunami-server.h
 *
 * Revision 1.2  2006/10/16 08:51:28  jwagnerhki
 * fixed zero-sized second transfer files
 *
 * Revision 1.1.1.1  2006/07/20 09:21:20  jwagnerhki
 * reimport
 *
 * Revision 1.1  2006/07/10 12:37:21  jwagnerhki
 * added to trunk
 *
 */
