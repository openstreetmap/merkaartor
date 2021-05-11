# Merkaartor

An opensource OSM editor, written in C++ and Qt.

## Health status

Travis-CI (Linux, Windows and Mac OS X) [![Build Status](https://travis-ci.org/openstreetmap/merkaartor.svg?branch=master)](https://travis-ci.org/openstreetmap/merkaartor)

## Installation

### Binaries

Binary installation files are available for various Linux distributions and Windows. 

These Linux distributions are known to provide current versions of Merkaartor:
 - Arch via [AUR](https://aur.archlinux.org/packages/merkaartor-git/) ([git clone url](https://aur.archlinux.org/merkaartor-git.git))
 - Debian
 - Fedora
 - Gentoo

The Windows installer and Mac OS X bundle is available on the website: http://merkaartor.be/p/download

### Source

You can clone this repository to obtain the current development version, or checkout a specific release (all of them are tagged). Detailed instructions can be found in the [COMPILE.md](COMPILE.md) file in this directory.

## Nightly builds

For Windows and Mac OS X, nightly builds are automatically generated using Travis-CI, uploaded to cloudsmith:

https://cloudsmith.io/~merkaartor/repos/merkaartor-nightly/packages/

There are currently no nightlies for Linux. It's usually easy to compile from
source, and the vast amount of distributions doesn't make it easy. I'm
considering making Ubuntu PPA. Let us know if you're interested!

## Hacking the code

I'm always happy to see improvements done by other people. Feel free to
contribute by sending pull requests for big or small stuff! There is some (very
little, but it'll grow) useful stuff in [HACKING.md](HACKING.md), but if you
have questions, contacts us on merkaartor@openstreetmap.org to have them
answered!

## Thank you!

The following projects provide valuable services for Merkaartor (and other opensource projects) for free:

 - [github.com](https://github.com) free repository hosting and issue tracking
 - [travis-ci.org](https://travis-ci.org) free continuous integration and testing on Linux, Windows and MacOS
 - [![cloudsmith.io](https://img.shields.io/badge/OSS%20hosting%20by-cloudsmith-blue?logo=cloudsmith&style=flat)](https://cloudsmith.com) free binary hosting for nightly builds
