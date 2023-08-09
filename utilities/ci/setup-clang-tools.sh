#!/usr/bin/env bash
set -ex

mkdir clang-tools
curl -L https://github.com/llvm/llvm-project/releases/download/llvmorg-15.0.6/clang+llvm-15.0.6-x86_64-linux-gnu-ubuntu-18.04.tar.xz --output clang.tar.gz
test -f clang.tar.gz
tar -xf clang.tar.gz
cd ./clang+llvm-15.0.6-x86_64-linux-gnu-ubuntu-18.04/
cp bin/clang-format bin/clang-tidy bin/git-clang-format bin/clang-check bin/run-clang-tidy ../clang-tools/