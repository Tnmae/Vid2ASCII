#!/usr/bin/env bash

# script to check which package manager this linux distribution uses
if command -v pacman >/dev/null 2>&1; then
  sudo pacman -S gcc cmake sdl3 sdl3_image sdl3_ttf ffmpeg
elif command -v apt >/dev/null 2>&1; then
  sudo apt install g++ cmake libsdl3-dev libsdl3-image-dev libsdl3-ttf-dev ffmpeg libavformat-dev libavcodec-dev libavutil-dev libswscale-dev libavdevice-dev pkg-config
elif command -v dnf >/dev/null 2>&1; then
  echo "Make sure RPM Fusion is enabled in your fedora system"
  sudo dnf install gcc-c++ cmake SDL3-devel SDL3_image-devel SDL3_ttf-devel ffmpeg
elif command -v yum >/dev/null 2>&1; then
  sudo zypper install gcc-c++ cmake SDL3-devel SDL3_image-devel SDL3_ttf-devel ffmpeg
elif command -v emerge >/dev/null 2>&1; then
  sudo emerge -av sys-devel/gcc dev-util/cmake \
    media-libs/libsdl3 media-libs/sdl3-image \
    media-libs/sdl3-ttf media-video/ffmpeg
else
  echo "Could not figure out a package manager. Manually install the packages using a preferred package manager"
fi

#build commands
rm -rf config.bat build.bat

mkdir -p build

cd build

cmake -S .. -B .

make

./Vid2ASCII

