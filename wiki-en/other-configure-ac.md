# Other — configure.ac

# configure.ac

The `configure.ac` is an Autoconf input file used to generate the `configure` script for the Tsunami software package. This script automates the process of detecting system features, setting compilation options, and generating configuration files necessary for building the project.

## Purpose

This module ensures that the build environment meets the requirements for compiling Tsunami. It checks for:
- Required tools like GCC, ranlib
- Required libraries such as pthread
- Compilation flags and settings
It also sets up variables used in other parts of the build system, including those needed by packaging tools.

## Key Components

### Initialization

```autoconf
AC_INIT([tsunami], [1.1b43])
echo "Configuring Tsunami version AC_PACKAGE_VERSION"
```

Initializes the Autoconf environment with the package name (`tsunami`) and version (`1.1b43`). The version string is automatically substituted using `AC_PACKAGE_VERSION`.

### Automake Integration

```autoconf
AM_INIT_AUTOMAKE([foreign])
AM_CONFIG_HEADER(config.h:config.in)
```

Enables integration with Automake, which generates Makefiles from `Makefile.am`. Also specifies that `config.h` should be generated from `config.in`.

### Host Detection

```autoconf
AC_CANONICAL_HOST
```

Detects the canonical host type for cross-compilation support.

### Compiler Checks

```autoconf
CFLAGS_save="$CFLAGS"
AC_PROG_CC
if test "$GCC" != "yes"; then
    AC_MSG_WARN([*** WARNING! *** You are not using the GNU C compiler.])
fi
AC_PROG_RANLIB
CFLAGS="$CFLAGS_save"
unset CFLAGS_save
```

Ensures that the GNU C compiler (`gcc`) is being used, issuing warnings otherwise. Saves and restores original `CFLAGS` to avoid overwriting them during tool detection.

### Compilation Flags

```autoconf
EXTRA_CFLAGS="-O3 -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64"
```

Sets optimization level (`-O3`) and defines required preprocessor macros for proper compilation on POSIX systems.

### Debug Flag Handling

```autoconf
#AC_ARG_ENABLE([debug], ...)
#...
EXTRA_CFLAGS="-g -Wall $EXTRA_CFLAGS"
```

Uncommented code shows how debug flags can be added via command-line option `--enable-debug`, enabling debugging symbols and warnings.

### Library Dependencies

```autoconf
AC_CHECK_LIB(pthread, main, [HAPPY=1], [HAPPY=0])
if test "$HAPPY" = "0"; then
    AC_MSG_WARN([Could not find the pthread library])
    AC_MSG_ERROR([Cannot continue])
fi
```

Verifies that the `pthread` library is available. If not found, it issues a warning and exits with an error.

### Version Substitution

```autoconf
version=AC_PACKAGE_VERSION
AC_SUBST(version)
```

Exports the current package version as a variable usable in other build scripts or spec files.

## Output Files

The script generates several output files based on its configuration:

```autoconf
AC_OUTPUT([
    Makefile
    tsunami.spec
    client/Makefile
    rtclient/Makefile
    common/Makefile
    include/Makefile
    server/Makefile
    rtserver/Makefile
    util/Makefile
])
```

These include:
- Main `Makefile`
- Platform-specific `Makefile`s (e.g., `client/Makefile`)
- Package metadata file `tsunami.spec` for RPM packaging

## Execution Flow

This module does not have internal execution flows; it serves as input to Autoconf which processes it into a shell script. The flow of operations is linear and deterministic, involving checks and substitutions performed by Autoconf's macro system.

## Diagram

No diagram needed — this module is purely declarative and procedural, defining what should be checked and generated without complex architectural relationships.