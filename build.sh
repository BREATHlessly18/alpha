#! /bin/bash

mkdir build

cd build && cmake .. -DCMAKE_INSTALL_PREFIX=$(pwd)/../ -DCMAKE_BUILD_TYPE=Release 

make -j4 clean all && make install

cd ..