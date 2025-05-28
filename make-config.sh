#!/usr/bin/env bash

set -e

if [ $# -ne 3 ]; then
    echo "Usage: $0 <actions.xml> <behaviors.xml> <img/>"
    exit 1
fi

if [ ! -d "$3" ]; then
    echo "Not a directory: $3"
    exit 1
fi

if [ "$(ls -1 "$3/"*.png | wc -l)" -lt 3 ]; then
    echo "img/ folder has less than 3 pngs. Did you choose the right folder?"
    exit 1
fi

if [ ! -f libs/libshijima/build/shijima-sandbox ]; then
    echo "==> Building shijima-sandbox..."
    pushd libs/libshijima
    cmake -Bbuild -DCMAKE_BUILD_TYPE=Release
    make -Cbuild -j`nproc`
    popd
fi

if [ ! -f tesla-packer/build/tesla-packer-cli ]; then
    echo "==> Building tesla-packer-cli..."
    pushd tesla-packer
    cmake -DCMAKE_BUILD_TYPE=Release -Bbuild
    make -Cbuild -j`nproc`
    popd
fi

echo "==> Generating mascot.cereal..."

libs/libshijima/build/shijima-sandbox serialize "$1" "$2" mascot.cereal

echo "==> Generating img.bin..."

tesla-packer/build/tesla-packer-cli "$3" img.bin

echo
echo "Done! Configuration files saved to mascot.cereal and img.bin"
echo "To use with Shijima-NX, copy mascot.cereal and img.bin to this folder:"
echo
echo "  sd:/config/Shijima-NX/"
echo
