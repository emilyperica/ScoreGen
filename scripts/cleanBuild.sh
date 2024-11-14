#!/bin/bash

cd "$(git rev-parse --show-toplevel)"

rm -r build
mkdir build
cd build
cmake ..
cmake --build .

echo "Press 'Enter' to close this window."
read
