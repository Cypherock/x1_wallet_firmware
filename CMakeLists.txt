cmake_minimum_required(VERSION 3.19.0)
set(CMAKE_MESSAGE_LOG_LEVEL debug)

if("${CMAKE_BUILD_PLATFORM}" STREQUAL "Device")
    message("Device platform selected")
    set(CMAKE_TOOLCHAIN_FILE utilities/cmake/arm-none-eabi-gcc.cmake)
    set(PROJECT_NAME_BASE Cypherock-${FIRMWARE_TYPE})
elseif("${CMAKE_BUILD_PLATFORM}" STREQUAL "Simulator")
    message("Simulator platform selected")
    set(PROJECT_NAME_BASE Cypherock_Simulator)
else()
    message(WARNING "No platform specified, defaulting to Simulator. Specify using -DCMAKE_BUILD_PLATFORM=<Type>")
    set(PROJECT_NAME_BASE Cypherock_Simulator)
endif()

# Define all build options
OPTION(DEV_SWITCH "Additional features/logs to aid developers" OFF)
OPTION(UNIT_TESTS_SWITCH "Compile build for main firmware or unit tests" OFF)
OPTION(BTC_ONLY "Build firmware for Bitcoin only" OFF)

# Append suffix to the project name if BTC_ONLY is enabled
set(PROJECT ${PROJECT_NAME_BASE})
if (BTC_ONLY)
    set(PROJECT ${PROJECT}-btc)
endif()

# Define the project with its final calculated name
project(${PROJECT})

# Make static functions testable via unit-tests
IF(UNIT_TESTS_SWITCH)
    add_compile_definitions( STATIC= )
ELSE()
    add_compile_definitions( STATIC=static )
ENDIF(UNIT_TESTS_SWITCH)

# Set firmware type (Main or Initial)
if ("${FIRMWARE_TYPE}" STREQUAL "Main")
    add_compile_definitions(X1WALLET_INITIAL=0 X1WALLET_MAIN=1)
elseif("${FIRMWARE_TYPE}" STREQUAL "Initial")
    add_compile_definitions(X1WALLET_INITIAL=1 X1WALLET_MAIN=0)
endif()

# Set variant-specific definition (BTC_ONLY_BUILD)
if(BTC_ONLY)
    add_compile_definitions(BTC_ONLY_BUILD=1)
endif()

# Set hash calculation definition based on build type
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    add_compile_definitions(FIRMWARE_HASH_CALC=1)
else()
    add_compile_definitions(FIRMWARE_HASH_CALC=0)
endif()

# Enable support for dynamically allocated fields in nanopb
add_compile_definitions(PB_ENABLE_MALLOC=1 PB_NO_ERRMSG=1)

find_package( Python3 REQUIRED COMPONENTS Interpreter )

# Conditionally generate protobuf files based on the BTC_ONLY option
IF(BTC_ONLY)
    message(STATUS "Generating protobufs for BTC-only build")
    execute_process(COMMAND bash utilities/proto/generate-protob.sh --btc-only
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                    COMMAND_ERROR_IS_FATAL ANY)
ELSE()
    message(STATUS "Generating protobufs for full build")
    execute_process(COMMAND bash utilities/proto/generate-protob.sh
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                    COMMAND_ERROR_IS_FATAL ANY)
ENDIF()

# Populate version.c
include(utilities/cmake/version.cmake)

file(GLOB_RECURSE PROTO_SRCS "generated/proto/*.*")
list(APPEND PROTO_SRCS
    "vendor/nanopb/pb_common.c"
    "vendor/nanopb/pb_decode.c"
    "vendor/nanopb/pb_encode.c"
    "vendor/nanopb/pb_common.h"
    "vendor/nanopb/pb_decode.h"
    "vendor/nanopb/pb_encode.h"
    "vendor/nanopb/pb.h"
)

list (APPEND MINI_GMP_SRCS
    "vendor/mini-gmp/mini-gmp-helpers.c"
    "vendor/mini-gmp/mini-gmp.c"
)
list (APPEND POSEIDON_SRCS
    "vendor/poseidon/sources/f251.c"
    "vendor/poseidon/sources/poseidon.c"
    "vendor/poseidon/sources/poseidon_rc.c"
)

# This is where add_executable(${PROJECT} ...) is called
if("${CMAKE_BUILD_PLATFORM}" STREQUAL "Device")
    include(utilities/cmake/firmware/firmware.cmake)
else() # Simulator or default
    include(utilities/cmake/simulator/simulator.cmake)
endif()

# This must come AFTER the add_executable call in the included files
target_include_directories( ${PROJECT} PRIVATE
    vendor/nanopb
    generated/proto
    vendor/mini-gmp
    vendor/poseidon/sources
)