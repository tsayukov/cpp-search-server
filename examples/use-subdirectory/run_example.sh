#!/usr/bin/env bash

TEST_NAME="use-subdirectory"

GENERATOR="Ninja"
CXX_COMPILER="clang++"

PROJECT_NAME="search_server"

PROJECT_SOURCE_DIR="../.."

TEST_BUILD_TYPE="Release"
TEST_SOURCE_DIR="."
TEST_BINARY_DIR="${PROJECT_SOURCE_DIR}/build/${TEST_NAME}"

cat <<EOF
Run '${TEST_NAME}' test
Clean up binary directories
EOF

    rm -rf "${TEST_BINARY_DIR}"

cat <<EOF
Clean up binary directories - done
'${PROJECT_NAME}' (copying to subdirectory)
EOF

    rm -rf "${TEST_SOURCE_DIR}/search_server"
    mkdir -p "${TEST_SOURCE_DIR}/search_server"

    cp -r "${PROJECT_SOURCE_DIR}/cmake" "${TEST_SOURCE_DIR}/search_server/"
    cp -r "${PROJECT_SOURCE_DIR}/include" "${TEST_SOURCE_DIR}/search_server/"
    cp -r "${PROJECT_SOURCE_DIR}/src" "${TEST_SOURCE_DIR}/search_server/"
    cp "${PROJECT_SOURCE_DIR}/CMakeLists.txt" "${TEST_SOURCE_DIR}/search_server/CMakeLists.txt"

cat <<EOF
'${PROJECT_NAME}' (copying to subdirectory) - done
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
