# Other — mk5server

# Mk5 Server Module Documentation

## Purpose

The **mk5server** module implements a file transfer server specifically designed for handling data transfers from Mark5 disk systems using StreamStor hardware APIs. It provides an interface between client applications and physical storage devices by managing TCP and UDP sockets for communication, supporting both IPv4 and IPv6 protocols.

This module is part of the Tsunami software package used in radio astronomy data acquisition systems.

## Key Components

### 1. Network Management (`network.c`)
- `create_tcp_socket()`: Creates and configures a TCP socket for incoming connections.
- `create_udp_socket()`: Sets up a UDP socket for data transmission with configurable buffer size.

Both functions support dual-stack networking via IPv6 or IPv4 depending on configuration flags.

### 2. File Access API (`mk5api.c`, `mk5api.h`)
Provides high-level file operations that wrap around StreamStor API functions:
- `mk5_fopen64()`: Opens a file handle for reading/writing.
- `mk5_fclose()`: Closes an open file handle.
- `mk5_fseek()`: Seeks within a file.
- `mk5_ftello()`: Reports current position in a file.
- `mk5_fread()`: Reads data into memory buffers.

These functions are implemented as wrappers over standard StreamStor API calls like `XLROpen`, `XLRRead`, etc., which interact directly with the underlying hardware.

### 3. Timestamp Utilities (`tstamp.c`)
Handles conversion between UTC timestamps and ASCII representations:
- `getDuration()`: Parses duration strings into double values.
- `getDateTime()`: Converts date-time string formats into Unix seconds.
- `formatTimestamp()` / `getUTC()` / `toTimestamp()`: Utility functions for formatting time-related data.

Used primarily for logging purposes but can be extended for other time-sensitive tasks.

### 4. Configuration & Main Loop (`config.c`, `main.c`)
- Handles command-line argument parsing and initialization parameters.
- Initializes network services and starts listening loops.
- Manages global state variables such as port numbers and IP version settings.

### 5. Protocol Handling (`protocol.c`)
Implements protocol-specific logic including:
- Parsing commands received through TCP.
- Routing requests to appropriate backend handlers (e.g., file access).

### 6. Logging (`log.c`)
Manages log output generation:
- Logs events related to connection establishment, read/write operations, errors, etc.
- Uses timestamp utilities defined above.

### 7. Transcript Support (`transcript.c`)
Supports transcript-based logging functionality:
- Records detailed session transcripts for debugging and analysis.

## Compilation Notes

The mk5server requires compilation using **g++** instead of gcc due to dependencies on libssapi.a from the StreamStor library, which was built with g++ v3.x.

To compile:

```bash
./configure
cd common
# Edit Makefile: change "CC = gcc" to "CC = g++"
make
cd ../mk5server
# Edit Makefile: change "CC = gcc" to "CC = g++"
make
```

Alternatively, you may link manually during quick checks:
```bash
g++ mk5api.c ../common/libtsunami_common.a -lm ../common/libssapi.a -lpthread
```

> Note: The automake configuration does not support setting `mk5tsunamid_CC = g++` directly; manual edits to generated Makefiles are required.

## Integration Points

### External Dependencies
- **StreamStor Library**: Requires `libssapi.a` compiled with g++
- **Tsunami Common Module**: Depends on shared libraries located at `../common/`
- **Standard C Libraries**: Includes POSIX socket APIs (`sys/socket.h`, `netdb.h`, etc.)

### Internal Calls
- `create_tcp_socket()` → calls `warn()` from `include/tsunami.h`
- `create_udp_socket()` → also calls `warn()` from `include/tsunami.h`
- `getUTC()` → uses `gettimeofday()` from `common/common_win32.c`
- `formatTimestamp()` → used by both `getUTC()` and `toTimestamp()`

### API Entry Points
- Clients connect via TCP or UDP sockets.
- File I/O is handled through functions declared in `mk5api.h`.
- These include wrappers around core StreamStor APIs like:
    - `XLROpen`
    - `XLRRead`
    - `XLRClose`
    - `XLRSetMode`

## Architecture Overview

```mermaid
graph TD
    A[Client] --> B[TCP/UDP Socket]
    B --> C[mk5server Main Loop]
    C --> D[Protocol Handler]
    D --> E[File Access Layer (mk5api)]
    E --> F[StreamStor API]
    F --> G[HDD / Storage Device]
```

This diagram illustrates how client requests flow into the server, processed through a protocol layer before being dispatched to hardware-specific operations.