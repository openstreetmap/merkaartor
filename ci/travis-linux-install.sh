#!/bin/sh

set -ev

sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 1397BC53640DB551
sudo echo "deb http://archive.ubuntu.com/ubuntu trusty main universe restricted multiverse" \> /etc/apt/sources.list
sudo apt-add-repository -y ppa:ubuntu-sdk-team/ppa
sudo apt-add-repository -y ppa:beineri/opt-qt591-trusty
sudo apt-get update -qq
sudo apt-get -y install gdb libgdal-dev libproj-dev qt59-meta-minimal qt59svg build-essential libgl1-mesa-dev
qtchooser -list-versions
