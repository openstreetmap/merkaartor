# Compiling Merkaartor

These are the instructions for compiling Merkaartor from git source. Here is the
short version, given you have all the dependencies and are familiar with your
build environment:

```
$ git clone https://github.com/openstreetmap/merkaartor.git && cd merkaartor
$ qmake -r
$ make -j8 release
```

And run it using:

```
$ binaries/bin/merkaartor
```

If this is not enough, here are more detailed instructions:

## Prerequisites

You will need the following packages installed:

 - Working C++ compiler
 - Qt 4.x (4.4.0 or newer) or Qt 5.x (5.3.1 or later)
 - Proj.4
 - GDAL (2.0.0 or newer for GDAL exports)
 - Exiv2 (for geoimage support)
 - (For Windows Installer) NSIS-3

The OS specifics will be explained further down.

### Linux

Install the above packages using your package manager. For Debian/Ubuntu, this would
look like this:

```
 $ sudo apt-get install build-essential libgdal-dev libproj-dev
```

For Qt4:

```
 $ sudo apt-get install qt4-default libqt4-xml libqt4-network libqt4-gui libqt4-svg libqt4-webkit libqt4-dev qt4-qmake
```

Or for Qt5: 

```
 $ sudo apt-get install qt5-default libqt5xml5* libqt5network5* libqt5gui5* libqt5svg5* libqt5webkit5* libqt5quick5* qtdeclarative5-dev qttools5-dev qtbase5-dev qt5-qmake qtchooser
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
	mingw64/mingw-w64-i686-exiv2
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
	mingw64/mingw-w64-x86_64-exiv2
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
brew install qt5
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

### Run qmake

```
$ qmake -r
```

Note that you need the Qt4 or Qt5 version of qmake - running the Qt3 version
will generate incorrect makefiles.  If both are installed, take care to run the
correct version.  For example, on Fedora/Debian run

```
$ qmake-qt4
```

and on (K)ubuntu run

```
$ /usr/share/qt4/bin/qmake
```

There are some parameters you can pass to qmake to customize build:

| Parameter | Meaning |
| --- | ---
| PREFIX=<path>               | base prefix for installation (unix only) |
| TRANSDIR_MERKAARTOR=<path>  | where will the Merkaartor translations be installed |
| TRANSDIR_SYSTEM=<path>      | where your global Qt translation directory is | 
| NODEBUG=1                   | release target |
| USEWEBENGINE=1              | enable use of WebEngine (required for some external plugins) |
| SYSTEM_QTSA                 | use system copy of qtsingleapplication instead of internal |


### Run make

On Windows:           $ make release
On other platforms:   $ make
Debug build:          $ make debug

On most Windows installations, debug is the default and we need to specify
the release manually. Sorry about that.

### Done!

If you are lucky, you'll find an executable in the binaries subdirectory.
Otherwise, check our [github page](http://github.com/openstreetmap/merkaartor), especially check the issues and possibly
report yours.

### Setting up CLion

There are [instructions from JetBrains](https://www.jetbrains.com/help/clion/managing-makefile-projects.html).
Additional hints:

Remove the automatically created CMake profile, it doesn't work. That removes
the attempt to execute it on every start of CLion.

It seems the File Watcher to automatically recreate the compiledb on a
Makefile change can't be used since something touches the Makefiles during
that process and thus creates an infinite loop.

`cd` to the project root.
```
$ compiledb -n make debug
```

In case there are problems make sure that the directories `src/release` and
`src/debug` exist, sometimes they don't appear automatically.

Don't forget to follow the link at the bottom of the page on how to setup
custom build targets and custom Run/Debug configurations.

The Build/Build Project menu item doesn't seem to work, it only complains
about a missing CMake profile. It's also not possible to recompile the
current file alone. It seems that, contrary to the description, you can only
build after finishing the Run/Debug configuration. That provides a
"Build (your build target name)" menu item and toolbar button which works.
Funnily enough you can't set the name of the binary until you build it manually
or at least touch the file.

The Makefile Run/Debug configuration works as well, maybe even better. Use
the green run button to start a build, not the build button.

In order to have file names mentioned by the compiler clickable in the CLion
console install `tools/abspath-last-g++` as described there.

## Packaging

If you want to create installer package, read further.

### Linux

Nothing special here, just follow your distribution's guidelines.

### Windows

You first have to copy all dependencies of merkaartor to the binaries/bin
directory. The windows/copydeps.sh script does just that:

```
$ ./windows/copydeps.sh
```
Note that if you made a debug build, all the debug libraries will end up in
binaries/bin, and there they take almost 1.1G (that's why I'm not publishing
installer with debug symbols).

You can take the binaries/bin and distribute it as a zip, or create an
executable installer by invoking NSIS:

```
$ makensis windows/installer.nsi
```

### Mac OS X

Running `macdeployqt` should be enough, but for now, there is a bit more work to
do:

```
$ cd binaries/bin
mv plugins merkaartor.app/Contents
macdeployqt merkaartor.app -dmg
```

This should give you merkaartor.dmg. Due to a [bug in
macdeployqt](https://bugreports.qt.io/browse/QTBUG-53738), you might need to
create the archive manually:

```
$ hdiutil create "merkaartor.dmg" -srcfolder merkaartor.app -format UDZO -volname merkaartor -verbose -size 130m
```

But you still need to run `macdeployqt` to copy all the dependencies.
