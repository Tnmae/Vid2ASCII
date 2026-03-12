#ifndef DYN_TEXTURE_HPP
#define DYN_TEXTURE_HPP

#include "sdl.hpp"

class StreamingTexture {
  public:

    StreamingTexture(SDL_Renderer* renderer, int width, int height);
    ~StreamingTexture();
    void lockTexture();
    void copyPixels(uint8_t* frameBuffer, int height, int width, std::vector<SDL_Surface* > fontSurface);
    void unlockTexture();
    SDL_Texture* getDynTexture();

  private:

    void* texturePixels = nullptr;
    int texturePitch = 0;
    SDL_Texture* texture = nullptr;

    void createBlank(SDL_Renderer* renderer, int width, int height);
};

#endif /* DYN_TEXTURE_HPP */
