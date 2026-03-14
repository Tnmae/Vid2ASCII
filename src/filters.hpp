#include "../include/headers/filters.hpp"


void filters::pixelate_frame(uint8_t* frameBuffer, int width, int height, int glyph_size) {
  int total = glyph_size * glyph_size * 3;
  for (int y = 0 ; y < height; y = y + glyph_size) {
    for (int x = 0 ; x < width; x = x + glyph_size ) {
      double avg_red = 0;
      double avg_green = 0;
      double avg_blue = 0;
      for (int i = 0 ; i < glyph_size ; i++ ) {
        int row = (y + i) * width * 4;
        for (int j = 0 ; j < glyph_size ; j++ ) {
          int column = (x + j) * 4;
          uint8_t red = frameBuffer[ row + column ];
          uint8_t green = frameBuffer[ row + column + 1 ];
          uint8_t blue = frameBuffer[ row + column + 2 ];
          avg_red += red;
          avg_green += green;
          avg_blue += blue;
        }
      }
      avg_red = (avg_red + avg_green + avg_blue) / total;
      //avg_green = avg_green / (total/3);
      //avg_blue = avg_blue / (total/3);
      for (int i = 0 ; i < glyph_size ; i++ ) {
        int row = (y + i) * width * 4;
        for (int j = 0 ; j < glyph_size ; j++ ) {
          int column = (x + j) * 4;
          frameBuffer[ row + column ] = (uint8_t)avg_red;
          frameBuffer[ row + column + 1 ] = (uint8_t)avg_red;
          frameBuffer[ row + column + 2 ] = (uint8_t)avg_red;
        }
      }
    }
  }
}
