#!/bin/bash

cd "$(git rev-parse --show-toplevel)"

mkdir build
cd build
cmake .. --fresh
cd ..
cmake --build build

if [[ "$OSTYPE" == "msys"* || "$OSTYPE" == "cygwin"* ]]; then
  echo "Press 'Enter' to close this window."
  read
fi
