# Video2ASCII

A real-time video to ASCII art converter built with C++, FFmpeg, and SDL3. It decodes video frames (or a live webcam feed), maps pixel luminosity to ASCII characters, and renders the result in an SDL3 window using a TTF font.

![demo placeholder](resources/demo.gif)

## How it works

Each video frame is decoded via FFmpeg and scaled to the window resolution. A post-processing thread pixelates the frame into `10x10` blocks and maps each block's average luminosity to one of 10 ASCII characters (` .:-=+*#%@`). The resulting character grid is rendered by blitting pre-cached TTF glyph surfaces into a streaming SDL texture, which is then presented to the screen.

The pipeline runs three concurrent threads:
- frame decoder — reads and decodes packets from FFmpeg
- post-processor — pixelates the decoded frame
- main thread — renders and presents the texture

## Dependencies

| Library | Version |
|---|---|
| SDL3 | latest |
| SDL3_ttf | latest |
| FFmpeg | 8.0 (Windows), distro package (Linux) |
| CMake | ≥ 3.20 |
| C++ | 20 |

---

## Building

### Linux

Run the build script. It auto-detects your package manager and installs all dependencies before building.

```bash
chmod +x build.sh
./build.sh
```

Supported package managers: `pacman`, `apt`, `dnf`, `yum`, `emerge`.

> If you get a compile error in `VR_State.cpp` with a newer system FFmpeg, change `sws_free_context(&ctx)` to `sws_freeContext(ctx)`.

---

### Windows

Requires `tar.exe` (included in Windows 10 1803+).

**Step 1 — configure and download dependencies:**
```bat
config.bat
```

This script will:
- Download and set up MinGW-w64 (g++) if not found in PATH
- Download and set up CMake if not found in PATH
- Download and install GnuWin32 make if not found in PATH
- Download FFmpeg 8.0 shared build into `include/ffmpeg-8.0/`
- Run CMake configuration

The first run takes a while due to dependency downloads. Subsequent builds are fast.

**Step 2 — build:**
```bat
build.bat
```

The executable and all required DLLs are placed in the `build/` directory.

---

## Usage

```
Usage: ./Video2ASCII [options]

Options:
  -p, --path <file>           Path to video file
  -w, --webcam                Use default webcam
  -d, --device <name>         Specify a webcam device (mainly for Windows)
  -f, --font <file>           Path to TTF font file
  --framerate <fps>           Override playback framerate
  -h, --help                  Show this help message

Examples:
  ./Video2ASCII --path sample.mp4
  ./Video2ASCII --path sample.mp4 --font Roboto-Regular.ttf
  ./Video2ASCII --webcam
  ./Video2ASCII --device "Integrated Camera"
```

A default font is expected at `../resources/Roboto-Regular.ttf` relative to the executable. Pass `--font` to use a different one.

### Webcam on Windows

Windows uses DirectShow (`dshow`) for webcam capture. To find your camera's device name, run:

```bat
ffmpeg -list_devices true -f dshow -i dummy
```

Then pass the name with `--device`:

```bat
Video2ASCII.exe --device "video=Integrated Camera"
```

### Webcam on Linux

Uses `v4l2`. The default device is `/dev/video0`, selected automatically with `--webcam`.

---

## Project structure

```
.
├── src/
│   ├── main.cpp          # Entry point, threading, render loop
│   ├── VR_State.cpp      # FFmpeg video decoding and webcam setup
│   ├── sdl.cpp           # SDL3 window, renderer, input, FPS limiter
│   ├── DynTexture.cpp    # Streaming texture + ASCII glyph blitting
│   └── FontTexture.cpp   # TTF font loading and glyph surface cache
├── include/
│   └── headers/
│       ├── VR_State.hpp
│       ├── sdl.hpp
│       ├── DynTexture.hpp
│       ├── FontTexture.hpp
│       └── filters.hpp   # Pixelation filter (namespace filters)
├── resources/
│   └── Roboto-Regular.ttf
├── CMakeLists.txt
├── config.bat            # Windows dependency setup
└── build.bat             # Windows build script
```
