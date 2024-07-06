#!/bin/sh

set -ev

env
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1397BC53640DB551
sudo apt-key adv --fetch-keys https://apt.kitware.com/keys/kitware-archive-latest.asc
sudo echo "deb http://archive.ubuntu.com/ubuntu ${TRAVIS_DIST} main universe restricted multiverse" \> /etc/apt/sources.list
sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main'
sudo apt-add-repository -y ${QT_REPO}
sudo apt-get update -qq
sudo apt-get -y install gdb libgdal-dev libproj-dev qt${QT_PREFIX}base qt${QT_PREFIX}tools qt${QT_PREFIX}svg qt${QT_PREFIX}networkauth build-essential libgl1-mesa-dev cmake git libexiv2-dev protobuf-compiler
