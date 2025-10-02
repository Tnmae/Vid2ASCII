#!/bin/bash

#add script for checking if ffmpeg, sdl3 and sdl3_image exist, and download if they don't

rm -rf CMakeLists{WIN32}.txt config.bat build.bat

mv CMakeLists{Linux}.txt CMakeLists.txt

mkdir -p build

cd build

cmake -S .. -B .

make

./hello
