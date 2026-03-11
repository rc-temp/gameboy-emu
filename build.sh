#!/bin/bash

# From your repo/ directory
cmake -B build -S . -D CMAKE_CXX_COMPILER=g++-15 -D CMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake

# Compile
cmake --build build
