if [ ! -d initramfs/home ]; then
    exit 1
fi

pushd initramfs
pushd home
# riscv32-unknown-linux-gnu-gcc -static `pwd`/../../../examples/open.c -o open -L `pwd`/../../../build/lib/ -lci
# riscv32-unknown-linux-gnu-gcc -static `pwd`/../../../examples/cx_reg.c -o reg -L `pwd`/../../../build/lib/ -lci
riscv32-unknown-linux-gnu-gcc -static `pwd`/../../../examples/fork.c -o f -L `pwd`/../../../build/lib/ -lci
# riscv32-unknown-linux-gnu-gcc -static `pwd`/../../../examples/open.c -o open_m -L $CX2_ROOT/build/lib/ -lci_m
# riscv32-unknown-linux-gnu-gcc -static `pwd`/../../../examples/vector_test.c -o vector -L $CX2_ROOT/build/lib/ -lci_m

riscv32-unknown-linux-gnu-gcc -static `pwd`/../../../examples/cx_open.c -o cx_open -L $CX2_ROOT/build/lib/ -lci
riscv32-unknown-linux-gnu-gcc -static `pwd`/../../../examples/state_test.c -o state -L $CX2_ROOT/build/lib/ -lci
riscv32-unknown-linux-gnu-gcc -static `pwd`/../../../examples/stateless.c -o sl -L $CX2_ROOT/build/lib/ -lci
riscv32-unknown-linux-gnu-gcc -static `pwd`/../../../examples/intra_virt.c -o iv -L $CX2_ROOT/build/lib/ -lci
riscv32-unknown-linux-gnu-gcc -static `pwd`/../../../examples/threaded_test.c -o tt -L $CX2_ROOT/build/lib/ -lci -pthread


popd
find . -print0 | cpio --null -ov --format=newc | gzip -9 > initramfs.cpio.gz
popd
