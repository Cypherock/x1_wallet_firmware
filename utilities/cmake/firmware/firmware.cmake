enable_language(C ASM)
# The EXECUTABLE variable is removed. We will use ${PROJECT} directly,
# which is passed down from the parent CMakeLists.txt and holds the correct name (e.g., "Cypherock-Main-btc").
set(LINKER_SCRIPT STM32L486RGTX_FLASH.ld)
set(STARTUP_FILE startup_stm32l486xx.s)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Define base source files (always built)
file(GLOB_RECURSE CORE_STM_HAL_SOURCES "stm32-hal/*.*")
file(GLOB_RECURSE CORE_SRC_SOURCES "src/*.*")

# Common files from 'common/' subdirectories that are always needed
file(GLOB_RECURSE CORE_COMMON_INTERFACES_SOURCES "common/interfaces/*.*")
file(GLOB_RECURSE CORE_COMMON_LIBRARIES_SOURCES "common/libraries/*.*")
file(GLOB_RECURSE CORE_COMMON_STARTUP_SOURCES "common/startup/*.*")
file(GLOB_RECURSE CORE_COMMON_LOGGER_SOURCES "common/logger/*.*")
file(GLOB_RECURSE CORE_COMMON_FLASH_SOURCES "common/flash/*.*")
file(GLOB_RECURSE CORE_COMMON_FIREWALL_SOURCES "common/Firewall/*.*")
file(GLOB_RECURSE CORE_COMMON_TIMERS_SOURCES "common/timers/*.*")
# For LVGL, source files are primarily in its 'src' subdirectory.
# common/lvgl/lvgl.h is picked up by include path. common/lv_conf.h by 'common/' include path.
file(GLOB_RECURSE CORE_COMMON_LVGL_SOURCES "common/lvgl/src/**/*.c" "common/lvgl/src/**/*.h")

# Source files from common/core/ (assumed common or internally guarded)
file(GLOB_RECURSE CORE_COMMON_CORE_SOURCES "common/core/*.c")

set(CORE_COMMON_GENERAL_SOURCES_FROM_SUBDIRS
    ${CORE_COMMON_INTERFACES_SOURCES}
    ${CORE_COMMON_LIBRARIES_SOURCES}
    ${CORE_COMMON_STARTUP_SOURCES}
    ${CORE_COMMON_LOGGER_SOURCES}
    ${CORE_COMMON_FLASH_SOURCES}
    ${CORE_COMMON_FIREWALL_SOURCES}
    ${CORE_COMMON_TIMERS_SOURCES}
    ${CORE_COMMON_LVGL_SOURCES}
    ${CORE_COMMON_CORE_SOURCES}
)

# Specific common files that are always compiled.
# These are assumed to have internal guards for different build configurations
# or are genuinely generic.
set(CORE_COMMON_SPECIFIC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/common/assert_def.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common/coin_support/coin_specific_data.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common/coin_support/coin_utils.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/common/coin_support/wallet.c"
    # common/core/core_flow_init.c is now covered by CORE_COMMON_CORE_SOURCES glob.
    # Ensure it and other common/core/*.c files have internal guards where needed.
)

set(BASE_SOURCES
    ${CORE_STM_HAL_SOURCES}
    ${CORE_SRC_SOURCES}
    ${CORE_COMMON_GENERAL_SOURCES_FROM_SUBDIRS}
    ${CORE_COMMON_SPECIFIC_FILES}
)

# Define app-specific sources
file(GLOB_RECURSE MANAGER_APP_SOURCES "apps/manager_app/*.*")
file(GLOB_RECURSE BTC_FAMILY_SOURCES "apps/btc_family/*.*")

# Full build: include all apps' source files
file(GLOB_RECURSE EVM_FAMILY_SOURCES "apps/evm_family/*.*")
file(GLOB_RECURSE NEAR_APP_SOURCES "apps/near_app/*.*")
file(GLOB_RECURSE SOLANA_APP_SOURCES "apps/solana_app/*.*")
file(GLOB_RECURSE TRON_APP_SOURCES "apps/tron_app/*.*")
file(GLOB_RECURSE INHERITANCE_APP_SOURCES "apps/inheritance_app/*.*")
file(GLOB_RECURSE EXCHANGE_APP_SOURCE "apps/exchange_app/*.*")
file(GLOB_RECURSE CONSTELLATION_APP_SOURCE "apps/constellation_app/*.*")
file(GLOB_RECURSE STARKNET_APP_SOURCES "apps/starknet_app/*.*")
file(GLOB_RECURSE XRP_APP_SOURCES "apps/xrp_app/*.*")
file(GLOB_RECURSE ICP_APP_SOURCES "apps/icp_app/*.*")

# Define sources from common/coin_support that are specific to non-BTC builds
set(COMMON_NON_BTC_SOURCES "")
IF(NOT BTC_ONLY)
    # Source files from common/coin_support/eth_sign_data/
    set(ETH_SIGN_DATA_COMMON_SRCS "")
    file(GLOB ETH_SIGN_DATA_COMMON_SRCS_TMP "common/coin_support/eth_sign_data/*.c")
    foreach(file ${ETH_SIGN_DATA_COMMON_SRCS_TMP})
        list(APPEND ETH_SIGN_DATA_COMMON_SRCS "${file}")
    endforeach()

    # Source files from common/coin_support/tron_parse_txn/
    set(TRON_PARSE_TXN_COMMON_SRCS "")
    file(GLOB TRON_PARSE_TXN_COMMON_SRCS_TMP "common/coin_support/tron_parse_txn/*.c")
    foreach(file ${TRON_PARSE_TXN_COMMON_SRCS_TMP})
        list(APPEND TRON_PARSE_TXN_COMMON_SRCS "${file}")
    endforeach()

    list(APPEND COMMON_NON_BTC_SOURCES
        ${ETH_SIGN_DATA_COMMON_SRCS}
        ${TRON_PARSE_TXN_COMMON_SRCS}
        "${CMAKE_CURRENT_SOURCE_DIR}/common/coin_support/eth.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/common/coin_support/solana.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/common/coin_support/near.c"
        # Add other .c files from common/coin_support that are exclusively for non-BTC chains if any
    )
ENDIF()


IF(BTC_ONLY)
    # BTC-only build: include only Bitcoin-related apps
    set(APP_SOURCES
    ${MANAGER_APP_SOURCES}
    ${BTC_FAMILY_SOURCES}
    ${INHERITANCE_APP_SOURCES}
)
ELSE()
    set(APP_SOURCES
        ${MANAGER_APP_SOURCES}
        ${BTC_FAMILY_SOURCES}
        ${EVM_FAMILY_SOURCES}
        ${NEAR_APP_SOURCES}
        ${SOLANA_APP_SOURCES}
        ${EXCHANGE_APP_SOURCE}
        ${TRON_APP_SOURCES}
        ${INHERITANCE_APP_SOURCES}
        ${STARKNET_APP_SOURCES}
        ${XRP_APP_SOURCES}
        ${ICP_APP_SOURCES}
        ${CONSTELLATION_APP_SOURCE})
ENDIF(BTC_ONLY)

IF(UNIT_TESTS_SWITCH)
        # Add test sources
        file(GLOB_RECURSE TEST_SOURCES "tests/*.*")
        set(SOURCES ${BASE_SOURCES} ${APP_SOURCES} ${COMMON_NON_BTC_SOURCES} ${TEST_SOURCES})

        #exclude src/main.c from the compilation list as it needs to be overriden by unit_tests_main.c
        LIST(REMOVE_ITEM SOURCES "${PROJECT_SOURCE_DIR}/src/main.c")

        #need these macros to correctly configure unity test framework
        add_compile_definitions(UNITY_INCLUDE_CONFIG_H)
        add_compile_definitions(UNITY_FIXTURE_NO_EXTRAS)
ELSE()
        set(SOURCES ${BASE_SOURCES} ${APP_SOURCES} ${COMMON_NON_BTC_SOURCES})
ENDIF(UNIT_TESTS_SWITCH)

add_executable(${PROJECT}
    ${SOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/version.c
    ${MINI_GMP_SRCS}
    ${POSEIDON_SRCS}
    ${PROTO_SRCS}
    ${PROTO_HDRS}
    ${INCLUDES}
    ${LINKER_SCRIPT}
    ${STARTUP_FILE})

target_compile_definitions(${PROJECT} PRIVATE
    -DUSE_HAL_DRIVER
    -DSTM32L486xx )

# NOTE: The compile definitions for DEV_SWITCH, BTC_ONLY_BUILD, and FIRMWARE_TYPE
# have been removed from this file as they are now centrally managed in the
# main CMakeLists.txt. This avoids redundancy and ensures a single source of truth.
add_compile_definitions(
    USE_SIMULATOR=0
    USE_BIP32_CACHE=0
    USE_BIP39_CACHE=0
    STM32L4
    USBD_SOF_DISABLED
    ENABLE_HID_WEBUSB_COMM=1)

if ("${FIRMWARE_TYPE}" STREQUAL "Main")
    target_include_directories(${PROJECT} PRIVATE
            main/config/
            )
elseif("${FIRMWARE_TYPE}" STREQUAL "Initial")
    target_include_directories(${PROJECT} PRIVATE
            initial/config/
            )
else()
    message(FATAL_ERROR "Firmware type not specified. Specify using -DFIRMWARE_TYPE=<Type> Type can be Main or Initial")
endif()

# Base include directories (always included)
target_include_directories(${EXECUTABLE} PRIVATE
        ${CMAKE_BINARY_DIR}

        apps/manager_app # Manager app is always included

        src/
        src/menu
        src/wallet
        src/restricted_app
        src/onboarding
        src/settings
        src/card_operations
        src/card_flows
        src/level_one/controller
        src/level_one/tasks
        src/level_two/controller
        src/level_two/tasks
        src/level_three/add_wallet/controller
        src/level_three/add_wallet/tasks
        src/level_three/advanced_settings/controller
        src/level_three/advanced_settings/tasks
        src/level_three/old_wallet/controller
        src/level_three/old_wallet/tasks
        src/level_four/core/controller
        src/level_four/core/tasks
        src/level_four/card_health_check/
        src/level_four/factory_reset/
        src/level_four/tap_cards/controller
        src/level_four/tap_cards/tasks

        common
        common/interfaces/card_interface
        common/interfaces/desktop_app_interface
        common/interfaces/flash_interface
        common/interfaces/user_interface
        common/libraries/atecc
        common/libraries/atecc/atcacert
        common/libraries/atecc/basic
        common/libraries/atecc/crypto
        common/libraries/atecc/crypto/hashes
        common/libraries/atecc/hal
        common/libraries/atecc/host
        common/libraries/atecc/jwt
        common/libraries/crypto
        common/libraries/crypto/mpz_operations
        common/libraries/crypto/aes
        common/libraries/crypto/chacha20poly1305
        common/libraries/crypto/ed25519-donna
        common/libraries/crypto/monero
        common/libraries/crypto/random_gen
        common/libraries/proof_of_work
        common/libraries/shamir
        common/libraries/util
        common/startup
        common/logger
        common/coin_support
        common/flash
        common/Firewall
        common/core
        common/timers
        common/lvgl
        common/lvgl/porting
        common/lvgl/src
        common/lvgl/src/lv_core
        common/lvgl/src/lv_draw
        common/lvgl/src/lv_font
        common/lvgl/src/lv_hal
        common/lvgl/src/lv_misc
        common/lvgl/src/lv_objx
        common/lvgl/src/lv_themes

        # Device specific includes
        stm32-hal
        stm32-hal/BSP
        stm32-hal/Inc
        stm32-hal/Drivers/CMSIS/Include
        stm32-hal/Drivers/CMSIS/Device/ST/STM32L4xx/Include
        stm32-hal/Drivers/STM32L4xx_HAL_Driver/Inc
        stm32-hal/Drivers/STM32L4xx_HAL_Driver/Inc/Legacy
        stm32-hal/Drivers/STM32L4xx_HAL_Driver/Src
        stm32-hal/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc
        stm32-hal/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src
        stm32-hal/Middlewares/ST/STM32_USB_Device_Library/Core/Inc
        stm32-hal/Middlewares/ST/STM32_USB_Device_Library/Core/Src
        stm32-hal/Peripherals
        stm32-hal/Peripherals/Buzzer
        stm32-hal/Peripherals/display
        stm32-hal/Peripherals/display/SSD1306
        stm32-hal/Peripherals/flash/
        stm32-hal/Peripherals/logger/
        stm32-hal/Peripherals/nfc/
        stm32-hal/Src
        stm32-hal/Startup
        stm32-hal/porting
        stm32-hal/libusb/
        stm32-hal/libusb/inc
        stm32-hal/libusb/src

        #unit test framework includes (conditional on UNIT_TESTS_SWITCH)
        $<$<BOOL:UNIT_TESTS_SWITCH>:${PROJECT_SOURCE_DIR}/tests/framework/unity>
        $<$<BOOL:UNIT_TESTS_SWITCH>:${PROJECT_SOURCE_DIR}/tests/framework/unity/src>
        $<$<BOOL:UNIT_TESTS_SWITCH>:${PROJECT_SOURCE_DIR}/tests/framework/unity/extras/fixture/src>

        #unit test modules includes (conditional on UNIT_TESTS_SWITCH)
        $<$<BOOL:UNIT_TESTS_SWITCH>:${PROJECT_SOURCE_DIR}/tests>
        $<$<BOOL:UNIT_TESTS_SWITCH>:${PROJECT_SOURCE_DIR}/tests/common/core>
        $<$<BOOL:UNIT_TESTS_SWITCH>:${PROJECT_SOURCE_DIR}/tests/common/util>
        $<$<BOOL:UNIT_TESTS_SWITCH>:${PROJECT_SOURCE_DIR}/tests/p0_events>
        $<$<BOOL:UNIT_TESTS_SWITCH>:${PROJECT_SOURCE_DIR}/tests/ui/ui_events_test>
        $<$<BOOL:UNIT_TESTS_SWITCH>:${PROJECT_SOURCE_DIR}/tests/usb/events>
        $<$<BOOL:UNIT_TESTS_SWITCH>:${PROJECT_SOURCE_DIR}/tests/nfc/events>
        $<$<BOOL:UNIT_TESTS_SWITCH>:${PROJECT_SOURCE_DIR}/tests/nfc>
        $<$<BOOL:UNIT_TESTS_SWITCH>:${PROJECT_SOURCE_DIR}/tests/apps/manager_app>
        $<$<BOOL:UNIT_TESTS_SWITCH>:${PROJECT_SOURCE_DIR}/tests/apps/btc_app>
)

# Conditional include directories based on BTC_ONLY flag
IF(BTC_ONLY)
    # BTC-only build: include only Bitcoin family apps
    target_include_directories(${PROJECT} PRIVATE
        apps/btc_family
        apps/btc_family/btc
        apps/inheritance_app
    )
ELSE()
    # Full build: include all cryptocurrency apps and their specific common support includes
    target_include_directories(${PROJECT} PRIVATE
        apps/btc_family # BTC family is also part of full build
        apps/btc_family/btc
        apps/btc_family/dash
        apps/btc_family/doge
        apps/btc_family/ltc
        apps/evm_family
        apps/evm_family/eth
        apps/evm_family/polygon
        apps/evm_family/bsc
        apps/evm_family/fantom
        apps/evm_family/avalanche
        apps/evm_family/optimism
        apps/evm_family/arbitrum
        apps/near_app
        apps/solana_app
        apps/tron_app
        apps/inheritance_app
        apps/starknet_app
        apps/xrp_app
        apps/icp_app
        apps/constellation_app
        apps/exchange_app
        apps/stellar_app
        
        # Common coin support sub-module includes for non-BTC builds
        common/coin_support/eth_sign_data  # Headers for eth_sign_data module
        common/coin_support/tron_parse_txn # Headers for tron_parse_txn module
        # If other coin-specific helper headers (e.g., solana_txn_helpers.h, near_context.h)
        # reside in specific subdirectories under common/coin_support/, add those paths here.
        # If they are within the app-specific directories (e.g. apps/solana_app/),
        # those are already covered by the app include paths.

        # All unit tests for full build (conditional on UNIT_TESTS_SWITCH)
        $<$<BOOL:UNIT_TESTS_SWITCH>:${PROJECT_SOURCE_DIR}/tests/apps/evm_app>
        $<$<BOOL:UNIT_TESTS_SWITCH>:${PROJECT_SOURCE_DIR}/tests/apps/near_app>
        $<$<BOOL:UNIT_TESTS_SWITCH>:${PROJECT_SOURCE_DIR}/tests/apps/solana_app>
        $<$<BOOL:UNIT_TESTS_SWITCH>:${PROJECT_SOURCE_DIR}/tests/apps/inheritance_app>
        $<$<BOOL:UNIT_TESTS_SWITCH>:${PROJECT_SOURCE_DIR}/tests/apps/xrp_app>
        $<$<BOOL:UNIT_TESTS_SWITCH>:${PROJECT_SOURCE_DIR}/tests/apps/constellation_app>
        $<$<BOOL:UNIT_TESTS_SWITCH>:${PROJECT_SOURCE_DIR}/tests/apps/icp_app>
        $<$<BOOL:UNIT_TESTS_SWITCH>:${PROJECT_SOURCE_DIR}/tests/apps/stellar_app>
    )
ENDIF(BTC_ONLY)

target_compile_options(${PROJECT} PRIVATE
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
    -fdata-sections
    -ffunction-sections
    -Wall
    -Wno-format-truncation
    -Wno-unused-but-set-variable
    -Wno-return-type
    -D_POSIX_C_SOURCE=200809L
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Release>:-Werror>
)

target_link_options(${PROJECT} PRIVATE
    -T${CMAKE_SOURCE_DIR}/STM32L486RGTX_FLASH.ld
    -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16
    -mfloat-abi=hard -u _printf_float -lc -lm -lnosys
    -Wl,-Map=${PROJECT}.map,--cref -Wl,--gc-sections
)

# Used to suppress compile time warnings in libraries
file(GLOB_RECURSE LIBRARIES_SRC_DIR
    "common/libraries/atecc/*.c"
    "common/libraries/atecc/*.h"
    "common/lvgl/src/**/*.c"
    "common/lvgl/src/**/*.h"
    "common/libraries/crypto/*.c"
    "common/libraries/crypto/*.h"
    "stm32-hal/Peripherals/*.c"
    "stm32-hal/Peripherals/*.h")
set_source_files_properties(${LIBRARIES_SRC_DIR} PROPERTIES COMPILE_FLAGS "-w")

# Print executable size
add_custom_command(TARGET ${PROJECT}
    POST_BUILD
    COMMAND arm-none-eabi-size $<TARGET_FILE:${PROJECT}>)

# Create hex and bin files
add_custom_command(TARGET ${PROJECT}
    POST_BUILD
    COMMAND arm-none-eabi-objcopy -O ihex $<TARGET_FILE:${PROJECT}> ${PROJECT}.hex
    COMMAND arm-none-eabi-objcopy -O binary $<TARGET_FILE:${PROJECT}> ${PROJECT}.bin)

if (SIGN_BINARY)
    add_custom_command(TARGET ${PROJECT}
        POST_BUILD
        COMMAND python3 ${CMAKE_SOURCE_DIR}/utilities/script/index.py add-header --input="${PROJECT}.bin" --output=${PROJECT}_Header.bin --version=${CMAKE_SOURCE_DIR}/version.txt --private-key=${CMAKE_SOURCE_DIR}/utilities/script/private_key1.h
        COMMAND python3 ${CMAKE_SOURCE_DIR}/utilities/script/index.py sign-header --input=${PROJECT}_Header.bin --output=${PROJECT}-signed.bin --private-key=${CMAKE_SOURCE_DIR}/utilities/script/private_key2.h
        COMMAND rm ${PROJECT}_Header.bin)
endif()
