#!/usr/bin/env bash

set -e

# Default to a full build
BUILD_MODE="FULL"

# Check for the --btc-only flag
if [[ "$1" == "--btc-only" ]]; then
  BUILD_MODE="BTC_ONLY"
  echo "Build mode: BTC-only"
fi

# --- Define paths and tools relative to the project root ---
PROTO_COMMON_SRC="common/cypherock-common/proto"
PROTO_TRON="common/coin_support/tron_parse_txn"
OUTPUT_DIR="generated/proto"
OPTIONS_DIR="common/proto-options"
NANOPB_GEN="vendor/nanopb/generator/nanopb_generator.py"

# --- Validate paths and create output directory ---
test -d "${PROTO_COMMON_SRC}" || { echo "Error: Directory ${PROTO_COMMON_SRC} not found."; exit 1; }
echo -e "Detected $(python3 --version) (at $(which python3))"
mkdir -p "${OUTPUT_DIR}" || exit 1

# --- Select and Generate Protobufs ---
PROTO_FILES_TO_GENERATE=()

if [[ "${BUILD_MODE}" == "BTC_ONLY" ]]; then
    echo "Finding protobufs for BTC-only build..."
    # For a BTC-only build, find all .proto files within the essential directories.
    # This is robust and does not depend on specific filenames.
    BTC_ONLY_DIRS=(
        "${PROTO_COMMON_SRC}/manager"
        "${PROTO_COMMON_SRC}/btc"
        "${PROTO_COMMON_SRC}/inheritance"
    )
    PROTO_FILES_TO_GENERATE=($(find "${BTC_ONLY_DIRS[@]}" -name "*.proto"))
else
    echo "Finding all protobufs for full build..."
    # For a full build, find all .proto files in all subdirectories
    ALL_COMMON_PROTOS=($(find "${PROTO_COMMON_SRC}" -name "*.proto"))
    ALL_TRON_PROTOS=($(find "${PROTO_TRON}" -name "*.proto"))
    PROTO_FILES_TO_GENERATE=("${ALL_COMMON_PROTOS[@]}" "${ALL_TRON_PROTOS[@]}")
fi

echo "Generating selected protobuf files..."
python3 "${NANOPB_GEN}" \
    -q \
    --generated-include-format "#include <%s>" \
    --proto-path="${PROTO_COMMON_SRC}" \
    --proto-path="${PROTO_TRON}" \
    --options-path="${OPTIONS_DIR}" \
    --output-dir="${OUTPUT_DIR}" \
    --c-style -s anonymous_oneof:true -s long_names:false \
    "${PROTO_FILES_TO_GENERATE[@]}"

echo "Protobuf generation complete."