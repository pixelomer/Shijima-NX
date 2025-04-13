#!/usr/bin/env bash

set -ve
mkdir -p build && cd build
${DEVKITPRO}/portlibs/switch/bin/aarch64-none-elf-cmake ..
make -j`nproc`
