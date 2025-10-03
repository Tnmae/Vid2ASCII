#include "../include/headers/sdl.hpp"
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>

SDL_AppResult sdl::init(std::string name, std::string version,
                        std::string identifier, Uint32 flag, std::string title,
                        Uint32 height, Uint32 width) {
  SDL_SetAppMetadata(name.c_str(), version.c_str(), identifier.c_str());

  if (!SDL_Init(flag)) {
    std::cerr << "error occured while initialising SDL" << SDL_GetError()
              << std::endl;
    return SDL_APP_FAILURE;
  }

  SDL_ClearError();

  sdl::initWindow(title, height, width);

  sdl::initRenderer();

  return SDL_APP_CONTINUE;
}

SDL_AppResult sdl::initWindow(std::string title, int height, int width) {

  this->width = width;
  this->height = height;

  gWindow =
      SDL_CreateWindow(title.c_str(), width, height, SDL_WINDOW_RESIZABLE);
  if (!gWindow) {
    std::cerr << "error creating window" << SDL_GetError() << std::endl;
    return SDL_APP_FAILURE;
  } else {
    std::cout << "window created successfully\n";
  }

  SDL_ClearError();

  return SDL_APP_CONTINUE;
}

SDL_AppResult sdl::initRenderer() {
  gRenderer = SDL_CreateRenderer(gWindow, NULL);
  if (!gRenderer) {
    std::cerr << "error creating renderer" << SDL_GetError() << std::endl;
    return SDL_APP_FAILURE;
  } else {
    std::cout << "renderer initialied successfully\n";
  }

  SDL_ClearError();

  return SDL_APP_CONTINUE;
}

void sdl::quit() {
  SDL_DestroyWindow(gWindow);
  SDL_DestroyRenderer(gRenderer);
  SDL_DestroyWindowSurface(gWindow);
}

Uint32 sdl::getWindowHeight() { return this->height; }

Uint32 sdl::getWindowWidth() { return this->width; }
