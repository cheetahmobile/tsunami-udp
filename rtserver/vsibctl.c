/* Copyright (C) 2001--2002 Ari Mujunen, Ari.Mujunen@hut.fi

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */


/* Ensure 64-bit file support regardless of '-D_LARGEFILE64_SOURCE=1'
   in Makefile. */
#define _LARGEFILE64_SOURCE 1  /* Large File Support (LFS) '*64()' functions. */ 
#define _FILE_OFFSET_BITS 64  /* Automatic '*()' --> '*64()' replacement. */ 

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <sys/types.h>  /* open() */
#include <sys/stat.h>  /* open() */
#include <fcntl.h>  /* open() */

#include <sys/time.h>  /* gettimeofday() */
#include <time.h>
#include <unistd.h>  /* gettimeofday(), usleep() */

#include <string.h>  /* strstr() */

#include <sys/ipc.h>  /* for shared memory */
#include <sys/shm.h>

#include <sys/ioctl.h>
#include "vsib_ioctl.h"

#include <tsunami-server.h>



/* Upcoming wrshm.h: */
#define fourCharLong(a,b,c,d) ( ((long)a)<<24 | ((long)b)<<16 | ((long)c)<<8 | d )

/* Default values for mode */
int vsib_mode = 0;                     /* 0: 32 bit mode, 2: low 16 bit mode */
int vsib_mode_gigabit = 0;             /* 0: no gigabit mode */
int vsib_mode_embed_1pps_markers = 0;  /* 0: don't embed PPS markers, 1: embed markers */
int vsib_mode_skip_samples = 0;        /* 0: do not skip samples */

typedef struct sSh {
  int relSeekBlocks;
} tSh, *ptSh;


int readMode = 0;
int vsib_started = 0;
int usleeps = 0;

/* Shared memory unique identifier (key) and the returned reference id. */
key_t shKey;
int shId = -1;
ptSh sh;


/* Depending on readMode either standard input or output. */
int vsib_fileno;

/* A protected (error-checking) ioctl() for VSIB driver. */
static void vsib_ioctl(unsigned int mode, unsigned long arg) {
    if (ioctl(vsib_fileno, mode, arg)) {
       char err[255];
       snprintf(err, sizeof(err), "Fatal VSIB access error: ioctl(vsib_fileno=0x%04x, 0x%04x, %ld) failed", 
                vsib_fileno, mode, arg);
       perror(err);
       exit(EXIT_FAILURE);
    }
}

double tim(void) {
    struct timeval tv;
    double t = 0.0;
    if (gettimeofday(&tv, NULL) == 0) {
       t = (double)tv.tv_sec + (double)tv.tv_usec/1000000.0;
    }
    return t;
}

void start_vsib(ttp_session_t *session)
{
  ttp_transfer_t  *xfer  = &session->transfer;
 
  /* Find out the file number */ 
  vsib_fileno = fileno(xfer->vsib);
 
  /* Create and initialize 'wr<-->control' shared memory. */
  shKey = fourCharLong('v','s','i','b');
  if( (shId = shmget(shKey, sizeof(tSh), IPC_CREAT | 0777)) == -1 ) {
    fprintf(stderr, "Fatal VSIB access error: could not shmget() shared memory!\n"
                    "Previous instance of Tsunami was probably run as 'root' and interrupted. Reload 'vsib' kernel module.\n");
    abort();
  }
  if( (sh = (ptSh)shmat(shId, NULL, 0)) == (void *)-1 ) {
    fprintf(stderr, "Fatal VSIB access error: could not shmat() shared memory, seeking not possible, exiting!\n");
    abort();
  }
  sh->relSeekBlocks = 0;

  vsib_ioctl(VSIB_SET_MODE,
             (VSIB_MODE_MODE(vsib_mode)
              | VSIB_MODE_RUN
              | (vsib_mode_gigabit ? VSIB_MODE_GIGABIT : 0)
              | (vsib_mode_embed_1pps_markers ? VSIB_MODE_EMBED_1PPS_MARKERS : 0)
              | (vsib_mode_skip_samples & 0x0000ffff))
            );
}  /* start_VSIB */


/* Read a block from VSIB; if one fread() does not return enough, sleep a little and try again. */
void read_vsib_block(ttp_session_t* session, unsigned char *memblk, size_t blksize)
{
  size_t nread;
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 1000000L;
  nread = fread(memblk, 1, blksize, session->transfer.vsib);
  while (nread < blksize) {
     nanosleep(&ts, NULL);
     // usleeps++;
     nread += fread(memblk+nread, 1, blksize-nread, session->transfer.vsib);
  } 
}



void stop_vsib(ttp_session_t *session)
{
  int timeout = 0;
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 100000000L;

  /* Stop the board, first DMA, and when the last descriptor */
  /* has been transferred, then write stop to board command register. */
  vsib_ioctl(VSIB_DELAYED_STOP_DMA, 0);
  {
    unsigned long b;

    vsib_ioctl(VSIB_IS_DMA_DONE, (unsigned long)&b);
    while ((!b) && (timeout++ < 25)) {
      fprintf(stderr, "Waiting for last DMA descriptor (sl=%d)\n", usleeps);
      // usleep(100000); usleep not with pthreads!
      nanosleep(&ts, NULL);
      // usleeps++;
      vsib_ioctl(VSIB_IS_DMA_DONE, (unsigned long)&b);
    }
  }

  vsib_ioctl(VSIB_SET_MODE, VSIB_MODE_STOP);

  /* Remove shared memory to mark that 'vsibctl' is no more running. */
  if ((shId != -1) && (sh != (ptSh)-1) && (sh != NULL)) {
    //assert( shmctl(shId, IPC_RMID, NULL) == 0 );
    //assert( shmdt(sh) == 0 );
    if( shmctl(shId, IPC_RMID, NULL) != 0 ) {
       fprintf(stderr, "VSIB access error: shared memory ID remove shmctl() returned non-0\n");
    } else {
       if( shmdt(sh) != 0 ) {
          fprintf(stderr, "VSIB access error: shared memory data remove shmdt() returned non-0\n");
       }
    }
  }
}

