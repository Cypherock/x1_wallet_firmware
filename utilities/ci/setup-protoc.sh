#!/usr/bin/env bash
set -ex

mkdir -p proto-tools
curl -L https://github.com/protocolbuffers/protobuf/releases/download/v21.12/protoc-21.12-linux-x86_64.zip --output protoc-21.12-linux-x86_64.zip
test -f protoc-21.12-linux-x86_64.zip
unzip -o protoc-21.12-linux-x86_64.zip -d proto-tools
