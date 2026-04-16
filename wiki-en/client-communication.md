# Client Communication

# Client Communication Module

## Purpose

The Client Communication module provides the core functionality for interacting with Tsunami servers via the command-line interface. It handles all client-side operations including connection management, file transfer initiation, data reception, disk I/O handling, and protocol negotiation.

## Key Components

### Command Processing Functions

- `command_close()` - Closes active control session
- `command_connect()` - Establishes new control session to server
- `command_dir()` - Requests list of available remote files
- `command_get()` - Initiates single or multiple file transfers
- `command_help()` - Provides help on commands
- `command_quit()` - Closes connection and exits client
- `command_set()` - Sets configuration parameters

### Core Transfer Logic

#### Data Reception Thread (`disk_thread`)
A dedicated thread that:
- Processes received blocks from network
- Writes blocks to disk in order
- Handles block accounting and statistics updates
- Terminates when receiving a zero-block marker

#### Block Tracking (`got_block`, `dump_blockmap`)
Functions for checking whether a specific block has been received:
```c
int got_block(ttp_session_t* session, u_int32_t blocknr);
```

### Protocol Negotiation Routines

- `ttp_negotiate()` - Verifies protocol revision compatibility
- `ttp_authenticate()` - Performs shared secret authentication
- `ttp_open_transfer()` - Submits file request to server
- `ttp_open_port()` - Creates UDP socket and communicates port to server

### Network Socket Management

- `create_tcp_socket()` - Opens TCP control channel
- `create_udp_socket()` - Creates UDP data socket
- Platform-specific implementations: `create_tcp_socket_v4()`, `create_tcp_socket_v6()`, `create_udp_socket_v4()`, `create_udp_socket_v6()`

## Execution Flow Overview

```mermaid
graph TD
    A[main] --> B{User Input}
    B --> C[parse_command]
    C --> D[dispatch_command]
    D --> E[command_connect|command_get|command_dir|etc]
    E --> F[Network Operations]
    F --> G[Protocol Negotiation]
    G --> H[Data Transfer Initiated]
    H --> I[create_udp_socket]
    I --> J[disk_thread]
    J --> K[accept_block]
    K --> L[write_to_disk]
    L --> M[Statistics Updates]
    M --> N[Retransmit Requests]
    N --> O[Ring Buffer Management]
```

## Usage Patterns

### Connection Establishment
1. User invokes `connect` with optional host/port arguments
2. Module creates TCP socket using DNS resolution
3. Negotiates protocol version with server
4. Authenticates using shared secret
5. Returns initialized session object

### File Transfer (Single)
1. User invokes `get <remote_file>`
2. Module negotiates transfer parameters with server
3. Creates UDP socket for data reception
4. Starts `disk_thread` for background I/O
5. Receives blocks via UDP until complete or error

### Multiple File Transfer (`GET *`)
1. User invokes `get *`
2. Module requests all available files from server
3. For each file:
   - Negotiates transfer parameters
   - Sets up UDP socket
   - Processes blocks through `disk_thread`
4. Aggregates final statistics across all transfers

## Configuration Parameters

All configurable values are stored in the global `ttp_parameter_t` structure:

| Parameter | Default Value | Description |
|-----------|---------------|-------------|
| `server_name` | "localhost" | Hostname of Tsunami server |
| `server_port` | TS_TCP_PORT | TCP port for control connection |
| `client_port` | TS_UDP_PORT | UDP port for data connection |
| `block_size` | 1024 bytes | Size of individual file blocks |
| `target_rate` | 650 Mbps | Target bandwidth for transfers |
| `error_rate` | 7.5% | Maximum acceptable packet loss |
| `lossless` | true | Whether to request retransmissions for missing packets |
| `losswindow_ms` | 1000 ms | Time window for semi-lossless mode |

## Thread Safety and Concurrency

The module uses pthreads extensively, particularly:
- The `disk_thread` handles disk I/O operations concurrently with network reception
- Ring buffer management is thread-safe with proper locking mechanisms
- Statistics updates may require synchronization between threads

## Error Handling

All functions return integer status codes where:
- Zero indicates success
- Nonzero indicates failure that should be handled by caller

Common errors include:
- Network connectivity issues
- Authentication failures
- Disk write permissions
- Invalid command syntax
- Server-side transfer rejection

## Integration Points

This module integrates with:
- `tsunami-client.h`: Core client API definitions
- `ring.c`: Block ring buffer implementation
- `transcript.c`: Statistical logging functionality
- Platform-specific networking code: `network_v4.c`, `network_v6.c`

It depends on standard system libraries including:
- `pthread` for threading support
- `sys/socket.h` for BSD sockets
- `netdb.h` for DNS resolution
- `unistd.h` for Unix system calls