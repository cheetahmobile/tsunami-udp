# Other — tsunami.spec.in

# Tsunami UDP File Transport Module

## Overview

The tsunami module implements a fast UDP-based file transport protocol designed for high-speed data transfer in radio astronomy applications, particularly within Very Long Baseline Interferometry (VLBI) systems. It provides both client and server components for efficient network-based file transmission.

## Purpose

This module enables rapid data transfer between nodes in VLBI networks where low latency and high bandwidth are critical requirements. The implementation was originally developed at Indiana University and later enhanced by contributors from Metsahovi and NRAO.

## Key Components

### Executables
- `tsunami`: Client application for file transport
- `tsunamid`: Server daemon for file transport
- `rttsunami`: Real-time client for file transport
- `rttsunamid`: Real-time server daemon for file transport

### Build Process
The module uses standard RPM build macros:
- `%prep`: Extracts and prepares source files using `%setup`
- `%build`: Configures and compiles with `./configure` and `make`
- `%install`: Installs built binaries using `make install`

## Architecture

```
┌─────────────┐    ┌─────────────┐
│   rttsunami  │    │   rttsunamid  │
└─────────────┘    └─────────────┘
       │                  │
       └──────────────────┘
              ▼
        ┌─────────────┐
        │   Tsunami     │
        │   Protocol    │
        └─────────────┘
              ▼
        ┌─────────────┐
        │   UDP Layer  │
        └─────────────┘
```

## Implementation Details

The tsunami protocol operates over UDP, leveraging its speed advantages for real-time astronomical data streaming. The module supports both regular and real-time variants:

1. **Regular tsunami**: Standard UDP-based transport
2. **Real-time tsunami**: Enhanced version optimized for time-critical applications

Both implementations use the same underlying transport mechanism but differ in their handling of timing constraints and buffer management.

## Integration Points

The module appears to be self-contained without internal dependencies or external call relationships. It integrates into larger VLBI systems through its command-line interfaces which can be invoked directly or used as part of automated workflows.

## Usage Context

This module is primarily intended for use in VLBI data processing pipelines where large amounts of observational data need to be transferred quickly between stations. The real-time variants suggest it's also suitable for applications requiring strict timing guarantees during data acquisition.

## Version History

- **0.92wb** (Sat Jul 20 2013): Initial version by Walter Brisken
- **Merged version** (Thu Aug 15 2013): Integration of Walter's changes by Jan Wagner