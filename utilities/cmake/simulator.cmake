file(GLOB_RECURSE SOURCES "simulator/*.*" "common/*.*" "src/*.*")
add_compile_definitions(USE_SIMULATOR=1 ATCAPRINTF USE_MONERO=1 USE_BIP32_CACHE=0 USE_BIP39_CACHE=0)
IF (DEV_SWITCH)
    add_compile_definitions(DEV_BUILD)
ENDIF(DEV_SWITCH)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(EXECUTABLE ${PROJECT_NAME})
find_package(SDL2 REQUIRED SDL2)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(${EXECUTABLE} ${SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/version.c ${INCLUDES})

if ("${FIRMWARE_TYPE}" STREQUAL "Main")
    add_compile_definitions(X1WALLET_INITIAL=0 X1WALLET_MAIN=1)
    target_include_directories(${PROJECT_NAME} PRIVATE
            main/config/
            )
elseif("${FIRMWARE_TYPE}" STREQUAL "Initial")
    add_compile_definitions(X1WALLET_INITIAL=1 X1WALLET_MAIN=0)
    target_include_directories(${PROJECT_NAME} PRIVATE
            initial/config/
            )
else()
    message(FATAL_ERROR "Firmware type not specified. Specify using -DFIRMWARE_TYPE=<Type> Type can be Main or Initial")
endif()
target_include_directories(${PROJECT_NAME} PRIVATE
        src/

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
        common/libraries/crypto/aes
        common/libraries/crypto/chacha20poly1305
        common/libraries/crypto/ed25519-donna
        common/libraries/crypto/monero
        common/libraries/crypto/random_gen
        common/libraries/proof_of_work
        common/libraries/shamir
        common/libraries/util
        common/libraries/nanopb
        common/startup
        common/logger
        common/coin_support
        common/coin_support/eth_sign_data
        common/flash
        common/Firewall

        common
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

        # Simulator
        simulator
        simulator/BSP
        simulator/Buzzer
        simulator/Flash
        simulator/nfc
        simulator/lv_drivers
        simulator/lv_drivers/display
        simulator/lv_drivers/indev
        simulator/porting
        simulator/USB
        )

target_link_libraries(${EXECUTABLE} PRIVATE ${SDL2_LIBRARIES} -lm)
add_custom_target (run COMMAND ${EXECUTABLE_OUTPUT_PATH}/${EXECUTABLE})
