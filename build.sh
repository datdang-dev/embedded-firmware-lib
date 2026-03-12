#!/bin/bash
# Manual build script for Embedded HSM C++

set -e

echo "=== Building Embedded HSM C++ ==="

SRC_DIR="src"
INC_DIR="include"
BUILD_DIR="build_manual"

mkdir -p $BUILD_DIR

CXX="g++"
CXXFLAGS="-std=c++17 -Wall -Wextra -Wpedantic -g -O0"
CFLAGS="-std=c11 -Wall -Wextra -g -O0"
LDFLAGS=""

INCLUDES="-I$INC_DIR"

echo "Compiling types..."
$CXX $CXXFLAGS $INCLUDES -c $SRC_DIR/types/status.cpp -o $BUILD_DIR/status.o

echo "Compiling MCAL (C)..."
gcc $CFLAGS $INCLUDES -c $SRC_DIR/mcal/uart/mcal_uart_stub.c -o $BUILD_DIR/mcal_uart_stub.o

echo "Compiling services..."
$CXX $CXXFLAGS $INCLUDES -c $SRC_DIR/services/crypto/crypto_service.cpp -o $BUILD_DIR/crypto_service.o
$CXX $CXXFLAGS $INCLUDES -c $SRC_DIR/services/crypto/aes128_algorithm.cpp -o $BUILD_DIR/aes128_algorithm.o
$CXX $CXXFLAGS $INCLUDES -c $SRC_DIR/services/crypto/aes256_algorithm.cpp -o $BUILD_DIR/aes256_algorithm.o
$CXX $CXXFLAGS $INCLUDES -c $SRC_DIR/services/keystore_service.cpp -o $BUILD_DIR/keystore_service.o

echo "Compiling middleware..."
$CXX $CXXFLAGS $INCLUDES -c $SRC_DIR/middleware/session_manager.cpp -o $BUILD_DIR/session_manager.o

echo "Compiling API..."
$CXX $CXXFLAGS $INCLUDES -c $SRC_DIR/api/hsm_api_impl.cpp -o $BUILD_DIR/hsm_api_impl.o

echo "Compiling main..."
$CXX $CXXFLAGS $INCLUDES -c main.cpp -o $BUILD_DIR/main.o

echo "Linking..."
$CXX $CXXFLAGS $BUILD_DIR/*.o -o $BUILD_DIR/ehsm_host $LDFLAGS

echo ""
echo "=== Build successful! ==="
echo "Run: ./$BUILD_DIR/ehsm_host"
