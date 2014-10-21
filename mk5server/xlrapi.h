#ifndef XLRAPI_H
#define XLRAPI_H
//
// Header file for StreamStor API functions
//
// Copyright 2000 Conduant Corporation
//
#ifdef WIN32
#include <windows.h>
#endif
#include "xlrtypes.h"

#ifdef WIN32
#  ifdef SSAPI_EXPORTS
#    define XLREXPORT extern "C" __declspec(dllexport)
#  else
#    ifdef __cplusplus
#      define XLREXPORT extern "C" __declspec(dllimport)
#    else
#      define XLREXPORT __declspec(dllimport)
#    endif
#  endif
#else
#  define XLREXPORT   extern
#endif

#if defined( WIN32 )
# if defined( NOSTDCALL )
#  define XLRCALLTYPE
# else
#  define XLRCALLTYPE  __stdcall
# endif
#else
# define XLRCALLTYPE
#endif


#define XLR_SUCCESS           0     // API Function succeeded
#define XLR_FAIL              1     // API Function failed
#define XLR_DATECODE_LENGTH   12    // Character buffer length required for version datecode
#define XLR_VERSION_LENGTH    8     // Character buffer length required for version number
#define XLR_ERROR_LENGTH      81    // Character buffer length required for error string
#define XLR_WRAP_DISABLE      0     // Switch value to disable data wrapping in XlrRecord()
#define XLR_WRAP_ENABLE       1     // Switch value to enable data wrapping in XlrRecord()
#define XLR_MAX_NAME          12    // Length of maximum name string
#define XLR_MAX_VRS           4     // Maximum number of configurable virtual recorders
#define XLR_MAX_DRIVENAME     44
#define XLR_MAX_DRIVESERIAL   24
#define XLR_MAX_DRIVEREV      12
#define XLR_MAX_UDIR_LENGTH   10485760  // Maximum size of a user directory is 
                                        //10 megabyte.

#define XLR_MASTER_DRIVE      0
#define XLR_SLAVE_DRIVE       1

// FIFO mode definitions
#define XLR_MODE_DEFAULT      0     // Default recorder mode
#define XLR_MODE_FIFO         1     // Normal FIFO mode (destructive reads)

// Read status return values
#define XLR_READ_COMPLETE  0
#define XLR_READ_ERROR     1
#define XLR_READ_WAITING   2
#define XLR_READ_RUNNING   3

// XLRSetMode modes
#define SS_MODE_PCI             0  // default mode
#define SS_MODE_EXT             1  // Data received and played over external 
                                   // port
#define SS_MODE_READ_EXT        2  // Data passed from external to PCI (no disk)
#define SS_MODE_WRITE_EXT       3  // Data passed from PCI to external (no disk)
#define SS_MODE_PCI_FORK        4  // Data forked from PCI to disk and ext port
#define SS_MODE_EXT_FORK        5  // Data forked from ext port to disk and PCI
#define SS_MODE_EXT_TO_PCI_FIFO 7  // FIFO data through disk from external to PCI
#define SS_MODE_EXT_FORK_TOP    8  // Data forked from front port to disk and top port
#define SS_MODE_EXT_FORK_FRT    9  // Data forked from top port to disk and front port
#define SS_MODE_EXT_PASS_TOP    10 // Pass through from front port to top (no disk)
#define SS_MODE_EXT_PASS_FRT    11 // Pass through from top port to front port (no disk)

//
// Options for the XLRSetOption() and XLRGetOption commands.
// NOTE:  These SS_OPT_ names must reference an enumerated B_ name defined 
// in xlrtypes.h so that the SS_OPT_ names will always match the bits 
// defined for firmware.
//
// - DRVSTATS:  If set, enable collection of drives stats.  If not, disable 
//    collection.
//
// - SKIPCHECKDIR:  If set, skip the directory check. 
//
// - RESERVE_END_BLOCK:  If set, reserve space at the end of a recording or 
//    drive  module.
//
// - RECORDWWAIT:  If set, recording will not begin until the SWITCHOVER  
//    bit is asserted.
//
// - PLAYARM:  If set, Enables "play arm" feature so that a play request will 
//    not start sending data until a PLAY_TRIGGER command.
//
// - ALL_OPTIONS:  Used to call XLRClearOption to set the option vector back 
//   to its default condition - which is all bits turned off.
//

#define SS_OPT_DRVSTATS          (1<<B_OPT_DRIVESTATS)   
#define SS_OPT_SKIPCHECKDIR      (1<<B_OPT_SKIPCHECKDIR) 
#define SS_OPT_RESERVE_END_BLOCK (1<<B_OPT_RESERVE_END_BLOCK) 
#define SS_OPT_RECORDWAIT        (1<<B_OPT_RECORDWAIT)  
#define SS_OPT_PLAYARM           (1<<B_OPT_PLAYARM)
#define SS_ALL_OPTIONS           0xFFFFFFFF     

// Modes for the XLRSetFPDPMode() command.
#define SS_FPDP_RECVMASTER          0     // Terminated.  This is the default.
#define SS_FPDP_RECV                1     // Non-terminated.
#define SS_FPDP_XMIT                2     // does not drive clock
#define SS_FPDP_XMITMASTER          3     // normal transmit
#define SS_FPDP_RECVMASTER_CLOCKS   4     // Like RECVMASTER but drives clocks.

//
// Options for the XLRSetFPDPMode() command.  These must have corresponding
// values in xlrpci.h  Option values must equate to a single set bit so that
// they can be OR'ed together to set multiple options.
//
#define SS_OPT_FPDPNRASSERT     0x00001   // Causes FPDP port to assert
                                          // "Not Ready" when in external port
                                          // mode.
#define SS_OPT_FPDPEXTCONN      0x00002   // Use the connector on the card 
                                          // bracket  (external access)
#define SS_OPT_FPDPSYNC         0x00004   // Wait for synch pulse before 
                                          // recording
#define SS_OPT_FPDPSTROB        0x00008   // Enable data strobe clock (TTL stobe
                                          // signals).  Default is pstrobe clock
                                          // (PECL strobe signals).
#define SS_OPT_FPDPNOPLL        0x00010   // No Phase Lock Loop - allows 0 - 
                                          // 40 MHz operation without this 
                                          // option permitted speeds are 
                                          // 10 - 50 MHz.
#define SS_OPT_FPDPSINGLEFRAME  0x00020   // Single frame data format while
                                          // recording and playing back.
#define SS_OPT_FPDPEVENTLOG     0x00040   // Event Logging - sync pulse triggers
                                          // the recording of the position w/in
                                          // the data where pulse occured
#define SS_OPT_FPDPSYNCARM      0x00080   // Preps Recording but waits for FPDP sync
                                          // Recording of data will not occur until
                                          // XLRArmFPDP call is made and then sync is
                                          // received

#define SS_OPT_FPDPALL (SS_OPT_FPDPNRASSERT|SS_OPT_FPDPEXTCONN|SS_OPT_FPDPSYNC \
                        |SS_OPT_FPDPSTROB|SS_OPT_FPDPNOPLL|SS_OPT_FPDPSINGLEFRAME \
                        |SS_OPT_FPDPEVENTLOG|SS_OPT_FPDPSYNCARM)

// Settings for the XLRSetPortClock() command.
#define SS_PORTCLOCK_6MHZ    0
#define SS_PORTCLOCK_8MHZ    1
#define SS_PORTCLOCK_10MHZ   2
#define SS_PORTCLOCK_11MHZ   3  // 11.4 MHz
#define SS_PORTCLOCK_13MHZ   4  // 13.33 MHz
#define SS_PORTCLOCK_16MHZ   5
#define SS_PORTCLOCK_20MHZ   6
#define SS_PORTCLOCK_25MHZ   7  // 26.56 MHz
#define SS_PORTCLOCK_26MHZ   8  // 26.66 MHz
#define SS_PORTCLOCK_32MHZ   9
#define SS_PORTCLOCK_40MHZ   10
#define SS_PORTCLOCK_50MHZ   11
#define SS_PORTCLOCK_51MHZ   12 // 50.04 MHz

// Bank Mode definitions
enum _SSBANKMODE {
   SS_BANKMODE_NORMAL   = 0,        // Puts the system into bank mode
   SS_BANKMODE_DISABLED = 1,        // Puts the system into non-bank mode
   SS_BANKMODE_AUTO_ON_FULL = 2,    // Puts the system into bank mode with
                                    // auto-continue when bank is full.
   SS_BANKMODE_AUTO_ON_SYNC = 3     // Puts the system into bank mode
                                    // switches bank when recording bank is
                                    // near full and sync pulse is received
};
typedef enum _SSBANKMODE S_BANKMODE;

#define XLR_LABEL_LENGTH   256   // Maximum length of label that can be
                                 // written using XLRSetLabel command.

#define MAX_EVENTS         512

// Chassis Types
#define UNKNOWN_CHASSIS_TYPE  0  // For chassis types prior to the bank
                                 // enabled type.  Also for unknown chassis
                                 // types

#define TK200                 1  // TK200 chassis (supports bank switching)


// For collecting drive statistics.
#define XLR_MAXBINS        8     // Number of bins to use when collecting
                                 // frequency data for GetDriveStats.

// To indicate that XLRGetDiskLength could not return a valid value.
#define SS_UNDEFINED_LENGTH   1

// Applications that can be loaded.
#define SS_DEFAULT_APP        0
#define SS_APP_1              1

// Data Recovery modes
#define SS_RECOVER_POWERFAIL  0
#define SS_RECOVER_OVERWRITE  1
#define SS_RECOVER_UNERASE    2

// Playback buffer status values.
#define SS_PBS_IDLE     0
#define SS_PBS_FILLING  1
#define SS_PBS_FULL     2
#define SS_PBS_PLAYING  3
#define SS_PBS_UNKNOWN  10

typedef struct _DEVSTATUS
{
   BOOLEAN SystemReady;             // The system is ready for read/write
   BOOLEAN BootmonReady;            // The flash monitor is ready
   BOOLEAN Recording;               // At least one VR is recording
   BOOLEAN Playing;                 // System is playing data.
   BOOLEAN Reserved1;               // unused 
   BOOLEAN Reserved2;               // unused
   BOOLEAN Reserved3;               // unused
   BOOLEAN Reserved4;               // unused
   BOOLEAN RecordActive[XLR_MAX_VRS];  // VR is recording
   BOOLEAN ReadActive[XLR_MAX_VRS];    // VR is currently reading
   BOOLEAN FifoActive;              // Recorder is in FIFO mode
   BOOLEAN DriveFail;               // Drive failure
   UINT DriveFailNumber;            // Drive number of failed drive (valid only if DriveFail set)
   BOOLEAN SysError;                // A system error occurred
   UINT SysErrorCode;               // System error code
   BOOLEAN CtlrError;               // A drive controller failed to initialize
   BOOLEAN FifoFull;                // The fifo overflowed
   BOOLEAN Overflow[XLR_MAX_VRS];   // True if recorder has reach capacity 
                                    // during record.
} S_DEVSTATUS, *PS_DEVSTATUS;

typedef struct _DEVINFO
{
   char     BoardType[XLR_MAX_NAME];
   UINT     SerialNum;           // Board serial number
   UINT     NumDrives;           // Number of drives attached
   UINT     NumBuses;            // Number of ATA buses in use
   UINT     TotalCapacity;       // Total capacity expressed in system pages
   UINT     MaxBandwidth;        // Reserved
   UINT     PciBus;              // PCI bus number
   UINT     PciSlot;             // PCI slot number
   UINT     NumExtPorts;         // Number of external ports
}S_DEVINFO, *PS_DEVINFO;

typedef struct _DRIVEINFO
{
   char     Model[XLR_MAX_DRIVENAME];      // Drive Model
   char     Serial[XLR_MAX_DRIVESERIAL];   // Drive serial number
   char     Revision[XLR_MAX_DRIVEREV];    // Drive firmware revision
   UINT     Capacity;                      // Capacity in blocks (512 bytes)
   BOOLEAN  SMARTCapable;
   BOOLEAN  SMARTState;                    // Drive SMART state
}S_DRIVEINFO, *PS_DRIVEINFO;   

//
// For collection of drive statistics.  Need to declare one of thes
// for each bin.  See XLR_MAXBINS.
//
typedef struct _S_DRIVESTATS {
   ULONG range;                           // end range of this bin (nanosecs)
   ULONG count;                           // number of observations within 
                                          // the range.
} S_DRIVESTATS, *PS_DRIVESTATS; 
    
typedef struct _DIR
{
   DWORDLONG   Length;           // Total length recorded
   DWORDLONG   AppendLength;     // Length of last data set
   BOOLEAN     Full;             // If FALSE (i.e. 0), system is not full 
                                 // If TRUE  (i.e. 1), system is full
   BOOLEAN     WriteProtected;   // If FALSE (i.e. 0), system is not 
                                 // write protected.
                                 // If TRUE  (i.e. 1), system is write 
                                 // protected.
}S_DIR, *PS_DIR;

typedef struct _READDESC{
   PULONG   BufferAddr;
   ULONG    AddrHi;
   ULONG    AddrLo;
   ULONG    XferLength;
}S_READDESC, *PS_READDESC;


typedef struct _XLRSWREV
{
   char     ApiVersion[XLR_VERSION_LENGTH];
   char     ApiDateCode[XLR_DATECODE_LENGTH];
   char     FirmwareVersion[XLR_VERSION_LENGTH];
   char     FirmDateCode[XLR_DATECODE_LENGTH];
   char     MonitorVersion[XLR_VERSION_LENGTH];
   char     XbarVersion[XLR_VERSION_LENGTH];
   char     AtaVersion[XLR_VERSION_LENGTH];
   char     UAtaVersion[XLR_VERSION_LENGTH];
   char     DriverVersion[XLR_VERSION_LENGTH];
}S_XLRSWREV, *PS_XLRSWREV;

//
// Structure of items in the array of error messages and their
// corresponding error codes.
struct XLR_ERRMSGS
{
   char msg_string[XLR_ERROR_LENGTH];
      UINT msg_code;
};


//
// A structure of type S_BANKSTATUS is populated and returned to the
// caller where XLRGetBankStatus is called.  Note that all char type
// declarations must be at the beginning of the structure.
//

typedef struct _BANKSTATUS
{

   char Label[XLR_LABEL_LENGTH]; // The label on the drive module.

   DWORDLONG   Length;        // Amount of data recorded on the drive set or
                              // (if bank mode) the currently selected drive
                              // module.

   UINT  State;               // If ready, this drive module is ready (powered 
                              // up, etc.) and can be used immediately for 
                              // recording or playback.  Otherwise, this
                              // drive module is not ready.

   UINT  Selected;            // If TRUE, the specified bank in 
                              // XLRGetBankStatus is the selected bank.

   UINT  PowerRequested;      // If TRUE, a request for power up has been
                              // received on this drive module.  Otherwise, a
                              // request has not been made.

   UINT  PowerEnabled;        // If TRUE, this, this drive module has power.
                              // Otherwise, it has not been powered up.

   UINT  MediaStatus;         // See below for valid values

   UINT  WriteProtected;      // If TRUE, this drive module is write 
                              // protected. Otherwise, this drive module 
                              // is not write protected.

   UINT  ErrorCode;           // If 0, there is no error, otherwise 
                              // this value is an error code.
                              
   UINT  ErrorData;           // For additional data (if required) related 
                              // to ErrorCode.

   UINT  TotalCapacity;       // Total capacity expressed in system pages
                             
} S_BANKSTATUS, *PS_BANKSTATUS;

// ** BANKSTATUS **
// MediaStatus
#define MEDIASTATUS_EMPTY           0     // Module is empty
#define MEDIASTATUS_NOT_EMPTY       1     // Module is not empty - but not full
#define MEDIASTATUS_FULL            2     // Module is filled to capacity
#define MEDIASTATUS_FAULTED         3     // Module is not empty, but faulted
                                          // must be erased before it can be used

// State
#define STATE_NOT_READY             0     // Module is not ready
#define STATE_TRANSITION            1     // Module is transitioning between ready and not ready
#define STATE_READY                 2     // Module is ready to be used


// Function prototypes
//------------------------------------------------------
//

#ifndef WIN32
XLR_RETURN_CODE
XLRInit( void );
#endif

XLREXPORT void XLRCALLTYPE 
XLRApiVersion( char * string );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRAppend( IN SSHANDLE xlrHandle );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRCardReset ( UINT Index );

XLREXPORT void XLRCALLTYPE
XLRClose ( IN SSHANDLE xlrHandle );

XLREXPORT UINT XLRCALLTYPE
XLRDeviceFind ( void );

XLREXPORT ULONG XLRCALLTYPE
XLRGetBaseAddr( IN SSHANDLE xlrHandle );

XLREXPORT ULONG XLRCALLTYPE
XLRGetBaseRange( IN SSHANDLE xlrHandle );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRGetDeviceInfo ( IN SSHANDLE xlrHandle, OUT PS_DEVINFO devInfo);

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRGetDeviceStatus( IN SSHANDLE xlrHandle, OUT PS_DEVSTATUS devStatus );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRGetDirectory (IN SSHANDLE xlrHandle, OUT PS_DIR Dir );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRGetDriveInfo (IN SSHANDLE device, IN UINT Bus, IN UINT MasterSlave,
                 OUT PS_DRIVEINFO driveInfo );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRGetErrorMessage( char *errstr, XLR_ERROR_CODE error );

XLREXPORT XLR_ERROR_CODE XLRCALLTYPE
XLRGetLastError( void );

XLREXPORT DWORDLONG XLRCALLTYPE
XLRGetLength( IN SSHANDLE xlrHandle );

XLREXPORT ULONG XLRCALLTYPE
XLRGetLengthHigh (IN SSHANDLE xlrHandle );

XLREXPORT ULONG XLRCALLTYPE
XLRGetLengthLow (IN SSHANDLE xlrHandle );

XLREXPORT ULONG XLRCALLTYPE
XLRGetLengthPages (IN SSHANDLE device );

XLREXPORT void XLRCALLTYPE
XLRGetLengthLowHigh (IN SSHANDLE device, PULONG low, PULONG high );

XLREXPORT ULONG XLRCALLTYPE
XLRGetSystemAddr( IN SSHANDLE xlrHandle );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRGetVersion( IN SSHANDLE xlrHandle, OUT PS_XLRSWREV swRev );

XLREXPORT PULONG XLRCALLTYPE
XLRGetWindowAddr( IN SSHANDLE xlrHandle );

XLREXPORT ULONG XLRCALLTYPE
_XLRGetWindowAddr( IN SSHANDLE xlrHandle );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLROpen ( IN UINT devIndex, OUT PSSHANDLE newHandle );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRPlay ( IN SSHANDLE xlrHandle, IN PS_READDESC readData );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRPlayback( IN SSHANDLE device, IN ULONG AddrHi, IN ULONG AddrLo );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRRead ( IN SSHANDLE xlrHandle, IN PS_READDESC readData );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRReadFifo ( IN SSHANDLE xlrHandle, IN PULONG Buffer, IN ULONG Length, IN BOOLEAN Direct );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
_XLRRead ( IN SSHANDLE xlrHandle, IN PULONG BufferAddr, IN ULONG AddrHi,
          IN ULONG AddrLo, IN ULONG XferLength );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRReadData ( IN SSHANDLE xlrHandle, IN PULONG BufferAddr, IN ULONG AddrHi,
          IN ULONG AddrLo, IN ULONG XferLength );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRReadImmed ( IN SSHANDLE device, IN PS_READDESC readDesc );

XLREXPORT XLR_READ_STATUS XLRCALLTYPE
XLRReadStatus ( IN BOOLEAN Wait );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRRecord ( IN SSHANDLE xlrHandle, IN BOOLEAN WrapEnable, IN SHORT ZoneRange );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRReset ( IN SSHANDLE xlrHandle );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRSetFifoMode( IN SSHANDLE xlrHandle, IN ULONG Mode );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRSetReadLimit( SSHANDLE xlrHandle, IN ULONG ReadLimit );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRStop ( IN SSHANDLE xlrHandle );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRWrite ( IN SSHANDLE xlrHandle, IN PS_READDESC writeData );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRWriteData ( IN SSHANDLE xlrHandle, IN PVOID BufferAddr, IN ULONG XferLength );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE 
XLRSetMode ( IN SSHANDLE device, IN UINT mode);

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE 
XLRGetMode ( IN SSHANDLE device, OUT PS_SSMODE mode_ptr);

XLREXPORT UINT XLRCALLTYPE 
XLRSkip ( IN SSHANDLE device, IN UINT skip, IN BOOLEAN Forward);

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE 
XLRSetOption ( IN SSHANDLE device, IN UINT options_to_set);

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE 
XLRClearOption ( IN SSHANDLE device, IN UINT options_to_clear);

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE 
XLRGetOption ( IN SSHANDLE device, IN UINT options_to_get, 
OUT PBOOLEAN options_on);

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE 
XLRSetDriveStats ( IN SSHANDLE device, IN S_DRIVESTATS driveStats[] );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE 
XLRGetDriveStats ( IN SSHANDLE device, IN UINT Bus, IN UINT MasterSlave, 
  OUT PS_DRIVESTATS driveStats );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE 
XLRSetFPDPMode ( IN SSHANDLE device, IN FPDPMODE mode, IN FPDPOP options );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRSetPortClock ( IN SSHANDLE device, IN UINT value );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRSetPortBaseClock ( IN SSHANDLE device, IN UINT Rdw, IN UINT Vdw, 
  IN UINT outputDivider);

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRDeleteAppend( IN SSHANDLE device, IN ULONG AddrHi, IN ULONG AddrLo );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRTruncate( IN SSHANDLE device, IN ULONG AddrHi, IN ULONG AddrLo );

XLREXPORT DWORDLONG XLRCALLTYPE 
XLRGetPlayLength (IN SSHANDLE device );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE 
XLRSetFillData ( IN SSHANDLE device, IN UINT pattern );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE 
XLRSetBankMode( IN SSHANDLE device, IN S_BANKMODE mode );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRSelectBank( IN SSHANDLE device, IN UINT bankID );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRSetLabel( IN SSHANDLE device, IN char *label, IN UINT labelsize );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRGetLabel( IN SSHANDLE device, OUT char *label );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRSetWriteProtect ( IN SSHANDLE device );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRClearWriteProtect ( IN SSHANDLE device );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRGetBankStatus( IN SSHANDLE device, IN UINT bankID, 
      OUT PS_BANKSTATUS bankStatus );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRSyncArm( IN SSHANDLE xlrDevice, IN BOOLEAN armed );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRRecoverData( IN SSHANDLE xlrDevice, IN UINT Mode );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRSetPlaybackLength( IN SSHANDLE xlrDevice, IN ULONG AddrHi, 
   IN ULONG AddrLo );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRErase( IN SSHANDLE xlrDevice, IN SS_OWMODE mode );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRSetUserDir( IN SSHANDLE xlrDevice, IN PVOID udirPtr, UINT udirSize );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRGetUserDir( IN SSHANDLE xlrDevice, IN UINT XferLength, 
   IN UINT Offset, OUT PVOID udirPtr );

XLREXPORT UINT XLRCALLTYPE 
XLRGetUserDirLength( IN SSHANDLE xlrDevice );

XLREXPORT UINT XLRCALLTYPE
XLRGetChassisType( IN SSHANDLE xlrDevice );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE 
XLREdit( IN SSHANDLE device, IN PS_READDESC readDesc );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE 
XLRMountBank( IN SSHANDLE device, IN UINT bankId );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE 
XLRDismountBank( IN SSHANDLE device, IN UINT bankId );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRSelfTest( IN SSHANDLE device, IN SS_SELFTEST test );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRSetSwitchOver( IN SSHANDLE device ); 

XLREXPORT DWORDLONG XLRCALLTYPE 
XLRGetDiskLength( IN SSHANDLE device, IN UINT Bus, IN UINT MasterSlave );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE 
XLRPlayTrigger( IN SSHANDLE device );

XLREXPORT DWORD XLRCALLTYPE 
XLRDiskRepBlkCount( IN SSHANDLE device, IN UINT Bus, IN UINT MasterSlave );

XLREXPORT DWORD XLRCALLTYPE 
XLRTotalRepBlkCount( IN SSHANDLE device);

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE
XLRReadToPhy ( IN SSHANDLE xlrHandle, IN PS_READDESC readData );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE 
XLRSetAppID( IN UINT Index, IN UINT appID );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE 
XLRGetAppID ( IN UINT Index, OUT PUINT appID );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE 
XLRGetPlayBufferStatus ( IN SSHANDLE device, OUT PUINT statPtr );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE 
XLRDumpMemory ( IN SSHANDLE device, IN PULONG BufferAddr, 
   IN ULONG Offset, IN ULONG XferLength );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE XLREditData ( IN SSHANDLE device,
   IN PULONG BufferAddr, IN ULONG AddrHi, IN ULONG AddrLo,
   IN ULONG XferLength );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE XLRRetrieveEvents ( IN SSHANDLE device,
   IN DWORDLONG EventLog[] );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE XLRArmFPDP ( IN SSHANDLE device );

XLREXPORT DWORDLONG XLRCALLTYPE XLRGetFIFOLength( IN SSHANDLE device );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE XLRNetOpen ( IN UINT devIndex,
   IN PVOID address, IN USHORT port, OUT PSSHANDLE newHandle );

XLREXPORT XLR_RETURN_CODE XLRCALLTYPE XLRSerialOpen ( IN UINT devIndex,
   IN UINT commPort, OUT PSSHANDLE newHandle );
#endif
