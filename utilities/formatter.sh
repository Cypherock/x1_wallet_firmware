#!/usr/bin/env bash

FOLDERS=(common/Firewall
    common/coin_support
    common/flash
    common/interfaces
    common/logger
    common/startup
    common/libraries/proof_of_work
    common/libraries/shamir
    common/libraries/util
    simulator
    src
)

FILES=(
    common/assert_conf.h
    common/assert_def.c
    common/lv_conf.h
)

for FOLDER in ${FOLDERS[*]}; do echo Formatting ${FOLDER}...; find "${FOLDER}" -regex '.*\.\(cpp\|hpp\|cu\|c\|h\)' | xargs -n 1 clang-format --style file -i; done
for FILE in ${FILES[*]}; do echo Formatting ${FILE}...; clang-format --style file -i "${FILE}"; done