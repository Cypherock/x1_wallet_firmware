#!/bin/bash

FOLDERS=(common/Firewall
    common/coin_support
    common/flash
    common/interfaces
    common/logger
    common/startup
    common/interfaces/card_interface
    common/interfaces/desktop_app_interface
    common/interfaces/flash_interface
    common/interfaces/user_interface
    common/libraries/proof_of_work
    common/libraries/shamir
    common/libraries/util
    simulator
    src
    tests
)

FILES=(
    common/assert_conf.h
    common/assert_def.c
    common/lv_conf.h
)

for FOLDER in ${FOLDERS[*]}; do echo Formatting ${FOLDER}...; find "${FOLDER}" -regex '.*\.\(cpp\|hpp\|cu\|c\|h\)' | xargs -n 1 clang-format -i; done