#!/bin/bash

# From your repo/ directory
# g++-15 is just to pin a version, and it's what i had
cmake -B build -S . -D CMAKE_CXX_COMPILER=g++-15 -D CMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake

# Compile
cmake --build build
