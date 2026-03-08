#ifndef VIDEO_HPP
#define VIDEO_HPP

#include <iostream>
#include <string>
#include <vector>
#include "DynTexture.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}

#define PIXEL_SIZE 10

class VideoReaderState {

public:
  VideoReaderState(std::string videoPath, int scaler_width, int scaler_height);
  ~VideoReaderState();
  bool video_reader_open(std::string videoPath, int scaler_width, int scaler_height);
  bool decode_per_frame(uint8_t* frameBuffer);
  void video_display_frame(uint8_t* frameBuffer, std::vector<SDL_Texture* > glyph_textures, StreamingTexture* strmText);

private:
  AVFormatContext *avformat_ctx;
  AVCodecContext* avcodec_ctx;
  int video_stream_idx;
  int frame_width, frame_height;
  int scaler_width, scaler_height;
  AVFrame* av_frame;
  AVPacket* av_packet;
  SwsContext* sws_scaler_ctx;
  void pixelate_frame(uint8_t* frameBuffer);
};

#endif /*VIDEO_HPP*/
