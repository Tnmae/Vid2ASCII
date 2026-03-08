#include "../include/headers/VR_State.hpp"

VideoReaderState::VideoReaderState(std::string videoPath, int scaler_width, int scaler_height) {
  VideoReaderState::scaler_width = scaler_width;
  VideoReaderState::scaler_height = scaler_height;
  if (!VideoReaderState::video_reader_open(videoPath, scaler_width, scaler_height)) {
    exit(1);
  }
}

VideoReaderState::~VideoReaderState() {
  if (sws_scaler_ctx)
    sws_free_context(&sws_scaler_ctx);
  
  if (avformat_ctx) {
    avformat_close_input(&avformat_ctx);
    avformat_free_context(avformat_ctx);
  }

  if (av_frame)
    av_frame_free(&av_frame);
  
  if (av_packet)
    av_packet_free(&av_packet);
  
  if (avcodec_ctx)
    avcodec_free_context(&avcodec_ctx);
}

bool VideoReaderState::video_reader_open(std::string videoPath, int scaler_width, int scaler_height) {
  avformat_ctx = avformat_alloc_context();

  if (!avformat_ctx) {
    std::cerr << "error allocating context for AVFormat\n";
    return false;
  }else {
    std::cout << "format context created successfully\n";
  }

  if (avformat_open_input(&avformat_ctx, videoPath.c_str(), NULL, NULL) != 0) {
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
    
  for (int i = 0 ; i < avformat_ctx->nb_streams ; i++ ) {
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
  for (int y = 0 ; y < scaler_height - PIXEL_SIZE; y = y + PIXEL_SIZE) {
    for (int x = 0 ; x < scaler_width - PIXEL_SIZE; x = x + PIXEL_SIZE ) {
      double avg = 0;
      for (int i = 0 ; i < PIXEL_SIZE ; i++ ) {
        for (int j = 0 ; j < PIXEL_SIZE ; j++ ) {
          uint8_t red = frameBuffer[( (y + i) * scaler_width + (x + j) ) * 4 ];
          uint8_t green = frameBuffer[( (y + i) * scaler_width + (x + j) )  * 4 + 1 ];
          uint8_t blue = frameBuffer[( (y + i) * scaler_width + (x + j) ) * 4 + 2 ];
          avg += red + blue + green;
        }
      }
      avg = avg / (PIXEL_SIZE * PIXEL_SIZE * 3);
      for (int i = 0 ; i < PIXEL_SIZE ; i++ ) {
        for (int j = 0 ; j < PIXEL_SIZE ; j++ ) {
          frameBuffer[( (y + i) * scaler_width  + (x + j) ) * 4 ] = (uint8_t)avg;
          frameBuffer[( (y + i) * scaler_width  + (x + j) ) * 4 + 1 ] = (uint8_t)avg;
          frameBuffer[( (y + i) * scaler_width  + (x + j) ) * 4  + 2 ] = (uint8_t)avg;
        }
      }
    }
  }

}

void VideoReaderState::video_display_frame(uint8_t* frameBuffer, std::vector<SDL_Texture* > glyph_textures, StreamingTexture* strmText) {
  pixelate_frame(frameBuffer);
  ///for (int y = 0 ; y < HEIGHT ; y = y + GLYPH_SIZE ) {
  ///  for (int x = 0 ; x < WIDTH ; x = x + GLYPH_SIZE ) {
  ///    RenderText(glyph_texture[((float)frameBuffer[ y * WIDTH * 4 + x * 4] / 255.0f) * 9], (float)x , (float)y, GLYPH_SIZE, GLYPH_SIZE);
  ///  }
  ///}
  strmText->lockTexture();
  strmText->copyPixels(frameBuffer, scaler_height);
  strmText->unlockTexture();

}
