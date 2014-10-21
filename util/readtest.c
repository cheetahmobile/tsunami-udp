/*========================================================================
 * readtest  --  Disk read throughput benchmarking program.
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

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

int main(int argc, char *argv[])
{
    struct timeval start, stop;
    FILE *file;
    u_int32_t block_size;
    static u_char *block;
    u_int64_t file_size;

    if (sizeof(off_t) != 8) {
       printf("Warning: Not compiled with 64-bit Large File Support, results can be unreliable\n");
    }

    block_size = (argc > 2) ? atoi(argv[2]) : 32678;
    block = (u_char *) malloc(block_size);

    gettimeofday(&start, NULL);

    file = fopen(argv[1], "r");

    fseeko(file, 0, SEEK_END);
    file_size = ftello(file);
    fseeko(file, 0, SEEK_SET);

    while (fread(block, 1, block_size, file) > 0);
    fclose(file);

    gettimeofday(&stop, NULL);

    {
	int64_t usec = 1000000LL * (stop.tv_sec - start.tv_sec) + (stop.tv_usec - start.tv_usec);
	int64_t bits = file_size * 8;

	printf("Start time = %lu.%06lu\n",   (unsigned long)start.tv_sec, (unsigned long)start.tv_usec);
	printf("Stop time  = %lu.%06lu\n",   (unsigned long)stop.tv_sec,  (unsigned long)stop.tv_usec);
	printf("Interval   = %0.3lf sec\n",  usec / 1000000.0);
	printf("Read speed = %0.3lf Mbps\n", bits * 1.0 / usec);
    }

    return 0;
}


/*========================================================================
 * $Log: readtest.c,v $
 * Revision 1.2  2008/05/22 17:54:29  jwagnerhki
 * LFS support replacing *64() file funcs, __darwin_suseconds_t format fix
 *
 * Revision 1.1.1.1  2006/07/20 09:21:21  jwagnerhki
 * reimport
 *
 * Revision 1.1  2006/07/10 12:41:32  jwagnerhki
 * added to trunk
 *
 */
