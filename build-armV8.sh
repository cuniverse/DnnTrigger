#!/usr/bin/env bash

# Clean
rm -rf build-armv8

# Build
mkdir build-armv8 && cd build-armv8
# cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain.arm.cmake -DCMAKE_C_FLAGS="--cpu=Cortex-A8 --target=arm-arm-none-eabi -mcpu=cortex-a8 -mfloat-abi=hard" -DCMAKE_CXX_FLAGS="--cpu=Cortex-A8 --target=arm-arm-none-eabi -mcpu=cortex-a8 -mfloat-abi=hard" ../
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain.arm.cmake  ../
make -j 8
