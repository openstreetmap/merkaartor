# Compiling Merkaartor

These are the instructions for compiling Merkaartor from git source. Here is the
short version, given you have all the dependencies and are familiar with your
build environment:

```
$ git clone https://github.com/openstreetmap/merkaartor.git && cd merkaartor
$ mkdir build && cd build && cmake ..
$ make -j8
```

And run it using:

```
$ ./merkaartor
```

If this is not enough, here are more detailed instructions:

## Prerequisites

You will need the following packages installed:

 - Working C++ compiler (C++17)
 - Qt 6 (the codebase still compiles against Qt 5.15+, but only Qt 6 is CI-tested)
 - Proj 6.x or newer
 - GDAL 2.0.0 or newer
 - Exiv2 (for geoimage support)
 - protobuf (for .osm.pbf support)
 - CMake 3.19.0 or newer
 - (For Windows Installer) NSIS-3

The OS specifics are explained below. The package lists mirror what the CI uses
(see `.github/workflows/build.yml`); keep them in sync if you change one.

### Linux

For Debian/Ubuntu (22.04, 24.04):

```
 $ sudo apt-get install build-essential cmake git protobuf-compiler \
     libgdal-dev libproj-dev libexiv2-dev libgl1-mesa-dev \
     qt6-base-dev qt6-base-dev-tools qt6-tools-dev qt6-tools-dev-tools qt6-l10n-tools \
     libqt6svg6-dev libqt6networkauth6-dev libqt6core5compat6-dev
```

### Windows (64-bit)

Install [MSYS2](https://www.msys2.org/) and use the **UCRT64** environment
(`ucrt64.exe`, not the deprecated `mingw64.exe`). Install the packages:

```
$ pacman -S git msys/make msys/git \
    mingw-w64-ucrt-x86_64-toolchain \
    mingw-w64-ucrt-x86_64-python-pip \
    ucrt64/mingw-w64-ucrt-x86_64-gcc \
    ucrt64/mingw-w64-ucrt-x86_64-qt6 \
    ucrt64/mingw-w64-ucrt-x86_64-gdal \
    ucrt64/mingw-w64-ucrt-x86_64-proj \
    ucrt64/mingw-w64-ucrt-x86_64-openjpeg2 \
    ucrt64/mingw-w64-ucrt-x86_64-json-c \
    ucrt64/mingw-w64-ucrt-x86_64-cmake \
    ucrt64/mingw-w64-ucrt-x86_64-exiv2 \
    ucrt64/mingw-w64-ucrt-x86_64-nsis \
    ucrt64/mingw-w64-ucrt-x86_64-mesa \
    ucrt64/mingw-w64-ucrt-x86_64-protobuf
```

Then continue the build steps below from the UCRT64 shell.

### Mac OS X

You will need a working Xcode (or other C++ compiler) and libraries installed
from [homebrew](http://brew.sh):

```
$ brew install gdal proj qt exiv2 cmake protobuf pkg-config
```

Homebrew's `qt` formula provides Qt 6.

## Compilation

### Clone the repository

```
$ git clone https://github.com/openstreetmap/merkaartor.git && cd merkaartor
```

>  By default, this is the latest development version. If you want to use an older
>  one, you have to checkout the commit/tag:
>  
>  $ git checkout 0.17.0
>  
>  Please, use this only for testing (for example, if some feature does not
>  work, but it did in older release and you want to check which one), never
>  report bugs for older versions.

### Run cmake

```
$ mkdir build && cd build && cmake ..
$ make -jX
```

There are a few build options that can be passed to cmake to configure features
compiled-in. See the cmake output for a full list (or use CMake GUI to
configure it):

```
-- Build options (use -DOPT=ON/OFF to enable/disable):
--  * ZBAR        OFF
--  * GEOIMAGE    ON
--  * GPSD        OFF
--  * WEBENGINE   OFF
--  * PROTOBUF    ON
```

For example, compiling in support for GPSD would be:

```
cmake .. -DGPSD=ON
```

#### Choosing Qt version

To build with specific Qt version, use `CMAKE_PREFIX_PATH` to the Qt6 cmake directory -- that is the directory containing QtXConfig.cmake file:

```
$ ls /usr/lib/cmake/Qt6/Qt6Config.cmake
$ cmake .. -DCMAKE_PREFIX_PATH=/usr/lib/cmake/Qt6/
```

### Run qmake

No longer supported.

### Done!

If you are lucky, you'll find an executable in the binaries subdirectory.
Otherwise, check our [github page](http://github.com/openstreetmap/merkaartor), especially check the issues and possibly
report yours.

## Packaging

The project uses CPack to generate binary packages. In the build directory, invoke the `package` build target:

```
$ make package
```

