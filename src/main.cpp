#ifdef __linux

#include "sdl.hpp"
#include "video.hpp"
#include <random>

#define WIDTH 900
#define HEIGHT 600

static SDL_Renderer *renderer = nullptr;

constexpr int col_size = (int)HEIGHT/10;
constexpr int row_size = (int)WIDTH/10;

char frame[ (row_size + 1) * col_size + 1];

constexpr char lowercase_alphabets[] = "@%#*+=-:. ";

constexpr char uppercase_alphabets[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";


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

  if (!font) {
    std::cerr << "Error loading ttf file : " << SDL_GetError() << '\n';
    return 5;
  }else {
    std::cout << "Font loaded successfully\n";
  }

  SDL_Color fg = {255, 255, 255, 255};

  while (running) {

    SDL_PollEvent(&event);

    if (event.type == SDL_EVENT_QUIT) {
      running = false;
    }


    for (int i = 0 ; i < col_size ; i++) {
      for (int j = 0 ; j < row_size ; j++ ) {
        frame[j + i * (row_size + 1)] = lowercase_alphabets[rand() % 9];
      }
      frame[i * (row_size + 1) + row_size] = '\n';
    }
    frame[col_size * (row_size + 1)] = '\0';

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    CreateTextTexture(font, mFontTextTexture, frame, fg);

    RenderText(mFontTextTexture, 0, 0, WIDTH, HEIGHT);

    SDL_RenderPresent(renderer);

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
