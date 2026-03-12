#include "../include/headers/VR_State.hpp"

VideoReaderState::VideoReaderState(std::string videoPath, int scaler_width, int scaler_height) {
  VideoReaderState::scaler_width = scaler_width;
  VideoReaderState::scaler_height = scaler_height;
  if (!VideoReaderState::video_reader_open(videoPath)) {
    exit(1);
  }
  if (videoPath == "/dev/video0")
    VideoReaderState::webcam = true;
}

VideoReaderState::~VideoReaderState() {
  if (av_frame)
    av_frame_free(&av_frame);
  
  if (av_packet)
    av_packet_free(&av_packet);

  if (sws_scaler_ctx)
    sws_free_context(&sws_scaler_ctx);
  
  if (avcodec_ctx)
    avcodec_free_context(&avcodec_ctx);

  if (avformat_ctx) {
    avformat_close_input(&avformat_ctx);
    avformat_free_context(avformat_ctx);
  }
}

bool VideoReaderState::video_reader_open(std::string videoPath) {
  avdevice_register_all();

  avformat_ctx = avformat_alloc_context();

  if (!avformat_ctx) {
    std::cerr << "error allocating context for AVFormat\n";
    return false;
  }else {
    std::cout << "format context created successfully\n";
  }

  const AVInputFormat* av_input_fmt = nullptr;
  AVDictionary* options = nullptr;
  if (webcam) {
    do {
      av_input_fmt = av_input_video_device_next(av_input_fmt);
    }while (av_input_fmt != nullptr && strcmp(av_input_fmt->name, "video4linux2,v4l2"));

    if (!av_input_fmt) {
      std::cout << "Couldn't find AVInputFormat to get webcam\n";
      return false;
    }

    av_dict_set(&options, "framerate", "30", 0);
    av_dict_set(&options, "video_size", "640x480", 0);
    av_dict_set(&options, "pix_fmts", "0rgb", 0);
  }

  if (avformat_open_input(&avformat_ctx, videoPath.c_str(), av_input_fmt, &options) != 0) {
    std::cerr << "error opening video file\n";
    return false;
  }else {
    std::cout << "video frame loaded successfully\n";
  }

  if (avformat_find_stream_info(avformat_ctx, NULL) < 0) {
    std::cerr << "could not find stream info\n";
    return false;
  }else {
    std::cout << "stream info found\n";
  }

  video_stream_idx = -1;

  AVCodecParameters* av_codec_params;
  const AVCodec* avcodec;
  int stream_num = avformat_ctx->nb_streams;
  for (int i = 0 ; i < stream_num ; i++ ) {
    AVStream* av_stream = avformat_ctx->streams[i];

    av_codec_params = av_stream->codecpar;
    avcodec = avcodec_find_decoder(av_codec_params->codec_id);

    if (av_codec_params->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_stream_idx = i;
      frame_width = av_codec_params->width;
      frame_height = av_codec_params->height;
      break;
    }
  }

  if (video_stream_idx == -1 ) {
    std::cerr << "could not find a video stream\n";
    return false;
  }else {
    std::cout << "video stream index is at: " << video_stream_idx << '\n';
  }

  avcodec_ctx = avcodec_alloc_context3(avcodec);

  if (!avcodec_ctx) {
    std::cerr << "could not allocate context for codec\n";
    return false;
  }else {
    std::cout << "context for codec allocated\n";
  }

  if (avcodec_parameters_to_context(avcodec_ctx, av_codec_params) < 0) {
    std::cerr << "could not initialize AVCodec context\n";
    return false;
  }else {
    std::cout << "Initialized AVCodec context\n";
  }

  if (avcodec_open2(avcodec_ctx, avcodec, NULL)) {
    std::cerr << "Could not open context\n";
    return false;
  }else {
    std::cout << "Opened context successfully\n";
  }

  av_frame = av_frame_alloc();
  if (!av_frame) {
    std::cerr << "could not allocate frame\n";
    return false;
  }else {
    std::cout << "allocated frame\n";
  }

  av_packet = av_packet_alloc();
  if (!av_packet) {
    std::cerr << "could not allocate packet\n";
    return false;
  }else {
    std::cout << "allocated packet\n";
  }
  
  sws_scaler_ctx = sws_getContext(frame_width, frame_height, avcodec_ctx->pix_fmt, scaler_width, scaler_height, AV_PIX_FMT_RGB0, SWS_FAST_BILINEAR, NULL, NULL, NULL);
  if (!sws_scaler_ctx) {
    std::cerr << "could not allocate scaler context\n";
    return false;
  }

  return true;

}

bool VideoReaderState::decode_per_frame(uint8_t* frameBuffer) {
  int response;
  while (av_read_frame(avformat_ctx, av_packet) >= 0) {
    if (av_packet->stream_index != video_stream_idx) {
      av_packet_unref(av_packet);
      continue;
    }else {
      response = avcodec_send_packet(avcodec_ctx, av_packet);
      if (response < 0) {
        std::cerr << "failed to decode packet: " << av_err2str(response) << '\n';
        return false;
      }

      response = avcodec_receive_frame(avcodec_ctx, av_frame);
      if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
        av_packet_unref(av_packet);
        continue;
      }else if (response < 0 ) {
        std::cerr << "failed to decode packet : " << av_err2str(response) << '\n';
        return false;
      }

      av_packet_unref(av_packet);
      break;
    }
  }

  uint8_t* dest[4] = {frameBuffer, NULL, NULL, NULL};
  int dest_linesize[4] = {scaler_width * 4, 0, 0, 0};

  sws_scale(sws_scaler_ctx, av_frame->data, av_frame->linesize, 0, av_frame->height, dest, dest_linesize);

  return true;

}

void VideoReaderState::pixelate_frame(uint8_t* frameBuffer) {
  for (int y = 0 ; y < scaler_height; y = y + PIXEL_SIZE) {
    for (int x = 0 ; x < scaler_width; x = x + PIXEL_SIZE ) {
      double avg_red = 0;
      double avg_green = 0;
      double avg_blue = 0;
      for (int i = 0 ; i < PIXEL_SIZE ; i++ ) {
        for (int j = 0 ; j < PIXEL_SIZE ; j++ ) {
          uint8_t red = frameBuffer[( (y + i) * scaler_width + (x + j) ) * 4 ];
          uint8_t green = frameBuffer[( (y + i) * scaler_width + (x + j) )  * 4 + 1 ];
          uint8_t blue = frameBuffer[( (y + i) * scaler_width + (x + j) ) * 4 + 2 ];
          avg_red += red;
          avg_green += green;
          avg_blue += blue;
        }
      }
      avg_red = (avg_red + avg_green + avg_blue) / (PIXEL_SIZE * PIXEL_SIZE * 3);
      //avg_green = avg_green / (PIXEL_SIZE * PIXEL_SIZE);
      //avg_blue = avg_blue / (PIXEL_SIZE * PIXEL_SIZE);
      for (int i = 0 ; i < PIXEL_SIZE ; i++ ) {
        for (int j = 0 ; j < PIXEL_SIZE ; j++ ) {
          frameBuffer[( (y + i) * scaler_width  + (x + j) ) * 4 ] = (uint8_t)avg_red;
          frameBuffer[( (y + i) * scaler_width  + (x + j) ) * 4 + 1 ] = (uint8_t)avg_red;
          frameBuffer[( (y + i) * scaler_width  + (x + j) ) * 4  + 2 ] = (uint8_t)avg_red;
        }
      }
    }
  }

}

double VideoReaderState::getVideoFPS() {
  return av_q2d(avformat_ctx->streams[video_stream_idx]->r_frame_rate);
}

void VideoReaderState::video_display_frame(SDL_Renderer* renderer, uint8_t* frameBuffer, std::vector<SDL_Surface* > fontSurface, StreamingTexture* strmText) {
  pixelate_frame(frameBuffer);
  strmText->lockTexture();
  strmText->copyPixels(frameBuffer, scaler_height, scaler_width, fontSurface);
  strmText->unlockTexture();
}
