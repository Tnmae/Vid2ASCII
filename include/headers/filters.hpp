#ifndef FILTERS_HPP
#define FILTERS_HPP

#include <cstdint>

namespace filters {

  void pixelate_frame(uint8_t* frameBuffer, int width, int height, int glyph_size);
  
};

#endif /* FILTERS_HPP */
