#ifdef __linux

#include "sdl.hpp"
#include "VR_State.hpp"
#include "DynTexture.hpp"
#include <random>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>

#define WIDTH 900
#define HEIGHT 600

int main(int argc, char *argv[]) {

  std::string title = "Video to ASCII Converter";
  bool webcam = false;
  std::string ttfPath = "../resources/Roboto-Regular.ttf";
  std::string videoPath;
  
  std::string program = "./Video2ASCII";

  if (argc < 2 ) {
    std::cout << "Usage: " << program << " [options]\n\n";

    std::cout << "Options:\n";
    std::cout << "  -p, --path <file>     Path to video file\n";
    std::cout << "  -w, --webcam          Use webcam as input\n";
    std::cout << "  -f, --font <file>     Path to TTF font\n";
    std::cout << "  -h, --help            Show this help message\n\n";

    std::cout << "Examples:\n";
    std::cout << "  " << program << " --path sample.mp4 --font Roboto-Regular.ttf\n";
    std::cout << "  " << program << " --webcam\n";
    return EXIT_SUCCESS;
  }

  for (int i = 1 ; i < argc ; i++ ) {
    std::string arg = argv[i];
    if (arg == "--path" || arg == "-p" || arg == "--p")
      videoPath = argv[i+1];
    else if (arg == "--webcam" || arg == "--w" || arg == "-w" )
      webcam = true;
    else if (arg == "--font" || arg == "--f" || arg == "-f")
      ttfPath = argv[i+1];
    else if (arg == "--help" || arg == "--h" || arg == "-h") {
      std::cout << "Usage: " << program << " [options]\n\n";

      std::cout << "Options:\n";
      std::cout << "  -p, --path <file>     Path to video file\n";
      std::cout << "  -w, --webcam          Use webcam as input\n";
      std::cout << "  -f, --font <file>     Path to TTF font\n";
      std::cout << "  -h, --help            Show this help message\n\n";

      std::cout << "Examples:\n";
      std::cout << "  " << program << " --path sample.mp4 --font Roboto-Regular.ttf\n";
      std::cout << "  " << program << " --webcam\n";
      return EXIT_SUCCESS;
    }
  }

  if (webcam) {
    videoPath = "/dev/video0";
  }


  SDL_App app_instance(title, WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);

  FontTexture fontCache(ttfPath);
  fontCache.build_glyph_cache(app_instance.getRenderer());
  
  uint8_t* frameBuffer = new uint8_t[app_instance.getWindowWidth() * app_instance.getWindowHeight() * 4];

  VideoReaderState vr_state(videoPath, app_instance.getWindowWidth(), app_instance.getWindowHeight());

  StreamingTexture strmText(app_instance.getRenderer(), app_instance.getWindowWidth(), app_instance.getWindowHeight());

  app_instance.setFPS(vr_state.getVideoFPS());

  app_instance.appStartTimer();

  while (app_instance.getStatus()) {

    app_instance.inputHandler();

    auto frame_start = std::chrono::high_resolution_clock::now();

    if (!vr_state.decode_per_frame(frameBuffer)) {
      exit(1);
    }

    vr_state.video_display_frame(app_instance.getRenderer(), frameBuffer, fontCache.getGlyphCache(), &strmText);

    app_instance.update(frame_start, strmText.getDynTexture());

  }

  delete[] frameBuffer;
  return EXIT_SUCCESS;
}


#elif __WIN32__

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>

// FFmpeg headers
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

int main(int argc, char *argv[]) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow("SDL3 + SDL3_image + FFmpeg Test", 800,
                                        600, SDL_WINDOW_RESIZABLE);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr );

  // ---------------- Test SDL3_image ----------------
  SDL_Surface *surface = IMG_Load("../resources/sample.png"); // adjust path
  if (!surface) {
    std::cerr << "IMG_Load failed: " << SDL_GetError() << "\n";
  } else {
    std::cout << "SDL3_image loaded PNG successfully!\n";
    SDL_DestroySurface(surface);
  }

  // ---------------- Test FFmpeg ----------------
  const char *videoFile = "../resources/sample.mp4"; // adjust path
  AVFormatContext *fmtCtx = nullptr;
  if (avformat_open_input(&fmtCtx, videoFile, nullptr, nullptr) < 0) {
    std::cerr << "Could not open video file: " << videoFile << "\n";
  } else {
    std::cout << "FFmpeg opened video successfully!\n";
    avformat_close_input(&fmtCtx);
  }

  // ---------------- Show a blank SDL window ----------------
  bool running = true;
  SDL_Event event;

  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT)
        running = false;
    }

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    // In a full app, you would decode video frames into a texture here

    SDL_RenderPresent(renderer);
    SDL_Delay(16);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}

#else

#include <format>
#include <iostream>
#include <print>

int main() {
  std::cout << "this will run on others" << std::endl;
  return EXIT_SUCCESS;
}

#endif
