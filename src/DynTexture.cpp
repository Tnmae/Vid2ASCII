#include "../include/headers/DynTexture.hpp"

StreamingTexture::StreamingTexture(SDL_Renderer* renderer, int width, int height) {
  createBlank(renderer, width, height);
}

StreamingTexture::~StreamingTexture() {
  if (StreamingTexture::texture) {
    SDL_DestroyTexture(StreamingTexture::texture);
  }
}

void StreamingTexture::createBlank(SDL_Renderer* gRenderer, int width, int height) {
  StreamingTexture::texture = SDL_CreateTexture(
    gRenderer,
    SDL_PIXELFORMAT_RGBA32,
    SDL_TEXTUREACCESS_STREAMING,
    width,
    height);
  if (StreamingTexture::texture == NULL) {
    SDL_Log("Unable to create texture: %s", SDL_GetError());
    SDL_ClearError();
  }
}

void StreamingTexture::lockTexture() {
  if (!texture) return;
  if (SDL_LockTexture(StreamingTexture::texture, NULL, &(StreamingTexture::texturePixels), &(StreamingTexture::texturePitch)) != 0) {
    //std::cerr << "Unable to lock texture:" << SDL_GetError() << '\n';
    SDL_ClearError();
  }
}

void StreamingTexture::copyPixels(uint8_t *frameBuffer, int height, int width, std::vector<SDL_Surface*> fontSurface) {
  memset(texturePixels, 0 , texturePitch * height);
  if (!frameBuffer || !texturePixels)
        return;

  for (int i = 0 ; i < height ; i+= PIXEL_SIZE ) {
    for (int j = 0 ; j < width ; j+= PIXEL_SIZE ) {
      int idx = (int)((float)(frameBuffer[i * width * 4  + j * 4] / 255.0f) * 9.0f);
      uint8_t* src = (uint8_t*)fontSurface[idx]->pixels;
      uint8_t* dst = (uint8_t*)texturePixels;
      int fontWidth = fontSurface[idx]->w;
      int fontHeight = fontSurface[idx]->h;

      for (int y = 0 ; y < fontHeight ; y++ ) {
        if (i+y >= height) break;
        if (j+ fontWidth >= width) continue;
        memcpy( dst + (i + y) * texturePitch + j * 4,
              src + fontSurface[idx]->pitch * y,
              fontWidth * 4);
      }
    }
  }
}

void StreamingTexture::unlockTexture() {
  if (texture)
    SDL_UnlockTexture(StreamingTexture::texture);
}

SDL_Texture* StreamingTexture::getDynTexture() {
  return StreamingTexture::texture;
}
