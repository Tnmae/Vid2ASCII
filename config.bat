@echo off
setlocal

del CMakeLists{Linux}.txt
del build.sh

#add script to add a check if the make existing in user's system is linux make or gnu win32 make

echo Test to see if mingw toolkit, gnu make and cmake exist

where g++ >nul 2>&1
if %ERRORLEVEL% == 1 (
    echo g++ not found in path. Handling download
    curl -L -o testing.7z https://github.com/niXman/mingw-builds-binaries/releases/download/15.2.0-rt_v13-rev0/x86_64-15.2.0-release-win32-seh-msvcrt-rt_v13-rev0.7z
    tar -xvf testing.7z -C .\
    del .\testing.7z
    set "var1=%cd%"
    set "PATH=%PATH%;%var1%\mingw64\bin\"
    echo mingw64 install complete
    echo g++ downloaded and added to path for current terminal session
)else (
    echo check passed successfully!
)

where cmake >nul 2>&1
if %ERRORLEVEL% == 1 (
    echo cmake not found in path. Handling download
    curl -L -o testing.zip https://github.com/Kitware/CMake/releases/download/v4.1.2/cmake-4.1.2-windows-x86_64.zip
    tar -xvf testing.zip -C .\
    del .\testing.zip
    ren ".\cmake-4.1.2-windows-x86_64" "cmake"
    set "var2=%cd%"
    set "PATH=%PATH%;%var2%\cmake\bin\"
    echo CMake install complete and added to path for current terminal session    
)else (
    echo Check passed successfully!
)

where make >nul 2>&1
if %ERRORLEVEL% == 1 (
    echo make not found in path. Handling download
    curl -L -o make-3.80.exe https://gnuwin32.sourceforge.net/downlinks/make.php
    .\make-3.80.exe
    del make-3.80.exe
    echo make install complete
)else (
    echo check passed successfully!
)

set "Folder=.\include\ffmpeg-8.0"

echo For the first build, ffmpeg has to be setup, won't be downloaded again for subsequent builds
if not exist "%Folder%" (
    echo ffmpeg does not exist, downloading...
    curl -L -o testing.7z https://github.com/GyanD/codexffmpeg/releases/download/8.0/ffmpeg-8.0-full_build-shared.7z
    tar -xvf testing.7z -C .\include\
    ren ".\include\ffmpeg-8.0-full_build-shared" "ffmpeg-8.0"
    del testing.7z
    echo FFmpeg downloaded
)

ren "CMakeLists{WIN32}.txt" "CMakeLists.txt"

cmake -G "MinGW Makefiles" -S . -B build
