@echo off
setlocal

del CMakeLists{Linux}.txt
del build.sh

set "Folder=.\include\ffmpeg-8.0"

echo Test to see if ffmpeg exists
if exist "%Folder%" (
    echo Tests passed
) else (
    echo ffmpeg does not exist, downloading...
    curl -L -o testing.7z https://github.com/GyanD/codexffmpeg/releases/download/8.0/ffmpeg-8.0-full_build-shared.7z
    tar -xvf testing.7z -C .\include\
    ren ".\include\ffmpeg-8.0-full_build-shared" "ffmpeg-8.0"
    del testing.7z
    echo FFmpeg downloaded
)

ren "CMakeLists{WIN32}.txt" "CMakeLists.txt"

cmake -G "MinGW Makefiles" -S . -B build
