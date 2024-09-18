#!/usr/bin/env bash

TEST_NAME="use-fetch-content"

GENERATOR="Ninja"
CXX_COMPILER="clang++"

PROJECT_NAME="search_server"

PROJECT_SOURCE_DIR="../.."

TEST_BUILD_TYPE="Release"
TEST_SOURCE_DIR="."
TEST_BINARY_DIR="${PROJECT_SOURCE_DIR}/build/${TEST_NAME}"

cat <<EOF
Run '${TEST_NAME}' test
Clean up binary directory
EOF

    rm -rf "${TEST_BINARY_DIR}"

cat <<EOF
Clean up binary directory - done
'${TEST_NAME}' (configure and generate steps)

EOF

    cmake \
        --fresh \
        -G "${GENERATOR}" \
        -DCMAKE_BUILD_TYPE="${TEST_BUILD_TYPE}" \
        -DCMAKE_CXX_COMPILER="${CXX_COMPILER}" \
        -S "${TEST_SOURCE_DIR}" \
        -B "${TEST_BINARY_DIR}"

cat <<EOF

'${TEST_NAME}' (configure and generate steps) - done
'${TEST_NAME}' (build step)

EOF

    cmake --build "${TEST_BINARY_DIR}"

cat <<EOF

'${TEST_NAME}' (build step) - done
'${TEST_NAME}' (run)

EOF

    "${TEST_BINARY_DIR}/main"

cat <<EOF

Run '${TEST_NAME}' test - done
EOF
