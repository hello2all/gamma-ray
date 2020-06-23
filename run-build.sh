if [ ! -d "build" ]; then
  mkdir build
fi
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug .. 
ninja
LD_LIBRARY_PATH=/usr/local/lib ./main