#!/bin/bash
mkdir -p build
cd build
cmake -S ../ -B .
make && make Shaders && ./GEngine
cd ..