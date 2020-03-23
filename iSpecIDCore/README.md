# An Efficient, Scalable, Extensible and Accurate Framework to Analyse Large-Scale Sequences of DNA Barcodes

## Dependencies
- [Boost](https://www.boost.org/doc/libs/1_72_0/more/getting_started/unix-variants.html)
- [libcurl](https://curl.haxx.se/docs/install.html)
- GCC 7.5.0
## Guide
```bash
$ git clone git@github.com:lmpn/ecsi.git
$ cd ecsi
$ git submodule update --init --recursive
$ git clone https://github.com/google/googletest.git third_party/benchmark/googletest
$ mkdir build
$ cmake -DCMAKE_BUILD_TYPE:STRING=Release -H. -Bbuild -G "Unix Makefiles"
$ cmake --build build --config Release --target all -- -j 6
```
If your main compiler isn't GCC 7.5.0 use the following command to configure:
```bash
$ cmake -DCMAKE_BUILD_TYPE:STRING=Release
	-DCMAKE_C_COMPILER:FILEPATH=/usr/local/bin/gcc-7
	-DCMAKE_CXX_COMPILER:FILEPATH=/usr/local/bin/g++-7
	-H. -Bbuild -G "Unix Makefiles"
$ cmake --build build --config Release --target all -- -j 6
```
