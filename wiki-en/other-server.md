# Other — server

# Server Module Documentation

The `server` module implements the core functionality of the Tsunami daemon (`tsunamid`). It handles configuration loading, I/O operations, logging, network communication, protocol processing, and transcript management. This module serves as the central hub that orchestrates all major activities within the daemon process.

## Purpose

This module provides the foundational infrastructure for running the Tsunami daemon, including:
- Parsing command-line arguments and configuration files
- Managing system logs and output
- Handling network connections and data transfer
- Processing client-server protocols
- Maintaining session transcripts

## Key Components

### Main Entry Point
- **main.c**: Contains the entry point function `main()` which initializes the daemon, parses arguments, sets up signal handlers, and starts the main event loop.

### Configuration Management
- **config.c**: Implements functions for reading and parsing configuration from files or command line options. Handles daemon settings like port numbers, paths, etc.

### Input/Output Operations
- **io.c**: Manages standard input/output streams and file I/O operations used by the daemon during runtime.

### Logging System
- **log.c**: Provides centralized logging capabilities with support for different log levels and output destinations such as console or log files.

### Network Communication
- **network.c**: Implements socket creation, connection handling, and network-level message passing between clients and the server.

### Protocol Handling
- **protocol.c**: Processes incoming messages according to the defined protocol specification, routing them appropriately based on type and content.

### Transcript Management
- **transcript.c**: Records and manages session transcripts for debugging and auditing purposes, storing information about interactions between client and server.

## Build System Integration

### Autotools Makefile.am
```makefile
AM_CPPFLAGS = -I$(top_srcdir)/include
common_lib = $(top_builddir)/common/libtsunami_common.a
bin_PROGRAMS = tsunamid
tsunamid_SOURCES = config.c io.c log.c main.c network.c protocol.c transcript.c
tsunamid_LDADD = $(common_lib)
tsunamid_DEPENDENCIES = $(common_lib)
```

### Standalone Makefile
```makefile
SRC = config.c  io.c  log.c  main.c  network.c  protocol.c  transcript.c \
   ../common/common.c  ../common/error.c  ../common/md5.c
CFLAGS = -Wall -O3 -I../common/ -I../include/ -pthread -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
tsunamid: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o tsunamid
```

## External Dependencies

The server module depends on several components from other parts of the codebase:

1. **Common Library** (`libtsunami_common.a`): 
   - Shared utilities and error handling routines
   - Common data structures and helper functions

2. **Include Directory**:
   - Header files defining interfaces and constants used throughout the module

3. **System Libraries**:
   - POSIX threading library (pthread)
   - Standard C library functions

## Architecture Overview

This module is designed around a single executable program `tsunamid`. It follows a modular structure where each `.c` file encapsulates specific functionality related to system management, communication, and processing.

No complex execution flows are detected in this module's call graph, indicating that it operates primarily through straightforward procedural control flow without significant branching logic.

## Compilation Notes

When building using autotools, ensure that the common library path is correctly set via `AM_CPPFLAGS` and `tsunamid_LDADD`.

For standalone builds, all source files must be compiled with appropriate include paths and linking flags including `-pthread` for thread support.

## Usage

To run the Tsunami daemon:
```bash
./tsunamid [options]
```

Where options typically include configuration file paths or runtime parameters handled by the modules within this directory.