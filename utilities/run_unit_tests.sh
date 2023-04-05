#!/bin/sh

usage () {
    echo -e "\tUSAGE: $0 <main|initial> <device|simulator>"
    exit 1
}

set -e

if [ $# -lt 2 ]; then
    usage
fi

case $1 in
    main)
    ;;

    initial)
    ;;

    *)
    echo "Wrong type selection"
    usage
    ;;
esac

case $2 in
    device)
    ;;

    simulator)
    ;;

    *)
    echo "Wrong platform selection"
    usage
    ;;
esac

echo "Invoking build script"
echo "./utilities/build.sh "$1" "unit_tests" "$2""
./utilities/build.sh "$1" "unit_tests" "$2"

if [ "$2" == "device" ]; then
    echo "Flashing unit tests on the target"
    echo "STM32_Programmer_CLI -c port=swd -w build/${1^}/Cypherock-${1^}.bin 0x08023000"
    STM32_Programmer_CLI -c port=swd -w build/${1^}/Cypherock-${1^}.bin 0x08023000

    echo "Running unit tests on the target"
    echo "STM32_Programmer_CLI -c port=swd -startswv freq=80 portnumber=all -RA"
    STM32_Programmer_CLI -c port=swd -startswv freq=80 portnumber=all -RA
else
    echo "Running unit tests on the simulator"
    ./bin/Cypherock_Simulator
fi