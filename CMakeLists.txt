cmake_minimum_required(VERSION 3.15.3)
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

# Populate version.c
include(utilities/cmake/version.cmake)

OPTION(DEV_SWITCH "Additional features/logs to aid developers" OFF)
OPTION(UNIT_TESTS_SWITCH "Compile build for main firmware or unit tests" OFF)

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