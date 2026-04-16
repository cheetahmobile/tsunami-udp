# Other — sys

# win32compat/sys Module Documentation

## Overview

The `win32compat/sys` module provides compatibility definitions for system-level headers required by POSIX-compliant applications when running under Windows. It includes redefinitions or extensions of standard types, macros, and structures from `<sys/socket.h>`, `<sys/time.h>`, and `<sys/types.h>` that are not natively available in the Windows environment.

This module is part of a larger compatibility layer designed to bridge differences between Unix-like systems and Microsoft Windows platforms, particularly for network programming interfaces such as those defined in RFCs related to IP multicast and IPv6 support.

## Key Files

### `socket.h`
Provides forward declarations for socket-related headers (`<winsock2.h>` and `<ws2tcpip.h>`), ensuring proper inclusion order without conflicts with older WinSock APIs.

```c
#include <winsock2.h>
#include <sys/ws2tcpip.h>
```

### `time.h`
Defines a minimal `time_t` type using `long`.

```c
typedef long time_t;
```

### `types.h`
Contains exact-width integer type definitions compatible across compilers (`_MSC_VER` vs GNU C). Also defines additional types like `_ssize_t`, `off64_t`, etc., which are used throughout the system API.

#### Type Definitions

| Type         | Description                          |
|--------------|--------------------------------------|
| int8_t       | Signed 8-bit integer                 |
| u_int8_t     | Unsigned 8-bit integer               |
| u_char       | Unsigned character                   |
| int16_t      | Signed 16-bit integer                  |
| u_int16_t    | Unsigned 16-bit integer                |
| int32_t      | Signed 32-bit integer                  |
| u_int32_t    | Unsigned 32-bit integer                |
| int64_t      | Signed 64-bit integer                  |
| u_int64_t    | Unsigned 64-bit integer              |

Additionally:
- `time_t`: Defined as `long`.
- `ssize_t`: Alias for `_ssize_t`.
- `off64_t`: Defined as `u_int64_t`.
- `fpos64_t`: Defined as `u_int64_t`.

### `ws2tcpip.h`
Includes TCP/IP-specific extensions from Windows Sockets 2 (WinSock2) including:

- **IP Multicast Options**: Macros like `IP_MULTICAST_IF`, `IP_ADD_MEMBERSHIP`, and `IP_DROP_MEMBERSHIP`.
- **IPv6 Support Structures**:
  - `in6_addr`: Union-based structure representing an IPv6 address.
  - `sockaddr_in6`: Structure for storing IPv6 socket addresses.
- **Address Testing Macros**:
  - `IN6_IS_ADDR_UNSPECIFIED()`
  - `IN6_IS_ADDR_LOOPBACK()`
  - `IN6_IS_ADDR_LINKLOCAL()`
  - And others for checking various IPv6 address properties.
- **Network Information Functions**:
  - `getnameinfo()` – Converts a socket address to a corresponding host and service name.
  - `getaddrinfo()` / `freeaddrinfo()` – Resolves hostnames into IP addresses.
- **Error Handling Constants**:
  - `EAI_AGAIN`, `EAI_BADFLAGS`, `EAI_FAIL`, etc. — mapped error codes from WinSock.

## Purpose and Usage

This module serves primarily as a compatibility shim between Unix-style networking APIs and their equivalents in Windows. It allows developers using libraries or code that assumes POSIX headers to compile and run correctly on Windows platforms by providing necessary definitions without requiring full emulation of the entire POSIX stack.

It is intended to be included directly via standard include directives, e.g.,

```c
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
```

These files are typically used alongside other components such as `win32compat/net` or `win32compat/unistd` to provide complete network I/O support under Windows.

## Integration Points

The module integrates with several parts of the system:

- **Networking Layer (`net`)**: Used by functions like `socket()`, `bind()`, `connect()` when interfacing with low-level sockets.
- **System Calls (`unistd`)**: Provides types needed for system call interfaces involving file offsets and sizes.
- **Windows Sockets API (`<winsock2.h>` & `<ws2tcpip.h>`)**: This module acts as a wrapper around native Windows socket functionality, exposing it through familiar POSIX-like names.

No internal calls or execution flows exist within this module; its role is purely header definition and inclusion management.

## Example Use Case

A typical usage scenario involves defining a custom struct for handling both IPv4 and IPv6 addresses transparently:

```c
struct addr_info {
    int family;
    union {
        struct sockaddr_in v4;
        struct sockaddr_in6 v6;
    } addr;
};
```

Then using `getaddrinfo()` to resolve domain names into these structures, which can then be passed to higher-level functions expecting generic socket addresses.

## Notes

Some macros and constants defined here may conflict with those already present in older versions of `<winsock.h>`. The inclusion order must ensure that `<winsock2.h>` is included before including any headers that depend on it (such as `ws2tcpip.h`).