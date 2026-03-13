@echo off
setlocal

set "var1=%cd%"
set "PATH=%PATH%;%var1%\mingw64\bin\"

set "var2=%cd%"
set "PATH=%PATH%;%var2%\cmake\bin\"

set "var2=%cd%"
set "PATH=%PATH%;C:\Program Files (x86)\GnuWin32\bin"

cd build
cmake --build .
echo cd into build directory to run executable