## Steps to build & verify firmware releases

X1Vault runs this open-source project providing transparent & trustless execution environment. Users of Cypherock can understand what is happening under the hood. But how do you verify if the binary has been built from this source code only? To ensure that, all our firmware builds are "[Reproducible build](https://en.wikipedia.org/wiki/Reproducible_builds)" which means, you can generate the same binary file which goes into you X1Vault (but cannot use it for OTA due to supply-chain safety checks). With our reproducible builds, anyone can build the firmware locally and verify it's sha256  checksum with each of our published firmware releases. This document will guide you through the steps to generate the X1Vault firmware on your local machine.

Each firmware release is built in a docker environment (Versions since 0.4.773). The firmware binaries can be regenerated in an indentical environment to our CI pipeline.

### Pre-requisites
- docker (recommended for linux distros)
- docker-desktop (recommended for windows)

### Build steps (with docker-compose)
1. Create a docker-compose.yml file and copy the below code-snippet into the file.
```yml
version: '3'

services:
  build-firmware:
    image: cypherock/x1-firmware-builder:v0.0.0
    command: >
      bash -c '
      git clone --branch ${VERSION_TAG} --depth 1 https://github.com/Cypherock/x1_wallet_firmware.git --recurse-submodules;
      cd x1_wallet_firmware && mkdir -p build && cd build;
      cmake -DCMAKE_BUILD_TYPE="Release" -DFIRMWARE_TYPE="Main" -DCMAKE_BUILD_PLATFORM="Device" -G "Ninja" ..;
      ninja;
      sha256sum Cypherock-Main.bin
      '

```

___Example___: To verify the build for version `0.5.2306`, use the following command-line arguments

#### Command prompt
1. `set VERSION_TAG=v0.5.2306`
2. `docker-compose up`

#### Bash / Git-Bash terminal
1. `VERSION_TAG=v0.5.2306 docker-compose up`

Note the sha256 checksum of the local build
```
build-firmware-1  | 55456f181ef34c0fb646f0a063c3efd5419f91bfa5122b8753d18579862410fc  Cypherock-Main.bin
```

### Build steps (with docker only)
1. `docker run -it cypherock/x1-firmware-builder:v0.0.0 ash`
2. Run the following bash script in the container terminal
```bash
VERSION_TAG=v0.5.2306
git clone --branch ${VERSION_TAG} --depth 1 https://github.com/Cypherock/x1_wallet_firmware.git --recurse-submodules
cd x1_wallet_firmware && mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE="Release" -DFIRMWARE_TYPE="Main" -DCMAKE_BUILD_PLATFORM="Device" -G "Ninja" ..
ninja
sha256sum Cypherock-Main.bin
```

Note the sha256sum of the local build
```
/x1_wallet_firmware/build # sha256sum Cypherock-Main.bin
55456f181ef34c0fb646f0a063c3efd5419f91bfa5122b8753d18579862410fc  Cypherock-Main.bin
```

### Verification
visit the [release page on github](https://github.com/Cypherock/x1_wallet_firmware/releases) and match the SHA256 of official binary
```
SHA256 of binaries:
Cypherock-Initial.bin : c5f7ec25ebfcff6b9074c45e4e8e330bbaffeed040cae3bf51b3a747877e9f52
Cypherock-Main.bin : 55456f181ef34c0fb646f0a063c3efd5419f91bfa5122b8753d18579862410fc
```
