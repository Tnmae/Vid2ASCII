@echo off
setlocal

set "var1=%cd%"
set "PATH=%PATH%;%var1%\mingw64\bin\"

set "PATH=%PATH%;%var1%\cmake\bin\"

set "PATH=%PATH%;C:\Program Files (x86)\GnuWin32\bin"

cd build
cmake -G "MinGW Makefiles" -S .. -B .
cmake --build .
echo cd into build directory to run executable
