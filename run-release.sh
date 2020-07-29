if [ ! -d "build" ]; then
  mkdir build
fi
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release .. || exit 1
ninja || exit 1
LD_LIBRARY_PATH=/usr/local/lib ./main