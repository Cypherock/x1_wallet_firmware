# X1Wallet Firmware

# Cypherock Secure Wallet Simulator

The future is privacy first and decentralized enabled through Blockchain. Personal data will be personal wealth. The current issues with key management pose massive barriers to the adoption of decentralized technologies. We believe digital assets will have exponentially more value in the future and that will just not be limited to Cryptocurrency. Our mission to empower the people to be self-sovereign, trust themselves and to have the same peace of mind and control with their crypto and personal data that they have with their savings in the bank vaults.

**TODO: Add about device**
* What this project does
  * This project provide a desktop simulator for Cypherock's novel hardware wallet. 
  * This simulator allows users to test their code on the systems before touching the wallet.
  * This simulator mimics the experience that users will have while using an actual hardware wallet.
* Why people should consider using your project
  * This project allows developers from any background to contribute to the hardware wallet code without ever using any hardware.

# Table of Contents

- [Cypherock Secure Wallet Simulator](#cypherock-secure-wallet-simulator)
- [Table of Contents](#table-of-contents)
- [About the Project](#about-the-project)
- [Directories](#directories)
- [Project Status](#project-status)
- [Getting Started](#getting-started)
  - [Dependencies](#dependencies)
    - [Device](#device)
    - [Simulator](#simulator)
  - [Getting the Source](#getting-the-source)
  - [Building](#building)
    - [Device](#device-1)
    - [Device (using build script)](#device-alternative-using-build-script)
    - [Simulator](#simulator-1)
  - [Using node cli tool](#using-node-cli-tool-build-from-source)
    - [Building the cli tool](#building-the-cli-tool)
    - [Flash firmware (using CLI tool)](#update-the-device-firmware-only-for-developers)
- [Containerised build](#containerised-build)
- [Release Process](#release-process)
- [How to Get Help](#how-to-get-help)
- [Contributing](#contributing)
- [Further Reading](#further-reading)
- [License](#license)


# About the Project
**TODO: Add about device**
* Cypherock Simulator uses the lvgl and SDL2 library to run device code on the desktop.
* It uses the file system in place of the flash to mimic cards, storage and desktop communication

**[Back to top](#table-of-contents)**

# Directories

```
├── Application     # Contains Application Side files, common for both Device and Simulator
│   └── common      # Core libraries and utilities required by application
│       └── flash             # Contains wrappers for flash
│       └── interfaces        # Contains interfaces files for desktop , flash , card and USB
│       └── libraries         # Contains libraries for proof of work , Shamir's and utils.
│       └── logger            # Contains logger files for Debugging
│       └── lvgl              # Contains lvgl library
│       └── src               # Contains source code
│       └── coin_support      # Contains headers and source files required for various Cryptocurrency wallets
│   └── config      # Configuration files
│   └── flows       # Contains files to handle various flows of different levels consisting of controllers and tasks
│
├── bin            # Contains dll for SDL2 and binaries of the built simulator
├── build          # Contains build files of CMake and hex, bin, map and out files for device (Created only after build starts)
├── Device         # Contains external required libraries
├── Simulator      # Contains files required for various peripherals connectivity



```
# Project Status


The project is currently in beta stage of its development.

**[Back to top](#table-of-contents)**


# Getting Started
TODO

## Dependencies

### Device
1. git
2. arm-none-eabi-gcc
3. cmake (version 3.15.3)
4. mingw32-make or make or ninja

### Simulator

1. gcc version 7 or higher
2. make/mingw32-make
3. CMake version 3.XX
4. SDL2

## Getting the Source
The project repo consists of several modules maintained on individual repository and managed using `git submodules`.

Because of the existance of submodules, the correct way to clone this repo would be as follows:
- `git clone git@gitlab.com:cypherock-tech/device_stm32.git --recursive`

#### Alternative
If you have already cloned the repo w/o using the `--recursive` flag, execute the following commands in sqeuence to get the submodules ready.
- `git submodule init`
- `git submodule update`

**[Back to top](#table-of-contents)**

## Building

### Device
1. Install the required build tools
   - arm-none-eabi-gcc
   - git
   - ninja or make
   - cmake
2. issue the following commands to get the build ready (can use Git bash on windows)
   - `mkdir -p build/Debug`
   - `cd build/Debug`
   - `cmake -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=OFF -DCMAKE_BUILD_PLATFORM:STRING=Device -G "Unix Makefiles" ../../`
   - `make all`
3. Find the binary file (Cypherock.bin) generated in the `build/Debug` folder.

**NOTE**: This is an unsinged binary. You need to generate a signed binary using the cli-tool.

**NOTE**: In the cmake command the value for -G switch can be one of following:
- Ninja
- Unix Makefiles
- MinGW Makefiles

### Device alternative (using build script)
This method uses the build script for generating a signed binary of the firmware.
1. Install the required build tools
   - arm-none-eabi-gcc
   - git
   - ninja or make
   - cmake
   - python3
2. Make sure the cli tool is built [(steps here)](#building-the-cli-tool)
3. copy all the `.h` and `-version.txt` files from [this repo](https://gitlab.com/cypherock-tech/privatekeypairs) into communication-testing (cli tool source directory) folder
4. specify the cli tool source directory using the following command
  - `export CLI_ROOT_DIR=<path to cli tool source directory>`
5. Start the build process by calling the build script as `./stm-build.sh`

### Simulator

1. Install GCC 64 bit and Make

    [https://jmeubank.github.io/tdm-gcc/download/](https://jmeubank.github.io/tdm-gcc/download/)


2. Install CMake

   [https://cmake.org/download/](https://cmake.org/download/)

3. Install SDL2
   1. Download SDL2 64 bit
   
        [https://www.libsdl.org/download-2.0.php](https://www.libsdl.org/download-2.0.php)
 
   2. Extract the folder
        ```
        The 32-bit files are in i686-w64-mingw32
        The 64-bit files are in x86_64-w64-mingw32
        ```

   3. Folders in `x86_64-w64-mingw32` 
        ```
        bin
        include
        lib
        share
        ```

   4.  Find the installation location of TDM GCC (TDM-GCC-64)
       1.   Copy the contents from `x86_64-w64-mingw32/include` to include folder of `your_tdm_installation_directory/TDM-GCC-64/include`
       2.   Copy the contents from `x86_64-w64-mingw32/lib` to include folder of `your_tdm_installation_directory/TDM-GCC-64/lib`

   5. Restart your system

4. Clone this repository
5. Change directory to the root folder of the project
6. Create a `build` folder
7. Run CMake
    ```
    cmake --no-warn-unused-cli -DCMAKE_BUILD_PLATFORM:STRING=Simulator -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_BUILD_TYPE:STRING=Debug -G "MinGW Makefiles"
    ```
8. Build
   ```
   cmake --build --config Debug --target Cypherock_Simulator -j 6 --
   ```
9. The generated exe will be present the bin folder

10. Copy `SDL2.dll` and `sdl2-config` to the bin folder

11. Run executable to start the simulator

12. To enable support for USB communication into simulator follow the steps
    - Download the package for required platform [Windows](https://drive.google.com/file/d/1QjpRdPnI9if6y8WtWz0FTfbj051bSAVS/view?usp=sharing) | [Linux](https://drive.google.com/file/d/13sLbiav0SAbvQ0rorZRRthLiOFX4oSC6/view?usp=sharing)
    - Make sure that the `.env` file containes the line `MOCK=true` in it
    - Now start the cli using the command `./custom`
    - NOTE: ***It is required that the cli tool is started after starting the device simulator***

#### Valid data packet from cli to X1Wallet
After starting the cli by following step 12, it is possible to send commands to Simulator
- Initiate the add wallet flow
  - type `43,00` from cli
  - the device will respond hex array in the structure `<16-byte wallet name ASCII encoded value><1-byte wallet-info><32-byte wallet-id>`. exmaple output `Received Command 44 : 4141000000000000000000000000000000c372af88f64e0a40439f97ee98a3a0a03e9b2ac348b464d0cab7f32ee8482298`
  - optionally to complete the flow, send `42,01` from cli for confirmation (if not sent, X1Wallet will show error prompt after it times out)
- Initiate the add coin flow
  - type `45,<32-byte wallet-id>00<1-byte coin count><4-byte coin-index><1-byte chain-id>` from cli. example `45,c372af88f64e0a40439f97ee98a3a0a03e9b2ac348b464d0cab7f32ee84822980002800000008000003c0003`. Where wallet-id is fetched from previous step. We mean to add 2 coins so `02` and followed by `80000000` for bitcoin and `8000003c` for ethereum. Followed by `00` for bitcoin (fixed for compatibility) and `03` for ropsten network id.
  - follow the prompt on device to continue
  - the device will respond
  - optionally to complete the flow, send `42,01` from cli for confirmation (if not sent, X1Wallet will show error prompt after it times out)
- Initiate the add wallet flow
  ``

## Using [node cli tool](https://gitlab.com/cypherock-tech/communication-testing) (build from source)

### Building the cli tool
- Clone the cli tool source code
- make sure to install the following tools
  - `node`
  - `yarn`
  - `python3`
- copy/rename the provided `.env.example` to `.env`
- execute the following command to build the tool before using
  - `yarn && yarn build`

### Update the device firmware (only for developers)
- download the firmware from [gitlab releases](https://gitlab.com/cypherock-tech/device_stm32/-/releases) (pick one of the bin file named '...signed', preferably x1wallet-main-release-signed.bin)
- follow the steps to build cli tool using the [build steps mentioned](#building-the-cli-tool)
- put device in bootloader mode (**It is important the device is connected to laptop via USB before starting the next step**)
- start cli using the command (`yarn start`)
- choose from the command line options - "STM CLI" --> "STM Update"
- locate the path to downloaded binary file
- the device should finish the process and restart to "Main menu"

### Signing firmware binary
1. Build a normal binary of the project [steps here](#device)
2. Make sure the cli tool is built [(steps here)](#building-the-cli-tool)
3. copy all the `.h` and `-version.txt` files from [this repo](https://gitlab.com/cypherock-tech/privatekeypairs) into communication-testing (cli tool source directory) folder
4. Start the cli using the command (`yarn start`)
5. First step: 
   - In the command line options choose "STM CLI" --> "Add Header" and add header to the binary
   - input binary: the bin file generated in step 1
   - output binary: output file name
   - version file: The '-version.txt' downloaded in step 3
   - private key file: The `private_key1.h` file downloaded in step 3
6. Second step: 
   - Restart the cli tool (`yarn start`)
   - Choose from the command line options "STM CLI" --> "Sign Header" and add signature to the binary
   - input file: the bin file generated in step 5
   - output file: final signed binary
   - private key file: The `private_key2.h` file downloaded in step 3

**NOTE**: If any of the step 5 or step 6 fail, then try the [build script](#device-alternative-using-build-script) to generate a signed binary.

# Containerised build
- Create a docker image and run the build process inside it.<br/>
  ```
  docker image build --tag x1-wallet-main-app .
  ```

- Now from the docker image, copy the application binaries to build folder.<br/>
  ```
  mkdir -p build
  docker run -v $(pwd)/build:/out x1-wallet-main-app cp -a dist/. /out
  ```

### Output files (binaries)
The output of the build process can be found inside `~/build` folder

### Next steps
The generated binaries should be appended with signed header to be useable with device.

# Release Process (Gitlab)

- Create a tag on gitlab 
  - Provide the tag name (ex: v0.0.7). This will be used as the release title.
  - Optionally add a message (**NOTE: This message should not include any line breaks. Keep it a simple text.**). This will be used as the release description.
- A CI/CD job will run on gitlab which will create the release and add the binaries.
  - `firmware.bin` - Unsigned binary
  - `firmware-signed.bin` - Signed binary with headers from `version.txt`

**[Back to top](#table-of-contents)**

# How to Get Help

We can be contacted through our [mail](mailto:info@cypherock.com).

# Contributing

Currently only developer within the company are allowed to contribute or make commits on their specific branches.

**[Back to top](#table-of-contents)**

# Further Reading

[Security](https://www.cypherock.com/security/)

[Hardware Wallets](https://www.cypherock.com/hardware/)

**[Back to top](#table-of-contents)**

# License

Copyright (c) 2017 Cypherock

This project is licensed under the MIT License - see [LICENSE.md](LICENSE.md) file for details.


**[Back to top](#table-of-contents)**
