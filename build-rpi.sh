#!/usr/bin/env bash

# Clean
rm -rf build-rpi

# Build
mkdir build-rpi && cd build-rpi
cmake -DCMAKE_C_FLAGS="-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard" -DCMAKE_CXX_FLAGS="-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard" ../
make -j 4
