#!/bin/bash

#add script for checking if make, cmake, ffmpeg, sdl3 and sdl3_image exist, and download if they don't
if ! command -v ffmpeg >/dev/null 2>&1; then
  echo "ffmpeg not found, downloading dependancy"
  sudo pacman -S ffmpeg
else
  echo "ffmpeg found"
fi

if ! pacman -Ql sdl3 >/dev/null 2>&1; then
  echo "sdl3 not found, downloading dependancy"
  sudo pacman -S sdl3
else
  echo "sdl3 found"
fi

if ! pacman -Ql sdl3_image >/dev/null 2>&1; then
  echo "sdl3_image not found, handling dependancy"
  sudo pacman -S sdl3_image
else
  echo "sdl3_image found"
fi

rm -rf CMakeLists{WIN32}.txt config.bat build.bat

mv CMakeLists{Linux}.txt CMakeLists.txt

mkdir -p build

cd build

cmake -S .. -B .

make

./Vid2ASCII
