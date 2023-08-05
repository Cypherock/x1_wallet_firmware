#!/usr/bin/env bash

if [ $# -lt 1 ]
then
  echo "USAGE: $0 <GIT_COMMIT_ID>"
  echo "This script takes GIT_COMMIT_ID as only parameter"
  exit 1
fi

GIT_COMMIT_ID=$1
CHANGED_FILES=$(git diff --name-only "$GIT_COMMIT_ID"...HEAD)

CHANGED=0

for file in $CHANGED_FILES
do
  [ -e "$file" ] || continue
  case "$file" in

  common/lvgl/*|common/libraries/atecc/*|common/libraries/crypto/*|common/libraries/nanopb/*|tests/framework/*) continue ;;

  *.c|*.h|*.cpp|*.hpp)
    echo Checking "$file"
    PATCH="$(basename "$file").patch"
    git clang-format -q --style file --diff "$GIT_COMMIT_ID" "$file" > "$PATCH"
    if [ -s "$PATCH" ]
    then
      printf "\033[31mError:\033[0m Formatting error in %s\n" "$file"
      CHANGED=1
    else
      rm "$PATCH"
    fi
  esac
done

if [ $CHANGED = 1 ]
then
  cat *.patch
  printf "\033[31mError:\033[0m Issues found. You may use the patches provided as artifacts to format the code."
  exit 1
fi

exit 0
