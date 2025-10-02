#ifndef SDL_H
#define SDL_H

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <string>

class sdl {
public:
  void initWindow(std::string title, int height, int width);
  void initRenderer();

private:
};

#endif /*SDL_H*/
