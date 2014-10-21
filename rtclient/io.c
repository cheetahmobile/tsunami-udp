/*========================================================================
 * io.c  --  Disk I/O routines for Tsunami client.
 *
 * This contains disk I/O routines for the Tsunami file transfer client.
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

#include <tsunami-client.h>

/*------------------------------------------------------------------------
 * int accept_block(ttp_session_t *session,
 *                  u_int32_t block_index, u_char *block);
 *
 * Accepts the given block of data, which involves writing the block
 * to disk.  Returns 0 on success and nonzero on failure.
 *------------------------------------------------------------------------*/
int accept_block(ttp_session_t *session, u_int32_t block_index, u_char *block)
{
    ttp_transfer_t  *transfer   = &session->transfer;
    u_int32_t        block_size = session->parameter->block_size;
    u_int32_t        write_size;
    int              status;
    #ifdef VSIB_REALTIME
    u_int32_t       ringbuf_pointer;
    #endif

    /* figure out how many bytes to write */
    if (block_index == transfer->block_count) {
        write_size = transfer->file_size % block_size;
        if (write_size == 0) 
            write_size = block_size;
    } else {
        write_size = block_size;
    }

    #ifdef VSIB_REALTIME
    /* These were added for real-time eVLBI */
    ringbuf_pointer = ((block_index-1) % RINGBUF_BLOCKS) * session->parameter->block_size;
    
    if (session->parameter->ringbuf != NULL) /* If we have a ring buffer */
        memcpy(session->parameter->ringbuf + ringbuf_pointer, block, write_size);

    /* check if we need to feed the VSIB */
    write_vsib_block(session, block, write_size);
    #endif
 
    #ifndef DEBUG_DISKLESS
    /* seek to the proper location */
    status = fseeko(transfer->file, ((u_int64_t) block_size) * (block_index - 1), SEEK_SET);
    if (status < 0) {
        sprintf(g_error, "Could not seek at block %d of file", block_index);
        return warn(g_error);
    }

    /* write the block to disk */
    status = fwrite(block, 1, write_size, transfer->file);
    if (status < write_size) {
        sprintf(g_error, "Could not write block %d of file", block_index);
        return warn(g_error);
    }
    #endif

    /* we succeeded */
    return 0;
}


/*========================================================================
 * $Log: io.c,v $
 * Revision 1.8  2008/05/25 15:39:32  jwagnerhki
 * file client merge
 *
 * Revision 1.7  2008/05/22 18:30:44  jwagnerhki
 * Darwin fix LFS support fopen() not fopen64() etc
 *
 * Revision 1.6  2008/05/20 18:20:38  jwagnerhki
 * got_block use
 *
 * Revision 1.5  2007/12/07 18:10:28  jwagnerhki
 * cleaned away 64-bit compile warnings, used tsunami-client.h
 *
 * Revision 1.4  2007/01/11 15:15:49  jwagnerhki
 * rtclient merge, io.c now with VSIB_REALTIME, blocks_left not allowed negative fix, overwriting file check fixed, some memset()s to keep Valgrind warnings away
 *
 * Revision 1.3  2006/12/15 12:57:41  jwagnerhki
 * added client 'blockdump' block bitmap dump to file feature
 *
 * Revision 1.2  2006/10/28 17:08:42  jwagnerhki
 * fixed jr's nonworking rtclient
 *
 * Revision 1.1.1.1  2006/07/20 09:21:19  jwagnerhki
 * reimport
 *
 * Revision 1.1  2006/07/10 12:35:11  jwagnerhki
 * added to trunk
 *
 */
