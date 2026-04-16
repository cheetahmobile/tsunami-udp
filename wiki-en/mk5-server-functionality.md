# MK5 Server Functionality

# MK5 Server Functionality

The **MK5 Server Functionality** module implements a Tsunami file transfer server tailored for Mark5 hardware, specifically supporting StreamStor disk I/O via the `xlrapi` library.

## Purpose

This module provides core functionality for handling client requests in a Tsunami-based file transfer system that operates on Mark5 hardware with StreamStor access. It supports real-time data streaming over UDP using a custom protocol (`TTP`) built upon standard Unix socket APIs. The server listens for TCP connections from clients, negotiates transfers, and sends out blocks of data as requested or retransmitted.

It is designed to be used alongside other modules such as `tsunami-server`, which defines shared types and utilities like logging functions, authentication routines, and socket creation helpers.

## Key Components

### Configuration Management (`config.c`)
- Defines default parameters for the server.
- Provides `reset_server()` function to initialize a `ttp_parameter_t` struct with defaults:
    - Block size: 32768 bytes
    - Secret: `"kitten"`
    - TCP port: `TS_TCP_PORT`
    - UDP buffer size: 20MB
    - Verbose mode enabled by default
    - Transcript mode disabled by default
    - IPv6 support disabled by default

### Disk I/O Routines (`io.c`)
- Implements `build_datagram()`: Reads a block from an open Mark5 file handle and constructs a TTP datagram containing the block data.
    - Uses `mk5_fseek()` and `mk5_fread()` internally.
    - Handles non-contiguous reads efficiently by seeking when necessary.

### Logging Mechanism (`log.c`)
- Provides `log()` function for timestamped log entries to a specified file descriptor.
    - Logs time-stamped messages including date/time information.

### Main Execution Loop (`main.c`)
- Entry point of the server process.
- Sets up listening socket and forks child processes per connection.
- Contains `client_handler()` which handles each connected client's session:
    - Negotiates protocol version and authenticates client.
    - Accepts new transfer requests.
    - Manages UDP transmission of data blocks.
    - Supports retransmission requests (e.g., REQUEST_RETRANSMIT).
    - Optionally logs transcript data during transfer.
- Includes signal handler (`reap`) to clean up zombie children.

### Protocol Handling (`protocol.c`)
- Implements core TTP protocol logic:
    - `ttp_negotiate()`: Ensures both ends use compatible protocol revisions.
    - `ttp_authenticate()`: Authenticates client using challenge-response mechanism involving MD5 hashes.
    - `ttp_open_transfer()`: Processes incoming filename request; opens file via `mk5_fopen64()` and returns metadata.
    - `ttp_accept_retransmit()`: Handles retransmission requests based on type:
        - REQUEST_ERROR_RATE: Adjusts inter-packet delay (IPD) dynamically.
        - REQUEST_RESTART: Resumes transfer at a given block index.
        - REQUEST_RETRANSMIT: Sends back a specific block upon demand.
    - `ttp_open_port()`: Creates a UDP socket for sending data blocks.

### Transcript Generation (`transcript.c`)
- Generates detailed transcripts of transfers in a structured format.
- Functions include:
    - `xscript_open()`: Initializes transcript file with header info.
    - `xscript_data_start()`, `xscript_data_stop()`: Marks start/end times.
    - `xscript_close()`: Finalizes transcript with summary stats.

### StreamStor API Wrapper (`mk5api.c`)
- Wraps low-level `xlrapi` functions into standard C library-style I/O operations:
    - `mk5_fopen64()`: Opens a StreamStor device or file.
    - `mk5_fclose()`: Closes a StreamStor file handle.
    - `mk5_fseek()`: Seeks within a StreamStor file.
    - `mk5_ftello()`: Returns current position in a StreamStor file.
    - `mk5_fread()`: Reads from a StreamStor file.

## Module Interactions

```mermaid
graph TD
    A[main] --> B[create_tcp_socket]
    A --> C[fork()]
    D[client_handler] --> E[ttp_negotiate]
    D --> F[ttp_authenticate]
    D --> G[ttp_open_transfer]
    G --> H[mk5_fopen64]
    D --> I[ttp_open_port]
    I --> J[create_udp_socket]
    D --> K[build_datagram]
    K --> L[mk5_fread]
    D --> M[xscript_open]
    M --> N[xscript_data_log]
    M --> O[xscript_close]
```

## Command-Line Interface

The server supports the following command-line options:

| Option              | Description |
|---------------------|-------------|
| `--verbose`         | Enable verbose output. |
| `--transcript`      | Enable transcript logging mode. |
| `--v6`              | Use IPv6 instead of IPv4. |
| `--port=N`          | Listen on TCP port N. |
| `--secret=S`        | Set shared secret to S. |
| `--datagram=B`       | Set datagram size to B bytes. |
| `--buffer=B`         | Set UDP send buffer size to B bytes. |

Additionally, it accepts a list of filenames that can be retrieved using the special GET request `"get *"`.

Example usage:
```bash
tsunamid --verbose --transcript --port=8080 --secret="my_secret" --datagram=32768 --buffer=10000000 /path/to/file1 /path/to/file2
```

This starts the server listening on port 8080, enabling verbose and transcript modes, setting a custom block size and UDP buffer size, with two files available for download via `get *`.

## Notes

- The module assumes Mark5 hardware is present (`XLRDeviceFind()`).
- It uses standard Unix socket APIs for communication over TCP and UDP.
- All data transfers are performed through `sendto()` on a UDP socket.
- File access is abstracted via `mk5api.c`, which interfaces directly with `xlrapi`.
- Transcripts are written to disk in `.tsus` format with metadata including timestamps, throughput, and error rates.