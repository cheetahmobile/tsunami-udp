/*========================================================================
 * md5.c  --  MD5 message digest routines.
 *
 * This is a from-scratch implementation of the MD5 Message-Digest
 * Algorithm.  This code is NOT based on the reference implementation
 * found in RFC 1321 and is thus not subject to the licensing
 * restrictions of RSA Data Security, Inc.
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


#include "md5.h"


/*------------------------------------------------------------------------
 * Rotate an unsigned integer left some number of bits.
 *------------------------------------------------------------------------*/
#define rotate(X, bits) (((X) << (bits)) | ((X) >> (32 - (bits))))


/*------------------------------------------------------------------------
 * The package-wide T array.
 *------------------------------------------------------------------------*/

u_int32_t T[64] = { 0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,    /* 01 - 04 */
		    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,    /* 05 - 08 */
		    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,    /* 09 - 12 */
		    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,    /* 13 - 16 */

		    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,    /* 17 - 20 */
		    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,    /* 21 - 24 */
		    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,    /* 25 - 28 */
		    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,    /* 29 - 32 */

		    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,    /* 33 - 36 */
		    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,    /* 37 - 40 */
		    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,    /* 41 - 44 */
		    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,    /* 45 - 48 */

		    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,    /* 49 - 52 */
		    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,    /* 53 - 56 */
		    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,    /* 57 - 60 */
		    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 };  /* 61 - 64 */

u_char pad[64] = { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

u_char md5_table[64][2] = { { 0, 7}, { 1,12}, { 2,17}, { 3,22}, { 4, 7}, { 5,12}, { 6,17}, { 7,22},
	 		    { 8, 7}, { 9,12}, {10,17}, {11,22}, {12, 7}, {13,12}, {14,17}, {15,22},
			    { 1, 5}, { 6, 9}, {11,14}, { 0,20}, { 5, 5}, {10, 9}, {15,14}, { 4,20},
			    { 9, 5}, {14, 9}, { 3,14}, { 8,20}, {13, 5}, { 2, 9}, { 7,14}, {12,20},
			    { 5, 4}, { 8,11}, {11,16}, {14,23}, { 1, 4}, { 4,11}, { 7,16}, {10,23},
			    {13, 4}, { 0,11}, { 3,16}, { 6,23}, { 9, 4}, {12,11}, {15,16}, { 2,23},
			    { 0, 6}, { 7,10}, {14,15}, { 5,21}, {12, 6}, { 3,10}, {10,15}, { 1,21},
			    { 8, 6}, {15,10}, { 6,15}, {13,21}, { 4, 6}, {11,10}, { 2,15}, { 9,21} };

u_int32_t md5_F(u_int32_t x, u_int32_t y, u_int32_t z) { return (x & y) | (~x & z); }
u_int32_t md5_G(u_int32_t x, u_int32_t y, u_int32_t z) { return (x & z) | (y & ~z); }
u_int32_t md5_H(u_int32_t x, u_int32_t y, u_int32_t z) { return x ^ y ^ z;          }
u_int32_t md5_I(u_int32_t x, u_int32_t y, u_int32_t z) { return y ^ (x | ~z);       }

u_int32_t (*md5_dispatch[4])(u_int32_t, u_int32_t, u_int32_t) = { md5_F, md5_G, md5_H, md5_I };


#ifndef ARCH_IS_BIG_ENDIAN
/*------------------------------------------------------------------------
 * void md5_digest(u_char *buffer, size_t size, u_char *digest);
 *
 * Given a message of the given size, computes the MD5 digest of the
 * messages and stores it in the given pre-allocated 16-byte buffer.
 *
 * Note that this code assumes a little-endian machine and a message
 * size of at most 2**29!
 *------------------------------------------------------------------------*/
void md5_digest(u_char *buffer, size_t size, u_char *digest)
{
    u_int32_t X[16], state[4], tempState[4], func, sum;
    int       i, blocks, j;

    /* initialize the state array */
    state[0] = 0x67452301;  state[1] = 0xefcdab89;
    state[2] = 0x98badcfe;  state[3] = 0x10325476;

    /* calculate the number of blocks */
    blocks = size / 64;
    if ((size % 64) > 56) ++blocks;

    /* for each block */
    for (i = 0; i <= blocks; ++i) {

	/* if it's not the last block or next-to-last block */
	if (i < blocks - 1) {

	    /* copy the block into X */
	    memcpy((u_char *) X, buffer + (64 * i), 64);

	/* if it's the next-to-last block */
	} else if (i == blocks - 1) {

	    /* copy most or all of the block into X */
	    if ((64 * (i + 1)) > size) {
		memcpy((u_char *) X, buffer + (64 * i), size % 64);
		memcpy(((u_char *) X) + (size % 64), pad, 64 - (size % 64));
	    } else {
		memcpy((u_char *) X, buffer + (64 * i), 64);
	    }

	/* otherwise, copy just the needed bytes */
	} else {

	    /* copy part of the message and the padding block into X */
	    if ((64 * i) > size) {
		memset((u_char *) X, 0x00, 64);
	    } else {
		memcpy((u_char *) X, buffer + (64 * i), size % 64);
		memcpy(((u_char *) X) + (size % 64), pad, 64 - (size % 64));
	    }

	    /* put the message size into X */
	    X[14] = size * 8;
	    X[15] = 0x00000000;
	}

	/* save the current state */
	memcpy((u_char *) tempState, (u_char *) state, 16);

	/* do the computation */
	for (j = 0; j < 64; ++j) {
	    func = md5_dispatch[j/16](state[(4 - j%4 + 1) % 4], state[(4 - j%4 + 2) % 4], state[(4 - j%4 + 3) % 4]);
	    sum  = state[(4-j%4)%4] + func + X[md5_table[j][0]] + T[j];
	    state[(4-j%4)%4] = state[(4-j%4+1)%4] + rotate(sum, md5_table[j][1]);
	}

	/* do the necessary additions */
	for (j = 0; j < 4; ++j)
	    state[j] += tempState[j];
    }

    /* copy the state into the digest */
    memcpy(digest, (u_char *) state, 16);
}
#else

/* Big Endian architecture, code copied from Aladdin, ghostscript md5.c, cf later below */
void md5_digest(u_char *buffer, size_t size, u_char *digest)
{
    md5_state_t ms;
    md5_init(&ms);
    md5_append(&ms, (const u_char*) buffer, size);
    md5_finish(&ms, digest);
}

#endif // ARCH_IS_BIG_ENDIAN

/*------------------------------------------------------------------------
 * void md5_fprint_digest(FILE *file, u_char *digest);
 *
 * Prints the given digest on the given open file handle.
 *------------------------------------------------------------------------*/
void md5_fprint_digest(FILE *file, u_char *digest)
{
    int i;

    /* print the bytes one-by-one */
    for (i = 0; i < 16; ++i)
	fprintf(file, "%02x", digest[i]);
}


/*------------------------------------------------------------------------
 * void md5_sprint_digest(char *buffer, u_char *digest);
 *
 * Prints the given digest in hex form into the given string buffer,
 * which must be at least 33 characters in size.
 *------------------------------------------------------------------------*/
void md5_sprint_digest(char *buffer, u_char *digest)
{
    /* print the digest */
    sprintf(buffer, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
	    digest[0], digest[1], digest[2],  digest[3],  digest[4],  digest[5],  digest[6],  digest[7],
	    digest[8], digest[9], digest[10], digest[11], digest[12], digest[13], digest[14], digest[15]);
}




/*========================================================================*/

/*
  Copyright (C) 1999, 2000, 2002 Aladdin Enterprises.  All rights reserved.

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  L. Peter Deutsch
  ghost@aladdin.com

 */
/* $Id: md5.c,v 1.2 2007/08/27 13:37:04 jwagnerhki Exp $ */
/*
  Independent implementation of MD5 (RFC 1321).

  This code implements the MD5 Algorithm defined in RFC 1321, whose
  text is available at
        http://www.ietf.org/rfc/rfc1321.txt
  The code is derived from the text of the RFC, including the test suite
  (section A.5) but excluding the rest of Appendix A.  It does not include
  any code or documentation that is identified in the RFC as being
  copyrighted.

  The original and principal author of md5.c is L. Peter Deutsch
  <ghost@aladdin.com>.  Other authors are noted in the change history
  that follows (in reverse chronological order):

  2007-08-27 Jan Wagner; copied into Tsunami for big-endian machine support
  2002-04-13 lpd Clarified derivation from RFC 1321; now handles byte order
        either statically or dynamically; added missing #include <string.h>
        in library.
  2002-03-11 lpd Corrected argument list for main(), and added int return
        type, in test program and T value program.
  2002-02-21 lpd Added missing #include <stdio.h> in test program.
  2000-07-03 lpd Patched to eliminate warnings about "constant is
        unsigned in ANSI C, signed in traditional"; made test program
        self-checking.
  1999-11-04 lpd Edited comments slightly for automatic TOC extraction.
  1999-10-18 lpd Fixed typo in header comment (ansi2knr rather than md5).
  1999-05-03 lpd Original version.
 */

#undef BYTE_ORDER       /* 1 = big-endian, -1 = little-endian, 0 = unknown */
#ifdef ARCH_IS_BIG_ENDIAN
#  define BYTE_ORDER (ARCH_IS_BIG_ENDIAN ? 1 : -1)
#else
#  define BYTE_ORDER 0
#endif

#define T_MASK ((md5_word_t)~0)
#define T1 /* 0xd76aa478 */ (T_MASK ^ 0x28955b87)
#define T2 /* 0xe8c7b756 */ (T_MASK ^ 0x173848a9)
#define T3    0x242070db
#define T4 /* 0xc1bdceee */ (T_MASK ^ 0x3e423111)
#define T5 /* 0xf57c0faf */ (T_MASK ^ 0x0a83f050)
#define T6    0x4787c62a
#define T7 /* 0xa8304613 */ (T_MASK ^ 0x57cfb9ec)
#define T8 /* 0xfd469501 */ (T_MASK ^ 0x02b96afe)
#define T9    0x698098d8
#define T10 /* 0x8b44f7af */ (T_MASK ^ 0x74bb0850)
#define T11 /* 0xffff5bb1 */ (T_MASK ^ 0x0000a44e)
#define T12 /* 0x895cd7be */ (T_MASK ^ 0x76a32841)
#define T13    0x6b901122
#define T14 /* 0xfd987193 */ (T_MASK ^ 0x02678e6c)
#define T15 /* 0xa679438e */ (T_MASK ^ 0x5986bc71)
#define T16    0x49b40821
#define T17 /* 0xf61e2562 */ (T_MASK ^ 0x09e1da9d)
#define T18 /* 0xc040b340 */ (T_MASK ^ 0x3fbf4cbf)
#define T19    0x265e5a51
#define T20 /* 0xe9b6c7aa */ (T_MASK ^ 0x16493855)
#define T21 /* 0xd62f105d */ (T_MASK ^ 0x29d0efa2)
#define T22    0x02441453
#define T23 /* 0xd8a1e681 */ (T_MASK ^ 0x275e197e)
#define T24 /* 0xe7d3fbc8 */ (T_MASK ^ 0x182c0437)
#define T25    0x21e1cde6
#define T26 /* 0xc33707d6 */ (T_MASK ^ 0x3cc8f829)
#define T27 /* 0xf4d50d87 */ (T_MASK ^ 0x0b2af278)
#define T28    0x455a14ed
#define T29 /* 0xa9e3e905 */ (T_MASK ^ 0x561c16fa)
#define T30 /* 0xfcefa3f8 */ (T_MASK ^ 0x03105c07)
#define T31    0x676f02d9
#define T32 /* 0x8d2a4c8a */ (T_MASK ^ 0x72d5b375)
#define T33 /* 0xfffa3942 */ (T_MASK ^ 0x0005c6bd)
#define T34 /* 0x8771f681 */ (T_MASK ^ 0x788e097e)
#define T35    0x6d9d6122
#define T36 /* 0xfde5380c */ (T_MASK ^ 0x021ac7f3)
#define T37 /* 0xa4beea44 */ (T_MASK ^ 0x5b4115bb)
#define T38    0x4bdecfa9
#define T39 /* 0xf6bb4b60 */ (T_MASK ^ 0x0944b49f)
#define T40 /* 0xbebfbc70 */ (T_MASK ^ 0x4140438f)
#define T41    0x289b7ec6
#define T42 /* 0xeaa127fa */ (T_MASK ^ 0x155ed805)
#define T43 /* 0xd4ef3085 */ (T_MASK ^ 0x2b10cf7a)
#define T44    0x04881d05
#define T45 /* 0xd9d4d039 */ (T_MASK ^ 0x262b2fc6)
#define T46 /* 0xe6db99e5 */ (T_MASK ^ 0x1924661a)
#define T47    0x1fa27cf8
#define T48 /* 0xc4ac5665 */ (T_MASK ^ 0x3b53a99a)
#define T49 /* 0xf4292244 */ (T_MASK ^ 0x0bd6ddbb)
#define T50    0x432aff97
#define T51 /* 0xab9423a7 */ (T_MASK ^ 0x546bdc58)
#define T52 /* 0xfc93a039 */ (T_MASK ^ 0x036c5fc6)
#define T53    0x655b59c3
#define T54 /* 0x8f0ccc92 */ (T_MASK ^ 0x70f3336d)
#define T55 /* 0xffeff47d */ (T_MASK ^ 0x00100b82)
#define T56 /* 0x85845dd1 */ (T_MASK ^ 0x7a7ba22e)
#define T57    0x6fa87e4f
#define T58 /* 0xfe2ce6e0 */ (T_MASK ^ 0x01d3191f)
#define T59 /* 0xa3014314 */ (T_MASK ^ 0x5cfebceb)
#define T60    0x4e0811a1
#define T61 /* 0xf7537e82 */ (T_MASK ^ 0x08ac817d)
#define T62 /* 0xbd3af235 */ (T_MASK ^ 0x42c50dca)
#define T63    0x2ad7d2bb
#define T64 /* 0xeb86d391 */ (T_MASK ^ 0x14792c6e)


static void
md5_process(md5_state_t *pms, const md5_byte_t *data /*[64]*/)
{
    md5_word_t
        a = pms->abcd[0], b = pms->abcd[1],
        c = pms->abcd[2], d = pms->abcd[3];
    md5_word_t t;
#if BYTE_ORDER > 0
    /* Define storage only for big-endian CPUs. */
    md5_word_t X[16];
#else
    /* Define storage for little-endian or both types of CPUs. */
    md5_word_t xbuf[16];
    const md5_word_t *X;
#endif

    {
#if BYTE_ORDER == 0
        /*
         * Determine dynamically whether this is a big-endian or
         * little-endian machine, since we can use a more efficient
         * algorithm on the latter.
         */
        static const int w = 1;

        if (*((const md5_byte_t *)&w)) /* dynamic little-endian */
#endif
#if BYTE_ORDER <= 0             /* little-endian */
        {
            /*
             * On little-endian machines, we can process properly aligned
             * data without copying it.
             */
            if (!((data - (const md5_byte_t *)0) & 3)) {
                /* data are properly aligned */
                X = (const md5_word_t *)data;
            } else {
                /* not aligned */
                memcpy(xbuf, data, 64);
                X = xbuf;
            }
        }
#endif
#if BYTE_ORDER == 0
        else                    /* dynamic big-endian */
#endif
#if BYTE_ORDER >= 0             /* big-endian */
        {
            /*
             * On big-endian machines, we must arrange the bytes in the
             * right order.
             */
            const md5_byte_t *xp = data;
            int i;

#  if BYTE_ORDER == 0
            X = xbuf;           /* (dynamic only) */
#  else
#    define xbuf X              /* (static only) */
#  endif
            for (i = 0; i < 16; ++i, xp += 4)
                xbuf[i] = xp[0] + (xp[1] << 8) + (xp[2] << 16) + (xp[3] << 24);
        }
#endif
    }

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

    /* Round 1. */
    /* Let [abcd k s i] denote the operation
       a = b + ((a + F(b,c,d) + X[k] + T[i]) <<< s). */
#define F(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define SET(a, b, c, d, k, s, Ti)\
  t = a + F(b,c,d) + X[k] + Ti;\
  a = ROTATE_LEFT(t, s) + b
    /* Do the following 16 operations. */
    SET(a, b, c, d,  0,  7,  T1);
    SET(d, a, b, c,  1, 12,  T2);
    SET(c, d, a, b,  2, 17,  T3);
    SET(b, c, d, a,  3, 22,  T4);
    SET(a, b, c, d,  4,  7,  T5);
    SET(d, a, b, c,  5, 12,  T6);
    SET(c, d, a, b,  6, 17,  T7);
    SET(b, c, d, a,  7, 22,  T8);
    SET(a, b, c, d,  8,  7,  T9);
    SET(d, a, b, c,  9, 12, T10);
    SET(c, d, a, b, 10, 17, T11);
    SET(b, c, d, a, 11, 22, T12);
    SET(a, b, c, d, 12,  7, T13);
    SET(d, a, b, c, 13, 12, T14);
    SET(c, d, a, b, 14, 17, T15);
    SET(b, c, d, a, 15, 22, T16);
#undef SET

     /* Round 2. */
     /* Let [abcd k s i] denote the operation
          a = b + ((a + G(b,c,d) + X[k] + T[i]) <<< s). */
#define G(x, y, z) (((x) & (z)) | ((y) & ~(z)))
#define SET(a, b, c, d, k, s, Ti)\
  t = a + G(b,c,d) + X[k] + Ti;\
  a = ROTATE_LEFT(t, s) + b
     /* Do the following 16 operations. */
    SET(a, b, c, d,  1,  5, T17);
    SET(d, a, b, c,  6,  9, T18);
    SET(c, d, a, b, 11, 14, T19);
    SET(b, c, d, a,  0, 20, T20);
    SET(a, b, c, d,  5,  5, T21);
    SET(d, a, b, c, 10,  9, T22);
    SET(c, d, a, b, 15, 14, T23);
    SET(b, c, d, a,  4, 20, T24);
    SET(a, b, c, d,  9,  5, T25);
    SET(d, a, b, c, 14,  9, T26);
    SET(c, d, a, b,  3, 14, T27);
    SET(b, c, d, a,  8, 20, T28);
    SET(a, b, c, d, 13,  5, T29);
    SET(d, a, b, c,  2,  9, T30);
    SET(c, d, a, b,  7, 14, T31);
    SET(b, c, d, a, 12, 20, T32);
#undef SET

     /* Round 3. */
     /* Let [abcd k s t] denote the operation
          a = b + ((a + H(b,c,d) + X[k] + T[i]) <<< s). */
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define SET(a, b, c, d, k, s, Ti)\
  t = a + H(b,c,d) + X[k] + Ti;\
  a = ROTATE_LEFT(t, s) + b
     /* Do the following 16 operations. */
    SET(a, b, c, d,  5,  4, T33);
    SET(d, a, b, c,  8, 11, T34);
    SET(c, d, a, b, 11, 16, T35);
    SET(b, c, d, a, 14, 23, T36);
    SET(a, b, c, d,  1,  4, T37);
    SET(d, a, b, c,  4, 11, T38);
    SET(c, d, a, b,  7, 16, T39);
    SET(b, c, d, a, 10, 23, T40);
    SET(a, b, c, d, 13,  4, T41);
    SET(d, a, b, c,  0, 11, T42);
    SET(c, d, a, b,  3, 16, T43);
    SET(b, c, d, a,  6, 23, T44);
    SET(a, b, c, d,  9,  4, T45);
    SET(d, a, b, c, 12, 11, T46);
    SET(c, d, a, b, 15, 16, T47);
    SET(b, c, d, a,  2, 23, T48);
#undef SET

     /* Round 4. */
     /* Let [abcd k s t] denote the operation
          a = b + ((a + I(b,c,d) + X[k] + T[i]) <<< s). */
#define I(x, y, z) ((y) ^ ((x) | ~(z)))
#define SET(a, b, c, d, k, s, Ti)\
  t = a + I(b,c,d) + X[k] + Ti;\
  a = ROTATE_LEFT(t, s) + b
     /* Do the following 16 operations. */
    SET(a, b, c, d,  0,  6, T49);
    SET(d, a, b, c,  7, 10, T50);
    SET(c, d, a, b, 14, 15, T51);
    SET(b, c, d, a,  5, 21, T52);
    SET(a, b, c, d, 12,  6, T53);
    SET(d, a, b, c,  3, 10, T54);
    SET(c, d, a, b, 10, 15, T55);
    SET(b, c, d, a,  1, 21, T56);
    SET(a, b, c, d,  8,  6, T57);
    SET(d, a, b, c, 15, 10, T58);
    SET(c, d, a, b,  6, 15, T59);
    SET(b, c, d, a, 13, 21, T60);
    SET(a, b, c, d,  4,  6, T61);
    SET(d, a, b, c, 11, 10, T62);
    SET(c, d, a, b,  2, 15, T63);
    SET(b, c, d, a,  9, 21, T64);
#undef SET

     /* Then perform the following additions. (That is increment each
        of the four registers by the value it had before this block
        was started.) */
    pms->abcd[0] += a;
    pms->abcd[1] += b;
    pms->abcd[2] += c;
    pms->abcd[3] += d;
}

void
md5_init(md5_state_t *pms)
{
    pms->count[0] = pms->count[1] = 0;
    pms->abcd[0] = 0x67452301;
    pms->abcd[1] = /*0xefcdab89*/ T_MASK ^ 0x10325476;
    pms->abcd[2] = /*0x98badcfe*/ T_MASK ^ 0x67452301;
    pms->abcd[3] = 0x10325476;
}

void
md5_append(md5_state_t *pms, const md5_byte_t *data, int nbytes)
{
    const md5_byte_t *p = data;
    int left = nbytes;
    int offset = (pms->count[0] >> 3) & 63;
    md5_word_t nbits = (md5_word_t)(nbytes << 3);

    if (nbytes <= 0)
        return;

    /* Update the message length. */
    pms->count[1] += nbytes >> 29;
    pms->count[0] += nbits;
    if (pms->count[0] < nbits)
        pms->count[1]++;

    /* Process an initial partial block. */
    if (offset) {
        int copy = (offset + nbytes > 64 ? 64 - offset : nbytes);

        memcpy(pms->buf + offset, p, copy);
        if (offset + copy < 64)
            return;
        p += copy;
        left -= copy;
        md5_process(pms, pms->buf);
    }

    /* Process full blocks. */
    for (; left >= 64; p += 64, left -= 64)
        md5_process(pms, p);

    /* Process a final partial block. */
    if (left)
        memcpy(pms->buf, p, left);
}

void
md5_finish(md5_state_t *pms, md5_byte_t digest[16])
{
    static const md5_byte_t pad[64] = {
        0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    md5_byte_t data[8];
    int i;

    /* Save the length before padding. */
    for (i = 0; i < 8; ++i)
        data[i] = (md5_byte_t)(pms->count[i >> 2] >> ((i & 3) << 3));
    /* Pad to 56 bytes mod 64. */
    md5_append(pms, pad, ((55 - (pms->count[0] >> 3)) & 63) + 1);
    /* Append the length. */
    md5_append(pms, data, 8);
    for (i = 0; i < 16; ++i)
        digest[i] = (md5_byte_t)(pms->abcd[i >> 2] >> ((i & 3) << 3));
}



/*========================================================================
 * $Log: md5.c,v $
 * Revision 1.2  2007/08/27 13:37:04  jwagnerhki
 * build 28: big endian platform support
 *
 * Revision 1.1.1.1  2006/07/20 09:21:19  jwagnerhki
 * reimport
 *
 * Revision 1.1  2006/07/10 12:27:29  jwagnerhki
 * added to trunk
 *
 */
