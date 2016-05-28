# Merkaartor


An opensource OSM editor, written in C++ and Qt.

## Health status

| Platform | Status |
| -------- | ------ |
| Windows (Tea-CI) | [![Build Status](https://tea-ci.org/api/badges/openstreetmap/merkaartor/status.svg)](https://tea-ci.org/openstreetmap/merkaartor) 
| Linux (Travis-CI) | [![Build Status](https://travis-ci.org/openstreetmap/merkaartor.svg?branch=master)](https://travis-ci.org/openstreetmap/merkaartor) |

## Installation

### Binaries

Binary installation files are available for various Linux distributions and Windows. 

These Linux distributions are known to provide current versions of Merkaartor:
 - Arch (via AUR)
 - Debian
 - Fedora
 - Gentoo

The Windows installer is available on the website: http://merkaartor.be/p/download

### Source

You can clone this repository to obtain the current development version, or checkout a specific release (all of them are tagged). Detailed instructions can be found in the INSTALL file in this directory.

## Nightly builds

For Windows, nightly builds are automatically generated using Tea-CI and uploaded to bintray:
https://bintray.com/krakonos/nightly/Merkaartor .

There are currently no nightlies for Linux. It's usually easy to compile from
source, and the vast amount of distributions doesn't make it easy. I'm
considering making Ubuntu PPA. Let us know if you're interested!
