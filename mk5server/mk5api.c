/*========================================================================
 * mk5api.c  --  Mark5 A/B/C API functions
 *
 * This contains the code that logs Tsunami transfer statistics
 * available on the server to disk for later analysis.
 *
 * Written by Jan Wagner (jwagner@kurp.hut.fi).
 * Copyright © 2007 Jan Wagner. The original Tsunami license applies..
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

#include "mk5api.h"
#include <err.h>
#include <errno.h>

#define XLR_CARD_NR    1    // currently supporting just one card, first
#define DO_ERROR(x, y, z)   errno=x; warn(y); return z

MK5FILE* mk5_fopen64(const char *path, const char *mode) {
    if (XLRDeviceFind() <= 0) {
        DO_ERROR(ENODEV, "No StreamStor card found", NULL);
    }
    // if(XLRCardReset(XLR_CARD_NR) != XLR_SUCCESS) return NULL;
    MK5FILE* fp = (MK5FILE*)calloc(1, sizeof(MK5FILE));
    if (XLROpen(XLR_CARD_NR, &(fp->sshandle)) != XLR_SUCCESS) {
        XLRClose(fp->sshandle);
        DO_ERROR(EACCES, "Could not open StreamStor card", NULL);
    }
    if (XLRSetMode(fp->sshandle, SS_MODE_PCI) != XLR_SUCCESS) {
        XLRClose(fp->sshandle);
        DO_ERROR(EACCES, "Could not set StreamStor PCI mode", NULL);
    }
    return fp;
}

int mk5_fclose(MK5FILE *fp) {
    if (fp != NULL) {
        XLRClose(fp->sshandle);
        free(fp);
    }
    return 0;
}

int mk5_fseek(MK5FILE *stream, off_t offset, int whence) {
    if (stream != NULL) {
        if (whence != SEEK_SET) {
            DO_ERROR(EINVAL, "currently mk5_fseek supports only SEEK_SET", -1);
        }
        stream->rdesc.AddrHi = (offset >> 32);
        stream->rdesc.AddrLo = offset & (unsigned long)0xFFFFFFFF;
    }
    return 0;
}

off_t mk5_ftello(MK5FILE *fp) {
    if (fp != NULL) {
        warn("mk5_ftello currently not supported");
    }
    return 0;
}

size_t mk5_fread(void *ptr, size_t size, size_t nmemb, MK5FILE *stream) {
    stream->rdesc.BufferAddr = (PULONG)ptr;
    stream->rdesc.XferLength = size * nmemb;
    if (XLRRead(stream->sshandle, &(stream->rdesc)) != XLR_SUCCESS) {
        XLRStop(stream->sshandle);
        DO_ERROR(EIO, "XLRRead failed", stream->rdesc.XferLength);
    }
    return stream->rdesc.XferLength;
}
