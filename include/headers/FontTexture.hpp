#ifndef FONT_TEXTURE_HPP
#define FONT_TEXTURE_HPP

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>
#include <string>
#include <chrono>

static char luminosity[] = " .:-=+*#%@";

class FontTexture {
  public:

    FontTexture(std::string ttfPath);
    ~FontTexture();
    bool loadFont(std::string ttfPath);
    void build_glyph_cache(SDL_Renderer* gRenderer);
    Uint32 getGlyphSize();
    std::vector<SDL_Texture*> getGlyphCache();

  private:

    TTF_Font* mFont = nullptr;
    Uint32 glyph_size;
    std::vector<SDL_Texture*> glyph_textures{ std::vector<SDL_Texture*>(10, nullptr)};
    void generate_glyph_texture(SDL_Renderer* gRenderer, SDL_Texture*& glyph_texture, char* glyph);
};

#endif /* FONT_TEXTURE_HPP */
