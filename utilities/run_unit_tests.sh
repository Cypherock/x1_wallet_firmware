#!/bin/sh

#usage: ./utilities/run_unit_tests.sh [main|initial] [device|simulator]

set -e

echo "Invoking build script"
echo "./utilities/build.sh "$1" "unit_tests" "$2""

./utilities/build.sh "$1" "unit_tests" "$2"

if [ "$2" == "device" ]; then
    echo "Flashing unit tests on the target"
    echo "STM32_Programmer_CLI -c port=swd -w build/Main/Cypherock-Main.bin 0x08023000"
    STM32_Programmer_CLI -c port=swd -w build/Main/Cypherock-Main.bin 0x08023000

    echo "Running unit tests on the target"
    echo "STM32_Programmer_CLI -c port=swd -startswv freq=80 portnumber=all -RA"
    STM32_Programmer_CLI -c port=swd -startswv freq=80 portnumber=all -RA
else
    echo "Running unit tests on the simulator"
    ./bin/Cypherock_Simulator.exe
fi