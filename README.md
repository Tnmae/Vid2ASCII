a video to ASCII converted using ffmpeg and SDL.
for terminal might only use ffmpeg

when building for windows,
write
./config.bat
./build.bat
make sure tar.exe exists on your windows.
in command prompt, path for cmake, make and gcc if not pre existing in the machine, get set during the setup for the current terminal session.
first build on windows might take time due to handling dependancies and setting up ffmpeg, but subsequent builds will be much faster

when building on linux systems,
./build.sh
in terminal, also give executable permission the build script if not given already by the command
chmod +x build.sh
