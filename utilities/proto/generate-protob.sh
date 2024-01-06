#!/usr/bin/env bash

set -e

NANOPB_GEN="$(pwd)/vendor/nanopb/generator/nanopb_generator.py"

# Generate protobuf sources using nanopb
PYTHON_VERSION="$(python3 --version)" || exit 1
OUTPUT_DIR="$(pwd)/generated/proto"
PROTO_SRC="$(pwd)/common/cypherock-common/proto"
OPTIONS_DIR="$(pwd)/common/proto-options/"

test -d "${PROTO_SRC}"
test -f "${NANOPB_GEN}"

echo -e "Detected ${PYTHON_VERSION} (at $(which python3))"
mkdir -p "${OUTPUT_DIR}" || exit 1
cd "${PROTO_SRC}"

# --generated-include-format specifies the include format. Two viable options
# are "#include <%s>" and "#include \"proto/%s\"".
# The default include format "#include \"%s\"" creates problems in cases of
# duplicate file names residing under different packages (eg. error.proto and
# btc.error.proto) as it would result into a collision.
# By using <> format instead of "proto/" helps because in future, relocation
# of generated files would not require updating this parameter.
python3 "${NANOPB_GEN}" -q --generated-include-format "#include <%s>" --proto-path="${PROTO_SRC}" --options-path="${OPTIONS_DIR}"  $(find "${PROTO_SRC}" -name "*.proto") --output-dir="${OUTPUT_DIR}" --c-style -s anonymous_oneof:true -s long_names:false