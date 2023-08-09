#!/usr/bin/env bash

# Uncomment the following line to disable clang-tidy checks
# exit 0

set -e

if [ $# -lt 1 ]
then
  echo "USAGE: $0 <GIT_COMMIT_ID>"
  echo "This script takes GIT_COMMIT_ID as only parameter"
  exit 1
fi

GIT_COMMIT_ID=$1
CHANGED_FILES=$(git diff --name-only "$GIT_COMMIT_ID"...HEAD)

for file in $CHANGED_FILES
do
  [ -e "$file" ] || continue
  case "$file" in

  common/lvgl/*|common/libraries/atecc/*|common/libraries/crypto/*|common/libraries/nanopb/*|tests/framework/*) continue ;;

  *.c|*.h|*.cpp|*.hpp)
    echo "::group::$file"
    clang-tidy -p build "$file" >> anaylysis.results || true
    echo "::endgroup::"
  esac
done

exit 0
