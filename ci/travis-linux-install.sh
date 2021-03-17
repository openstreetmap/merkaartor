#!/bin/sh

set -ev

env
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1397BC53640DB551
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys DE19EB17684BA42D
sudo echo "deb http://archive.ubuntu.com/ubuntu ${TRAVIS_DIST} main universe restricted multiverse" \> /etc/apt/sources.list
sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main'
sudo apt-add-repository -y ${QT_REPO}
sudo apt-get update -qq
which cmake
sudo apt-get -y install gdb libgdal-dev libproj-dev qt${QT_PREFIX}base qt${QT_PREFIX}tools qt${QT_PREFIX}svg build-essential libgl1-mesa-dev cmake git libexiv2-dev
which cmake
dpkg-query -L cmake
