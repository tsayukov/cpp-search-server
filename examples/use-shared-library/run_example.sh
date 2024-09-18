#!/usr/bin/env bash

TEST_NAME="use-shared-library"

GENERATOR="Ninja"
CXX_COMPILER="clang++"

PROJECT_NAME="search_server"
PROJECT_NAMESPACE="SEARCH_SERVER"

PROJECT_BUILD_TYPE="Release"
PROJECT_SOURCE_DIR="../.."
PROJECT_BINARY_DIR="${PROJECT_SOURCE_DIR}/build/${TEST_NAME}/${PROJECT_NAME}"
PROJECT_INSTALL_DIR="${PROJECT_SOURCE_DIR}/.installed/${TEST_NAME}"

TEST_BUILD_TYPE="Release"
TEST_SOURCE_DIR="."
TEST_BINARY_DIR="${PROJECT_SOURCE_DIR}/build/${TEST_NAME}"

cat <<EOF
Run '${TEST_NAME}' test
Clean up binary and install directories
EOF

    rm -rf "${PROJECT_BINARY_DIR}"
    rm -rf "${PROJECT_INSTALL_DIR}"
    rm -rf "${TEST_BINARY_DIR}"

cat <<EOF
Clean up binary and install directories - done
'${PROJECT_NAME}' (configure and generate steps)

EOF

    cmake \
        --fresh \
        -G "${GENERATOR}" \
        -DCMAKE_BUILD_TYPE="${PROJECT_BUILD_TYPE}" \
        -DCMAKE_CXX_COMPILER="${CXX_COMPILER}" \
        -DCMAKE_INSTALL_PREFIX="${PROJECT_INSTALL_DIR}" \
        -D${PROJECT_NAMESPACE}_BUILD_SHARED_LIBS=ON \
        -S "${PROJECT_SOURCE_DIR}" \
        -B "${PROJECT_BINARY_DIR}"

cat <<EOF

'${PROJECT_NAME}' (configure and generate steps) - done
'${PROJECT_NAME}' (build and install steps)

EOF

    cmake --build "${PROJECT_BINARY_DIR}" --target install

cat <<EOF

'${PROJECT_NAME}' (build and install steps) - done
'${TEST_NAME}' (configure and generate steps)

EOF

    cmake \
        --fresh \
        -G "${GENERATOR}" \
        -DCMAKE_BUILD_TYPE="${TEST_BUILD_TYPE}" \
        -DCMAKE_CXX_COMPILER="${CXX_COMPILER}" \
        -DCMAKE_PREFIX_PATH="${PROJECT_INSTALL_DIR}" \
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
