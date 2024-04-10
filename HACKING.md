## Setup Build Environment

### Docker container
[Clone repo](#clone-repository) & spin up a docker container using build environment image as follows.

```bash

docker run -v ./x1_wallet_firmware:/x1_wallet_firmware -it cypherock/x1-firmware-builder:v1.0.0 /bin/bash

```

### VSCode devcontainer

Open the project folder in dev container

### Local
1. Install the following Dependencies & [clone repo](#clone-repository)
- arm-none-eabi-gcc `10.3-2021.10` | [download link](https://developer.arm.com/downloads/-/gnu-rm)
- git
- ninja
- cmake `v3.25.3` | [download link](https://github.com/Kitware/CMake/releases/tag/v3.25.3)
- protobuf `v24.4` | [download link](https://github.com/protocolbuffers/protobuf/releases/tag/v24.4) | [install steps](https://grpc.io/docs/protoc-installation/)
- python3
- pip packages: `wheel`, `protobuf`, `grpcio-tools==1.47.0`

## Clone repository

The repository contains multiple submodules so it is important to properly clone the repository with all the necessary submodules. A proper way to clone is as follows:

`git clone https://github.com/Cypherock/x1_wallet_firmware --recurse-submodules`

## Build steps
```bash

cmake -B build/ -DDEV_SWITCH=OFF -DUNIT_TESTS_SWITCH:BOOL=OFF -DSIGN_BINARY=OFF -DCMAKE_BUILD_TYPE:STRING="Release" -DFIRMWARE_TYPE="Main" -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=OFF -DCMAKE_BUILD_PLATFORM:STRING="Device" -G "Ninja"
cmake --build build/ -- all

```

Locate the binary file (Cypherock-Main.bin) generated in the `build/` folder.


**NOTE**: This is an unsinged binary. You need to generate a signed binary using the cli-tool.

**NOTE**: In the cmake command the value for -G switch can be one of following:
- Ninja
- Unix Makefiles
- MinGW Makefiles


---
---


## OS Specific Installation Notes

### MacOS (M1)

#### Installing CMake

1. Visit <https://cmake.org/download/> and download the latest universal dmg. Double click on the dmg to mount it and drag the CMake app to your Applications folder.
2. Add CMake to your path by adding the following to your `~/.zshrc` or `~/.bashrc` file:

```bash
export PATH="/Applications/CMake.app/Contents/bin:$PATH"
```

3. Restart your terminal and run `cmake --version` to verify that it is installed.

##### Installing `arm-none-eabi-gcc`

1. Visit <https://developer.arm.com/downloads/-/gnu-rm> and download the latest version of the GNU Arm Embedded Toolchain for macOS.
2. Unzip the tarball and move the `gcc-arm-none-eabi-10-2020-q4-major` folder to your home directory.

```bash
tar -xjf gcc-arm-none-eabi-10.3-2021.10-mac.tar.bz2
mv gcc-arm-none-eabi-10.3-2021.10 ~/
```

3. Add the toolchain to your path by adding the following to your `~/.zshrc` or `~/.bashrc` file:

```bash
export PATH="$HOME/gcc-arm-none-eabi-10.3-2021.10/bin:$PATH"
```

4. Then run the following commands:

```bash
cd ~/gcc-arm-none-eabi-10.3-2021.10
xattr -r -d com.apple.quarantine arm-none-eabi
xattr -r -d com.apple.quarantine bin
xattr -r -d com.apple.quarantine lib
xattr -r -d com.apple.quarantine share
```

After this you should be able to run the build script `/utilities/build.sh` without any issues.
