# RT Client Interface

# RT Client Interface

The RT Client Interface is a command-line interface client for the Tsunami file transfer protocol that supports real-time data streaming over TCP and UDP connections.

## Purpose

This module provides functionality for:
- Establishing network connections with Tsunami servers
- Managing file transfers using TTP (Tsunami Transfer Protocol)
- Handling disk I/O operations for received blocks
- Supporting IPv4/IPv6 socket handling
- Providing transcript logging capabilities

## Key Components

### Configuration Management (`config.c`)
Manages default parameters and session configuration:

```c
typedef struct {
    u_int32_t block_size;
    char *server_name;
    u_int16_t server_port;
    u_int16_t client_port;
    u_int32_t udp_buffer;
    u_char verbose_yn;
    u_char transcript_yn;
    u_char ipv6_yn;
    u_char output_mode;
    u_int32_t target_rate;
    u_char rate_adjust;
    u_int32_t error_rate;
    u_int16_t slower_num;
    u_int16_t slower_den;
    u_int16_t faster_num;
    u_int16_t faster_den;
    u_int16_t history;
    u_char lossless;
    u_int32_t losswindow_ms;
    u_char blockdump;
} ttp_parameter_t;
```

Functions:
- `reset_client()` - Resets all parameters to defaults
- Global constants defining default values for various settings

### Network Operations (`network.c`, `network_v4.c`, `network_v6.c`)

Handles socket creation and connection management:

#### TCP Socket Creation
```c
int create_tcp_socket(ttp_session_t *session,
                      const char *server_name, 
                      u_int16_t server_port);
```

#### UDP Socket Creation
```c
int create_udp_socket(ttp_parameter_t *parameter);
```

Supports both IPv4 and IPv6 through separate implementations:
- `create_tcp_socket_v4()`
- `create_tcp_socket_v6()`
- `create_udp_socket_v4()`
- `create_udp_socket_v6()`

### Disk I/O Operations (`io.c`)

Manages writing received blocks to disk:

```c
int accept_block(ttp_session_t *session,
                u_int32_t block_index, 
                u_char *block);
```

Includes support for VSIB real-time processing when enabled via `VSIB_REALTIME`.

### Protocol Implementation (`protocol.c`)

Implements core TTP protocol functions:

#### Authentication
```c
int ttp_authenticate(ttp_session_t *session, u_char *secret);
```

#### Negotiation
```c
int ttp_negotiate(ttp_session_t *session);
```

#### File Transfer Setup
```c
int ttp_open_transfer(ttp_session_t *session,
                       const char *remote_filename,
                       const char *local_filename);
```

#### Port Management
```c
int ttp_open_port(ttp_session_t *session);
```

#### Retransmission Handling
```c
int ttp_repeat_retransmit(ttp_session_t *session);
int ttp_request_retransmit(ttp_session_t *session, u_int32_t block);
int ttp_request_stop(ttp_session_t *session);
```

### Statistics Update (`protocol.c`)

Maintains transfer statistics with IIR filtering:

```c
int ttp_update_stats(ttp_session_t *session);
```

### Ring Buffer Management (`ring.c`)

Provides thread-safe ring buffer operations for data staging between network and disk threads:

```c
typedef struct {
    int count_data;
    int count_reserved;
    int base_data;
    int space_ready;
    int data_ready;
    // ... other fields
} ring_buffer_t;

// Thread-safe operations
u_char *ring_peek(ring_buffer_t *ring);
u_char *ring_reserve(ring_buffer_t *ring);
int ring_confirm(ring_buffer_t *ring);
int ring_cancel(ring_buffer_t *ring);
int ring_pop(ring_buffer_t *ring);
int ring_full(ring_buffer_t *ring);
```

### Transcript Logging (`transcript.c`)

Handles logging of transfer statistics to files:

```c
void xscript_open(ttp_session_t *session);
void xscript_data_log(ttp_session_t *session, const char *logline);
void xscript_close(ttp_session_t *session, u_int64_t delta);
```

## Execution Flow

The client operates in a command loop where it processes user commands. Key execution paths include:

1. **Connection establishment**: `create_tcp_socket()` → `getaddrinfo()`
2. **UDP port binding**: `create_udp_socket()` → attempts multiple ports until success
3. **File transfer initiation**: `ttp_open_transfer()` → sends filename request → receives parameters
4. **Data reception**: Network thread → `accept_block()` (via `ring_reserve`) → Disk thread → `accept_block()`
5. **Retransmission handling**: `ttp_repeat_retransmit()` → filters blocks that were received or are already pending
6. **Statistics update**: Periodically called by main loop to compute rates and error metrics

## Mermaid Diagram: Client Session Lifecycle

```mermaid
graph TD
    A[Client Start] --> B{User Command}
    B --> C[connect]
    C --> D[create_tcp_socket()]
    D --> E[TCP Connection Established]
    E --> F[create_udp_socket()]
    F --> G[UDP Port Created]
    G --> H[open_transfer]
    H --> I[Send Request]
    I --> J[Receive Response]
    J --> K[Open Local File]
    K --> L[Start Data Transfer]
    L --> M[Network Thread Receives UDP]
    M --> N[Disk Thread Writes Blocks]
    N --> O[Update Stats]
    O --> P{Transfer Complete?}
    P -->|Yes| Q[Close Session]
    P -->|No| R[Continue Transfer]

    subgraph "Session"
        E
        G
        H
        J
        K
        L
        M
        N
        O
        P
    end
```

## Compilation Flags

- `VSIB_REALTIME`: Enables VSIB real-time processing support
- `DEBUG_RETX`: Enables retransmission debug output
- `STATS_MATLABFORMAT`: Changes stats display format
- `RETX_REQBLOCK_SORTING`: Enables insertion sort for retransmit requests

## Error Handling

All functions return 0 on success, non-zero on failure. Critical errors call `error()` while recoverable issues use `warn()`. Memory allocation failures result in program termination.

## Threading Model

Uses pthreads for synchronization between network and disk threads:
- Mutexes protect ring buffer access
- Condition variables signal when data is ready or space is available
- Ring buffer operations (`ring_pop`, `ring_reserve`, etc.) are thread-safe