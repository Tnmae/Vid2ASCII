#ifndef FONT_TEXTURE_HPP
#define FONT_TEXTURE_HPP

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>
#include <string>
#include <chrono>

#define PIXEL_SIZE 10

static char luminosity[] = " .:-=+*#%@";



class FontTexture {
  public:

    FontTexture(std::string ttfPath);
    ~FontTexture();
    bool loadFont(std::string ttfPath);
    void build_glyph_cache(SDL_Renderer* gRenderer);
    std::vector<SDL_Surface*> getGlyphCache();

  private:

    TTF_Font* mFont = nullptr;
    std::vector<SDL_Surface*> glyph_cache{ std::vector<SDL_Surface*>(10, nullptr)};
    bool generate_glyph_surface(SDL_Renderer* gRenderer, SDL_Surface*& glyph_texture, char glyph);
};

#endif /* FONT_TEXTURE_HPP */
