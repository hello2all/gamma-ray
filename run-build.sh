rm -rf build
mkdir build && cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug .. 
ninja
LD_LIBRARY_PATH=/usr/local/lib ./main