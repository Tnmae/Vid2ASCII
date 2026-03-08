#include "../include/headers/DynTexture.hpp"

StreamingTexture::StreamingTexture(SDL_Renderer* renderer, int width, int height) {
  createBlank(renderer, width, height);
}

StreamingTexture::~StreamingTexture() {
  if (texture) {
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
  if (SDL_LockTexture(StreamingTexture::texture, NULL, &(StreamingTexture::texturePixels), &(StreamingTexture::texturePitch)) != 0) {
    //std::cerr << "Unable to lock texture:" << SDL_GetError() << '\n';
    SDL_ClearError();
  }
}

void StreamingTexture::copyPixels(uint8_t *frameBuffer, int height) {
  if (!frameBuffer || !texturePixels)
        return;

  memcpy(texturePixels, frameBuffer, texturePitch * height);
}

void StreamingTexture::unlockTexture() {
  SDL_UnlockTexture(StreamingTexture::texture);
}

SDL_Texture* StreamingTexture::getDynTexture() {
  return StreamingTexture::texture;
}
