/* vsib_ioctl.h  -- Linux VSIbrute character driver ioctl() constants. */

/*
 * $Log: vsib_ioctl.h,v $
 * Revision 1.1.1.1  2006/07/20 09:21:20  jwagnerhki
 * reimport
 *
 * Revision 1.1  2006/07/10 12:35:12  jwagnerhki
 * added to trunk
 *
 * Revision 1.4  2002/06/14 13:00:39  amn
 * Dwingeloo test trip version.
 *
 * Revision 1.3  2002/03/21 11:23:38  amn
 * New-style command word bit assignments.
 *
 * Revision 1.2  2002/02/27 14:33:28  amn
 * Changed Log line to be on next line than the comment start characters.
 *
 * Revision 1.1  2002/02/27 14:24:38  amn
 * Initial version.
 *
 */

#define VSIB_SET_MODE 0x7801
#define VSIB_MODE_STOP 0x0
#define VSIB_MODE_DIVISOR 0x0000ffff
  /* 16-bit counter of # of samples to skip; i.e. div by (n+1) */
#define VSIB_MODE_RUN 0x80000000
  /* 0==stop/reset, 1==start */
#if 0
/* 1. generation wire-wrapped board. */
#define VSIB_MODE_2BIT 0x00020000
  /* 0==no bit shuffling, 1==rearrange 2-bit data in non-compat mode */
#define VSIB_MODE_COMPAT 0x00040000
  /* 0==rearrange 8 succ. bits into one byte, 1==parallel bits */
  /* directly to byte */
#else
/* 2. generation multi-mode board. */
#define VSIB_MODE_MODE_MASK 0x000f0000
#define VSIB_MODE_MODE(mode) (((mode) & 0x0f) << 16)
  /* xxx */
#define VSIB_MODE_GIGABIT 0x40000000
#define VSIB_MODE_EMBED_1PPS_MARKERS 0x20000000
#endif

#define VSIB_GET_STATUS 0x7802
#define VSIB_STATUS_RXSTATE 0x01
  /* 0==dma logic not running, 1==dma running, capturing data (sync found) */
#define VSIB_STATUS_INVCLOCKPHASE 0x08
  /* 0==clock and data rising edges apart, 1== clock and data rise simult. */
#define VSIB_STATUS_INVDATA 0x10
  /* 0==normal sync word found, 1==inverted sync word found */
#define VSIB_STATUS_DMAOVERRUN 0x20
  /* 0==ok, 1==new DMAREQ generated before the previous was DACKed */

#define VSIB_GET_DMA_RETRIES 0x7803
  /* gets the retry counter as 'unsigned long' into address pointed by 'arg' */

#define VSIB_GET_BYTES_IN_BIGBUF 0x7804
  /* gets the amount of data in big ring buffer */
  /* as 'unsigned long' into address pointed by 'arg' */

#define VSIB_RESET_DMA 0x7805
  /* resets the DMA controller on PLX chip */
  /* if arg == 0, clears memory buffers and r/w pointers, */
  /* if arg != 0, leaves memory buffers and r/w pointers alone */

#define VSIB_DELAYED_STOP_DMA 0x7806
  /* clears scatter-gather ring pointers, i.e. stops after current block */

#define VSIB_IS_DMA_DONE 0x7807
 /* returns ==0 if DMA still running, !=0 if done */
