# Other — include

# Tsunami Protocol Header Files

## Purpose

The **Other — include** module defines global headers used by both client and server applications in the Tsunami file transfer protocol suite. These files contain shared constants, data structures, function prototypes, and type definitions that are essential across different parts of the system.

## Key Components

### `md5.h`
Provides MD5 message digest functionality for cryptographic operations within the Tsunami protocol.

#### Functions:
- `md5_digest`: Computes MD5 hash of buffer data.
- `md5_fprint_digest`: Prints digest to a file.
- `md5_sprint_digest`: Converts digest to string format.

#### Types:
```c
typedef struct md5_state_s {
    md5_word_t count[2];
    md5_word_t abcd[4];
    md5_byte_t buf[64];
} md5_state_t;
```

### `tsunami-client.h`
Defines client-side global constants, data structures, and function prototypes for the Tsunami client application.

#### Constants:
- Default values for block size, table size, server name, ports, etc.
- Output modes (`SCREEN_MODE`, `LINE_MODE`)
- Maximum limits for commands, buffers, blocks

#### Data Structures:
- `command_t`: Parsed command with text words
- `statistics_t`: Transfer statistics including rates and counts
- `retransmit_t`: Retransmission state management
- `ring_buffer_t`: Queuing mechanism for disk writes
- `ttp_parameter_t`: Client transfer parameters
- `ttp_transfer_t`: Current transfer state
- `ttp_session_t`: Overall session state

#### Function Prototypes:
- Command handling functions (`command_close`, `command_connect`, `command_get`, etc.)
- Configuration reset (`reset_client`)
- I/O operations (`accept_block`)
- Network socket creation (`create_tcp_socket`, `create_udp_socket`)
- Protocol negotiation (`ttp_authenticate`, `ttp_negotiate`)

### `tsunami-server.h`
Defines server-side global constants, data structures, and function prototypes for the Tsunami server application.

#### Constants:
- Default TCP port, UDP buffer sizes, heartbeat timeouts
- File naming and storage limits

#### Data Structures:
- `ttp_parameter_t`: Server transfer parameters
- `ttp_transfer_t`: Transfer state information
- `ttp_session_t`: Session-wide state tracking

#### Function Prototypes:
- Configuration reset (`reset_server`)
- Datagram building (`build_datagram`)
- VSIB control functions (`start_vsib`, `stop_vsib`, `read_vsib_block`)
- Network socket creation (`create_tcp_socket`, `create_udp_socket`)
- Protocol functions (`ttp_accept_retransmit`, `ttp_authenticate`, `ttp_negotiate`)

### `tsunami-cvs-buildnr.h`
Contains version/build number information that is embedded in the software during compilation.

#### Macro:
- `TSUNAMI_CVS_BUILDNR`: String identifier for current build version

### `tsunami.h`
The primary header file containing core definitions shared by both client and server components.

#### Macros:
- `warn(message)`: Error reporting macro
- `error(message)`: Fatal error reporting macro
- `min(a,b)` / `max(a,b)`: Utility comparison macros
- `tv_diff_usec(newer,older)`: Time difference calculation

#### Constants:
- Protocol revision numbers
- Request types (`REQUEST_RETRANSMIT`, `REQUEST_RESTART`, etc.)
- Block type identifiers (`TS_BLOCK_ORIGINAL`, `TS_BLOCK_TERMINATE`, `TS_BLOCK_RETRANSMISSION`)
- Port defaults (`TS_TCP_PORT`, `TS_UDP_PORT`)

#### Types:
```c
typedef struct {
    u_int16_t           request_type;
    u_int32_t           block;
    u_int32_t           error_rate;
} retransmission_t;
```

## Architecture Overview

This module serves as a foundational layer providing:

1. **Cryptographic support** through MD5 hashing (in `md5.h`)
2. **Protocol constants and types** (in `tsunami.h`)
3. **Client-specific interfaces** (in `tsunami-client.h`)
4. **Server-specific interfaces** (in `tsunami-server.h`)
5. **Version identification** (in `tsunami-cvs-buildnr.h`)

## External Dependencies

These headers depend on standard system libraries including:
- `<sys/types.h>`
- `<stdio.h>`
- `<string.h>`
- `<netinet/in.h>`
- `<sys/time.h>`

They also include other headers from this same module to provide complete interface definitions.

## Usage Patterns

All applications within the Tsunami suite must include these headers to access protocol constants, data structures, and function declarations. The pattern of inclusion ensures consistent behavior across different implementations while maintaining modularity between client and server code paths.

For example, when implementing network operations or protocol handling, developers reference these headers to obtain proper data structure layouts and function signatures needed for interoperability with other parts of the system.