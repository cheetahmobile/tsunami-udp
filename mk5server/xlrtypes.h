#ifndef _SSTYPES_
#define _SSTYPES_
/*******************************************************************
 *
 *    DESCRIPTION:
 *
 *    AUTHOR:
 *
 *    HISTORY:    
 *
 *******************************************************************/
#ifndef _WINDRVR_H_
#ifndef I2O
#ifndef __VOID
#ifdef TARGET
typedef void                  VOID;
#endif
typedef void *                PVOID;
#endif	/* __VOID */
#endif	/* I2O */

typedef char                  CHAR;
typedef char *                PCHAR;
typedef unsigned char         UCHAR;

typedef int                   INT;
typedef int *                 PINT;
typedef unsigned int          UINT;

typedef unsigned short        USHORT;

#ifndef WIN32
typedef unsigned long         DWORD;
#endif


typedef long                  LONG;
typedef long *                PLONG;
typedef unsigned long         ULONG;
#ifdef TARGET
typedef unsigned long long    ULONGLONG;
#endif

typedef UCHAR                 BYTE;

#endif  // _WINDRVR_H_

typedef unsigned char *       PUCHAR;
typedef unsigned short *      PUSHORT;
typedef unsigned long *       PULONG;
typedef unsigned int *        PUINT;
typedef short                 SHORT;
typedef short *               PSHORT;

#ifndef WIN32
typedef unsigned long long    DWORDLONG;
typedef UCHAR                 BOOLEAN;
typedef UCHAR *               PBOOLEAN;
#endif

#ifndef SSPORTDRIVER
typedef PULONG                PHYSICAL_ADDRESS;
typedef ULONG                 NTSTATUS;
#endif
typedef ULONG                 XLRHEADER;
typedef PULONG                PXLRHEADER;
typedef UINT                  SSHANDLE;
typedef SSHANDLE *            PSSHANDLE;
#define INVALID_SSHANDLE	((SSHANDLE)0xFFFFFFFF)

#ifndef SSPORTDRIVER
typedef char *                PSTRING; /* Pointer to string	*/
#endif
typedef ULONG                 XLR_RETURN_CODE;
typedef ULONG                 XLR_ERROR_CODE;
typedef ULONG                 XLR_READ_STATUS;

#ifdef TARGET
typedef volatile PUCHAR    PREG8;
typedef volatile PUSHORT   PREG16;
typedef volatile PUINT     PREG32;
typedef DWORDLONG          BLOCKHEADER;
typedef BLOCKHEADER *      PBLOCKHEADER;
typedef DWORDLONG          BLOCKNUM;      // Seperate type to allow partitioning
typedef BLOCKNUM *         PBLOCKNUM;
#endif

#ifdef TARGET
#define TRUE   1
#define FALSE  0
#define PASS   0
#define FAIL   1
//typedef enum {FALSE, TRUE} BOOLEAN;
//typedef enum {PASS, FAIL} BOOLEAN;
typedef BOOLEAN FLAG;
#endif
  

//
// XLR typedefs
//
typedef ULONG        CMDSTATUS;

//For XLRGetMode and XLRSetMode.
typedef UINT         SSMODE;
typedef UINT *       PS_SSMODE;

//For XLRSetFPDPMode().
typedef UINT         FPDPMODE;
typedef UINT         FPDPOP;

typedef struct _VERSION
{
   UCHAR    Major;
   UCHAR    Minor;
}S_VERSION, *PS_VERSION;

typedef struct _XLRVERSION
{
   S_VERSION   Firmware;
   S_VERSION   Monitor;
   S_VERSION   Xbar;
   S_VERSION   Ata;

   S_VERSION   Uata;
   char     FirmBuildDate[14];
}S_XLRVERSION, *PS_XLRVERSION;

// MONVERSION is old structure, kept for monitor for backward compatibility
typedef struct _MONVERSION
{
   UINT  FirmMajor;
   UINT  FirmMinor;
   char  FirmBuildDate[12];
   UINT  XbarMajor;
   UINT  XbarMinor;
}S_MONVERSION, *PS_MONVERSION;


typedef struct _FLASH_STRUCT
{
   PUSHORT  Addr;
   ULONG    Size;
}FLASH_STRUCT;

//For Bank Mode
typedef enum _BANK
{
   BANK_A,
   BANK_B,
   BANK_INVALID
} E_BANK;


//
// For the XLRErase command.
//
enum _SS_OWMODE
{
   SS_OVERWRITE_NONE = 0,
   SS_OVERWRITE_RANDOM_PATTERN = 1,
   SS_OVERWRITE_RW_PATTERN = 2,
   SS_OVERWRITE_DIRECTORY = 3
};

typedef enum _SS_OWMODE SS_OWMODE;

//
// Self-test level definition list
//
enum _SS_SELFTEST
{ 
   XLR_BIST_PCI,                   // test communications with PLX
   XLR_BIST_BUFFER,                // memory test on StreamStor buffer
   XLR_BIST_DISK0,                 // read/write verify test on disk drive
   XLR_BIST_DISK1,                 // read/write verify test on disk drive
   XLR_BIST_DISK2,                 // read/write verify test on disk drive
   XLR_BIST_DISK3,                 // read/write verify test on disk drive
   XLR_BIST_DISK4,                 // read/write verify test on disk drive
   XLR_BIST_DISK5,                 // read/write verify test on disk drive
   XLR_BIST_DISK6,                 // read/write verify test on disk drive
   XLR_BIST_DISK7,                 // read/write verify test on disk drive
   XLR_BIST_ALL,                   // all self tests except the loopback test
   XLR_NUM_BISTLEVELS              // number of self-test levels
}; 

typedef enum _SS_SELFTEST SS_SELFTEST;

//
// The B_SS_OPTIONS enumeration defines bit positions of various recorder options.
// These enum values are referenced in xlrapi.h and xlrpci.h and are designed to 
// keep coherency between API and firmware.  
//
// NOTE:  For legacy reasons, when adding a new enum value, append it rather than 
//        insert it.  Also note that this enum should not exceed 31 since options 
//        are currently stored as bits in one 32-bit integer.
//
enum  B_SS_OPTIONS
{
   B_OPT_DRIVESTATS,               // Enable drive statistics gathering
   B_OPT_SKIPCHECKDIR,             // Skip the directory check.
   B_OPT_RESERVE_END_BLOCK,        // Reserve space at end of recording.
   B_OPT_RECORDWAIT,               // Set wait on the waiting recorder.
   B_OPT_PLAYARM                   // Enables "play arm" feature so that a play request will not
                                   // start sending data until a PLAY_TRIGGER command.
};
#if( GEN2 | GEN3 )
typedef PUCHAR P_ATATYPE;
typedef UCHAR ATATYPE;
#else
typedef PUSHORT P_ATATYPE;
typedef USHORT ATATYPE;
#endif

#define BITISSET(x,y)   ((x)&(1<<(y)))
#define BITISCLR(x,y)   ((~(x))&(1<<(y)))

#ifndef WIN32
#ifndef TARGET
# define IN
# define OUT
# ifndef _WINDRVR_H_
//typedef void *HANDLE;
# endif
//typedef HANDLE *PHANDLE;
#endif
#endif
#endif
