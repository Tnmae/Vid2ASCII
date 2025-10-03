#ifndef SDL_H
#define SDL_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_time.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <string>

static SDL_Window *gWindow = nullptr;
static SDL_Renderer *gRenderer = nullptr;

class sdl {
public:
  SDL_AppResult init(std::string name, std::string version,
                     std::string identifier, Uint32 flag, std::string title,
                     Uint32 height, Uint32 width);
  SDL_AppResult initWindow(std::string title, int height, int width);
  SDL_AppResult initRenderer();
  void quit();
  Uint32 getWindowHeight();
  Uint32 getWindowWidth();
  void appLoop();

private:
  Uint32 width;
  Uint32 height;
};

#endif /*SDL_H*/
