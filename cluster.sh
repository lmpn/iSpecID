
export CMAKE_PREFIX_PATH="/home/a77763t/dissertation/playground/third_party/curl"
export CMAKE_PREFIX_PATH="/home/a77763t/dissertation/playground/boost:$CMAKE_PREFIX_PATH"
export BOOSTROOT=/home/a77763t/dissertation/playground/boost
export BOOST_ROOT=/home/a77763t/dissertation/playground/boost
export BOOST_LIB=/home/a77763t/dissertation/playground/boost/lib
export BOOST_IGNORE_SYSTEM_PATHS=1
export BOOST_LIBRARYDIR=/home/a77763t/dissertation/playground/boost/lib
module load cmake/3.10.0
export LD_LIBRARY_PATH="/usr/lib64:$LD_LIBRARY_PATH"
export LD_LIBRARY_PATH="/home/a77763t/dissertation/playground/boost/lib:$LD_LIBRARY_PATH"
module load gcc/5.3.0
module load gcc/7.2.0
cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_CXX_COMPILER=/share/apps/gcc/7.2.0/bin/g++ ..
cmake --build . --config RELEASE --target all -- -j 6


b2 toolset=gcc cxxflags="-std=c++11"  threading=multi link=shared runtime-link=shared variant=release --prefix=/home/a77763t/dissertation/playground/boost/ --build-dir=/home/a77763t/dissertation/playground/boost/boost_build/ install 

amplxe-cl --collect=hotspots --result-dir=/home/a77763t/results -target-tmp-dir=/home/a77763t/tmp -- /home/a77763t/dissertation/playground/iSpecID/build/isid /home/a77763t/dissertation/playground/datasets/culicidae.tsv