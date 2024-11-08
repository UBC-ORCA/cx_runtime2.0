#!/usr/bin/env bash

#####
# NOTE: This is a very slow build (2h+) due to gnu toolchain
#####

pushd $CX2_ROOT/utils/riscv-gnu-toolchain
git submodule update --init binutils gcc gdb glibc linux-headers newlib dejagnu

if [ $? ne 0 ]; then
    echo "issue with cloning riscv toolchain submodules"
    exit 1
fi

if [ -d build ]; then
    rm -rf build/
fi

mkdir build
pushd build
../configure --prefix=$CX2_ROOT/utils/riscv --with-arch=rv32imav --with-abi=ilp32

if [ $? -ne 0 ]; then
	echo "issue with configuring gnu build"
    exit 1
fi

# Note: changing this to have multiple jobs causes the build to fail
make linux

if [ $? -ne 0 ]; then
	echo "issue with building gnu"
    exit 1
fi

popd

popd
