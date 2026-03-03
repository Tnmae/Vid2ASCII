#ifdef __linux

#include "sdl.hpp"
#include "video.hpp"
#include <random>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>

#define WIDTH 900
#define HEIGHT 600

#define GLYPH_SIZE 15

static SDL_Renderer *renderer = nullptr;

constexpr int columns = (int)WIDTH/GLYPH_SIZE;
constexpr int rows = (int)HEIGHT/GLYPH_SIZE;

constexpr int num_iter = rows * columns;

static uint8_t frame[ num_iter ];
static uint8_t back_buffer[ num_iter ];

constexpr char luminosity[] = "@%#*+=-:. ";

std::atomic_bool new_frame_ready = false;
std::atomic_bool running = true;

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


void generate_frame(std::mt19937 &generator, std::uniform_int_distribution<std::mt19937::result_type> &rng) {
  /*
   This function will take raw pixel data and convert a luminosity map from it, and then from that generate a back buffer containing ASCII character corresponding to it's respective luminosity
   */
  for ( int y = 0 ; y < num_iter ; y++ ) {
    back_buffer[y] = rng(generator);
  }
}

void generate_glyph_texture(SDL_Texture *& glyph_texture, TTF_Font* font, char* glyph) {
  SDL_Surface *textSurface = TTF_RenderText_Blended_Wrapped(font, glyph, 0, SDL_Color{255, 255, 255, 255}, 0);
  glyph_texture = SDL_CreateTextureFromSurface(renderer, textSurface);
  SDL_DestroySurface(textSurface);
}

void build_glyph_cache(std::vector<SDL_Texture* > &textures, TTF_Font *font) {
  for (int i = 0 ; i < 10 ; i++ ) {
    char glyph[2] = { luminosity[i] , '\0'};
    generate_glyph_texture(textures[i], font, glyph);
  }
}

void RenderFrame(std::vector<SDL_Texture* > &textures, TTF_Font *font) {
  for (int y = 0 ; y < rows ; y++ ) {
    int row_num = y * columns;
    for (int x = 0 ; x < columns ; x++) {
      RenderText(textures[frame[row_num + x]], (float)x*GLYPH_SIZE, (float)y*GLYPH_SIZE, GLYPH_SIZE, GLYPH_SIZE);
    }
  }
}

void worker_function() {
  std::random_device rdev;
  std::mt19937 generator(rdev());
  std::uniform_int_distribution<std::mt19937::result_type> range(0, 9);

  while (running) {
    generate_frame(generator, range);

    new_frame_ready.store(true, std::memory_order_release);

    while (new_frame_ready.load(std::memory_order_acquire) && running) {
      std::this_thread::yield();
    }
  }
}



int main(int argc, char *argv[]) {

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

  renderer = SDL_CreateRenderer(window, nullptr);

  if (!renderer) {
    std::cerr << "Error creating renderer : " << SDL_GetError() << '\n';
    return 4;
  }else {
    std::cout << "Renderer created successfully\n";
  }

  bool running1 = true;
  SDL_Event event;

  std::string ttfPath = "../resources/Roboto-Regular.ttf";
  
  TTF_Font* font = TTF_OpenFont(ttfPath.c_str(), 60);

  if (!font) {
    std::cerr << "Error loading ttf file : " << SDL_GetError() << '\n';
    return 5;
  }else {
    std::cout << "Font loaded successfully\n";
  }

  SDL_Color fg = {255, 255, 255, 255};

  std::vector<SDL_Texture* > glyph_texture(10);
  build_glyph_cache(glyph_texture, font);

  auto start = std::chrono::high_resolution_clock::now();
  int frame_count = 0;

  std::thread worker_thread(worker_function);

  while (running1) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running1 = false;
      }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    if (new_frame_ready.load(std::memory_order_acquire)) {
      memcpy(frame, back_buffer, sizeof(frame));
      new_frame_ready.store(false);
    }

    RenderFrame(glyph_texture, font);

    auto end = std::chrono::high_resolution_clock::now();
    frame_count++;
    double time_elapsed = std::chrono::duration<double>(end-start).count();
    if (time_elapsed > 1.0) {
      std::cout << "fps: " << frame_count/time_elapsed << '\n';
      frame_count = 0;
      start = end;
    }

    SDL_RenderPresent(renderer);

  }

  running.store(false, std::memory_order_release);
  worker_thread.join();

  for (int i = 0 ; i < 10 ; i++ ) {
    SDL_DestroyTexture(glyph_texture[i]);
  }
  TTF_CloseFont(font);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  TTF_Quit();
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
  SDL_Renderer *renderer = SDL_CreateRenderer(window, SDL_RENDERER_ACCELERATED );

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
