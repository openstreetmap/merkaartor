#!/bin/sh

set -ev

sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1397BC53640DB551
sudo echo "deb http://archive.ubuntu.com/ubuntu trusty main universe restricted multiverse" \> /etc/apt/sources.list
sudo apt-add-repository -y ppa:ubuntu-sdk-team/ppa
sudo apt-add-repository -y ppa:beineri/opt-qt591-trusty
sudo apt-get update -qq
sudo apt-cache search qt
sudo apt-get -qq install gdb libgdal-dev libproj-dev
#sudo apt-get -qq install qt56xml qt56network qt56gui qt56svg qt56webengine qt56quick qt57declarative qt57tools qtbase56-dev qt56-qmake qtchooser
sudo apt-cache search qt59
sudo apt-get -y install qt59-meta-full
qtchooser -list-versions
