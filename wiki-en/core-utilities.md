# Core Utilities

# Core Utilities Module

The **Core Utilities** module provides shared functionality used across both the Tsunami client and server implementations. It includes essential routines such as random data generation, time handling utilities, MD5 digest computation, and cross-platform compatibility functions.

## Purpose

This module serves as a foundational component that encapsulates common operations required by multiple parts of the Tsunami system. Its primary goal is to abstract platform-specific behaviors and provide consistent interfaces for core tasks like:

- Generating cryptographically secure random data.
- Handling time measurements with microsecond precision.
- Computing MD5 message digests.
- Reading lines from file descriptors without buffering.
- Implementing precise sleep functionality.

These utilities are designed to be portable between Unix-like systems and Windows platforms while maintaining performance and correctness.

## Key Components

### Global Constants

```c
const u_int32_t PROTOCOL_REVISION = 0x20061025; // yyyymmdd
const u_int16_t REQUEST_RETRANSMIT = 0;
const u_int16_t REQUEST_RESTART    = 1;
const u_int16_t REQUEST_STOP       = 2;
const u_int16_t REQUEST_ERROR_RATE = 3;
```

Global constants define protocol version and request types used in communication between client and server modules.

### Random Data Generation

#### `int get_random_data(u_char *buffer, size_t bytes)`

Attempts to read the specified number of bytes of random data from `/dev/urandom` into the provided buffer. Returns:
- `0` on success.
- `-1` on failure.

> On Win32, uses `CryptGenRandom()` instead.

### Time Measurement & Conversion

#### `u_int64_t get_usec_since(struct timeval *old_time)`

Returns the number of microseconds elapsed since the given timestamp (`old_time`). This function handles potential overflow correctly when computing differences over seconds.

#### `u_int64_t htonll(u_int64_t value)` / `u_int64_t ntohll(u_int64_t value)`

Converts a 64-bit integer between host byte order and network byte order. The conversion logic detects endianness at runtime using `htons(5)`.

#### `void usleep_that_works(u_int64_t usec)`

Sleeps for the specified number of microseconds with higher accuracy than standard `usleep()`. Uses real sleep for large intervals (up to 10ms granularity), then busy-waits for remaining time.

> On Win32, this implementation uses `timeBeginPeriod`, `Sleep`, and `QueryPerformanceCounter`.

### Transcript Filename Construction

#### `char *make_transcript_filename(char *buffer, time_t epoch, const char *extension)`

Constructs a filename string based on UTC time components in format:  
`YYYY-MM-DD-HH-MM-SS.extension`

The buffer must have sufficient space to hold the resulting string including null terminator.

### Proof Preparation

#### `u_char *prepare_proof(u_char *buffer, size_t bytes, const u_char *secret, u_char *digest)`

Prepares an MD5 hash as proof that both parties know the same shared secret:

1. XORs the secret repeatedly over the input buffer.
2. Computes the MD5 digest of the modified buffer.
3. Stores result in `digest`.
4. Returns pointer to `digest`.

This is used during authentication phases where secrets are verified without transmitting them directly.

### Line Reading Utilities

#### `int read_line(int fd, char *buffer, size_t buffer_length)`

Reads a newline-terminated line from a file descriptor into a buffer. No internal buffering is performed.

#### `int fread_line(FILE *f, char *buffer, size_t buffer_length)`

Same functionality as above but operates on FILE pointers instead of raw file descriptors.

Both functions return:
- `0` on successful read.
- Negative error code if reading fails or exceeds buffer length.

### UDP Error Counter Access

#### `u_int64_t get_udp_in_errors()`

Attempts to retrieve the current UDP Input Error counter from `/proc/net/snmp` on Linux systems. For non-Linux platforms, returns zero.

## Architecture Overview

```mermaid
graph TD
    A[Client/Server Core] --> B[Random Data]
    A --> C[Time Handling]
    A --> D[MD5 Digest]
    A --> E[Line Reader]

    B --> F[/dev/urandom or CryptGenRandom]
    C --> G[gettimeofday()]
    C --> H[get_usec_since()]
    C --> I[ntohll(), htonll()]

    D --> J[md5_digest()]
    D --> K[md5_finish()]
    D --> L[md5_append()]
    D --> M[md5_process()]

    E --> N[read_line()]
    E --> O[fread_line()]
```

## Integration Points

These utilities are widely integrated throughout the Tsunami system:

| Module | Usage |
|--------|-------|
| `common.c` / `common_win32.c` | Used for basic operations like random data and time measurement |
| `error.c` | Provides logging infrastructure via `warn()` calls |
| `md5.c` | Implements core cryptographic hashing logic |
| `server/main.c`, `client/main.c` | Invoked by main loop for timing-related tasks |
| `rtclient/command.c`, `rtserver/protocol.c` | Called when processing commands or protocol updates |

The module ensures consistent behavior across different environments (Unix vs Win32), abstracting away platform-specific quirks while providing high-performance implementations for critical functions such as time tracking and secure randomness generation.