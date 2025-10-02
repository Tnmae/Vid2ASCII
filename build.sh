#!/bin/bash

rm -rf CMakeLists{WIN32}.txt

mv CMakeLists{Linux}.txt CMakeLists.txt

mkdir -p build

cd build

cmake -S .. -B .

make

./hello
