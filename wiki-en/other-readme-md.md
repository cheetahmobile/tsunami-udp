# Other — README.md

# tsunami-udp Module Documentation

## Purpose

The `tsunami-udp` module represents a UDP-based network communication system originally cloned from http://tsunami-udp.sourceforge.net/. It provides both server and client implementations for establishing UDP connections and performing file operations over a network.

## Overview

This is a simple network protocol implementation that allows clients to connect to a server and perform basic file operations using UDP as the transport layer. The system supports operations such as getting files, listing directories, setting configuration parameters, and managing connections.

## Key Components

### Server Implementation
- **Location**: `./server/tsunamid`
- **Functionality**: Provides UDP server capabilities for handling client requests
- **Usage**: `./server/tsunamid [options]`

### Client Implementation
- **Location**: `./client/tsunami`
- **Functionality**: Implements client-side command interface for interacting with the server
- **Commands Supported**:
  - `close` - Close connection
  - `connect` - Establish connection to server
  - `get` - Retrieve file from server
  - `dir` - List directory contents on server
  - `help` - Display available commands
  - `quit` - Terminate client session
  - `set` - Configure client settings

## Architecture

```
┌─────────────┐    ┌──────────────┐
│   Client      │    │    Server      │
│  (client)     │    │   (server)    │
└─────────────┘    └──────────────┘
       │                 │
       ├─connect→        │
       │                 │
       ├─get→            │
       │                 │
       ├─dir→            │
       │                 │
       ├─set→            │
       │                 │
       └←response─        │
```

## Usage Examples

### Basic Setup
```bash
# Compile the entire project
./recompile.sh

# Start server
./server/tsunamid *

# Run client in interactive mode
./client/tsunami
```

### Command Line Operations
```bash
# Connect to server and retrieve a file
./client/tsunami connect your.server.add set udpport 51031 get path/to/yourfile quit

# Get help information
./server/tsunamid --help
./client/tsunami help
```

## Configuration

The client accepts various configuration options through the `set` command. The primary configurable parameter is the UDP port (`udpport`) which can be specified during connection setup.

## Execution Flow

No execution flows were detected for this module. This documentation serves as an overview of the module's purpose and structure rather than detailed code analysis.

## Build Process

The build process relies on the provided `recompile.sh` script which handles compilation of both server and client components.

## Limitations

Based on the TODO section, one limitation is that the system lacks "Interactive difference" functionality, suggesting it doesn't provide real-time or interactive comparison features between different versions or states of files.