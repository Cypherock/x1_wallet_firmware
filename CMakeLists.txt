cmake_minimum_required(VERSION 3.19.0)
set(CMAKE_MESSAGE_LOG_LEVEL debug)

if("${CMAKE_BUILD_PLATFORM}" STREQUAL "Device")
    message("Device platform selected")
    set(CMAKE_TOOLCHAIN_FILE utilities/cmake/arm-none-eabi-gcc.cmake)
    set(PROJECT Cypherock-${FIRMWARE_TYPE})
elseif("${CMAKE_BUILD_PLATFORM}" STREQUAL "Simulator")
    message("Simulator platform selected")
    set(PROJECT Cypherock_Simulator)
else()
    message(WARNING "No platform specified defaulting to Simulator. Specify using -DCMAKE_BUILD_PLATFORM=<Type> Type can be Device or Simulator")
    set(PROJECT Cypherock_Simulator)
endif()

# Define all options upfront, including BTC_ONLY
OPTION(DEV_SWITCH "Additional features/logs to aid developers" OFF)
OPTION(UNIT_TESTS_SWITCH "Compile build for main firmware or unit tests" OFF)
OPTION(BTC_ONLY "Build firmware for Bitcoin only" OFF)

# Logic to append -btc to project name if BTC_ONLY is enabled
if (BTC_ONLY)
    set(PROJECT ${PROJECT}-btc)
endif()

# Make static functions testable via unit-tests
IF(UNIT_TESTS_SWITCH)
    add_compile_definitions( STATIC= )
ELSE()
    add_compile_definitions( STATIC=static )
ENDIF(UNIT_TESTS_SWITCH)

# Now define the project with its final name, after all modifications
project(${PROJECT})

# python is needed for compiling proto files using nanopb
# also for generating & appending firmware signature headers
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

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    add_compile_definitions(FIRMWARE_HASH_CALC=1)
else()
    add_compile_definitions(FIRMWARE_HASH_CALC=0)
endif()

if("${CMAKE_BUILD_PLATFORM}" STREQUAL "Device")
    include(utilities/cmake/firmware/firmware.cmake)
else() # Simulator or default
    include(utilities/cmake/simulator/simulator.cmake)
endif()

# Include nanopb source headers
target_include_directories( ${EXECUTABLE} PRIVATE
    vendor/nanopb
    generated/proto
    vendor/mini-gmp
    vendor/poseidon/sources
)

# Enable support for dynamically allocated fields in nanopb
# Ref: vendor/nanopb/pb.h
add_compile_definitions(PB_ENABLE_MALLOC=1 PB_NO_ERRMSG=1)
