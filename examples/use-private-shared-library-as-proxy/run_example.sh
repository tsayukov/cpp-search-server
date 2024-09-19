#!/usr/bin/env bash

TEST_NAME="use-private-shared-library-as-proxy"

GENERATOR="Ninja"
CXX_COMPILER="clang++"

PROJECT_NAME="search_server"
PROJECT_NAMESPACE="SEARCH_SERVER"

PROJECT_BUILD_TYPE="Release"
PROJECT_SOURCE_DIR=$(realpath "../..")
PROJECT_BINARY_DIR="${PROJECT_SOURCE_DIR}/build/${TEST_NAME}/${PROJECT_NAME}"
PROJECT_INSTALL_DIR="${PROJECT_SOURCE_DIR}/.installed/shared-library-for-private-proxy/${PROJECT_NAME}"

TEST_BUILD_TYPE="Release"
TEST_SOURCE_DIR="."
TEST_BINARY_DIR="${PROJECT_SOURCE_DIR}/build/${TEST_NAME}"
TEST_INSTALL_DIR="${PROJECT_SOURCE_DIR}/.installed/${TEST_NAME}"

PRIVATE_LIBRARY_NAME="private-shared-library-as-proxy"
PRIVATE_LIBRARY_BUILD_TYPE="Release"
PRIVATE_LIBRARY_SOURCE_DIR="${TEST_SOURCE_DIR}/${PRIVATE_LIBRARY_NAME}"
PRIVATE_LIBRARY_BINARY_DIR="${PROJECT_SOURCE_DIR}/build/${PRIVATE_LIBRARY_NAME}"
PRIVATE_LIBRARY_INSTALL_DIR="${PROJECT_SOURCE_DIR}/.installed/${PRIVATE_LIBRARY_NAME}"

cat <<EOF
Run '${TEST_NAME}' test
Clean up binary and install directories
EOF

    rm -rf "${PROJECT_BINARY_DIR}"
    rm -rf "${PROJECT_INSTALL_DIR}"
    rm -rf "${PRIVATE_LIBRARY_BINARY_DIR}"
    rm -rf "${PRIVATE_LIBRARY_INSTALL_DIR}"
    rm -rf "${TEST_BINARY_DIR}"
    rm -rf "${TEST_INSTALL_DIR}"

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
'${PRIVATE_LIBRARY_NAME}' (configure and generate steps)

EOF

    cmake \
        --fresh \
        -G "${GENERATOR}" \
        -DCMAKE_BUILD_TYPE="${PRIVATE_LIBRARY_BUILD_TYPE}" \
        -DCMAKE_CXX_COMPILER="${CXX_COMPILER}" \
        -DCMAKE_PREFIX_PATH="${PROJECT_INSTALL_DIR}" \
        -DCMAKE_INSTALL_PREFIX="${PRIVATE_LIBRARY_INSTALL_DIR}" \
        -S "${PRIVATE_LIBRARY_SOURCE_DIR}" \
        -B "${PRIVATE_LIBRARY_BINARY_DIR}"

cat <<EOF

'${PRIVATE_LIBRARY_NAME}' (configure and generate steps) - done
'${PRIVATE_LIBRARY_NAME}' (build and install steps)

EOF

    cmake --build "${PRIVATE_LIBRARY_BINARY_DIR}" --target install

cat <<EOF

'${PRIVATE_LIBRARY_NAME}' (build and install steps) - done
'${TEST_NAME}' (configure and generate steps)

EOF

    cmake \
        --fresh \
        -G "${GENERATOR}" \
        -DCMAKE_BUILD_TYPE="${TEST_BUILD_TYPE}" \
        -DCMAKE_CXX_COMPILER="${CXX_COMPILER}" \
        -DCMAKE_PREFIX_PATH="${PRIVATE_LIBRARY_INSTALL_DIR};${PROJECT_INSTALL_DIR}" \
        -DCMAKE_INSTALL_PREFIX="${TEST_INSTALL_DIR}" \
        -S "${TEST_SOURCE_DIR}" \
        -B "${TEST_BINARY_DIR}"

cat <<EOF

'${TEST_NAME}' (configure and generate steps) - done
'${TEST_NAME}' (build and install steps)

EOF

    cmake --build "${TEST_BINARY_DIR}" --target install

cat <<EOF

'${TEST_NAME}' (build and install steps) - done
'${TEST_NAME}' (run the executable in the build tree)

EOF

    "${TEST_BINARY_DIR}/main"

cat <<EOF

'${TEST_NAME}' (run the installed executable)

EOF

    cp -a "${PROJECT_INSTALL_DIR}/lib/"* "${TEST_INSTALL_DIR}/bin/"
    cp -a "${PRIVATE_LIBRARY_INSTALL_DIR}/lib/"* "${TEST_INSTALL_DIR}/bin/"

    "${TEST_INSTALL_DIR}/bin/main"

cat <<EOF

Run '${TEST_NAME}' test - done
EOF
