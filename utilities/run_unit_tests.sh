#!/usr/bin/env bash

usage () {
    echo -e "\tUSAGE: $0 -f <main|initial> -p <device|simulator>"
    exit 1
}

validate_name () {
    if ! [[ "$FIRMWARE_TYPE" =~ ^(Main|Initial)$ ]]; then
        echo -e "Incorrect firmware ($FIRMWARE_TYPE) selected for build\n"
        usage
    fi
}

validate_platform () {
    if ! [[ "$BUILD_PLATFORM" =~ ^(Device|Simulator)$ ]]; then
        echo -e "Incorrect platform ($BUILD_PLATFORM) selected for build\n"
        usage
    fi
}

set -e
while getopts 'f:p:' flag; do
  case "${flag}" in
    f) FIRMWARE_TYPE=$(echo "${OPTARG}" | awk '{print toupper(substr($0, 1, 1)) tolower(substr($0, 2))}') ;;
    p) BUILD_PLATFORM=$(echo "${OPTARG}" | awk '{print toupper(substr($0, 1, 1)) tolower(substr($0, 2))}') ;;
    *) usage ;;
  esac
done

validate_name
validate_platform

echo "Invoking build script"
echo "./utilities/build.sh -c -f \"$FIRMWARE_TYPE\" -t \"unit_tests\" -p \"$BUILD_PLATFORM\""
./utilities/build.sh -c -f "$FIRMWARE_TYPE" -t "unit_tests" -p "$BUILD_PLATFORM"

if [ "$BUILD_PLATFORM" == "Device" ]; then
    echo "Flashing unit tests on the target"
    echo "STM32_Programmer_CLI -c port=swd -w build/${FIRMWARE_TYPE}/Cypherock-${FIRMWARE_TYPE}.bin 0x08023000"
    STM32_Programmer_CLI -c port=swd -w build/${FIRMWARE_TYPE}/Cypherock-${FIRMWARE_TYPE}.bin 0x08023000

    echo "Running unit tests on the target"
    echo "STM32_Programmer_CLI -c port=swd -startswv freq=80 portnumber=all -RA"
    STM32_Programmer_CLI -c port=swd -startswv freq=80 portnumber=all -RA
else
    echo "Running unit tests on the simulator"
    ./bin/Cypherock_Simulator || true

    which gcovr > /dev/null
    if [ $? == 0 ]; then
        mkdir -p build/coverage/
        echo "Generating the coverage report..."
        gcovr --html-nested build/coverage/report.html -s
        echo "Report generated at $(find "$(pwd)/build/coverage/" -name report.html)"
    else
        echo Error: gcovr not found
        echo try \"pip install gcovr\"
    fi
fi
