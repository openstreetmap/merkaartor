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

 - Working C++ compiler
 - Qt 5.15 or newer
 - Proj 6.x or newer
 - GDAL (2.0.0 or newer for GDAL exports)
 - Exiv2 (for geoimage support)
 - (For Windows Installer) NSIS-3
 - CMake 3.19.0 or newer

The OS specifics will be explained further down.

### Linux

Install the above packages using your package manager. For Debian/Ubuntu, this would
look like this:

```
 $ sudo apt-get install build-essential libgdal-dev libproj-dev libexiv2-dev cmake
```

Or for Qt5: 

```
 $ sudo apt-get install qt5-default libqt5xml5* libqt5network5* libqt5gui5* libqt5svg5* libqt5webkit5* libqt5quick5* qtdeclarative5-dev qttools5-dev qtbase5-dev qtchooser
```

### Windows (32bit/64bit)

In both cases, you will need to download MSYS2 for your architecture, and
install some packages from msys shell (msys2_shell.bat).

For 32bit, they are (the w64 is not a bug!):

```
$ pacman -S base-devel \
	msys/git \
	mingw32/mingw-w64-i686-gcc \
	mingw32/mingw-w64-i686-qt5 \
	mingw32/mingw-w64-i686-gdal \
	mingw32/mingw-w64-i686-proj \
	mingw32/mingw-w64-i686-openjpeg2 \
	mingw32/mingw-w64-i686-json-c \
	mingw64/mingw-w64-i686-exiv2 \
    mingw64/mingw-w64-i686-cmake
```

For 64bit, they are:

```
$ pacman -S base-devel \
	msys/git \
	mingw64/mingw-w64-x86_64-gcc \
	mingw64/mingw-w64-x86_64-qt5 \
	mingw64/mingw-w64-x86_64-gdal \
	mingw64/mingw-w64-x86_64-proj \
	mingw64/mingw-w64-x86_64-openjpeg2 \
	mingw64/mingw-w64-x86_64-json-c \
	mingw64/mingw-w64-x86_64-exiv2 \
    mingw64/mingw-w64-x86_64-cmake
```

Done? Continue to the next step, but run a different msys shell, the mingw32 or
mingw64, based on your architecture. Note that if you'll run mingw32 shell from
64bit msys installtion, strange stuff will happen, so don't do it.

### Mac OS X

You will need functional xcode (or other c++ compiler), and libraries installed
from [homebrew](http://brew.sh).

```
brew install gdal proj qt
```

If you want to use qt5, you need to force link it, as homebrew guys consider Qt4
the default:

```
brew install qt5 gdal proj exiv2 cmake
brew link --force qt5
```

You can do without linking, but you will need to manually supply some paths to
the build environment.

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
```

For example, compiling in support for GPSD would be:

```
cmake .. -DGPSD=ON
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

