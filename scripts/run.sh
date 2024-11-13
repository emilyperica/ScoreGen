#!/bin/bash

cd "$(git rev-parse --show-toplevel)"
cmake --build build

if [[ "$OSTYPE" == "msys"* || "$OSTYPE" == "cygwin"* ]]; then
  ./build/Debug/ScoreGen.exe
elif [[ "$OSTYPE" == "darwin"* ]]; then
  ./build/Debug/ScoreGen
fi

echo -e "\nPress 'Enter' to close this window."
read
