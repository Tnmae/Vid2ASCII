#!/bin/bash

mv CMakeLists{Linux}.txt CMakeLists.txt

mkdir -p build

cd build

cmake -S .. -B .

make

./hello
