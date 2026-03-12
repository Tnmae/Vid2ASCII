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
    SDL_DestroySurface(FontTexture::glyph_cache[i]);
  }
  if (FontTexture::mFont)
    TTF_CloseFont(FontTexture::mFont);
  TTF_Quit();
}

bool FontTexture::loadFont(std::string ttfPath) {
  FontTexture::mFont = TTF_OpenFont(ttfPath.c_str(), PIXEL_SIZE);

  if (!FontTexture::mFont) {
    std::cerr << "Error loading ttf file : " << SDL_GetError() << '\n';
    return false;
  }else {
    std::cout << "Font loaded successfully\n";
  }
  return true;
}

std::vector<SDL_Surface*> FontTexture::getGlyphCache() {
  return FontTexture::glyph_cache;
}

bool FontTexture::generate_glyph_surface(SDL_Renderer* gRenderer, SDL_Surface *& glyph_surface, char glyph) {
  glyph_surface = TTF_RenderGlyph_Blended(FontTexture::mFont, glyph, SDL_Color{255, 255, 255, 255});
  if (!glyph_surface) {
    std::cerr << "error creating surface" << SDL_GetError() << '\n';
    SDL_ClearError();
    return false;
  }
  return true;
}

void FontTexture::build_glyph_cache(SDL_Renderer* gRenderer) {
  for (int i = 0 ; i < 10 ; i++ ) {
    char glyph = luminosity[i];
    if (!generate_glyph_surface(gRenderer, FontTexture::glyph_cache[i], glyph)) {
      return;
    }
  }
}
