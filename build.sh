#!/usr/bin/env bash

# Clean
rm -rf build

# Build
mkdir build && cd build
cmake ../
make -j 4
