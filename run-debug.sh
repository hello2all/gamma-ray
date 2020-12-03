#!/bin/sh
if [ ! -d "debug" ]; then
  mkdir debug
fi
cd debug
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug .. || exit 1
ninja || exit 1
LD_LIBRARY_PATH=/usr/local/lib ./main