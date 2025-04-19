#!/usr/bin/env bash

set -ve
mkdir -p build && cd build
${DEVKITPRO}/portlibs/switch/bin/aarch64-none-elf-cmake -DCMAKE_BUILD_TYPE=Release ..
make -j`nproc` VERBOSE=1
