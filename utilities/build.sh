#!/usr/bin/env bash

usage () {
    echo -e "USAGE: $0 [-c] [-f <main|initial>] [-p <device|simulator>] [-t <dev|debug|release|unit_tests>]"
    echo -e "Parameters are optional and assumes 'main debug device' if not provided"
    echo -e "\n\n -c \t Performs a forced clean before invoking build"
    echo -e "\n\n -f \t Sets the preferred firmware to build. Can be main or initial"
    echo -e "\n\n -p \t Provides the preferred platform to build for. Can be simulator or device"
    echo -e "\n\n -t \t Tells the build type that should be generate. Can be a valid build type."
    echo -e "\t    \t For example release, debug, dev, unit_tests"
    exit 1
}

validate_name () {
    if ! [[ "$ACTIVE_TYPE" =~ ^(Main|Initial)$ ]]; then
        echo -e "Incorrect firmware ($ACTIVE_TYPE) selected for build\n"
        usage
    fi
}

validate_platform () {
    if ! [[ "$BUILD_PLATFORM" =~ ^(Device|Simulator)$ ]]; then
        echo -e "Incorrect platform ($BUILD_PLATFORM) selected for build\n"
        usage
    fi
}

validate_type () {
    if ! [[ "$BUILD_TYPE" =~ ^(Debug|Release|Dev|Unit_tests)$ ]]; then
        echo -e "Incorrect type ($BUILD_TYPE) selected for build\n"
        usage
    fi
}

ACTIVE_ROOT_DIR=$(pwd)
ACTIVE_TYPE=Main
BUILD_TYPE=Debug
BUILD_PLATFORM=Device
UNIT_TESTS=OFF
DEV=OFF

while getopts 'cf:p:t:' flag; do
  case "${flag}" in
    c) clean_flag="true" ;;
    f) ACTIVE_TYPE=$(echo "${OPTARG}" | awk '{print toupper(substr($0, 1, 1)) tolower(substr($0, 2))}') ;;
    p) BUILD_PLATFORM=$(echo "${OPTARG}" | awk '{print toupper(substr($0, 1, 1)) tolower(substr($0, 2))}') ;;
    t) BUILD_TYPE=$(echo "${OPTARG}" | awk '{print toupper(substr($0, 1, 1)) tolower(substr($0, 2))}') ;;
    *) usage ;;
  esac
done

shift "$((OPTIND-1))"
validate_name
validate_platform
validate_type

if [ $# -gt 0 ]; then
    usage
fi

case $BUILD_TYPE in
  Dev)
    DEV=ON
    BUILD_TYPE=Debug
    ;;

  Unit_tests)
    DEV=OFF
    BUILD_TYPE=Debug
    UNIT_TESTS=ON
    ;;
esac

cd "${ACTIVE_ROOT_DIR}" || exit
mkdir -p "build/${ACTIVE_TYPE}"
cd "build/${ACTIVE_TYPE}" || exit

# remove previous cmake configuration to ensure we are building with
# currently requested build configuration; it is important to delete
# the existing cmake configuration
if [ -f "CMakeCache.txt" ]; then
    rm "CMakeCache.txt"
fi

# Detect if any one (cmake or mingw32-cmake) exists
CMAKE=$(which cmake)
if [ "${CMAKE}" = "" ]; then
    CMAKE=$(which mingw32-cmake)
fi

# Check if cmake or mingw32-cmake exists
if [ "${CMAKE}" = "" ]; then
    echo -e "\tNo cmake installation found. Please install cmake to continue (or check your 'Path' Environment variable";
    exit 1;
fi

# Detect if any one (ninja, make or mingw32-make) build tool exists
BUILD_TOOL=$(which ninja)
GEN="Ninja"
if [ "${BUILD_TOOL}" = "" ]; then
    BUILD_TOOL=$(which make)
    GEN="Unix Makefiles"
    if [ "${BUILD_TOOL}" = "" ]; then
        BUILD_TOOL=$(which mingw32-make)
        GEN="MinGW Makefiles"
    fi
fi

# throw error & exit if no build tool detected
if [ "${BUILD_TOOL}" = "" ]; then
    echo -e "\tNo build tool (make/ninja) installation found. Please install one to continue (or check your 'Path' Environment variable";
    exit 1;
fi

if [[ "${clean_flag}" = "true" ]]; then
  rm -rf "${ACTIVE_ROOT_DIR}/generated/proto"
fi

"${CMAKE}"  -DDEV_SWITCH=${DEV}                                 \
            -DUNIT_TESTS_SWITCH:BOOL="${UNIT_TESTS}"            \
            -DSIGN_BINARY=ON                                    \
            -DCMAKE_BUILD_TYPE:STRING="${BUILD_TYPE}"           \
            -DFIRMWARE_TYPE="${ACTIVE_TYPE}"                    \
            -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON             \
            -DCMAKE_BUILD_PLATFORM:STRING="${BUILD_PLATFORM}"   \
            -G "${GEN}" ../../ > /dev/null

# exit if configuration failed with errors
if [ ! $? -eq 0 ]; then exit 1; fi
if [[ "${clean_flag}" = "true" ]]; then
  "${BUILD_TOOL}" clean
fi
"${BUILD_TOOL}" -j8 all
