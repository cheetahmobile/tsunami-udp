# Other — common

# Common Module Documentation

## Purpose

The `common` module provides foundational utilities and shared functionality used throughout the Tsunami software package. It contains essential functions for error handling, cryptographic operations (MD5), and basic system interactions that are commonly needed across different components of the application.

## Key Components

### Files
- `md5.c`: Implementation of MD5 hash algorithm functions
- `common.c`: General utility functions and helper routines  
- `error.c`: Error handling and reporting mechanisms

### Library
This module compiles into a static library `libtsunami_common.a` which can be linked with other modules requiring these common functionalities.

## Interface Functions

### From md5.c:
```c
void tsunami_md5(const unsigned char *input, size_t length, unsigned char *output);
```
Computes MD5 hash of input data and stores result in output buffer.

### From common.c:
```c
int tsunami_get_system_info(char *buffer, size_t buflen);
int tsunami_log_message(const char *format, ...);
```

### From error.c:
```c
void tsunami_error_report(int error_code, const char *message);
int tsunami_error_get_last(void);
```

## Build Configuration

### Makefile.am Settings
- **Library**: Creates `libtsunami_common.a` static library
- **Include Path**: Adds `-I$(top_srcdir)/include` for header file access
- **Sources**: Includes all three source files (`md5.c`, `common.c`, `error.c`)
- **Endian Handling**: Commented-out flag `AM_CFLAGS = -DARCH_IS_BIG_ENDIAN` for PlayStation3 compatibility

## Architecture Overview

The common module serves as a foundation layer providing:

1. Cryptographic hashing capabilities through MD5 implementation
2. System information gathering utilities 
3. Standardized error handling and logging interface

These functions are designed to be lightweight and portable while offering reliable core functionality that other modules depend upon.

```
┌─────────────┐
│   md5.c     │
├─────────────┤
│  MD5 Hashing│
└─────────────┘
       │
       ▼
┌─────────────┐
│  common.c     │
├─────────────┤
│  Utilities    │
└─────────────┘
       │
       ▼
┌─────────────┐
│  error.c    │
├─────────────┤
│  Error Hndlg │
└─────────────┘
```