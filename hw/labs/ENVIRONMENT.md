# Lab Environment Notes

This repository is on Ubuntu x86_64. Most labs build natively with `gcc`,
but `01-datalab`, `06-perflab`, and `08-malloclab` require IA-32
compilation with `-m32`.

The local IA-32 toolchain wrapper is:

```sh
/data/users/huanghao/cs224n/.i386-env/gcc-m32-local
```

It uses the sysroot stored under `.i386-env/sysroot`, populated from Ubuntu
packages downloaded into `.i386-env/debs`.

Cache Lab uses the native 64-bit compiler. Its Valgrind and Python 2 tools are
installed in `.lab-tools/` under the repository root. Before running its tests:

```sh
source /data/users/huanghao/cs224n/.lab-tools/cachelab-env.sh
cd /data/users/huanghao/cs224n/hw/labs/05-cachelab/cachelab-handout
make
./test-csim
python driver.py
```

Run the driver with `python driver.py` because its shebang points to a system
Python path that is not installed on this machine.
