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

project(${PROJECT})

# python is needed for compiling proto files using nanopb
# also for generating & appending firmware signature headers
find_package( Python3 REQUIRED COMPONENTS Interpreter )
execute_process(COMMAND sh utilities/proto/generate-protob.sh WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMAND_ERROR_IS_FATAL ANY )

# Populate version.c
include(utilities/cmake/version.cmake)

file(GLOB_RECURSE PROTO_SRCS "generated/proto/*.*")
list(APPEND PROTO_SRCS "vendor/nanopb/pb_common.c" "vendor/nanopb/pb_decode.c" "vendor/nanopb/pb_encode.c" "vendor/nanopb/pb_common.h" "vendor/nanopb/pb_decode.h" "vendor/nanopb/pb_encode.h" "vendor/nanopb/pb.h")

OPTION(DEV_SWITCH "Additional features/logs to aid developers" OFF)
OPTION(UNIT_TESTS_SWITCH "Compile build for main firmware or unit tests" OFF)

# Make static functions testable via unit-tests
IF(UNIT_TESTS_SWITCH)
    add_compile_definitions( STATIC= )
ELSE()
    add_compile_definitions( STATIC=static )
ENDIF(UNIT_TESTS_SWITCH)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    add_compile_definitions(FIRMWARE_HASH_CALC=1)
else()
    add_compile_definitions(FIRMWARE_HASH_CALC=0)
endif()

if("${CMAKE_BUILD_PLATFORM}" STREQUAL "Device")
    include(utilities/cmake/firmware/firmware.cmake)
else()
    include(utilities/cmake/simulator/simulator.cmake)
endif()

# Include nanopb source headers
target_include_directories( ${EXECUTABLE} PRIVATE vendor/nanopb generated/proto )

# Enable support for dynamically allocated fields in nanopb
# Ref: vendor/nanopb/pb.h
add_compile_definitions(PB_ENABLE_MALLOC=1 PB_NO_ERRMSG=1)