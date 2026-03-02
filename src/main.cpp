#ifdef __linux

#include "sdl.hpp"
#include "video.hpp"
#include <random>
#include <chrono>
#include <thread>
#include <vector>

#define WIDTH 900
#define HEIGHT 600

static SDL_Renderer *renderer = nullptr;

constexpr int columns = (int)WIDTH/10;
constexpr int rows = (int)HEIGHT/10;

static char frame[ rows * (columns + 1) + 1 ];

constexpr char lowercase_alphabets[] = "@%#*+=-:. ";

SDL_AppResult CreateTextTexture(TTF_Font* font, SDL_Texture*& mFontTextTexture, std::string text, SDL_Color fg) {

  SDL_Surface* textSurface = TTF_RenderText_Blended_Wrapped(font, text.c_str(), 0, fg, 0);

  mFontTextTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

  SDL_DestroySurface(textSurface);

  return SDL_APP_CONTINUE;
}

SDL_AppResult RenderText(SDL_Texture *mFontTextTexture, float x, float y, float w, float h) {
  SDL_FRect textRect= {x, y, w, h};
  SDL_RenderTexture(renderer, mFontTextTexture, nullptr, &textRect);
  return SDL_APP_CONTINUE;
}


void generate_row(int y) {
  for ( int x = 0 ; x < columns ; x++ ) {
    frame[ y * ( columns + 1) + x ] = lowercase_alphabets[rand() % 10];
  }
  frame[y * ( columns + 1 ) + columns ] = '\n';
}

void generate_frame() {
   
  for ( int y = 0 ; y < rows ; y++ ) {
    generate_row(y);
  }

  frame[ (columns + 1) * rows ] = '\0';
}



int main(int argc, char *argv[]) {

  srand(time(0));

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "SDL_Init failed to initialize with error: " << SDL_GetError() << "\n";
    return 1;
  }else {
    std::cout << "SDL3 Initialized successfully\n";
  }

  if (!TTF_Init()) {
    std::cerr << "SDL_Ttf failed to initialize with error: " << SDL_GetError() << "\n";
    return 2;
  }else {
    std::cout << "SDL_Ttf Initliazed successfully\n";
  }

  SDL_Window *window = SDL_CreateWindow("Video to ASCII converted", WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);

  if (!window) {
    std::cerr << "Error creating window : " << SDL_GetError() << '\n';
    return 3;
  }else {
    std::cout << "Window created successfully\n";
  }

  renderer = SDL_CreateRenderer(window, NULL);

  if (!renderer) {
    std::cerr << "Error creating renderer : " << SDL_GetError() << '\n';
    return 4;
  }else {
    std::cout << "Renderer created successfully\n";
  }

  bool running = true;
  SDL_Event event;

  SDL_Texture* mFontTextTexture = nullptr;

  std::string ttfPath = "../resources/Roboto-Regular.ttf";
  
  TTF_Font* font = TTF_OpenFont(ttfPath.c_str(), 60);

  generate_frame();

  if (!font) {
    std::cerr << "Error loading ttf file : " << SDL_GetError() << '\n';
    return 5;
  }else {
    std::cout << "Font loaded successfully\n";
  }

  SDL_Color fg = {255, 255, 255, 255};


  while (running) {

    Uint64 start = SDL_GetTicks();

    SDL_PollEvent(&event);

    if (event.type == SDL_EVENT_QUIT) {
      running = false;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    CreateTextTexture(font, mFontTextTexture, frame, fg);
    RenderText(mFontTextTexture, 0, 0, WIDTH, HEIGHT);

    SDL_RenderPresent(renderer);

    Uint64 end = SDL_GetTicks();

    Uint64 time_elapsed = (end - start == 0) ? 1 : end-start;
    Uint64 fps = 1000/time_elapsed;
    std::cout << fps << '\n';

  }

  SDL_DestroyTexture(mFontTextTexture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

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
  SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);

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
