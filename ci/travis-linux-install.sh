#!/bin/sh

set -ev

env
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1397BC53640DB551
sudo echo "deb http://archive.ubuntu.com/ubuntu ${TRAVIS_DIST} main universe restricted multiverse" \> /etc/apt/sources.list
if [ "${TRAVIS_DIST}" == trusty ]; then
	sudo apt-add-repository -y ppa:ubuntu-sdk-team/ppa
fi
sudo apt-add-repository -y ${QT_REPO}
sudo apt-get update -qq
sudo apt-get -y install gdb libgdal-dev libproj-dev qt${QT_PREFIX}-meta-minimal qt${QT_PREFIX}svg build-essential libgl1-mesa-dev
qtchooser -list-versions
