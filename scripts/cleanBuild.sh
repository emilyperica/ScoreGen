#!/bin/bash

cd "$(git rev-parse --show-toplevel)"

mkdir build
cd build
cmake .. --fresh
cmake --build .

if [[ "$OSTYPE" == "msys"* || "$OSTYPE" == "cygwin"* ]]; then
  echo "Press 'Enter' to close this window."
  read
fi
