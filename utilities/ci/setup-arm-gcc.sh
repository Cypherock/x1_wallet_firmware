#!/usr/bin/env bash

set -ex

curl -L https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2 --output arm-gcc.tar.bz
test -f arm-gcc.tar.bz
tar -xf arm-gcc.tar.bz
mv gcc-arm-none-eabi-10.3-2021.10 arm-gcc