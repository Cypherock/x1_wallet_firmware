#!/bin/sh

usage () {
    echo -e "\tUSAGE: $0 [-c] [main|initial] [dev|debug|release|unit_tests] [device|simulator]"
    echo -e "\tParameters are optional and assumes 'main debug device' if not provided"
    echo -e "\n\n\t -c \t Performs a forced clean before invoking build"
    exit 1
}

ACTIVE_ROOT_DIR=$(pwd)
ACTIVE_TYPE=Main
BUILD_TYPE=Debug
BUILD_PLATFORM=Device
UNIT_TESTS=OFF

while getopts 'c' flag; do
  case "${flag}" in
    c) clean_flag="true" ;;
    *) usage
       exit 1 ;;
  esac
  shift
done

if [ $# -gt 0 ]; then
    case $1 in
        main)
        ACTIVE_TYPE=Main
        ;;

        initial)
        ACTIVE_TYPE=Initial
        ;;

        *)
        echo "Wrong type selection"
        usage
        ;;
    esac
fi

if [ $# -gt 1 ]; then
    case $2 in
        dev)
        DEV=ON
        BUILD_TYPE=Debug
        ;;

        debug)
        DEV=OFF
        BUILD_TYPE=Debug
        ;;

        release)
        DEV=OFF
        BUILD_TYPE=Release
        ;;

        unit_tests)
        DEV=OFF
        BUILD_TYPE=Debug
        UNIT_TESTS=ON
        ;;

        *)
        echo "Wrong mode selection"
        usage
        ;;
    esac
fi

if [ $# -gt 2 ]; then
    case $3 in
        device)
        BUILD_PLATFORM=Device
        ;;

        simulator)
        BUILD_PLATFORM=Simulator
        ;;

        *)
        echo "Wrong platform selection"
        usage
        ;;
    esac
fi


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

"${CMAKE}"  -DDEV_SWITCH=${DEV}                                 \
            -DUNIT_TESTS_SWITCH:BOOL="${UNIT_TESTS}"            \
            -DSIGN_BINARY=ON                                    \
            -DCMAKE_BUILD_TYPE:STRING="${BUILD_TYPE}"           \
            -DFIRMWARE_TYPE="${ACTIVE_TYPE}"                    \
            -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON             \
            -DCMAKE_BUILD_PLATFORM:STRING="${BUILD_PLATFORM}"   \
            -G "${GEN}" ../../

# exit if configuration failed with errors
if [ ! $? -eq 0 ]; then exit 1; fi
if [[ ${clean_flag} = "true" ]]; then
  "${BUILD_TOOL}" clean
fi
"${BUILD_TOOL}" -j8 all
