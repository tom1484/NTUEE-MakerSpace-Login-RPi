#!/bin/bash

mkdir -p temp
cd ./temp

sudo apt install libcurl4-openssl-dev libjsoncpp-dev
export LD_LIBRARY_PATH=/usr/local/lib/arm-linux-gnueabihf
echo "export LD_LIBRARY_PATH=/usr/local/lib/arm-linux-gnueabihf" >> ~/.bashrc

git clone https://github.com/jpbarrette/curlpp.git
cd curlpp
mkdir -p build
cd build

cmake ..
make -j4
sudo make install
sudo ldconfig

cd ../../..
sudo rm -rf temp
