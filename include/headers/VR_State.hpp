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
  VideoReaderState(std::string video, bool webcam, int scaler_width, int scaler_height);
  ~VideoReaderState();
  bool video_reader_open(std::string videoPath);
  bool decode_per_frame(uint8_t* frameBuffer);
  double getVideoFPS();
  void video_display_frame(SDL_Renderer* renderer, uint8_t* frameBuffer, std::vector<SDL_Surface* > glyph_surfaces, StreamingTexture* strmText);
  bool GetScalerCtx();

private:
  AVFormatContext *avformat_ctx;
  AVCodecContext* avcodec_ctx;
  int video_stream_idx = -1;
  int frame_width, frame_height;
  int scaler_width, scaler_height;
  AVFrame* av_frame;
  AVPacket* av_packet;
  SwsContext* sws_scaler_ctx;
  bool webcam = false;
  void FindDevice(const AVInputFormat*& av_input_fmt, AVDictionary*& options);
  bool AllocateFormatCtx();
  bool FormatOpenInput(std::string video, const AVInputFormat* av_input_fmt, AVDictionary* options);
  bool FindStreamInfo();
  bool FindVideoStreamIdx(AVCodecParameters*& av_codec_params, const AVCodec*& avcodec);
  bool AllocateCodecCtx(const AVCodec* avcodec);
  bool ParamsToCtx(AVCodecParameters* av_codec_params, const AVCodec* avcodec);
  bool OpenCodecCtx(const AVCodec* avcodec);
  bool AllocateFrame(const AVCodec* avcodec);
  bool AllocatePacket();
};

#endif /*VIDEO_HPP*/
