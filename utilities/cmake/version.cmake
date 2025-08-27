execute_process(COMMAND git log --pretty=format:'%h' -n 1
                OUTPUT_VARIABLE GIT_REV
                ERROR_QUIET)
message(STATUS "Populating ${CMAKE_BINARY_DIR}/version.h")
# Check whether we got any revision (which isn't
# always the case, e.g. when someone downloaded a zip
# file from Github instead of a checkout)
if ("${GIT_REV}" STREQUAL "")
    set(GIT_REV "N/A")
    set(GIT_DIFF "")
    set(GIT_TAG "N/A")
    set(GIT_BRANCH "N/A")
else()
    execute_process(
        COMMAND sh -c "git diff --quiet --exit-code || echo +"
        OUTPUT_VARIABLE GIT_DIFF)
    execute_process(
        COMMAND git describe --exact-match --tags
        OUTPUT_VARIABLE GIT_TAG ERROR_QUIET)
    execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        OUTPUT_VARIABLE GIT_BRANCH)

    string(STRIP "${GIT_REV}" GIT_REV)
    string(SUBSTRING "${GIT_REV}" 1 7 GIT_REV)
    string(STRIP "${GIT_DIFF}" GIT_DIFF)
    string(STRIP "${GIT_TAG}" GIT_TAG)
    string(STRIP "${GIT_BRANCH}" GIT_BRANCH)
endif()

file(READ ${CMAKE_SOURCE_DIR}/version.txt VERSION_FILE_CONTENT)
string(REGEX MATCH "firmware version=([0-9]+):([0-9]+):([0-9]+):([0-9]+)" FW_VERSION_LINE ${VERSION_FILE_CONTENT})
set(FW_MAJOR ${CMAKE_MATCH_1})
set(FW_MINOR ${CMAKE_MATCH_2})
set(FW_PATCH ${CMAKE_MATCH_3})
set(FW_BUILD ${CMAKE_MATCH_4})
set(FIRMWARE_VERSION "${FW_MAJOR}.${FW_MINOR}.${FW_PATCH}.${FW_BUILD}")

if(BTC_ONLY)
    set(FIRMWARE_VARIANT "btc_only")
    set(VARIANT_ID 1)
else()
    set(FIRMWARE_VARIANT "multicoin")
    set(VARIANT_ID 0)
endif()

set(VERSION_HEADER "#ifndef VERSION_H\n#define VERSION_H\
extern const char* GIT_REV;
extern const char* GIT_TAG;
extern const char* GIT_BRANCH;
extern const char* FIRMWARE_VERSION;
extern const char* FIRMWARE_VARIANT;
extern const int VARIANT_ID;

#endif // VERSION_H
")

set(VERSION_SOURCE "#include <version.h>\n
const char* GIT_REV=\"${GIT_REV}${GIT_DIFF}\";
const char* GIT_TAG=\"${GIT_TAG}\";
const char* GIT_BRANCH=\"${GIT_BRANCH}\";
const char* FIRMWARE_VERSION=\"${FIRMWARE_VERSION}\";
const char* FIRMWARE_VARIANT=\"${FIRMWARE_VARIANT}\";
const int VARIANT_ID=${VARIANT_ID};\n")

if(EXISTS ${CMAKE_BINARY_DIR}/version.h)
    file(READ ${CMAKE_BINARY_DIR}/version.h VERSION_)
else()
    set(VERSION_ "")
endif()

if (NOT "${VERSION_HEADER}" STREQUAL "${VERSION_}")
    file(WRITE ${CMAKE_BINARY_DIR}/version.h "${VERSION_HEADER}")
endif()

if(EXISTS ${CMAKE_BINARY_DIR}/version.c)
    file(READ ${CMAKE_BINARY_DIR}/version.c VERSION_C_)
else()
    set(VERSION_C_ "")
endif()

if (NOT "${VERSION_SOURCE}" STREQUAL "${VERSION_C_}")
    file(WRITE ${CMAKE_BINARY_DIR}/version.c "${VERSION_SOURCE}")
endif()