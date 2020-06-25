if [ ! -d "build" ]; then
  mkdir build
fi
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug .. || exit 1
ninja || exit 1
LD_LIBRARY_PATH=/usr/local/lib ./unit_tests