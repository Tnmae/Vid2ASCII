#include "../include/headers/FontTexture.hpp"

FontTexture::FontTexture(std::string ttfPath) {
  if (!TTF_Init()) {
    std::cerr << "SDL_Ttf failed to initialize with error: " << SDL_GetError() << "\n";
    SDL_ClearError();
    exit(1);
  }else {
    std::cout << "SDL_Ttf Initliazed successfully\n";
  }

  if (!loadFont(ttfPath)) {
    exit(1);
  }
}

FontTexture::~FontTexture() {
  for (int i = 0 ; i < 10 ; i++ ) {
    SDL_DestroyTexture(FontTexture::glyph_textures[i]);
  }
  if (FontTexture::mFont)
    TTF_CloseFont(FontTexture::mFont);
  TTF_Quit();
}

bool FontTexture::loadFont(std::string ttfPath) {
  FontTexture::mFont = TTF_OpenFont(ttfPath.c_str(), 60);

  if (!FontTexture::mFont) {
    std::cerr << "Error loading ttf file : " << SDL_GetError() << '\n';
    return false;
  }else {
    std::cout << "Font loaded successfully\n";
  }
  return true;
}

std::vector<SDL_Texture*> FontTexture::getGlyphCache() {
  return FontTexture::glyph_textures;
}

void FontTexture::generate_glyph_texture(SDL_Renderer* gRenderer, SDL_Texture *& glyph_textures, char* glyph) {
  SDL_Surface *textSurface = TTF_RenderText_Blended_Wrapped(FontTexture::mFont, glyph, 0, SDL_Color{255, 255, 255, 255}, 0);
  glyph_textures = SDL_CreateTextureFromSurface(gRenderer, textSurface);
  if (!glyph_textures) {
    std::cerr << "unable to create glyph texture: " << SDL_GetError() << '\n';
    SDL_ClearError();
  }
  SDL_DestroySurface(textSurface);
}

void FontTexture::build_glyph_cache(SDL_Renderer* gRenderer) {
  for (int i = 0 ; i < 10 ; i++ ) {
    char glyph[2] = { luminosity[i] , '\0'};
    generate_glyph_texture(gRenderer, FontTexture::glyph_textures[i], glyph);
  }
}

Uint32 FontTexture::getGlyphSize() {
  return FontTexture::glyph_size;
}
