# # Universal cross-platform C99/C++ multithreading library.

<!-- ========================================================================================================================================================== -->

## Overview
This library provides universal platform-independent C99 based API for multithreading applications/libraries, include atomics, mutexes & threads.

## Thread-safe
Of course this library and its API is fully thread-safe.

<!-- ========================================================================================================================================================== -->

## Building the library

### Windows

#### MSVC
Requirements: `Visual Studio`, `CMake`

1. Clone this repo and open it **folder** in Visual Studio.
2. Visual Studio must generate MSBuild script through CMake automaticly. Just wait for it.
3. Build library and tests through Viusal Studio GUI, using hotkey (Ctrl + Shift + B by default) or terminal.

#### MinGW UCRT64
Requirements: `MSYS2 UCRT64`, `MinGW UCRT64`, `make` (`mingw32-make`), `CMake`.

1. Run **MSYS2 UCRT64 environment** (<ins>**recommended**</ins>). Install all requirements if you haven't it.
2. Clone this repo and go to repository root folder.
3. Create folder `build`. Go to this catalog.
4. Run `cmake -S .. -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX=$MSYSTEM_PREFIX`. Wait for generating Makefile.
5. Run `mingw32-make` and wait for building library.

### Linux (UNIX)
Requirements: `GCC`, `make`, `CMake`

1. Install all requirements if you haven't it.
2. Clone this repo and go to repository root folder.
3. Create folder `build`. Go to this catalog.
4. Run `cmake -S ..`. Wait for generating Makefile.
5. Run `make` and wait for building library.

<!-- ========================================================================================================================================================== -->

## Installation

### Windows

#### MinGW UCRT64
Just run `mingw32-make install`. Library will be installed to your MSYS2 system path (specified in `-DCMAKE_INSTALL_PATH` CMake option; `/ucrt64` in this example).

### Linux (UNIX)
Also just run `sudo make install`. Library will be installed to your system.

<!-- ========================================================================================================================================================== -->

## Usage in project

### Manual linking
> [!WARNING]
> Manual linking is not recommended library usage way. Please use CMake for correct linking with library.

Include `<libnthread.h>` header where you need to use this library. That link your executable with library by adding flag to command line `-lnthread`. On Linux (POSIX) you already need to link you executable with `pthread` library, so just add flag `-lpthread` command line. If you use static version of this library, please specify definition `LIBNTHREAD_STATIC` when you compile file, where included <ins>libnthread</ins> header.

### CMake
If you using CMake in your project, add `libnthread` by same method or with using `find_package`:

```cmake
find_package(libnthread REQUIRED)
target_link_libraries(<target> PRIVATE libnthread::nthread)
```

This will fully automatily setup <ins>libnthread</ins> for your project. By default CMake will use static version of <ins>libnthread</ins>, so if you want to use dynamic version of this library, set `BUILD_SHARED_LIBS` flag in your CMake config command prompt to `ON`.

## Debugging & Troubleshooting

### Got `NError_InternalUnknownError`
Rebuild library with CMake `-DDEBUG=ON` flag, rebuild & rerun you program to again got this error. You will see message in <ins>stderr</ins> like *Unhandled system error...*. Copy this message & open an issue on this repo.
