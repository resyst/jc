WIP

Supports building on Linux x86_64
tested with GCC 8, GCC 10, Clang 10, Clang 11 (trunk)
ICC 19 wip

future plans include testing on arm 64 (aarch64) and windows using msvc/mingw/llvm
decoupling blis/openssl/sleef into git submodules that are pulled/built if not found locally

usage
make a build folder in makec
cd into that build folder
run `cmake ..` and then `make`
defaults to `j64x=j64`, can be modified using `-Dj64x=<type>` where type = j64, j64avx, j64avx2
activate openmp using -DUSE_OpenMP=1
activate sleef using -DUSE_SLEEF=1
set compiled using CC environment variable or -DCMAKE_C_COMPILER
same for CFLAGS

`make install` copies jconsole, libj, libtsdll into jlibrary/bin
test as usual with `./jconsole ../../test/tsu.ijs`
also plan to add support for `make test` to use this and using an actual install in user folder

example
cd makec
mkdir build
cd build
CC=clang CFLAGS="-march=native" cmake -G Ninja -Dj64x=j64avx2 -DCMAKE_EXE_LINKER_FLAGS=-fuse-ld=lld -DUSE_OpenMP=1 ..
ninja
ninja install
cd ../../jlibrary/bin
strip -s jconsole libj.so
./jconsole ../../test/tsu.ijs
RUN ddall

if all tests pass, copy into PATH
