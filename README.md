# cx_runtime2.0
Runnier, Timely-ier, Newer, Table-less, Not a Ninja Sloth

Whenever using this repo, make sure to run `source settings.sh` to configure your environment.

1. run `./utils/build-riscv-gnu.sh`

2. run `make all`.

3. run `./utils/build-qemu.sh`

4. run `./utils/build-linux.sh`

5. run `./utils/build-busybox.sh`

6. run `./utils/build-files.sh`

Finally, to run QEMU, run `make qemu`. 