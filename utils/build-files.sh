if [ ! -d initramfs/home ]; then
    exit 1
fi

pushd initramfs
pushd home
riscv32-unknown-linux-gnu-gcc -static `pwd`/../../../examples/open.c -o open -L `pwd`/../../../build/lib/ -lci
riscv32-unknown-linux-gnu-gcc -static `pwd`/../../../examples/cx_reg.c -o reg -L `pwd`/../../../build/lib/ -lci

popd
find . -print0 | cpio --null -ov --format=newc | gzip -9 > initramfs.cpio.gz