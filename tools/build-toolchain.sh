#!/bin/bash
# Script to build cross-compiler toolchain for x86_64

set -e

# Configuration
TARGET=x86_64-elf
BINUTILS_VERSION=2.41
GCC_VERSION=13.2.0

# Download sources
wget https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.gz
wget https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz

# Extract
tar xf binutils-$BINUTILS_VERSION.tar.gz
tar xf gcc-$GCC_VERSION.tar.gz

# Build binutils
mkdir build-binutils
cd build-binutils
../binutils-$BINUTILS_VERSION/configure --target=$TARGET --prefix=/usr/local \
    --disable-nls --disable-werror
make -j$(nproc)
sudo make install
cd ..

# Build GCC
mkdir build-gcc
cd build-gcc
../gcc-$GCC_VERSION/configure --target=$TARGET --prefix=/usr/local \
    --disable-nls --enable-languages=c --without-headers
make all-gcc -j$(nproc)
make all-target-libgcc -j$(nproc)
sudo make install-gcc
sudo make install-target-libgcc
cd ..

# Cleanup
rm -rf build-* binutils-* gcc-* *.tar.gz

echo "Cross-compiler build complete"
