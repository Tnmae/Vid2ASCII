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
#include <libavdevice/avdevice.h>
}


class VideoReaderState {

public:
  VideoReaderState(std::string videoPath, int scaler_width, int scaler_height);
  ~VideoReaderState();
  bool video_reader_open(std::string videoPath);
  bool decode_per_frame(uint8_t* frameBuffer);
  double getVideoFPS();
  void video_display_frame(SDL_Renderer* renderer, uint8_t* frameBuffer, std::vector<SDL_Surface* > glyph_surfaces, StreamingTexture* strmText);

private:
  AVFormatContext *avformat_ctx;
  AVCodecContext* avcodec_ctx;
  int video_stream_idx;
  int frame_width, frame_height;
  int scaler_width, scaler_height;
  AVFrame* av_frame;
  AVPacket* av_packet;
  SwsContext* sws_scaler_ctx;
  bool webcam = false;
};

#endif /*VIDEO_HPP*/
