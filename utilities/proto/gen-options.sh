#!/usr/bin/env bash

#set -x

function wrong_dir {
  echo "Run from root directory"
  exit 1
}

test -d common/cypherock-common/proto || wrong_dir

# Make sure all directories exist for the proto-options
source_dir="common/cypherock-common/proto"
destination_dir="common/proto-options/"

mkdir -p "$destination_dir"

# Find all directories in the source directory
find "$source_dir" -type d -print0 | while IFS= read -r -d '' dir; do
    # Create corresponding directories in the destination directory
    rel_path="${dir#$source_dir}"
    mkdir -p "$destination_dir/$rel_path"
done

FILES=$(find common/cypherock-common/proto -name '*.proto' | cut -f -1 -d "." )
PWD="$(pwd)"

PKG_PATTERN="^ *(package) (.*) *;"
MSG_PATTERN="^ *(message) (.*) {"
MEMBER_PATTERN="^ *(repeated )?(string|bytes)?(.*){1} (.*) ="

# Below is an optimized version of above pattern. But is not supported by bash
# PKG_PATTERN="^ *(package) (\S*) *;"
# MSG_PATTERN="^ *(message) (\S*) {"
# MEMBER_PATTERN="^ *(repeated )?(string|bytes)?(\S*){1} (\S*) ="


PKG=""
MSG=""
MEMBER=""
PROTO_FILE=""
OPT_FILE=""

function search_line {
  LINE="${1}"
  if [[ "${LINE}" =~ $PKG_PATTERN ]]; then
    PKG=${BASH_REMATCH[2]}
  elif [[ "${LINE}" =~ $MSG_PATTERN ]]; then
    MSG=${BASH_REMATCH[2]}
  elif [[ "${LINE}" =~ $MEMBER_PATTERN ]]; then
    MEMBER=${BASH_REMATCH[4]}
    if [[ ! ${BASH_REMATCH[1]} = "" ]] || [[ ! ${BASH_REMATCH[2]} = "" ]]; then
      echo "${PKG}.${MSG}.${MEMBER} type:FT_POINTER" >> "${OPT_FILE}"
    fi
  fi
}

for FILE in ${FILES}; do
  PROTO_FILE="${PWD}/${FILE}.proto"
  OPT_FILE="${PWD}/"${FILE/"cypherock-common/proto"/proto-options}".options"
  echo "# Options for file ${FILE}.proto" > "${OPT_FILE}"
  while IFS= read -r line; do
    search_line "${line}"
  done < "${PROTO_FILE}"
done