# RT Server Components

# RT Server Components

The **RT Server Components** module implements a real-time file transfer server designed for high-performance data delivery with precise timing control. It supports both standard file transfers and specialized real-time operations using VSIB (VLBI Storage Interface Board) hardware.

## Purpose

This module provides:
- Real-time file transfer capabilities via Tsunami Transfer Protocol (TTP)
- Support for VSIB hardware-based real-time data streaming
- Configurable parameters for block size, buffer sizes, timeouts, etc.
- Logging and transcript generation for performance analysis
- EVN filename parsing utilities for real-time scheduling

It is used in conjunction with `tsunami-client` to provide synchronized, low-latency file transfers suitable for VLBI and other time-sensitive applications.

## Key Files & Components

### Configuration (`config.c`)
Manages default configuration values and parameter reset logic:

```c
const u_int32_t DEFAULT_BLOCK_SIZE = 1024;
const u_char *DEFAULT_SECRET = (u_char*)"kitten";
const u_int16_t DEFAULT_TCP_PORT = TS_TCP_PORT;
const u_int32_t DEFAULT_UDP_BUFFER = 20000000;
const u_char DEFAULT_VERBOSE_YN = 1;
const u_char DEFAULT_TRANSCRIPT_YN = 0;
const u_char DEFAULT_IPV6_YN = 0;
const u_int16_t DEFAULT_HEARTBEAT_TIMEOUT = 15;

void reset_server(ttp_parameter_t *parameter);
```

### Disk I/O (`io.c`)
Handles building datagrams from files or VSIB boards:

```c
int build_datagram(ttp_session_t *session, u_int32_t block_index,
                   u_int16_t block_type, u_char *datagram);

// Optional MODE_34TH support for packing 3-of-4 byte reads
```

### Logging (`log.c`)
Provides timestamped logging functionality:

```c
void log(FILE *log_file, const char *format, ...);
```

### Main Loop (`main.c`)
Entry point that handles client connections and session management:

```c
int main(int argc, char *argv[]);
void client_handler(ttp_session_t *session);
void process_options(int argc, char *argv[], ttp_parameter_t *param);
```

### Network Setup (`network.c`)
Socket creation and configuration routines:

```c
int create_tcp_socket(ttp_parameter_t *param);
int create_udp_socket(ttp_parameter_t *param);
```

### EVN Filename Parsing (`parse_evn_filename.c`)
Parses EVN-style filenames containing start times and auxiliary information:

```c
struct evn_filename* parse_evn_filename(char *filename);
char* get_aux_entry(char *key, char **auxinfo, int nr_auxinfo);
double interpret_as_utc(...);
```

### Protocol Handling (`protocol.c`)
Implements TTP protocol negotiation, authentication, and retransmission handling:

```c
int ttp_negotiate(ttp_session_t *session);
int ttp_authenticate(ttp_session_t *session, const u_char *secret);
int ttp_accept_retransmit(ttp_session_t *session, retransmission_t *retrans, u_char *datagram);
int ttp_open_port(ttp_session_t *session);
int ttp_open_transfer(ttp_session_t *session);
```

### Transcript Generation (`transcript.c`)

Generates performance transcripts of transfers:

```c
void xscript_open(ttp_session_t *session);
void xscript_close(ttp_session_t *session, u_int64_t delta);
void xscript_data_start(ttp_session_t *session, const struct timeval *epoch);
void xscript_data_stop(ttp_session_t *session, const struct timeval *epoch);
void xscript_data_log(ttp_session_t *session, const char *logline);
```

## Execution Flow

The server starts by initializing parameters via `reset_server()` and parsing command-line options in `process_options()`. It then creates a TCP socket using `create_tcp_socket()`.

When a new connection is accepted:
1. A new session is created.
2. The client authenticates with `ttp_authenticate()`.
3. Transfer parameters are negotiated through `ttp_negotiate()` and `ttp_open_transfer()`.
4. Data transfer begins via `build_datagram()` and `sendto()` calls on UDP sockets.
5. Retransmissions are handled by `ttp_accept_retransmit()` which may trigger `build_datagram()` again for the same block.
6. Transcripts are generated during transfer if enabled.

For VSIB real-time mode (when compiled with `-DVSIB_REALTIME`):
- Uses `/dev/vsib` to read data blocks directly from hardware.
- Parses EVN filename formats to determine start time and metadata.
- Starts/stop vsib board operations via `start_vsib()` / `stop_vsib()` functions.
- Optionally writes local disk copies of received data when `fileout` flag is set.

## Mermaid Diagram: Client Session Lifecycle

```mermaid
graph TD
    A[Main Loop] --> B{New Connection}
    B --> C[Create Session]
    C --> D[Accept Auth]
    D --> E[Authenticate Client]
    E --> F[Negotiate Parameters]
    F --> G[Open Transfer]
    G --> H[Set Up UDP Port]
    H --> I[Send File Blocks]
    I --> J{Retransmit Request?}
    J --> K[Handle Retransmit]
    K --> L[Build Datagram]
    L --> M[Send Block]
    M --> N[Monitor Heartbeat]
    N --> O{Timeout?}
    O --> P[Close Session]
    P --> Q[Return to Main Loop]
    
    subgraph "VSIB Mode"
        R[start_vsib]
        S[read_vsib_block]
        T[stop_vsib]
    end
    
    R --> S
    S --> T
    T --> U[close vsib file]
    U --> V[close local copy]
end
```

## Configuration Options

| Option | Description |
|------------|----------------|
| `--verbose`, `-v` | Enable verbose output |
| `--transcript`, `-t` | Enable transcript logging |
| `--v6`, `-6` | Use IPv6 instead of IPv4 |
| `--port=N`, `-p N` | Listen port number |
| `--secret=STR`, `-s STR` | Shared secret string |
| `--buffer=N`, `-b N` | UDP buffer size in bytes |
| `--hbtimeout=N`, `-h N` | Heartbeat timeout in seconds |

When compiling with `-DVSIB_REALTIME`:
| Option | Description |
|------------|----------------|
| `--vsibmode=N`, `-M N` | Set VSIB operating mode |
| `--vsibskip=N`, `-S N` | Skip N samples after each sample |

## Integration Points

This module integrates with the following components:

1. **Client-side**: Communicates using Tsunami Transfer Protocol (TTP) over TCP for negotiation, MD5 authentication, and UDP for data transfer.
2. **Common Headers**: Uses `tsunami-server.h` for shared types and constants.
3. **VSIB Driver**: When compiled with `-DVSIB_REALTIME`, interacts directly with `/dev/vsib`.
4. **EVN Parser**: Parses EVN-style filenames when real-time mode is enabled.

The server supports both single-user and multi-user modes depending on compile flags (`-DVSIB_REALTIME`). In real-time mode, it uses a single-threaded approach without forking child processes.