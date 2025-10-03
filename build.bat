@echo off
setlocal

set "var1=%cd%"
set "PATH=%PATH%;%var1%\mingw64\bin\"

set "var2=%cd%"
set "PATH=%PATH%;%var2%\cmake\bin\"

cd build
cmake --build .
.\app.exe