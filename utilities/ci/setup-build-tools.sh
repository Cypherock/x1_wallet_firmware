#!/usr/bin/env bash

set -ex

mkdir build-tools
curl -L https://github.com/Kitware/CMake/releases/download/v3.25.3/cmake-3.25.3-linux-x86_64.tar.gz --output cmake.tar.gz
test -f cmake.tar.gz
tar -xf cmake.tar.gz
mv cmake-3.25.3-linux-x86_64/bin/cmake ./build-tools/
curl -L https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-linux.zip --output ninja.zip
test -f ninja.zip
unzip -o ninja.zip
mv ninja ./build-tools/ninja