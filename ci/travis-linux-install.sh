#!/bin/sh

set -ev

sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1397BC53640DB551
sudo echo "deb http://archive.ubuntu.com/ubuntu trusty main universe restricted multiverse" \> /etc/apt/sources.list
sudo apt-add-repository -y ppa:ubuntu-sdk-team/ppa
sudo apt-get update -qq
sudo apt-cache search qt
sudo apt-get -qq install gdb libgdal-dev libproj-dev
sudo apt-get -qq install libqt4-xml libqt4-network libqt4-gui libqt4-svg libqt4-webkit libqt4-dev qt4-qmake
sudo apt-get -qq install libqt5xml5* libqt5network5* libqt5gui5* libqt5svg5* libqt5webkit5* libqt5quick5* qtdeclarative5-dev qttools5-dev qtbase5-dev qt5-qmake qtchooser
qtchooser -list-versions
