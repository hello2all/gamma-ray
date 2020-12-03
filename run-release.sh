#!/bin/sh
if [ ! -d "release" ]; then
  mkdir release
fi
cd release
cmake -GNinja -DCMAKE_BUILD_TYPE=Release .. || exit 1
ninja || exit 1
LD_LIBRARY_PATH=/usr/local/lib ./main