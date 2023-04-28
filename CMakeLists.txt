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

# python is needed for compiling proto files using nanopb
# also for generating & appending firmware signature headers
find_package( Python3 REQUIRED COMPONENTS Interpreter )

# Integrate nanopb
file(GLOB_RECURSE PROTO_SRC common/protob/proto/*.proto)

# --generated-include-format specifies the include format. Two viable options
# are "#include <%s>" and "#include \"proto/%s\"".
# The default include format "#include \"%s\"" creates problems in cases of
# duplicate file names residing under different packages (eg. error.proto and
# btc.error.proto) as it would result into a collision.
# By using <> format instead of "proto/" helps because in future, relocation
# of generated files would not require updating this parameter.
set(NANOPB_OPTIONS ",--generated-include-format=#include <%s>")
set(NANOPB_SRC_ROOT_FOLDER ${PROJECT_SOURCE_DIR}/vendor/nanopb)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${NANOPB_SRC_ROOT_FOLDER}/extra)
find_package( Nanopb REQUIRED )

NANOPB_GENERATE_CPP(PROTO_SRCS PROTO_HDRS RELPATH ${PROJECT_SOURCE_DIR}/common/protob/proto ${PROTO_SRC})

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

# Include nanopb (at ${NANOPB_INCLUDE_DIRS}) and protoc-generated (at
# ${CMAKE_CURRENT_BINARY_DIR}) headers. Ref: vendor/nanopb/extra/FindNanopb.cmake#63
target_include_directories( ${EXECUTABLE} PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${NANOPB_INCLUDE_DIRS} )

# Enable support for dynamically allocated fields in nanopb
# Ref: vendor/nanopb/pb.h
add_compile_definitions(PB_ENABLE_MALLOC=1)